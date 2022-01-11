//bluetooth library connection
#include "bluetoothConnection.h"

//display
#include "display.h"

#include "sensor.h"

void setup()
{
    myNex.begin();   
    //check display nextion
    if(!displaySetup() || !checkBluetooth() || !checkSensor())
    {
    while (true){
        myNex.writeNum("t8.bco", 65535);
        
        myNex.writeNum("t8.bco", 10262);
    }        
    }
    delay(2000);
    myNex.writeStr("page 1");
    SerialBT.register_callback(BTcallback);       
}
bool btstat = false;
void loop()
{   
	myNex.NextionListen();
    String btStatus;
    displayRefreshBTBATT();
    
}

bool checkSensor()
{
    if (initializeSensor())
    {
        myNex.writeStr("t7.txt", "OK");
        return true;
    }
    else
    {
        myNex.writeStr("t7.txt", "ERROR");
        myNex.writeNum("t7.pco", 63488);
        return false;
    }
}

bool checkBluetooth(){
  if(initBluetooth())
    {
        myNex.writeStr("t5.txt", "OK");
        return true;
    }
    else
    {
        myNex.writeStr("t5.txt", "ERROR");
        myNex.writeNum("t5.pco", 63488);
        return false;
    }
}


void BTcallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
  if(event == ESP_SPP_SRV_OPEN_EVT){
    btstat= true;
  }

  if(event == ESP_SPP_CLOSE_EVT || !event){
    btstat= false;
  }
  
}

void displayRefreshBTBATT(){
    myNex.writeNum("j0.val",batteryStatus()); // update battery status
    if (btstat == true){
        myNex.writeStr("t8.txt", "Bluetooth tersambung");
    }
    else if (btstat == false){
        myNex.writeStr("t8.txt", "Bluetooth terputus");
    }
}