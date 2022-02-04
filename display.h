// https://github.com/Seithan/EasyNextionLibrary
#include "EasyNextionLibrary.h"

EasyNex myNex(Serial2);

bool displaySetup()
{
  myNex.begin();
  delay(1000);
  myNex.writeStr("");
  delay(10);
  myNex.writeStr("page 0");
  delay(1000);
  myNex.lastCurrentPageId = 3; //aturan untuk mengaktifkan page 1 biar tidak tabrakan dengan current page ID
  myNex.writeStr("t3.txt", "OK");
  return true;
}