#include "KZGinput.h" 


///////////////////////////////////////////////////////////////
//////////////////   init          ////////////////////////////
///////////////////////////////////////////////////////////////

void KZGinput::init(uint8_t pin, char* name, uint8_t initState, bool activeLow)
{
  DPRINT(F("Wejscie init, pin=")); DPRINT(pin); DPRINT(F("; nazwa="));DPRINTLN(name);
  
  _pin=pin;
  strcpy(_name,name);
  pinMode(_pin,INPUT_PULLUP);
  digitalWrite(_pin, HIGH);   // turn on pulldown resistor
  _state = initState; // starting with state 0: waiting for button to be pressed

  if (activeLow) 
  {
    // button connects ground to the pin when pressed.
    _buttonReleased = HIGH; // notPressed
    _buttonPressed = LOW;
    
  } else 
  {
    // button connects VCC to the pin when pressed.
    _buttonReleased = LOW;
    _buttonPressed = HIGH;
  } // if
  
  debounceRead();
  DPRINT((_pin));DPRINT(F(", koniec init stan= ")); DPRINTLN((_buttonState));
}

///////////////////////////////////////////////////////////////
//////////////////   debounceRead /////////////////////////////
///////////////////////////////////////////////////////////////
uint8_t KZGinput::debounceRead()
{
  // read the state of the switch into a local variable:
  uint8_t reading;// =_digitalRead(_pin);
  reading=digitalRead(_pin);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH),  and you've waited
  // long enough since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != _lastButtonState) {
    // reset the debouncing timer
    DPRINT(F("start debounce wejscia "));DPRINT(_pin);DPRINT(F("-"));DPRINTLN(_name);
    _lastDebounceTime = millis();
  }

  if ((millis() - _lastDebounceTime) > KZGinput_DEBOUNCE_DELAY) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != _buttonState) {
       _buttonState = reading;
       DPRINT(F("Zmiana wejscia "));DPRINT(_pin);
       DPRINT(F("***buttonState"));DPRINTLN(_buttonState);
      
      }
    }
 
  _lastButtonState = reading;
  return _buttonState;
}

///////////////////////////////////////////////////////////////
//////////////////   getStatusString //////////////////////////
///////////////////////////////////////////////////////////////
String KZGinput::getStatusString()
{
  //StaticJsonBuffer<100> jsonBuffer;
  //JsonObject& root = jsonBuffer.createObject();
  
  String w="{\"type\":\"KZGinput\"";//,\"pin\":"+String(_pin);
  w+=",\"name\":\""+ String(_name) + "\"";
  if(_buttonState==_buttonPressed)
  {
      //root["buttonState"]="p";
      w+=String(",\"btnSt\":\"p\"");
  }
  else 
  {
    //root["buttonState"]="r";
    w+=String(",\"btnSt\":\"r\"");
  }
  w+=",\"st\":"+ String(_state);
  w+=",\"isC\":"+ String(_isClicked);
  w+=",\"isDblC\":"+ String(_isDblClicked);
  w+=",\"isP\":"+ String(_isPressed);
  w+=",\"isR\":"+ String(_isRelesed);
  w+=",\"isSw\":"+ String(_isSwitched);
  w+="}";

  /*root["isClicked"]=_isClicked;
  root["isDblClicked"]=_isDblClicked;
  root["isPressed"]=_isPressed;
  root["isRelesed"]=_isRelesed;
  root["isSwitched"]=_isSwitched;*/
return w;
}

char* KZGinput::getStatusChar(char* txt)
{
  //StaticJsonBuffer<100> jsonBuffer;
  //JsonObject& root = jsonBuffer.createObject();
  strcpy(txt,String("{\"type\":\"KZGinput\"").c_str());//,\"pin\":"+String(_pin);
  strcat(txt,",\"name\":\"");strcat(txt,_name); strcat(txt, "\"");

  if(_buttonState==_buttonPressed)
  {
      //root["buttonState"]="p";
      strcat(txt,",\"btnSt\":\"p\"");
  }
  else 
  {
    //root["buttonState"]="r";
    strcat(txt,",\"btnSt\":\"r\"");
  }
  char x[8];
  itoa(_state,x,10); strcat(txt,",\"st\":"); strcat(txt,x);
  itoa(_isClicked,x,10); strcat(txt,",\"isC\":");strcat(txt,x);
  itoa(_isDblClicked,x,10); strcat(txt,",\"isDblC\":");strcat(txt,x);
  itoa(_isPressed,x,10); strcat(txt,",\"isP\":");strcat(txt,x);
  itoa(_isRelesed,x,10); strcat(txt,",\"isR\":"); strcat(txt,x);
  itoa(_isSwitched,x,10); strcat(txt,",\"isSw\":");strcat(txt,x);
  strcat(txt,"}");

  /*root["isClicked"]=_isClicked;
  root["isDblClicked"]=_isDblClicked;
  root["isPressed"]=_isPressed;
  root["isRelesed"]=_isRelesed;
  root["isSwitched"]=_isSwitched;*/
return txt;
}

///////////////////////////////////////////////////////////////
//////////////////   loop         /////////////////////////////
///////////////////////////////////////////////////////////////

