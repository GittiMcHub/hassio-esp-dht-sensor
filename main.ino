#include <DHTesp.h>
#include <FS.h> 
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

DHTesp dht;
   
float temperature;
float humidity;

String run_wlan_ssid;
String run_wlan_pass;
String run_hass_api;
String run_hass_token;
String run_sensor_entity_temp;
String run_sensor_entity_hum;


const char* config_wlan_ssid = "SetMeUp!";
const char* config_wlan_pass = "SetMeUp!";

const char* spiff_is_formatted_file = "/spiff.txt";
const char* config_file = "/conf.txt";
const char* error_counter_file = "/errorcnt.txt";

ESP8266WebServer server(80);
boolean configfile_written;

void initFS(){
  //Initialize File System
  if(SPIFFS.begin()) {
    Serial.println("initFS | SPIFFS Initialize....ok");
    if(!SPIFFS.exists(spiff_is_formatted_file)){
    //Format File System
    Serial.println("initFS | File System not formated - Formatting now");
    if(SPIFFS.format()) {
      Serial.println("initFS | File System Formated, writing spiff_is_formatted_file");
      File f = SPIFFS.open(spiff_is_formatted_file, "w");
      if (!f) {
        Serial.println("initFS | spiff_is_formatted_file open failed");
      } else {
          f.println("initFS | Format Complete");
      }
    } else {
      Serial.println("initFS | File System Formatting Error");
    }
  } else {
    Serial.println("initFS | SPIFFS is formatted. Moving along...");
  }
  } else{
   Serial.println("initFS | SPIFFS Initialization FAILD");
  }
}


void spinUpWebServerAndCreateConfig(){
  Serial.println("spinUpWebServerAndCreateConfig | Starting Server and wait for connection");
  if (MDNS.begin("esp8266")) {              // Start the mDNS responder for esp8266.local
    Serial.println("spinUpWebServerAndCreateConfig | mDNS responder started");
  } else {
    Serial.println("spinUpWebServerAndCreateConfig | Error setting up MDNS responder!");
  }
  
  configfile_written = false;
  
  server.on("/", HTTP_GET, configWSHandleIndex);
  server.on("/config", HTTP_POST, configWSHandleConfig);
  server.onNotFound(configWSHandleNotFound);
  server.begin();
  
  Serial.println("spinUpWebServerAndCreateConfig | Webserver started!");
  while(!configfile_written){
    server.handleClient();
    delay(100);
  }
  server.stop();
}


void configWSHandleIndex(){
  server.send(200,"text/html", "<form action=\"/config\" method=\"POST\"><input type=\"text\" name=\"wlan_ssid\" placeholder=\"WLAN SSID\"></br><input type=\"text\" name=\"wlan_password\" placeholder=\"WLAN Password\"></br><input type=\"text\" name=\"hass_api\" placeholder=\"HASS API URL\"></br><input type=\"text\" name=\"hass_token\" placeholder=\"HASS Token\"></br><input type=\"text\" name=\"sensor_entity_temp\" placeholder=\"HASS Entity Temp. Sensor\"></br><input type=\"text\" name=\"sensor_entity_hum\" placeholder=\"HASS Entity Hum. Sensor\"></br><input type=\"submit\" value=\"Save\"></form>");
}


void configWSHandleConfig(){
  if( ! server.hasArg("wlan_ssid")
      || ! server.hasArg("wlan_password")
      || ! server.hasArg("hass_api")
      || ! server.hasArg("hass_token")
      || ! server.hasArg("sensor_entity_temp")
      || ! server.hasArg("sensor_entity_hum")){
        server.send(400, "text/plain", "400: Invalid Request");         // The request is invalid, so send HTTP status 400
        return;
      }
  File configFileToWrite = SPIFFS.open(config_file, "w");
  if(!configFileToWrite){
    Serial.println("configWSHandleIndex | Failed to open file for reading");
    return;
  }
  configFileToWrite.println(server.arg("wlan_ssid"));
  configFileToWrite.println(server.arg("wlan_password"));
  configFileToWrite.println(server.arg("hass_api"));
  configFileToWrite.println(server.arg("hass_token"));
  configFileToWrite.println(server.arg("sensor_entity_temp"));
  configFileToWrite.println(server.arg("sensor_entity_hum"));
  configFileToWrite.close();
  Serial.println("configWSHandleIndex | ConfigFile Complete!"); 
  configfile_written = true;
}


void configWSHandleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}


void initConfig(){
  if(!SPIFFS.exists(config_file)){
    Serial.println("initConfig | Config does not exists, will create");
    WiFi.softAP(config_wlan_ssid, config_wlan_pass);
    Serial.println("initConfig | SoftAP started");
    spinUpWebServerAndCreateConfig();
  }
  Serial.println("initConfig | Config exists");
  delay(100);
}


void readAndSetConfig(){
  File f = SPIFFS.open(config_file, "r");
  String config[7];
  int counter = 0;
  while(f.available()) {
      //Lets read line by line from the file
      String line = f.readStringUntil('\n');
      Serial.println("readAndSetConfig | read:" + line);

      if(counter < 7){
        config[counter] = line;
        counter++;
      } else {
        Serial.println("readAndSetConfig | IGNORING - TO MUCH CONTENT IN CONFIG");
      }
  }
  f.close();
  
  run_wlan_ssid = config[0];
  run_wlan_pass = config[1];
  run_hass_api = config[2];
  run_hass_token = config[3];
  run_sensor_entity_temp = config[4];
  run_sensor_entity_hum = config[5];
}



