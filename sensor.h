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
int EXPbatt = 0;
bool initializeSensor()
{
    return true;
}

int batteryStatus()
{
    int senBatt = analogRead(pinBatt);
    EXPbatt = 0.5 * senBatt + (1 - 0.5) * EXPbatt; //0.5 is the weight of the new reading
    int battPercentage = map(senBatt, 1800, 2000, 0, 100);
    if(EXPbatt > 100)
    {
        EXPbatt = 100;
    }
    else if(EXPbatt < 0)
    {
        EXPbatt = 0;
    }
    Serial.printf("batteryStatus: %d - battPercentage: %d\n", senBatt, EXPbatt);    
    return EXPbatt;
}

int estrusScan() //mengembalikan nilai pembacaan estrus
{
  //MA
  int millisNow = millis();
  int AVERAGED = 2000; //sengaja dibuat ditengah ADC, jadi ketika adc ada di atas atau dibawah. jaraknya tidak terlalu jauh
  while(millis() - millisNow < 7000){
    SUM = SUM - READINGS[INDEX];
    
    //pilih salah satu
    VALUE = analogRead(pinEstrus);
    // VALUE = (0.8432*analogRead(pinEstrus))- 668.21; //hasil konversi dari alat draminski
    /* HASIL DARI PERBANDINGAN DRAMINSKI DENGAN ADC */
    /*
    nilai adc	      nilai estrus
    866,54545	        40
    802,9090909	      50
    881,3636364	      60
    1049,545455	     190
    1204,111111	     370


    Didapatkan hasil 
    y = 0,8432x - 668,21
    R² = 0,9541
    y adalah nilai estrus draminski
    x adalah nilai adc

    */
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
