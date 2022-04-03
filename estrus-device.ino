//display
#include "display.h"
//include ota.h
#include "ota.h"
bool otaDone = false;
#include "sensor.h"

#include "WiFiConnection.h"

#include "SPI.h"
#include "SD.h"
File amountSaved;
File EstrusSaved;
File SapiSaved;

int DataRefreshRate = 50;// The time between each Data refresh of the page
                               // Depending on the needs of the project, the DataRefreshRate can be set
                               // to 50ms or 100ms without a problem. In this example, we use 1000ms, 
                               // as DHT sensor is a slow sensor and gives measurements every 2 seconds
unsigned long pageRefreshTimer = millis(); // Timer for DataRefreshRate

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
        myNex.writeStr("t8.txt", "terdapat masalah, hubungi tim");
        myNex.writeNum("t8.pco",754);
    while (true){
        myNex.writeNum("t8.bco", 65535);
        delay(250);
        myNex.writeNum("t8.bco", 10262);
        delay(250);
    }        
    }
    //cek nilai baterai dari batteryStatus, jika habis maka stop proses
    if(batteryStatus() == 0)
    {
        myNex.writeStr("t8.txt", "battery habis");
        myNex.writeNum("t8.pco",754);
        while (true){
            myNex.writeNum("t8.bco", 65535);
            delay(250);
            myNex.writeNum("t8.bco", 10262);
            delay(250);
        }
    }
    myNex.writeStr("t8.txt", "menghubungkan ke wifi");
    myNex.writeNum("t8.pco",65535);
    delay(20);
    WiFi.begin();
    if(WiFi.status() == WL_CONNECTED)
    {
        myNex.writeStr("t8.txt","Wifi tersambung");
        otaSetup();
        myNex.writeStr("t8.txt","OTA setup selesai");
        otaDone = true;
    }
    delay(2000);
    myNex.writeStr("page 1");
    //if wifi on, run otasetup and make otadone true
    if(WiFi.status() == WL_CONNECTED)
    {
        otaSetup();
        otaDone = true;
    }
    
}
void loop()
{   
    //check wifi is connected, then check otadone, if false run otasetup and server handleclient, if true run server handleclient
    if(WiFi.status() == WL_CONNECTED && otaDone)
    {
        server.handleClient();
    }
    else if(WiFi.status() == WL_CONNECTED && !otaDone)
    {
        otaSetup();
        server.handleClient();
    }
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
    //jika battery habis, maka berkedip 
    if(batteryStatus() == 0)
    {
        myNex.writeNum("j0.bco", 65535);
        delay(250);
        myNex.writeNum("j0.bco", 63488);
        delay(250);
    }
    // else if dibawah 10 dan diatas 0 berkedip 
    else if(batteryStatus() < 10 && batteryStatus() > 0)
    {
        myNex.writeNum("j0.bco", 65535);
        delay(250);
        myNex.writeNum("j0.bco", 10262);
        delay(250);
    }
    else
    {
        myNex.writeNum("j0.bco", 61342);
    }
    myNex.writeNum("j0.val",batteryStatus()); // update battery status
    (WiFi.status()==WL_CONNECTED) ? myNex.writeStr("t8.txt",WiFi.SSID()) : myNex.writeStr("t8.txt","tidak terhubung");
    (WiFi.status()==WL_CONNECTED) ? myNex.writeStr("ipaddress.txt",WiFi.localIP().toString().c_str()) : myNex.writeStr("ipaddress.txt","");
}


