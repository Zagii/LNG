#include <Arduino.h>

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  
#include <KZGinput.h>

#include <WiFi.h>
#include <SSD1306.h>

#define BAND  433E6

char  name[]= "TX"; //Esp8

//LORA
#define LORA_SCK     5    // GPIO5  -- SX1278's SCK
#define LORA_CS_PIN  18
#define LORA_MISO    19   // GPIO19 -- SX1278's MISnO
#define LORA_RST_PIN 14 // 23  //14??
#define LORA_IRQ_PIN 26  //DI00
#define LORA_MOSI    27   // GPIO27 -- SX1278's MOSI
#define LORA_DI01    33  //nie uzywam
#define LORA_DI02    32  //nie uzywam

// oled
#define OLED_SDA_pin 4
#define OLED_SCL_pin 15
#define OLED_RST_PIN 16
#define OLED_LED_PIN 2   //led

//btn
const int btn_ile=6;
KZGinput btn[btn_ile];
// dzialajace inputy {13,17,12,21,23,0,22};
const int btn_pin[]={22,23,17,13,21,12};//{13,17,12,21,23,0,22};

//vBAT
#define VBAT_PIN 35

SSD1306 display(0x3c, OLED_SDA_pin,OLED_SCL_pin);

char outgoing[100];              // outgoing message
byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0xBB;     // address of this device
byte destination = 0xAA;//0xFF;      // destination to send to
long lastSendTime = 0;        // last send time
int interval = 10000;          // interval between sends
 
char tmpTxt[100];

///*************************************oled *******************************************/

bool isMsgFlag=false;
char incoming[100];
char inUpTime[30];
char myUpTime[30];
char rxState[]="00000";
char txState[]="00000";

void calcInputMsg();

void oledInit()
{
  pinMode(OLED_RST_PIN,OUTPUT);
  pinMode(OLED_LED_PIN,OUTPUT);
  
  digitalWrite(OLED_RST_PIN, LOW);    // set GPIO16 low to reset OLED
  delay(50); 
  digitalWrite(OLED_RST_PIN, HIGH); // while OLED is running, must set GPIO16 in high
  
  display.init();
  display.flipScreenVertically();
   display.displayOn();
  display.setContrast(100);  
  display.setFont(ArialMT_Plain_10);
  delay(1500);
 
}

void oledLoop()
{
 
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  
  display.drawString(0, 0, F("Pilot: "));
  display.drawString(30, 0, myUpTime);

 // display.drawString(0, 12, F("RX: "));
  display.drawString(12, 12, inUpTime);
  display.drawString(0, 24, F("Stan: 1  2  3  4  S"));
  display.drawString(28, 36, String(rxState[0]));display.drawString(40, 36, String(rxState[1]));
  display.drawString(52, 36, String(rxState[2]));display.drawString(65, 36, String(rxState[3]));
  display.drawString(77, 36, String(rxState[4]));

  display.drawString(0, 48, F("RSSI: "));
  display.drawString(30, 48, String(LoRa.packetRssi()));
  
  display.drawString(60, 48, F("SNR: "));
  display.drawString(85, 48, String( LoRa.packetSnr()));
  
 
  display.drawString(95, 24, F("Vbat: "));
  int v=analogRead(VBAT_PIN);
 // Serial.print("V=");Serial.println(v);
  //int r=47;
  //int r1=100; z dzielnika wynika (r1+r)/r = 1,47
  // 3.3- 4095
  // x - v
  //uwy==v*3.3/4095
  // 3.3*1,47=4,851
  // dzielac 4,851/4095 mamy mnoznik
  const double mnoznik=0.0011846153846154;
  //float uwy= (float)(v) *3.3/4095;
  //Serial.print("Uwy=");Serial.println(uwy);
  float Vbat= v* mnoznik; //uwy*(r1+r)/r1;
  //Serial.print("Vbat=");Serial.println(Vbat);
  int proc= map(Vbat*100,300,420,0,100);
  if(proc>100)proc=100;
  //float uwe2=uwy*(r1+r)/r;
  //Serial.print("Proc=");Serial.println(proc);
  
 //Uwy=Uwe/(r1+r2)*r1
 //147/
 // float Vbat= (float)(v) / 4095*2*3.3*1.1;
  
 // itoa(Vbat,tmpTxt,10);
  //sprintf(tmpTxt,"%7.5f",Vbat);
  if(millis()%100>50)
    sprintf(tmpTxt,"%4.2fV",Vbat);
  else
    sprintf(tmpTxt," %d%%",proc);
  display.drawString(95, 36, tmpTxt);
  
  display.display();
}

