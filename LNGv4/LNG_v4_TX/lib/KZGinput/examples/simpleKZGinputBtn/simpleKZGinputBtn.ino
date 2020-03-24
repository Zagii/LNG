/* *******************************************************
** Przykład użycia biblioteki, 
** ******************************************************* */

#include <KZGinput.h>

KZGinput btn1;

void setup()
{
    Serial.begin(115200);
    btn1.init(A14,"btn1",KZGinput_STAN_RELEASED,true);
    Serial.println("Koniec Setup"); 
}


void loop()
{
  if(btn1.loop())
  {
    Serial.print("Btn event, new status: ");
    Serial.println(btn1.getStatusString());
  }
}