void refereshCurrentPage(){
    
// In this function we refresh the page currently loaded every DataRefreshRate
  if((millis() - pageRefreshTimer) > DataRefreshRate){
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
    if(currentPage == 3){
        DataRefreshRate = 120;
    }
    else{
        DataRefreshRate = 50;
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
                int amountSavedData = AmountSavedSDcard(false);
                Serial.printf("amountSavedData %d\n", amountSavedData);
                if(amountSavedData > 0)
                {
                    Serial.printf("ngirim beberapa kali. \n amountSavedData %d\n", amountSavedData);
                    // dataSendWiFi 1 by 1
                    int i = 0;
                    saveDataSDcard(SapiID, EstrusVal);
                    while(AmountSavedSDcard(false) > 0)
                    {
                        i++;
                        myNex.writeStr("t1.txt", "mengirim data ke-" + String(i));
                        if(!dataSendWiFi(SapiSavedSDcard(false),EstrusSavedSDcard(false)))
                        {
                            myNex.writeStr("t1.txt", "gagal mengirim data");
                            delay(200);
                            myNex.writeStr("t1.txt", "data akan disimpan ke SD Card");
                            delay(200);   
                            goto saveSD;
                            break;
                        }
                        else
                        {
                            SapiSavedSDcard(true);
                            EstrusSavedSDcard(true);
                            AmountSavedSDcard(true);
                            myNex.writeStr("t1.txt", " berhasil dikirim ");
                        }
                    }
                }
                else
                {
                    Serial.printf("ngirim data sekali");
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
                myNex.writeStr("t1.txt", "data akan disimpan ke SD Card");
                delay(200);   
                goto saveSD;
                //simpan data ke sdcard            
            }
        }
        else if (myNex.readNumber("via.val") == 1)
        {
            saveSD:
            if(!SD.begin(5))
            {
                myNex.writeStr("t1.txt", "gagal membuka SD");
            }
            else
            {
                myNex.writeStr("t1.txt", "membuka SD");
                delay(200);
                //simpan data ke sdcard
                Serial.printf("SapiID %d\n", SapiID);
                Serial.printf("EstrusVal %d\n", EstrusVal);
                if(saveDataSDcard(SapiID, EstrusVal)){
                    myNex.writeStr("t1.txt", "sukses menyimpan data ke SD");
                }
            }

        }
        myNex.writeNum("finish.val", 1);
        delay(50);
        myNex.writeNum("bt0.val",0);
        delay(200);
    }
}

int AmountSavedSDcard(bool deleteData){
    if(SD.exists("/amountSaved.txt"))
    {
        amountSaved = SD.open("/amountSaved.txt");
        String amountSavedString ="";
        while(amountSaved.available())
        {
            amountSavedString += amountSaved.readString();
        }
        int amountSaveInt = amountSavedString.toInt();
        amountSaved.close();
        if(!deleteData)
        {
            return amountSaveInt;
        }
        else
        {
            amountSaveInt--;
            amountSaved = SD.open("/amountSaved.txt", FILE_WRITE);
            amountSaved.print(amountSaveInt);
            amountSaved.close();
            return amountSaveInt;
        } 
    }

}

int EstrusSavedSDcard(bool deleteData){
    if(!deleteData) //baca doang
    {
        EstrusSaved = SD.open("/estrusSaved.txt");
        String EstrusSavedString ="";
        while(EstrusSaved.available())
        {
            EstrusSavedString += EstrusSaved.readString();
        }
        EstrusSaved.close();
        return EstrusSavedString.substring(0,EstrusSavedString.indexOf(",")).toInt();
    }
    else
    {
        EstrusSaved = SD.open("/estrusSaved.txt");
        String EstrusSavedString ="";
        while(EstrusSaved.available())
        {
            EstrusSavedString += EstrusSaved.readString();
        }
        EstrusSaved.close();
        EstrusSavedString.remove(0,EstrusSavedString.indexOf(",")+1);
        EstrusSaved = SD.open("/estrusSaved.txt", FILE_WRITE);
        EstrusSaved.print(EstrusSavedString);
        EstrusSaved.close();
        return -1;
    }
}

int SapiSavedSDcard(bool deleteData){
    if(!deleteData){
        SapiSaved = SD.open("/sapiSaved.txt");
        String SapiSavedString ="";
        while(SapiSaved.available())
        {
            SapiSavedString += SapiSaved.readString();
        }
        SapiSaved.close();
        return SapiSavedString.substring(0,SapiSavedString.indexOf(",")).toInt();
    }
    else
    {
        SapiSaved = SD.open("/sapiSaved.txt");
        String SapiSavedString ="";
        while(SapiSaved.available())
        {
            SapiSavedString += SapiSaved.readString();
        }
        SapiSaved.close();
        SapiSavedString.remove(0,SapiSavedString.indexOf(",")+1);
        SapiSaved = SD.open("/sapiSaved.txt", FILE_WRITE);
        SapiSaved.print(SapiSavedString);
        SapiSaved.close();
        return -1;
    }
}

