# hassio-esp-dht-sensor
Arduino Sketch for a configurable ESP8266 / ESP8285 DHT Temperature and Humindity Sensor

# Used setup
- Wemos D1 Mini Lite (https://wiki.wemos.cc/products:d1:d1_mini_lite)
- USB Driver in case you use windows or Mac OS
- DHT22 (AOSONG AM2302, SN: 160421442)
	HINT: Tested many DHT22 (ASAIR AM2302 SN: J1181101ACE) AND NONE OF THEM WORKED WITH ANY OF MY Wemos D1 and Mini Lites

- HASS.io 0.93.1

- Arduino IDE 1.8.9
- Library: DHT sensor library for ESPx by beegee_tokyo Version 1.0.9


# Prerequisites
## Arduino 
- Additional Boards Manager URL: https://arduino.esp8266.com/stable/package_esp8266com_index.json
	FYI: https://github.com/esp8266/Arduino
- Install Library "DHT sensor library for ESPx by beegee_tokyo"
- You need to enable SPIFFS for the FileSystem to work via "Tools" > "Flash size" > "1M (xxxK SPIFFS)" (128k is enough right now)
	
## HASS.io
- Enable the API component in your HomeAssistant Configuration  (https://developers.home-assistant.io/docs/en/external_api_rest.html)
```
# HOME ASSISTANT configuration.yml - add this to between a section
api:
```

- Generate a Token (http://xxx.xxx.xxx.xxx:8123/profile) on your profile at the bottom

- Configure the sensor in your HomeAssistant Configuration
```
# HOME ASSISTANT configuration.yml - add this to the Sensors section
sensor:
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

```

For a complete example, my configuration.yml looks like this:

```
homeassistant:
  # Name of the location where Home Assistant is running
  name: Home
  # Location required to calculate the time the sun rises and sets
  latitude: -27.1258125
  longitude: -109.2769375
  # Impacts weather/sunrise data (altitude above sea level in meters)
  elevation: 0
  # metric for Metric, imperial for Imperial
  unit_system: metric
  # Pick yours from here: http://en.wikipedia.org/wiki/List_of_tz_database
  _time_zones
  time_zone: Europe/London
  # Customization file
  customize: !include customize.yaml

# Configure a default setup of Home Assistant (frontend, api, etc)
default_config:

# Uncomment this if you are using SSL/TLS, running in Docker container, etc.
# http:
#   base_url: example.duckdns.org:8123

# Discover some devices automatically
discovery:

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

# Text to speech
tts:
  - platform: google_translate

group: !include groups.yaml
automation: !include automations.yaml
script: !include scripts.yaml

api:

media_player:
- platform: androidtv
  device_class: firetv
  name: FireTV
  host: 192.168.2.52
  adbkey: "/config/adbkey"
  adb_server_ip: 127.0.0.1

```

# Usage
- Flash main.ino
- Wait for ESP to boot
- Connect to Webinterface (typically http://192.168.4.1/
- Fill the inputs

# Any Problems?
## SPIFFS / FS does not work any more
```
//https://github.com/kentaylor/EraseEsp8266Flash
#include <WiFiManager.h>          //https://github.com/kentaylor/WiFiManager 
void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");  
  printf("SDK version:%s\n", system_get_sdk_version());
  size_t cfgAddr = 1024; //(ESP.getFlashChipSize() - 0x4000);
  size_t cfgSize = 1024;//ESP.getFlashChipSize();//(8*1024);
  int last = (ESP.getFlashChipSize()/0x10)/(SPI_FLASH_SEC_SIZE);
  Serial.println(last);
 
  noInterrupts();
  for (int j=0; j<last; j++) {
  Serial.print("j = ");
  Serial.println(j);
     if(spi_flash_erase_sector((j * SPI_FLASH_SEC_SIZE/0x100)) != SPI_FLASH_RESULT_OK) {
        printf("addressFail: %d - 0x%x\n", (j * SPI_FLASH_SEC_SIZE), (j * SPI_FLASH_SEC_SIZE));
      }
      else {
        printf("addressSucceed: %d - 0x%x\n", (j * SPI_FLASH_SEC_SIZE), (j * SPI_FLASH_SEC_SIZE));
      }    
    }
  interrupts(); 
}

void loop() {
}
```