bool KZGinput::loop(void)
{
  _isClicked=false;
  _isDblClicked=false;
  _isPressed=false;
  _isRelesed=false;
  _isSwitched=false;
  
  // Detect the input information 
  uint8_t buttonLevel = debounceRead(); // current button signal.
  unsigned long now = millis(); // current (relative) time in msecs.

  switch(_state)
  {
    case KZGinput_STAN_RELEASED: // przycisk puszczony stan początkowy
     if (buttonLevel == _buttonPressed)
     {
        _state = KZGinput_STAN_KLIK_D; // step to state 
        _startTime = now; // remember starting time
        DPRINT(F("**** btn pressed r->p: "));DPRINTLN(getStatusChar(tmpCharArray));
     } 
    break;
    case KZGinput_STAN_KLIK_D: // czeka na puszczenie przycisku
      if (buttonLevel == _buttonReleased)
      {
        _state = KZGinput_STAN_KLIK_DU; // step to state 
        _startTime = now;
        DPRINT(F("**** btn Clicked? r->p: "));DPRINTLN(getStatusChar(tmpCharArray));
      } else 
      {
        if ((buttonLevel == _buttonPressed) && (now - _startTime  >  KZGinput_pressTicks)) 
        {
          _isPressed=true;
          _isSwitched=true;
          _state = KZGinput_STAN_PRESSED; // step to state 
          DPRINT(F("**** btn Switched r->p: "));DPRINTLN(getStatusChar(tmpCharArray));
        } else {} // wait. Stay in this state.
      }//else
    break;
    case KZGinput_STAN_KLIK_DU:
      if (now-_startTime > KZGinput_clickTicks) 
      {
        _isClicked=true;
        _state = KZGinput_STAN_RELEASED; // restart.
        DPRINT(F("**** btn Clicked r->p->r: "));DPRINTLN(getStatusChar(tmpCharArray));
      } else 
      {
        if (buttonLevel == _buttonPressed) 
        {
          _state = KZGinput_STAN_KLIK_DUD; // step to state 
          _startTime=now;
          DPRINT(F("**** btn Clicked w8 4 dbl Click r->p->r->p: "));DPRINTLN(getStatusChar(tmpCharArray));
        } // if
      }
    break;
    case KZGinput_STAN_KLIK_DUD:
      if (now-_startTime > KZGinput_clickTicks)
      {
        _isDblClicked=true;
        _state = KZGinput_STAN_PRESSED; // koniec czasu uznaj ze byl klik.
        DPRINT(F("**** btn dblClicked r->p->r->press: "));DPRINTLN(getStatusChar(tmpCharArray));
      } else 
      {
        if (buttonLevel == _buttonReleased) 
        {
          _isDblClicked=true;
          _state = KZGinput_STAN_RELEASED;//KZGinput_STAN_KLIK_DUDU; // koniec
          DPRINT(F("**** btn dblClicked r->p->r->p->relese: "));DPRINTLN(getStatusChar(tmpCharArray));
        } // if
      }
    break;
    case KZGinput_STAN_PRESSED:
      if (buttonLevel == _buttonReleased) 
      {
        _state = KZGinput_STAN_KLIK_U; // step to state 
        _startTime = now; // remember starting time
        DPRINT(F("**** btn relesed p->r: "));DPRINTLN(getStatusChar(tmpCharArray));
     }
    break;
    case KZGinput_STAN_KLIK_U:
      if (buttonLevel == _buttonPressed) 
      {
        _state = KZGinput_STAN_KLIK_UD; // step to state 
        _startTime = now;
        DPRINT(F("**** btn Clicked? p->r: "));DPRINTLN(getStatusChar(tmpCharArray));
      }
      else 
      {
        if ((buttonLevel == _buttonReleased) && (now - _startTime  >  KZGinput_pressTicks)) 
        {
          _isSwitched=true;
          _isRelesed=true;
          _state = KZGinput_STAN_RELEASED; 
          DPRINT(F("**** btn Switched p->r: "));DPRINTLN(getStatusChar(tmpCharArray));
        } else {} // wait. Stay in this state.
      }
    break;
    case KZGinput_STAN_KLIK_UD:
      if (now-_startTime > KZGinput_clickTicks) 
      { 
        _isClicked=true;
        _state = KZGinput_STAN_PRESSED; // restart.
        DPRINT(F("**** btn Clicked p->r->p: "));DPRINTLN(getStatusChar(tmpCharArray));
        
      } else
      {
        if (buttonLevel == _buttonReleased) 
        {
          _state = KZGinput_STAN_KLIK_UDU; // step to state 
          _startTime=now;
          DPRINT(F("**** btn Clicked w8 4 dbl Click p->r->p->r: "));DPRINTLN(getStatusChar(tmpCharArray));
        } // if
      }
    break;
    case KZGinput_STAN_KLIK_UDU:
      if (now-_startTime > KZGinput_clickTicks)
      {
        _isDblClicked=true;
        _state = KZGinput_STAN_RELEASED;
        DPRINT(F("**** btn dblClicked p->r->p->release: "));DPRINTLN(getStatusChar(tmpCharArray));
      } else
      {
        if (buttonLevel == _buttonPressed) 
        {
          _isDblClicked=true;
          _state = KZGinput_STAN_PRESSED; // koniec
          DPRINT(F("**** btn dblClicked p->r->p->r->press: "));DPRINTLN(getStatusChar(tmpCharArray));
        } // if
      }
    break;
  }// end switch
  return _isClicked | _isDblClicked | _isPressed | _isRelesed | _isSwitched;
} // end loop
// end.
