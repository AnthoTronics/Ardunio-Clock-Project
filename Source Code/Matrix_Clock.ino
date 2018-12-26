/*Arduino Clock by AnthoTRONICS
* Last edit: December 21,2018
*/
// Libraries you'll need (Same as the ones in the guide):
// MD Parola: https://github.com/MajicDesigns/MD_Parola // click the link to download the library
// MD_MAX72XX: https://github.com/MajicDesigns/MD_MAX72XX //click the link to download the library
// DS3231: https://github.com/adafruit/RTClib

/*CODE:*/
// Header file includes
// These are for the matrix
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include "Font_Data.h"

//These are for the clock
#include <DS3231.h>
#include <Wire.h>
DS3231 Clock;

bool Century=false;
bool h12;
bool PM;
byte  dd,mm,yyy;
uint16_t  h, m, s;

#define MAX_DEVICES 4 // Set the number of devices
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW 
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define CLK_PIN   13
#define DATA_PIN  11
#define CS_PIN    10

// Hardware SPI connection
MD_Parola P = MD_Parola(HARDWARE_TYPE,CS_PIN, MAX_DEVICES);

#define SPEED_TIME 75 // speed of the transition
#define PAUSE_TIME  0  

#define MAX_MESG  20


// Global variables
char szTime[9];    // mm:ss\0
char szMesg[MAX_MESG+1] = "";

uint8_t degC[] = { 6, 3, 3, 56, 68, 68, 68 }; // Deg C
uint8_t degF[] = { 6, 3, 3, 124, 20, 20, 4 }; // Deg F

char *mon2str(uint8_t mon, char *psz, uint8_t len)

// Get a label from PROGMEM into a char array
{
  static const __FlashStringHelper* str[] =
  {
    F("Jan"), F("Feb"), F("Mar"), F("Apr"),
    F("May"), F("Jun"), F("Jul"), F("Aug"),
    F("Sep"), F("Oct"), F("Nov"), F("Dec")
  };

  strncpy_P(psz, (const char PROGMEM *)str[mon-1], len);
  psz[len] = '\0';

  return(psz);
}

char *dow2str(uint8_t code, char *psz, uint8_t len)
{
  static const __FlashStringHelper*	str[] =
  {
  F("Sunday"), F("Monday"), F("Tuesday"),
  F("Wednesday"), F("Thursday"), F("Friday"),
  F("Saturday"), F("Sunday")
  };

  strncpy_P(psz, (const char PROGMEM *)str[code-1], len);
  psz[len] = '\0';

  return(psz);
}
// Time Setup: Code for reading clock time
void getTime(char *psz, bool f = true)
{
  s = Clock.getSecond();
  m = Clock.getMinute();
  //h =Clock.getHour(h12,PM);//24HR Format
  sprintf(psz, "%02d%c%02d", h, (f ? ':' : ' '), m);
  //12hr Format
  //uncomment if you want the clock to be in 12hr Format
  if (Clock.getHour(h12,PM)>=13 || Clock.getHour(h12,PM)==0)
  {
    h = Clock.getHour(h12,PM) - 12;
  }
  else
  {
    h = Clock.getHour(h12,PM);
  }

}

void getDate(char *psz)
// Date Setup: Code for reading clock date
{
   char  szBuf[10];
 
  
   dd=Clock.getDate();
   mm=Clock.getMonth(Century); //12
   yyy=Clock.getYear();
   sprintf(psz, "%d %s %04d",dd , mon2str(mm, szBuf, sizeof(szBuf)-1),(yyy + 2000));
 //strcpy(szMesg, "29 Feb 2016");
}

void setup(void)
{
  P.begin(2);
  P.setInvert(false); //we don't want to invert anything so it is set to false
  Wire.begin();
  
  P.setZone(0,  MAX_DEVICES-4, MAX_DEVICES-1);
  
  P.setZone(1, MAX_DEVICES-4, MAX_DEVICES-1);
  P.displayZoneText(1, szTime, PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);

  P.displayZoneText(0, szMesg, PA_CENTER, SPEED_TIME, 0,PA_PRINT , PA_NO_EFFECT);

  P.addChar('$', degC);
  P.addChar('&', degF);

}

void loop(void)
{
  static uint32_t lastTime = 0; // millis() memory
  static uint8_t  display = 0;  // current display mode
  static bool flasher = false;  // seconds passing flasher

  P.displayAnimate();

   if (P.getZoneStatus(0))
  {
    switch (display)
    {
      case 0:    // Temperature deg C
      P.setPause(0,1000);
      P.setTextEffect(0, PA_SCROLL_LEFT, PA_SCROLL_UP);
      display++;    
      dtostrf(Clock.getTemperature(), 3, 1, szMesg);
      strcat(szMesg, "$");
      //strcat(szMesg, "26.5$");
    

        break;

      case 1: // Temperature deg F
        P.setTextEffect(0, PA_SCROLL_UP, PA_SCROLL_LEFT);
        display++;
        dtostrf((1.8 *Clock.getTemperature() )+32, 3, 1, szMesg);
        strcat(szMesg, "&");
   
   
   // strcpy(szMesg, "71.6&");

        break;

      case 2: // Clock
     
     P.setFont(0, numeric7Seg);
     P.setTextEffect(0, PA_PRINT, PA_NO_EFFECT);
     //Sleep Mode
     //Uncomment to enable Sleep Mode and adjust the hours to your needs
     /*if (h==12 || h<8)//Time intervals (in this case, from 12AM to 8AM) 
     {
      P.setIntensity(0); //Set display brightness to lowest setting
     }
     else
     {
      P.setIntensity(6); //Set display brightness to 6 (15 is the brighest)
     }*/
     P.setPause(0,0);

     if (millis() - lastTime >= 1000)
    {
    lastTime = millis();
    getTime(szMesg, flasher);
    flasher = !flasher;
    }
    if(s==00&& s<=30){
    display++;
    P.setTextEffect(0, PA_PRINT, PA_WIPE_CURSOR);
    }
    //  strcpy(szMesg, "36 % RH");               
    
        break;
      
      case 3: // day of week
       
        P.setFont(0,nullptr);
        P.setTextEffect(0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        display++;
        dow2str(Clock.getDoW()+1, szMesg, MAX_MESG); // Added +1 to get correct DoW

       //dow2str(5, szMesg, MAX_MESG);
        break;
      default:  // Calendar
        
        P.setTextEffect(0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        display = 0;
        getDate(szMesg);
        break;
    }

    P.displayReset(0); //rest zone zero
    
  
  }

}///////end of loop


