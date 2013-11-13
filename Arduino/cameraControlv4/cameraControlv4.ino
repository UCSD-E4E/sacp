/* cameraControlv4 -
This program controls the camera for the aerial imaging platform,
 gets a char over the serial connection and determines which command is sent, 
 takes pictures, sets multishoot mode, sets autofocus and
 sends reset signal to the Mega, if needed
 By Michael Carlson (mic2169853@maricopa.edu) 
 08/07/2013
 
 11/13/2013
 */
 
#define CAMERA 5
#define MEGA 4
#define AUTOFOCUS 3
//#define DEBUG
#include <GPS_UBLOX.h>
#include "SoftwareSerial.h"


char command;
boolean multishoot;
long lastPicTime;
long currTime;

//setup: set output pins, start serial connection and initialize variables
void setup()
{
  pinMode(CAMERA, OUTPUT);
  pinMode(MEGA, OUTPUT);
  pinMode(AUTOFOCUS, OUTPUT);
  Serial.begin(57600);
  multishoot = false;
  lastPicTime = 0;
  
  Serial.begin(57600);
  Serial.println("GPS UBLOX library test");
  GPS.Init();   // GPS Initialization
  delay(1000);
}//end setup()

void loop()
{
  GPS.Read();
  //takePicture();
  //if there is a value in the serial buffer
  if(Serial.available() > 0)
  {
    command = Serial.read();
  //commands: 't' = take picture, 'm' = multishoot mode on
  //'l' = multishoot mode off, 'r'=reset the Mega
  // 'c' = turn on autofocuss, 'j' = turn off autofocus
  switch(command)
  {
    case 't':
      takePicture();
      break;
      
    case 'm':
      multishoot = true;
      break;
      
    case 'l':
      multishoot = false;
      break;
    
    case 'r':
      digitalWrite(MEGA, LOW);
      delay(250);
      digitalWrite(MEGA, HIGH);
      break;
      
    case 'c':
      digitalWrite(AUTOFOCUS, HIGH);
      break;
      
    case 'j':
      digitalWrite(AUTOFOCUS, LOW);
      break;
      
    default:
      Serial.write(command);
      break;
      }//end switch(command)
  }//end if(Serial.available() > 0)
  //set command to null to keep the command from repeating
  command = NULL;
  //get the time for multishoot mode
  currTime = millis();
  #ifdef DEBUG
  Serial.print("currTime is ");
  Serial.println(currTime, DEC);
  #endif
  //if multishoot is enabled
  if(multishoot)
  {
    #ifdef DEBUG
    Serial.println("In multihsoot");
    Serial.print("Before TC, lastPicTime is ");
    Serial.println(lastPicTime, DEC);
    #endif
    //if it has been at least 2 seconds since last picture
    if((currTime - lastPicTime) > 2000)
   {
     takePicture();
     lastPicTime = currTime;
     #ifdef DEBUG
     Serial.print("After taking pic, lastPicTime is ");
     Serial.println(lastPicTime, DEC);
     #endif
   } //end if((currTime - lastPicTime) > 2000)
    
  }//end if(multishoot)
  
}//end loop

//void takePicture() sends signal to camera to take a picture
void takePicture() 
{
  #ifdef DEBUG
  Serial.println("in takePicture()");
  #endif
  //delay(500);
  digitalWrite(CAMERA, HIGH);
  delay(250);//delayMicroseconds(1000);
  digitalWrite(CAMERA, LOW);
  
    Serial.print("GPS:");
    Serial.print(" Time:");
    Serial.print(GPS.Time);
    Serial.print(" Fix:");
    Serial.print((int)GPS.Fix);
    Serial.print(" Lat:");
    Serial.print(GPS.Lattitude);
    Serial.print(" Lon:");
    Serial.print(GPS.Longitude);
    Serial.print(" Alt:");
    Serial.print(GPS.Altitude/1000.0);
    Serial.print(" Speed:");
    Serial.print(GPS.Ground_Speed/100.0);
    Serial.print(" Course:");
    Serial.print(GPS.Ground_Course/100000.0);
    Serial.println();

}//end takePicture()
