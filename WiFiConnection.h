#include "WiFi.h"
#include <HTTPClient.h>
String wifiListString;
int WifiPage = 0;
int lastWifiPage = 0;
String PostEstrusURL = "http://estrus.iotum.id/public/Sapi";

void ClearListWiFi()
{
    myNex.writeStr("t4.txt", "");
    delay(10);
    myNex.writeStr("t5.txt", "");
    delay(10);
    myNex.writeStr("t6.txt", "");
    delay(10);
}

bool WiFiStatus(){
    if (WiFi.status() == WL_CONNECTED)
    {
        myNex.writeStr("t2.txt", WiFi.SSID());
        delay(10);
        return true;
    }
    else
    {
        Serial.print("status WiFi");
        Serial.println(WiFi.status());
        myNex.writeStr("t2.txt", "Tidak terkoneksi");
        delay(10);
        return false;
    }
}
void WiFiStatusAfterConnecting()
{
        if(WiFi.status() == WL_CONNECTED){
        Serial.printf("WiFi berhasil terkoneksi\n");
        myNex.writeNum("bt0.val", 0);
        delay(10);
        myNex.writeNum("disconnect.val", 0);
        delay(10);
        // myNex.writeStr("t2.txt", WiFi.SSID());
        // myNex.writeNum("refreshWiFi.val", 0);
    }
    else{
        Serial.printf("WiFi gagal terkoneksi\n");
        myNex.writeNum("bt0.val", 1);
        delay(10);
        myNex.writeNum("disconnect.val", 1);
        delay(10);
    }
}
void WiFiDisconnect(){
    WiFi.disconnect();
    myNex.writeStr("t2.txt", "Tidak terkoneksi");
    delay(10);
}
void WiFiScan()
{
    Serial.printf("startScan \n");
    myNex.writeStr("t7.txt","sedang memindai WiFi");
    delay(10);    
    ClearListWiFi();
    wifiListString = "";
    WifiPage = 0;
    lastWifiPage = 0;
    int n = WiFi.scanNetworks();
    Serial.printf("jumlah wifi : %d\n", n);
    if (n > 0)
    {
        
        int i;
        for (i = 0; i < n; ++i)
        {
            if (WiFi.SSID(i) != "")
            {
                wifiListString += WiFi.SSID(i);
                wifiListString += ",";
            }
        }
        myNex.writeStr("t7.txt","");
        delay(10);
    }
    else{
        myNex.writeStr("t7.txt","WiFi tidak tersedia");
        delay(10);
    }
    myNex.writeNum("wifiPage.val", WifiPage);
    delay(10);
    myNex.writeNum("refreshWiFi.val", 0);
    delay(10);
}

void WiFiList()
{
    if (wifiListString != "")
    {
        Serial.printf("fungsi wifi list dipanggil\n");
        int startPos = 0;
        int commaPos = wifiListString.indexOf(",");
        // String WifiListArray[lengthWifiList];
        int i = 0;
        Serial.println(wifiListString);
        int startListonPage = (WifiPage * 3);
        if(lastWifiPage != WifiPage){
        ClearListWiFi();
        lastWifiPage = WifiPage;
        }
        while (commaPos > 0)
        {
            if(i >= startListonPage && i < startListonPage + 3){
                String WiFiName = wifiListString.substring(startPos, commaPos);
                Serial.println(WiFiName);
                switch(i-(WifiPage * 3)){
                    case 0:
                        myNex.writeStr("t4.txt", WiFiName);
                        delay(10);
                    break;
                    case 1:
                        myNex.writeStr("t5.txt", WiFiName);
                        delay(10);
                    break;
                    case 2:
                        myNex.writeStr("t6.txt", WiFiName);
                        delay(10);
                    break;
                }
            }
            startPos = commaPos + 1;
            commaPos = wifiListString.indexOf(",", startPos);
            i++;
            
        }
    }
}


void WiFiSettingPage(bool refresh)
{
    if(refresh)
    {
        Serial.printf("beda halaman, lakukan scan ulang\n");
        WiFiScan();
        WiFiList();
        WiFi.begin();
        int millisecond = millis();
        while(WiFi.status() != WL_CONNECTED && millis() - millisecond < 10000)
        {
        }
        WiFiStatusAfterConnecting();
    }
    
    Serial.printf("mengirim data wifi page ke nextion\n");
    WifiPage = myNex.readNumber("wifiPage.val");
    Serial.printf("wifi page: %d\n", WifiPage);
    Serial.printf("selesai membaca nilai\n");
    if(WiFiStatus())
    {
        myNex.writeNum("disconnect.val", 0);
        delay(10);
        if(myNex.readNumber("bt0.val")){
            delay(10);
            WiFiDisconnect();
        }
    }
    else
    {
        myNex.writeNum("bt0.val", 1);
        delay(10);
        myNex.writeNum("disconnect.val", 1);
        delay(10);
    }

    (myNex.readNumber("refreshWiFi.val") == 1) ? WiFiScan() : WiFiList();
}

// void WiFiPageButton(int PagePos, int listAmount){
//     if(PagePos == 0){
//         myNex.writeNum("b1.pco", 63488);
//     }
//     else if(PagePos == listAmount/3){
        
//     }
//     else{
        
//     }
// }

void WiFiConnectingPage(bool refresh){
    String SSID = myNex.readStr("t1.txt");
    delay(10);
    // char SSIDcon[SSID.length()];
    // SSID.toCharArray(SSIDcon, SSID.length()+1);
    const char* SSIDcon = SSID.c_str();
    String Pass = myNex.readStr("pass.txt");
    delay(10);
    const char* Passcon = Pass.c_str();
    // char Passcon[Pass.length()];
    // Pass.toCharArray(Passcon, Pass.length()+1);
    Serial.print("SSID: ");
    Serial.println(SSIDcon);
    delay(10);
    Serial.print("Password: ");
    Serial.println(Passcon);
    delay(10);
    Serial.printf("menyambung WiFi\n");
    if(Pass !=""){
        WiFi.begin(SSIDcon, Passcon);
    }
    else{
        WiFi.begin(SSIDcon);
    }
    int connectTime = millis();
    if(WiFi.status() == WL_CONNECTED){
        WiFi.disconnect();
    }
    while (WiFi.status() != WL_CONNECTED && millis() - connectTime < 30000){
        delay(50);
        Serial.printf(".");
        if(myNex.readNumber("abort.val") == 1){
            delay(10);
            WiFi.disconnect();
            myNex.writeStr("page WiFisetting");
            delay(10);
        }
    }
    Serial.printf("pindah halaman ke wifi setting\n");
    myNex.writeStr("page WiFisetting");
    delay(10);
    WiFiStatusAfterConnecting();
    // Serial.printf("selesai menyambung WiFi\n");

}

bool dataSendWiFi(int IDSapi, int EstrusVal){
    HTTPClient http;
    WiFiClient client;

    http.begin(client, PostEstrusURL);
    http.addHeader("Content-Type", "application/json");
    String json = "{\"ID_sapi\":\"sapi-"+String(IDSapi)+"\",\"data\":"+String(EstrusVal)+"}";
    int httpCode = http.POST(json);
    Serial.printf("HTTP code: %d\n", httpCode);
    if(httpCode ==HTTP_CODE_CREATED)
    {
        return true;
    }
    else
    {
        return false;
    }
}