bool saveDataSDcard(int SapiID, int EstrusVal){
    int amountSaveInt;
    String SapiSavedString ="";
    String EstrusSavedString ="";

    //jumlah data yang tersimpan    
    if(!SD.exists("/amountSaved.txt"))
    {
        Serial.println("file /amountSaved.txt tidak ada, membuat file baru");
        myNex.writeStr("t1.txt", "menambahkan data amountSaved.txt");
        amountSaveInt = 0;
        amountSaved = SD.open("/amountSaved.txt", FILE_WRITE);
        amountSaved.print(String(amountSaveInt));
        amountSaved.close();
    }
    amountSaved = SD.open("/amountSaved.txt",FILE_READ);
    if(amountSaved)
    {
        myNex.writeStr("t1.txt", "amountSaved.txt terbaca");
        String amountSavedString ="";
            while(amountSaved.available())
            {
                amountSavedString += amountSaved.readString();
            }
            amountSaveInt = amountSavedString.toInt();
            Serial.printf("amountSaveInt %d\n", amountSaveInt);
            amountSaved.close();
    }
    else
    {
        myNex.writeStr("t1.txt", "gagal membaca amountSaved.txt");
        Serial.println("gagal membuka amountSaved.txt");
        return false;
    }


    //ambil data sapi yang tersimpan
    if(!SD.exists("/SapiSaved.txt"))
    {
        Serial.println("file /SapiSaved.txt tidak ada, membuat file baru");
        SapiSaved = SD.open("/SapiSaved.txt", FILE_WRITE);
        SapiSaved.print("");
        SapiSaved.close();
    }
    SapiSaved = SD.open("/SapiSaved.txt",FILE_READ);
    if(SapiSaved)
    {
        myNex.writeStr("t1.txt", "/SapiSaved.txt terbaca");
        while(SapiSaved.available())
        {
            SapiSavedString += SapiSaved.readString();
        }
        Serial.printf("SapiSavedString %s\n", SapiSavedString.c_str());
        SapiSaved.close();
    }
    else
    {
        myNex.writeStr("t1.txt", "gagal membaca SapiSaved.txt");
        Serial.println("gagal membuka SapiSaved.txt");
        return false;
    }
    
    //ambil data estrus yang tersimpan
    if(!SD.exists("/EstrusSaved.txt"))
    {
        Serial.println("file /EstrusSaved.txt tidak ada, membuat file baru");
        EstrusSaved = SD.open("/EstrusSaved.txt", FILE_WRITE);
        EstrusSaved.print("");
        EstrusSaved.close();
    }
    EstrusSaved = SD.open("/EstrusSaved.txt",FILE_READ);
    if (EstrusSaved)
    {
        myNex.writeStr("t1.txt", "/EstrusSaved.txt terbaca");
        while(EstrusSaved.available())
        {
            EstrusSavedString += EstrusSaved.readString();
        }
        Serial.printf("EstrusSavedString %s\n", EstrusSavedString.c_str());
        EstrusSaved.close();
    }
    else
    {
        myNex.writeStr("t1.txt", "gagal membaca EstrusSaved.txt");
        Serial.println("gagal membuka EstrusSaved.txt");
        return false;
    }
    

    SapiSavedString +=  String(SapiID) + ",";
    EstrusSavedString += String(EstrusVal) + ",";
    amountSaveInt++;
    Serial.printf("amountSaveInt add %d\n", amountSaveInt);
    Serial.printf("SapiSavedString add %s\n", SapiSavedString.c_str());
    Serial.printf("EstrusSavedString add %s\n", EstrusSavedString.c_str());
    
    amountSaved = SD.open("/amountSaved.txt", FILE_WRITE);
    if(!amountSaved)
    {
        myNex.writeStr("t1.txt", "gagal membuka amountSaved.txt");
        Serial.println("gagal membuka amountSaved.txt");
        return false;
    }
    amountSaved.print(amountSaveInt);
    
    SapiSaved = SD.open("/SapiSaved.txt", FILE_WRITE);
    if (!SapiSaved)
    {
        myNex.writeStr("t1.txt", "gagal membuka SapiSaved.txt");
        Serial.println("gagal membuka SapiSaved.txt");
        return false;
    }
    SapiSaved.print(SapiSavedString);
    
    EstrusSaved = SD.open("/EstrusSaved.txt", FILE_WRITE);
    if(!EstrusSaved)
    {
        myNex.writeStr("t1.txt", "gagal membuka EstrusSaved.txt");
        Serial.println("gagal membuka EstrusSaved.txt");
        return false;
    }
    EstrusSaved.print(EstrusSavedString);

    amountSaved.close();
    SapiSaved.close();
    EstrusSaved.close();
    return true;
}