boolean configSetCompletely(){
  return run_wlan_ssid != "" && run_wlan_pass != "" && run_hass_api != "" && run_hass_token != "" && run_sensor_entity_temp != "" && run_sensor_entity_hum;
}



void eraseConfig(){
  run_wlan_ssid = "";
  run_wlan_pass = "";
  run_hass_api = "";
  run_hass_token = "";
  run_sensor_entity_temp = "";
  run_sensor_entity_hum = "";
  SPIFFS.remove(config_file);
}


void resetErrorCounter(){
  SPIFFS.remove(error_counter_file);
  File f = SPIFFS.open(error_counter_file, "w");
  f.print("0\n");
  f.close();
}


int getErrorCounter(){
  File f = SPIFFS.open(error_counter_file, "r");
  String line = f.readStringUntil('\n');
  f.close();
  return line.toInt();
}


void initWifi(){
  Serial.println("ssid: " + run_wlan_ssid + " - pass: " + run_wlan_pass);
  WiFi.mode(WIFI_STA);
  //WiFi.begin(run_wlan_ssid, run_wlan_pass);
  WiFi.begin(run_wlan_ssid, run_wlan_pass);
  int timeout_counter = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print("." + WiFi.status());
    timeout_counter++;
    if(timeout_counter == 60){
      Serial.println("initWifi | TIMEOUT in WIFI - ERASING CONFIG!");
      eraseConfig();
      return;
    }
  }
  Serial.println("");
  Serial.println("initWifi | WiFi connected");
  Serial.println("initWifi | IP address: ");
  Serial.println("initWifi |" + WiFi.localIP());
}


void setup() {
  Serial.begin(115200);
  Serial.println("- setup -");
  delay(10);
  initFS();
  
  Serial.println("- init -");
  while(!configSetCompletely()){    
    initConfig();
    readAndSetConfig();
    if(!configSetCompletely){
      Serial.println("setup | Config seems broken - ERASING!");
      eraseConfig();
      continue;
    }
    Serial.println("- connecting wifi -");
    initWifi();
  }
  Serial.println("- connecting wifi -");
  dht.setup(2, DHTesp::DHT22);
  dhtReadAndUpdate();
  sendData(run_sensor_entity_temp,temperature);
  sendData(run_sensor_entity_hum,humidity);
}

String createPOSTJSON(String sensorName, float sensorValue){
  return "{\"attributes\": {\"friendly_name\": \"" + sensorName +"\",\"unit_of_measurement\": \"C°\"},\"state\": \"" + sensorValue +"\"}";
}

void sendData(String sensorEntity, float sensorValue){
  HTTPClient http;    //Declare object of class HTTPClient
 
  http.begin(run_hass_api + "/states/" + sensorEntity);      // http://192.168.0.220:8123/api/states/sensor.bad_temperatur
  http.addHeader("Content-Type", "application/json");  //Specify content-type header
  http.addHeader("Authorization", "Bearer " + run_hass_token);
  String dataToSend = createPOSTJSON(sensorEntity,sensorValue);
  int httpCode = http.POST(dataToSend);   //Send the request
  Serial.println("sendData(" + sensorEntity + "," + sensorValue + ") | " + dataToSend  + " HTTP: " + httpCode);   //Print HTTP return code
   
  http.end();  //Close connection
}

/* HOME ASSISTANT configuration.yml

# Sensors
sensor:
  # Weather prediction
  - platform: yr

  - platform: template
    sensors:
        bad_temperatur:
            friendly_name: "Badezimmer Temperatur"
            unit_of_measurement: "C°"
            value_template: "{{ states(sensor.temperature) }}"
        bad_luftfeuchte:
            friendly_name: "Badezimmer Luftfeuchte"
            unit_of_measurement: 'C°'
            value_template: "{{ states(sensor.humidity) }}"

 */

// Senden von Daten
// HASS_API_URL = 192.168.0.220:8123/api/states
// HASS_SENSOR_ENTITY_ID = sensor.bad_temperatur (wie in der Configuration YML)
// HASS_SENSOR_UOM = C°
// HASS_FRIENDLY_NAME = "Badezimmer Temperatur"
/*
 * POST INHALT::
{
  "attributes": {
        "friendly_name": "Badezimmer Temperatur",
        "unit_of_measurement": "C°"
    },
    "state": "17.5"
}
*/

void loop() {
  Serial.println("- loop -");
  
  dhtReadAndUpdate();
  //ESP.deepSleep(5e6); 
  delay(10000);
}

void dhtReadAndUpdate(){
  Serial.println("- dhtReadAndUpdate -");
  
  delay(dht.getMinimumSamplingPeriod());
  temperature = dht.getTemperature(); // Gets the values of the temperature
  humidity = dht.getHumidity(); // Gets the values of the humidity 

  Serial.println("Status\tHumidity (%)\tTemperature (C)\t(F)\tHeatIndex (C)\t(F)");
  Serial.print(dht.getStatusString());
  Serial.print("\t");
  Serial.print(humidity, 1);
  Serial.print("\t\t");
  Serial.print(temperature, 1);
  Serial.print("\t\t");
  Serial.print(dht.toFahrenheit(temperature), 1);
  Serial.print("\t\t");
  Serial.print(dht.computeHeatIndex(temperature, humidity, false), 1);
  Serial.print("\t\t");
  Serial.println(dht.computeHeatIndex(dht.toFahrenheit(temperature), humidity, true), 1);
}