#ifndef KZGinput_h
#define KZGinput_h 

#include <Arduino.h>
//#include <ArduinoJson.h>

#define  KZGinput_DEBOUNCE_DELAY  100    // the debounce time; increase if the output flickers

#define KZGinput_clickTicks 200  // number of millisec that have to pass by before a click is detected.
#define KZGinput_pressTicks 300  // number of millisec that have to pass by before a lonn button press is detected.

 #define KZGinput_STAN_RELEASED 0 //puszczony przycisk
 #define KZGinput_STAN_KLIK_D   01 //był pojedynczy klik na wejsciu
 #define KZGinput_STAN_KLIK_DU  010 //click down and up
 #define KZGinput_STAN_KLIK_DUD 0101
 //#define KZGinput_STAN_KLIK_DUDU '44'
 
 #define KZGinput_STAN_PRESSED 2 //dlugie wcisniecie przycisku
 #define KZGinput_STAN_KLIK_U  20 //był pojedynczy klik na wejsciu
 #define KZGinput_STAN_KLIK_UD 201 //click down and up
 #define KZGinput_STAN_KLIK_UDU 212
 //#define KZGinput_STAN_KLIK_UDUD 213

 
//#undef DEBUG_KZGinput   //If you comment this line, the DPRINT & DPRINTLN lines are defined as blank.
#ifdef DEBUG_KZGinput    //Macros are usually in all capital letters.
  #define DPRINT(...)    Serial.print(__VA_ARGS__)     //DPRINT is a macro, debug print
  #define DPRINTLN(...)  Serial.println(__VA_ARGS__)   //DPRINTLN is a macro, debug print with new line
#else
  #define DPRINT(...)     //now defines a blank line
  #define DPRINTLN(...)   //now defines a blank line
#endif

#define INFO_KZGinput
#ifdef INFO_KZGinput    //Macros are usually in all capital letters.
  #define IPRINT(...)    Serial.print(__VA_ARGS__)     //DPRINT is a macro, debug print
  #define IPRINTLN(...)  Serial.println(__VA_ARGS__)   //DPRINTLN is a macro, debug print with new line
#else
  #define IPRINT(...)     //now defines a blank line
  #define IPRINTLN(...)   //now defines a blank line
#endif


class KZGinput
{
    uint8_t _pin;        // hardware pin number. 
    char _name[10];        // human button name
    static char tmpCharArray[130];
    uint8_t _buttonReleased;//==HIGH
    uint8_t _buttonPressed;//==LOW
    uint8_t _buttonState = HIGH;             // the current reading from the input pin
    uint8_t _lastButtonState = HIGH;   // the previous reading from the input pin
    unsigned long _lastDebounceTime = 0;  // the last time the output pin was toggled
    uint8_t _state;
    unsigned long _startTime; // will be set in state 1
    bool _isClicked;    //short click happened
    bool _isDblClicked; //double shord clicke happened
    bool _isPressed;  //long press happened
    bool _isRelesed;  //long relese happened
    bool _isSwitched; //statche changed pressed->relesed or relesed->pressed
    
    uint8_t debounceRead();
  public:
    KZGinput(){};
    void  init(uint8_t pin,char* name, uint8_t initState, bool activeLow);
    bool loop(void); // return true if btn detect a change
    String getStatusString();
    char* getStatusChar(char* txt);
    bool isClicked(){return _isClicked;}
    bool isDblClicked(){return _isDblClicked;}
    bool isPressed(){return _isPressed;}
    bool isReleased(){return _isRelesed;}
    bool isSwitched(){return _isSwitched;}
    uint8_t getState(){return _buttonState;}
};
#endif





