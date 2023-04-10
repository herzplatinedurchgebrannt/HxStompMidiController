// choose ATmega 328P (old Bootloader) before uploading (using a cheap Arduino nano copy)

// libraries
#include <Arduino.h>
#include <EEPROM.h>

// definitions
#define BUTTON_1  3
#define BUTTON_2  4
#define BUTTON_3  5
#define LEDPAGE_1  9
#define LEDPAGE_2 10
#define LEDPAGE_3 11
#define MIDI_BAUDRATE 31250
#define NUMBER_INPUTS 3
#define TIME_DEBOUNCE 20
#define TIME_CHANGE_PAGE 2000

int tasterInputs[3]= {BUTTON_1,BUTTON_2,BUTTON_3};    
int page; // current command page

// arrays for MIDI commands -> "CC" Control Change [statusbyte,databyte1,databyte2]; "PC" ProgramChange [statusbyte,databyte1]
// you get a lot of information about MIDI here: https://www.zem-college.de/midi/mc_cvm4.htm 

int statusByte[3];    
int dataByte[3];
int dataByte2[3];

bool buttonStatus[3]      = {false, false, false};            // is true during the button is pressed
bool buttonPressed[3]     = {false, false, false};            // detects that a button was already pressed
bool sendData[3]          = { true,  true,  true};            // to become true a button has to be off. otherwise you would send a million midi commands (you probably dont want that :-P )
bool timerCheck[3]        = {false, false, false};            // a timer check for timerButtonPressed that detects the moment (and only this moment!) when the button was pressed
bool buttonPressedLong[3] = {false, false, false};            // to detect a "long" command, which means you change the command page

unsigned long timerCurrent[3]      = {0,0,0};                 // current time
unsigned long timerButtonPressed[3]= {0,0,0};                 // a timer to check how long a button was pressed




void changePage()
{
  switch (page) {
    case 0:  
      // Helix Stomp FS4
      statusByte  [0] = 176;
      dataByte    [0] = 52;
      dataByte2   [0] = 127;
  
      // Helix Stomp FS5
      statusByte  [1] = 176;
      dataByte    [1] = 53;
      dataByte2   [1] = 127;
  
      // Helix ChangeView
      statusByte  [2] = 176;
      dataByte    [2] = 71;
      dataByte2   [2] = 4;
      
      digitalWrite(LEDPAGE_1, HIGH);
      digitalWrite(LEDPAGE_2,  LOW);
      digitalWrite(LEDPAGE_3,  LOW);
  
      //update EEPROM w. current page
      EEPROM.update(0, page);
      break;
      
    case 1:
      // tuner
      statusByte  [0] = 176;
      dataByte    [0] = 68;
      dataByte2   [0] = 127;
  
      // footswitchmode next mode
      statusByte  [1] = 176;
      dataByte    [1] = 71;
      dataByte2   [1] = 4;
  
      // footswitchmode previous mode
      statusByte  [2] = 176;
      dataByte    [2] = 71;
      dataByte2   [2] = 5;
      
      digitalWrite(LEDPAGE_1,  LOW);
      digitalWrite(LEDPAGE_2, HIGH);
      digitalWrite(LEDPAGE_3,  LOW);
  
      //update EEPROM w. current page
      EEPROM.update(0, page);
      break;
      
    case 2:
      // snapshot 1
      statusByte  [0] = 176;
      dataByte    [0] = 69;
      dataByte2   [0] = 0;
  
      // snapshot 2
      statusByte  [1] = 176;
      dataByte    [1] = 69;
      dataByte2   [1] = 1;
  
      // snapshot 3
      statusByte  [2] = 176;
      dataByte    [2] = 69;
      dataByte2   [2] = 2;
      
      digitalWrite(LEDPAGE_1,  LOW);
      digitalWrite(LEDPAGE_2,  LOW);
      digitalWrite(LEDPAGE_3, HIGH);
  
      //update EEPROM w. current page
      EEPROM.update(0, page);
      break;
  
    default:
      // 
      break; // 
  }
}





void setup() 
{
  Serial.begin(MIDI_BAUDRATE);

  pinMode(BUTTON_1, INPUT);
  pinMode(BUTTON_2, INPUT);
  pinMode(BUTTON_3, INPUT);
  pinMode(LEDPAGE_1, OUTPUT);
  pinMode(LEDPAGE_2, OUTPUT);
  pinMode(LEDPAGE_3, OUTPUT);

  page = EEPROM.read(0);                                      //read the last saved page-value in EEPROM. The first time you upload this code on an Arduino there could be another value than 1-3 in EEPROM register 0
  if (page < 0 || page > 2){
    page = 0;
  }
  changePage();
}

void loop() 
{
  for (int i = 0; i < NUMBER_INPUTS; i++) 
  {
    changePage();
  
    // read current status of buttons
    buttonStatus[i] = digitalRead(tasterInputs[i]);
  
    if (buttonStatus[i] == true) 
    {
      if (timerCheck[i] == false)                             
      {
        timerButtonPressed[i] = millis();                      // timer checks how long button was pressed -> when a button is pressed longer than #TIME_CHANGE_PAGE -> page changes
        timerCheck[i] = true;
      }
      timerCurrent[i] = millis();
      buttonPressedLong[i] = false;
      
      if (timerCurrent[i] - timerButtonPressed[i] > TIME_CHANGE_PAGE)         // if a button is pressed long -> change page commands
      {
        if (sendData[i] == true)
        {
          page = i;
          sendData[i] = false;
        }
        buttonPressedLong[i] = true;
      }
      buttonPressed[i] = true;       
    } 
  
    else if (buttonStatus[i] == false) 
    {
      sendData[i] = true;
      timerCheck[i] = false;
      timerButtonPressed[i] = 0;
    } 
  
    if ((millis() - timerCurrent[i] > TIME_DEBOUNCE) && buttonPressed[i] == true && sendData[i] == true && buttonPressedLong[i] == false)
    {
       Serial.write(statusByte[i]);
       Serial.write(dataByte[i]); 
       Serial.write(dataByte2[i]); 
       sendData[i] = false;
       buttonPressed[i] = false;
    }
  }
}


////////////////////////////////////////////////////////INFO////////////////////////////////////////////////////////////
//
// to use FS4/FS5 functionality go the the helix stomps options by pressing both "PAGE" buttons 
// -> "Global Settings" -> Select "Stomp 4" -> ´FS4 Function" & "Stomp 5" -> "FS5 Function"
//
// Page 1 is an extension mode -> Footswitch 4 (Button 1), Footswitch 5 (Button 2), Switching pages (Button 3)
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

