int pinBatt = 34; //pin untuk baca level baterai
int pinEstrus = 35; //pin untuk baca level estrus

//Moving Avarage Estrus
#define WINDOW_SIZE 5
int INDEX = 0;
int VALUE = 0;
int SUM = 0;
int READINGS[WINDOW_SIZE];
int AVERAGED = 0;

bool initializeSensor()
{
    return true;
}

int batteryStatus()
{
    int senBatt = analogRead(pinBatt);
    float Voltage = senBatt * (3.3 /4096);
    int battPercentage = 100 * (Voltage - 3.2)/(3.7-3.2);
    
    if (batPercentage < 0)
    {
        batPercentage = 0;
    }
    if (batPercentage > 100)
    {
        batPercentage = 100;
    }
    
    return batPercentage;
}

int estrusScan() //mengembalikan nilai pembacaan estrus
{
    
  
  VALUE = analogRead(pinEstrus);
  
  Serial.print(VALUE);
  Serial.print(",");
  delay(100);
  
  READINGS[INDEX] = VALUE;
  SUM = SUM + VALUE; 
  INDEX = (INDEX+1) % WINDOW_SIZE;
  AVERAGED = SUM / WINDOW_SIZE;
  Serial.println(AVERAGED);
  return AVERAGED;
}
