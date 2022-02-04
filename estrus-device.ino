//display
#include "display.h"

#include "sensor.h"

#include "WiFiConnection.h"

#define DATA_REFRESH_RATE 100 // The time between each Data refresh of the page
                               // Depending on the needs of the project, the DATA_REFRESH_RATE can be set
                               // to 50ms or 100ms without a problem. In this example, we use 1000ms, 
                               // as DHT sensor is a slow sensor and gives measurements every 2 seconds

unsigned long pageRefreshTimer = millis(); // Timer for DATA_REFRESH_RATE

bool newPageLoaded = false; // true when the page is first loaded ( lastCurrentPageId != currentPageId )
int lastPage = -1; // last current page ID
int currentPage = 0; // current page ID
void setup()
{
    Serial.begin(115200);
    myNex.begin();   
    myNex.writeStr("page 0");
    //check display nextion
    if(!displaySetup()  || !checkSensor()) //|| !checkBluetooth()
    {
    while (true){
        myNex.writeNum("t8.bco", 65535);
        delay(250);
        myNex.writeNum("t8.bco", 10262);
        delay(250);
    }        
    }
    WiFi.begin();
    delay(2000);
    myNex.writeStr("page 1");
    
}
void loop()
{   
	myNex.NextionListen();

    refereshCurrentPage();
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


void displayRefreshBTBATT(){
    myNex.writeNum("j0.val",batteryStatus()); // update battery status
    (WiFi.status()==WL_CONNECTED) ? myNex.writeStr("t8.txt",WiFi.SSID()) : myNex.writeStr("t8.txt","tidak terhubung");
}


void refereshCurrentPage(){
    
// In this function we refresh the page currently loaded every DATA_REFRESH_RATE
  if((millis() - pageRefreshTimer) > DATA_REFRESH_RATE){
    int currentPage = myNex.readNumber("pg.val");
    if(currentPage != lastPage){
        newPageLoaded = true;
        lastPage = currentPage;
    }
    Serial.printf("refreshCurrentPage %d\n", currentPage);
    switch(currentPage){
        case 2:
        scanEstrusPage(newPageLoaded);
        break;
        case 3:
        WiFiSettingPage(newPageLoaded);
        break;
        case 5:
        WiFiConnectingPage(newPageLoaded);
        break;
        case 9:
        statSaveSendPage(newPageLoaded);
        break;

    }
    displayRefreshBTBATT();
    pageRefreshTimer = millis();
    newPageLoaded = false;
  }
}

void scanEstrusPage(bool refresh){
    int scan = myNex.readNumber("va0.val");
    if(scan==1){
        int estrusVal = estrusScan();
        myNex.writeStr("t1.txt", String(estrusVal));
        myNex.writeNum("va0.val", 0);
        myNex.writeNum("b0.bco", 50712);
        myNex.writeStr("b0.txt", "Mulai Scan");
        myNex.writeNum("bt0.val",0);
        
    }
}


void statSaveSendPage(bool refresh)
{
    int SapiID = myNex.readStr("SapiID.txt").toInt();
    int EstrusVal = myNex.readStr("esVal.txt").toInt();
    if(refresh)
    {
        if(myNex.readNumber("via.val") == 0) //sambungan wifi
        {
            if(WiFi.status() == WL_CONNECTED)
            {
                myNex.writeStr("t1.txt", "tersambung ke WiFi");
                delay(200);
                int amountSavedData = AmountSavedSDcard();
                if(amountSavedData > 0)
                {
                    // dataSendWiFi 1 by 1
                    for (int i = 0; i <= amountSavedData - 1; i++)
                    {
                        myNex.writeStr("t1.txt", "("+String(i) + "/" + String(amountSavedData) + ")"+" Mengirim data" );
                        Serial.printf("dataSendWiFi %d\n", i);
                        (dataSendWiFi(SapiSavedSDcard(i), EstrusSavedSDcard(i))) ? myNex.writeStr("t1.txt", "("+String(i) + "/" + String(amountSavedData) + ")"+ "Data Terkirim") : myNex.writeStr("t1.txt","("+String(i) + "/" + String(amountSavedData) + ")"+ "Data Gagal Terkirim");
                        delay(100);    
                    }
                }
                else
                {
                    myNex.writeStr("t1.txt", "proses mengirim data");
                    delay(200);
                    (dataSendWiFi(SapiID, EstrusVal)) ? myNex.writeStr("t1.txt", "Data Terkirim") : myNex.writeStr("t1.txt", "Data Gagal Terkirim");
                    delay(200);
                }
            }
            else
            {
                myNex.writeStr("t1.txt", "gagal tersambung ke WiFi"); 
                delay(200);   
                //simpan data ke sdcard            
            }
        }
        myNex.writeNum("finish.val", 1);
        delay(50);
        myNex.writeNum("bt0.val",0);
        delay(200);
    }
}

int AmountSavedSDcard(){
    return 0;
}

int EstrusSavedSDcard(int array){
    return 0;
}

int SapiSavedSDcard(int array){
    return 0;
}