///*************************************oled ******************************************/

void timeToChar(unsigned long seconds,char* str);
void onReceive(int packetSize);
void sendMessage(char* outgoing); 
void postMsg();
void oledHello();
void setup() {
  
  Serial.begin(9600);                   // initialize serial
  WiFi.disconnect( true );
  WiFi.mode( WIFI_OFF );
  oledInit();
  oledHello();
  //WiFi.forceSleepBegin();//WiFi.forceSleepWake();
  delay( 1 );
  while (!Serial);
  Serial.print(name);
  Serial.println(" start.");
  delay(1000);
   pinMode(VBAT_PIN, INPUT);
   int v=analogRead(VBAT_PIN);
   Serial.print("V=");Serial.println(v);
 
 for(int i=0;i<btn_ile;i++)
 {
   char t[10];
   sprintf(t,"btn_%d",i);
   btn[i].init(btn_pin[i],t,KZGinput_STAN_RELEASED,true);
 }
///LORA 
  SPI.begin(LORA_SCK,LORA_MISO,LORA_MOSI,LORA_CS_PIN);
 // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(LORA_CS_PIN, LORA_RST_PIN, LORA_IRQ_PIN);// set CS, reset, IRQ pin

  if (!LoRa.begin(BAND)) {             // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }

  LoRa.onReceive(onReceive);
  LoRa.receive();
  Serial.println("LoRa init succeeded.");
  
}

void oledHello()
{
  digitalWrite(OLED_LED_PIN,HIGH);
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  
  //display.drawString(0, 0, F("Btn: "));
  display.drawString(60, 30, F("Pilot LNG"));
  display.setFont(ArialMT_Plain_10);
  display.drawString(80, 46, F(" by KZG"));
  display.display();
  delay(500);
  
  digitalWrite(OLED_LED_PIN,LOW);

}
void oledBtn(int i,char *c)
{
  digitalWrite(OLED_LED_PIN,HIGH);
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  
  display.drawString(0, 0, F("Btn: "));
  display.drawString(30, 0, String(i));
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 30, c);
  display.display();
  delay(500);
  
  digitalWrite(OLED_LED_PIN,LOW);

}

void getInputs()
{
  strcpy(txState,rxState);
  int czyZmianaStanu=0;
   char tmpC;
   
  for(int i=0;i<btn_ile;i++)
  {
    if(btn[i].loop())
    {
      Serial.print("Btn event: ");
      
      Serial.println(btn[i].getStatusChar(tmpTxt));
      if(btn[i].isClicked())
      { 
        strcpy(tmpTxt,"--KLIK");
        oledBtn(i,tmpTxt);
      }
      if(i<=5) 
      {
        if(btn[i].isDblClicked())
        {
          czyZmianaStanu++;
          strcpy(tmpTxt,"DBL KLIK");
          strcpy(txState,"00000");
          if(i<4)
          {
            txState[i]='1';
          }
          if(i==4){
            strcpy(txState,"11110");
        
          }
          oledBtn(i,tmpTxt);
        }
        if(i==5)
        {
          
         if(btn[i].isSwitched())
         {
            
           Serial.print("Switched tmpC: ");
           if( btn[5].getState()==0) tmpC='1'; else tmpC='0';
           
           txState[4]=tmpC;
           czyZmianaStanu++; 
           strcpy(tmpTxt,"Zawor zmiana: ");
           strcat(tmpTxt,txState); 
           oledBtn(i,tmpTxt);
         }
        }
      }
    }
  }
  
  //
  //if(rxState[4]!=tmpC)czyZmianaStanu++;
  if(czyZmianaStanu>0)
  {
    /*if(tmpC=='1')
      txState[4]='0';
    else 
      txState[4]='1';
    */
//    txState[4]=tmpC;
      if( btn[5].getState()==0) tmpC='1'; else tmpC='0';
      txState[4]=tmpC;
           
   postMsg();
  }
}

