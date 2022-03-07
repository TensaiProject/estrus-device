int pinBatt = 34; //pin untuk baca level baterai
int pinEstrus = 35; //pin untuk baca level estrus

//Moving Avarage Estrus
#define WINDOW_SIZE 5
int INDEX = 0;
int VALUE = 0;
int SUM = 0;
int READINGS[WINDOW_SIZE];

float EXP =0;
float bobot = 0.05;

bool initializeSensor()
{
    return true;
}

int batteryStatus()
{
    int senBatt = analogRead(pinBatt);
    float Voltage = senBatt * (3.3 /4096);
    int battPercentage = 100 * (Voltage - 3.3)/(3.7-3.3);
    
    if (battPercentage < 0)
    {
        battPercentage = 0;
    }
    if (battPercentage > 100)
    {
        battPercentage = 100;
    }
    
    return battPercentage;
}

int estrusScan() //mengembalikan nilai pembacaan estrus
{
  //MA
  int millisNow = millis();
  int AVERAGED = 2000; //sengaja dibuat ditengah ADC, jadi ketika adc ada di atas atau dibawah. jaraknya tidak terlalu jauh
  while(millis() - millisNow < 7000){
    SUM = SUM - READINGS[INDEX];
    VALUE = analogRead(pinEstrus);
    READINGS[INDEX] = VALUE;
    SUM = SUM + VALUE; 
    INDEX = (INDEX+1) % WINDOW_SIZE;
    AVERAGED = SUM / WINDOW_SIZE;

    //Exp
    EXP = bobot * VALUE + (1 - bobot) * EXP;
        
    Serial.print(VALUE);
    Serial.print(",");
    Serial.print(AVERAGED);
    Serial.print(",");
    Serial.println(EXP);
    delay(5);
  }    
  return EXP;
}
