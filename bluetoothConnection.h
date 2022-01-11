//ESP32 bluetooth initialization
#include "esp_bt_main.h"
#include "BluetoothSerial.h"
BluetoothSerial SerialBT;
bool initBluetooth()
{
  if(!btStart()) {
    // Serial.println("Failed to initialize controller");
    return false;
  }
 
  if(esp_bluedroid_init() != ESP_OK) {
    // Serial.println("Failed to initialize bluedroid");
    return false;
  }
 
  if(esp_bluedroid_enable() != ESP_OK) {
    // Serial.println("Failed to enable bluedroid");
    return false;
  }

  if (!SerialBT.begin("Estrus Device")) {
      // Serial.println("ERROR: Unable to initialize Bluetooth");
      return false;
  }
  return true;
}