char message[100];
void postMsg()
{
    strcpy(message,"s:");
    strcat(message,txState);
    strcat(message," ");
    strcat(message , name);
    strcat(message," ");
    strcat(message,myUpTime);   // send a message

    sendMessage(message);
    Serial.print("Sending ");
    Serial.println( message);
    LoRa.receive();                     // go back into receive mode
    lastSendTime = millis();
}
unsigned long oledMillis=0;
void loop() {
  getInputs();
  if(isMsgFlag)
  {
    calcInputMsg();
    isMsgFlag=false;
  }
  if(millis()-oledMillis>1000)
  {
    timeToChar(millis()/1000,myUpTime);
 
    oledLoop();
    oledMillis=millis();
    delay(5);
  }
  if (millis() - lastSendTime > interval) {
    //strcpy(txState,"0000");
   // postMsg();
                // timestamp the message
    //interval = random(5000) + 1000;     // 2-3 seconds
    }
}

void sendMessage(char* outgoing) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(strlen(outgoing));        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}

void calcInputMsg()
{
  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length

  strcpy(incoming,"");                 // payload of packet
  int i=0;
  char c;
  while (LoRa.available()) {            // can't use readString() in callback, so
    c=(char)LoRa.read();      // add bytes one by one
    if(c=='\0')break;
    incoming[i] = c;
    i++;
    if(i==99)break;
  }
  incoming[i]='\0';

  Serial.print("incomming: ");
    Serial.println(incoming);
  if (incomingLength != i) {   // check length for error
    Serial.println("error: message length does not match length");
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    return;                             // skip rest of function
  }

  // if message is for this device, or broadcast, print details:
  Serial.print("Received from: 0x");Serial.println(sender);
  
  Serial.print("Sent to: 0x");Serial.println(recipient);
  Serial.print("Message ID: ");Serial.println(incomingMsgId);
  Serial.print("Message length: ");Serial.println(incomingLength);
  Serial.print("Message: ");Serial.println(incoming);
  Serial.print("RSSI: " );Serial.println(LoRa.packetRssi());
  Serial.print("Snr: ");Serial.println(LoRa.packetSnr());
  Serial.println();
//display.drawString(0, 20, String("Odbior:"));
//display.drawString(40, 20, String(incoming));
//display.drawString(0, 40, String("RSSI: "));
//display.drawString(40, 40, String(LoRa.packetRssi()));
  if(incoming[0]=='s'&&incoming[1]==':')
  {
    
    if((incoming[2]=='0'||incoming[2]=='1')&&
        (incoming[3]=='0'||incoming[3]=='1')&&
          (incoming[4]=='0'||incoming[4]=='1')&&
          (incoming[5]=='0'||incoming[5]=='1')&&
          (incoming[6]=='0'||incoming[6]=='1'))
    {
      rxState[0]=incoming[2];rxState[1]=incoming[3];
      rxState[2]=incoming[4];rxState[3]=incoming[5];
      rxState[4]=incoming[6];
      if(strcmp(rxState,txState)!=0)oledLoop();
    }
    strncpy(inUpTime,incoming+8,strlen(incoming)-8);
  }
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  isMsgFlag=true;
}

void timeToChar(unsigned long seconds,char* str)
{
  int days = seconds / 86400;
  seconds %= 86400;
  byte hours = seconds / 3600;
  seconds %= 3600;
  byte minutes = seconds / 60;
  seconds %= 60;
 //  size_t size=sizeof(str);
  snprintf(str, sizeof(tmpTxt), "%01d dni %02d:%02d:%02lu", days, hours, minutes, seconds);
  
}