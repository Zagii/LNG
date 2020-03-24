#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  

/*
  LoRa Duplex communication wth callback
  Sends a message every half second, and uses callback
  for new incoming messages. Implements a one-byte addressing scheme,
  with 0xFF as the broadcast address.
  Note: while sending, LoRa radio is not listening for incoming messages.
  Note2: when using the callback method, you can't use any of the Stream
  functions that rely on the timeout, such as readString, parseInt(), etc.
  created 28 April 2017
  by Tom Igoe
*/


// LoRa.setPins(D8,D3, D2); // set CS, reset, IRQ pin
#define BAND  433E6
char name[]= "nano"; //Esp8

//LORA
#define LORA_SCK     13    // GPIO5  -- SX1278's SCK
#define LORA_CS_PIN  10
#define LORA_MISO    12   // GPIO19 -- SX1278's MISnO
#define LORA_RST_PIN A0 // 23  //14??
#define LORA_IRQ_PIN A1  //DI00
#define LORA_MOSI    11   // GPIO27 -- SX1278's MOSI
//#define LORA_DI01    33  //nie uzywam
//#define LORA_DI02    32  //nie uzywam

char outgoing[100];              // outgoing message
byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0xAA;     // address of this device
byte destination = 0xBB;      // destination to send to
long lastSendTime = 0;        // last send time
int interval = 10000;          // interval between sends

char tmpTxt[100];


char incoming[100];
char inUpTime[30];
char myUpTime[30];
char inState[]="0000";
char myState[]="0000";


void timeToChar(unsigned long seconds,char* str);
void onReceive(int packetSize);
void sendMessage(char* outgoing); 
void setup() {
  Serial.begin(9600);                   // initialize serial
  while (!Serial);
  Serial.print(name);
  Serial.println(" start.");
  //Serial.println(name+"- cs: "+csPin+", rst: "+resetPin+", irq: "+irqPin);
//  SPI.begin(SCK,MISO,MOSI,csPin);

  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(LORA_CS_PIN, LORA_RST_PIN, LORA_IRQ_PIN);

  if (!LoRa.begin(BAND)) {             // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }

  LoRa.onReceive(onReceive);
  LoRa.receive();
  Serial.println("LoRa init succeeded.");
  
}


char message[100];
unsigned long oledMillis=0;
void loop() {
  if(millis()-oledMillis>1000)
  {
    timeToChar(millis()/1000,myUpTime);
    oledMillis=millis();
    delay(5);
  }
  if (millis() - lastSendTime  > interval) {
    strcpy(message,"s:");
    strcat(message,myState);
    strcat(message," ");
    strcat(message , name);
    strcat(message," ");
    strcat(message,myUpTime);   // send a message

    sendMessage(message);
    Serial.print("Sending ");
    Serial.println( message);
    lastSendTime = millis();            // timestamp the message
    //interval = random(5000) + 1000;     // 2-3 seconds
    LoRa.receive();                     // go back into receive mode
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



void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

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
  //Serial.println("Snr: " + String(LoRa.packetSnr()));
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
          (incoming[5]=='0'||incoming[5]=='1'))
    {
      inState[0]=incoming[2];inState[1]=incoming[3];
      inState[2]=incoming[4];inState[3]=incoming[5];
    }
    strncpy(inUpTime,incoming,strlen(incoming)-4);
  }
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