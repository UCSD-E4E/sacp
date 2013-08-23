/*
AIPControl_and_StabilizationPIDv3
 This is the program to be used for control and stabilization of the Aerial Imaging Platform
 Version 3.1 uses PID controllers for roll, pitch and yaw stabilization, and commands have been
 changed to use a WASD steering control interface.Added anti-Integral Windup
 by Michael Carlson (mic2169853@maricopa.edu and includes code posted to the Arduino forums
 by Jon Russell, used by permission.
 08/07/2013
 */

//Constants
//#define DEBUG true
#define STATE_ZERO         0
#define STATE_S            1
#define STATE_SN           2
#define STATE_SNP          3
#define STATE_PT           4
#define STATE_READ_DATA    5
#define STATE_CHK1         6
#define STATE_CHK0         7
#define STATE_DONE         8 

#define BAUD 57600
#define UM6_GET_DATA       0xAE
#define UM6_REG_EULER_ROLL_PITCH  0x62
#define UM6_REG_EULER_YAW  0x63
#define UM6_EULER_SCALAR  0.0109863

#define PT_HAS_DATA  0b10000000
#define PT_IS_BATCH  0b01000000
#define PT_COMM_FAIL 0b00000001

#define DATA_BUFF_LEN  16
#define PAD .125
#define YAW_FLAT 1530
#define YAW_PROP_CONSTANT 5
#define ROLL_PROP_CONSTANT 4
#define ROLL_FLAT 1538
#define ZERO_ROLL 0.0
#define PITCH_PROP_CONSTANT 8
#define PITCH_FLAT 1535
#define ZG_COM_PCKT_SIZE 7
#define FILTER_VALUE 50
#define BUMP 5.0
#define KP_PITCH 8.75
#define KI_PITCH 0.00375
#define KD_PITCH 4.25
#define KP_ROLL 8.625
#define KI_ROLL 0.0015
#define KD_ROLL 3.875
#define KP_YAW 8.0
#define KI_YAW 0.00075
#define KD_YAW 4.25
#define MIN_SUM_YAW -200
#define MIN_SUM_PITCH -200
#define MIN_SUM_ROLL -100
#define INT_WIND_UP 199
#define INT_WIND_UP_ROLL 99

//includes
#include <Servo.h>

//Global variable declarations
boolean activateFilter;
boolean activateStabilize;
char command;
byte incoming;
int nState = 0;
byte aPacketData[DATA_BUFF_LEN];
int n = 0;
byte c = 0;
int nDataByteCount = 0;
float pitchCenter;
float newPitch;

Servo yawServo;
Servo rollServo;
Servo pitchServo;
float yawCenter;
int yawBump;
boolean notyawCentered;
int rollCtrl;
boolean doZeroGyros;
float mappedPitchCenter;
float mapActualPitch;
float pvPitch;
float spPitch;
float diffPitch;
float sumPitch;
float deltaDiffPitch;
float oldDiffPitch;
float pvRoll;
float spRoll;
float diffRoll;
float sumRoll;
float deltaDiffRoll;
float oldDiffRoll;
float diffYaw;
float sumYaw;
float deltaDiffYaw;
float oldDiffYaw;
int lastTime;
int deltaT;
float yaw;
float roll;
float pitch;
byte zeroGyroCommand[] = {
  0x73, 0x6E, 0x70, 0x00, 0xAC, 0x01, 0xFD};

typedef struct {
  boolean HasData;
  boolean IsBatch;
  byte BatchLength;
  boolean CommFail;
  byte Address;
  byte Checksum1;
  byte Checksum0;
  byte DataLength;
} 
UM6_PacketStruct ;

UM6_PacketStruct UM6_Packet;



//Setup, start up Serial connections, servos and initialize variables
void setup()
{
  Serial1.begin(BAUD);
  Serial.begin(BAUD);
  yawServo.attach(12);
  rollServo.attach(10);
  pitchServo.attach(9);
  pitchServo.writeMicroseconds(PITCH_FLAT);
  rollServo.writeMicroseconds(ROLL_FLAT);
  yawServo.writeMicroseconds(YAW_FLAT);
  notyawCentered = true;
  doZeroGyros = true;
  activateFilter = false;
  activateStabilize = false;
  mappedPitchCenter = 90.0;
  newPitch = 90.0;
  pvPitch = 0.0;
  spPitch = 0.0;
  diffPitch = 0.0;
  sumPitch = 0.0;
  deltaDiffPitch = 0.0;
  oldDiffPitch = 0.0;
  pvRoll = 0.0;
  spRoll = 0.0;
  diffRoll = 0.0;
  sumRoll = 0.0;
  deltaDiffRoll = 0.0;
  oldDiffRoll = 0.0;
  diffYaw = 0.0;
  sumYaw = 0.0;
  deltaDiffYaw = 0.0;
  oldDiffYaw = 0.0;
}
//main program loop
void loop(){
  //Get incoming commands
  if(Serial.available() > 0)
  {
    incoming = Serial.read();
    command = char(incoming);
  }
  //parse commands and set flags based on commands
  //commands are: 'w' = pitch BUMPed up, 's' = pitch BUMPed down, 
  //'p'N = pitch to N degrees , where N is float between 0-180.0,
  //'f' = activate filter, 'n' = deactivate filter 'q' = activate stabilization
  //'z' = deactivate stabilization, 'y'N = add N degrees to current yaw 
  //'d' = bump yaw right 10 degrees, 'a' = bump yaw left 10 degrees
  switch (command)
  {
  case 'd':
    if((yawCenter + 10) >= 360)
    {
      yawCenter = yawCenter - 360;
    }
    yawCenter = yawCenter + 10;
    break;
  
  case 'a':
    if((yawCenter - 10) < 0)
    {
      yawCenter = yawCenter + 360;
    }
    yawCenter = yawCenter - 10;
    break;  
  
  case 'y':
    yawBump = Serial.parseInt();
    if((yawBump <= 180) && (yawBump >= -180))
    {
      if( (yawCenter + yawBump)>= 360)
      {
        yawCenter = yawCenter - 360;
      }
      yawCenter = yawCenter + yawBump;
    }
    break;
    
  case 'f':
    activateFilter = true;
    break;

  case 'n':
    activateFilter = false;
    break;

  case 'q':
    activateStabilize = true;
    break;

  case 'z':
    activateStabilize = false;
    break;

  case 'w':
    mappedPitchCenter = mappedPitchCenter + BUMP;
    break;

  case 's':
    mappedPitchCenter = mappedPitchCenter - BUMP;
    break;

  case 'p':
    newPitch = Serial.parseFloat();
    if ( (newPitch >= 80.0) && (newPitch <= 120.0))
    {
      mappedPitchCenter = newPitch;
    } 
    break;
  }
  command = 0;
  //the IMU gyros tend to drift during the first 5 minutes since powerup, so 
  //send commands to zero gyros during the first 6 minutes
/*  if(doZeroGyros && millis() > 370000)
  {
    doZeroGyros = false;
    #ifdef DEBUG
    Serial.println("YAW_FLATping Zero Gyros commands");
    #endif
  }
  if(doZeroGyros && timeToZeroGyros())
  {
    sendReceiveZeroGyroCommand();
    #ifdef DEBUG
    //!!!!!debug comment out of final program
    Serial.println("Sending Zero Gyro Command");
    #endif
  }
*/

  //get data packet from IMU
  n = Serial1.available();
  if (n > 0){
    c = Serial1.read();
    //Serial.print("c=");
    //Serial.println(c, HEX);
    switch(nState){
    case STATE_ZERO : // Begin. Look for 's'.
      // Start of new packet...
      Reset();
      if (c == 's'){
        nState = STATE_S;
      } 
      else {
        nState = STATE_ZERO;
      }
      break;
    case STATE_S : // Have 's'. Look for 'n'.
      if (c == 'n'){
        nState = STATE_SN; 
      } 
      else {
        nState = STATE_ZERO;
      }
      break;
    case STATE_SN : // Have 'sn'. Look for 'p'.
      if (c == 'p'){
        nState = STATE_SNP; 
      } 
      else {
        nState = STATE_ZERO;
      }
      break;
    case STATE_SNP : // Have 'snp'. Read PacketType and calculate DataLength.
      UM6_Packet.HasData = 1 && (c & PT_HAS_DATA);
      UM6_Packet.IsBatch = 1 && (c & PT_IS_BATCH);
      UM6_Packet.BatchLength = ((c >> 2) & 0b00001111);
      UM6_Packet.CommFail = 1 && (c & PT_COMM_FAIL);
      nState = STATE_PT;
      if (UM6_Packet.IsBatch){
        UM6_Packet.DataLength = UM6_Packet.BatchLength * 4;
        //  Serial.print("Is Batch and batch length is ");
        //   Serial.println(UM6_Packet.BatchLength, DEC);
      } 
      else {
        UM6_Packet.DataLength = 4;
        //  Serial.println("Is not a batch");
      }
      break;
    case STATE_PT : // Have PacketType. Read Address.
      UM6_Packet.Address = c;
      nDataByteCount = 0;
      nState = STATE_READ_DATA; 
      break;
    case STATE_READ_DATA : // Read Data. (UM6_PT.BatchLength * 4) bytes.
      aPacketData[nDataByteCount] = c;
      nDataByteCount++;
      if (nDataByteCount >= UM6_Packet.DataLength){
        nState = STATE_CHK1;
      }
      break;
    case STATE_CHK1 : // Read Checksum 1
      UM6_Packet.Checksum1 = c;
      nState = STATE_CHK0;
      break;
    case STATE_CHK0 : // Read Checksum 0
      UM6_Packet.Checksum0 = c;
      nState = STATE_DONE;
      break;
    case STATE_DONE : // Entire packet consumed. Process packet
      ProcessPacket();
      nState = STATE_ZERO;
      break;
    }//end switch
  }//end if(n>0) ie if Serial1.available
  //!!!!!debug comment out of final program
  else //we didn't get a valid packet
  {
    #ifdef DEBUG
    Serial.println("Data not available");
    #endif
  } 
}//end loop()

//Helper functions

//ProcessPacket() code to extract data from the IMU
//originally designed for reading quaternion values, modified to use Euler angles
void ProcessPacket(){
  float scaledRoll = 0;
  float scaledPitch = 0;
  float scaledYaw = 0;
  float notUsedRegData = 0;
  short regData = 0;
  //if we have data from Euler registers, read it
  if(UM6_Packet.Address == UM6_REG_EULER_ROLL_PITCH) {
    //!!!!!debug comment out of final program
    /*#ifdef DEBUG
    Serial.print("Millis since startup: ");
    Serial.println(millis());
    Serial.print("Euler roll and pitch: ");
    #endif*/
    if (UM6_Packet.HasData && !UM6_Packet.CommFail){
      regData = (aPacketData[0] << 8) | aPacketData[1];
      scaledRoll = float(regData) * UM6_EULER_SCALAR;
      regData = (aPacketData[2] << 8) | aPacketData[3];
      scaledPitch = float(regData) * UM6_EULER_SCALAR;
      if (UM6_Packet.DataLength > 4){
        regData = (aPacketData[4] << 8) | aPacketData[5];
        scaledYaw = float(regData) * UM6_EULER_SCALAR;
        regData = (aPacketData[6] << 8) | aPacketData[7];
        notUsedRegData = regData * UM6_EULER_SCALAR;
      }
    }

    //make sure yaw is a positive number
    if(scaledYaw < 0)
    {
      scaledYaw = scaledYaw + 360;
    }
    #ifdef DEBUG2
    Serial.print("scaledYaw after constrain is ");
    Serial.println(scaledYaw);
    #endif
    //filter outlier values
    //If there is NOT a greater than FILTER_VALUE degree change since last read, use the data
    //otherwise we discard that data and keep last valid data
    if(activateFilter)
    {
      #ifdef DEBUG
      Serial.println("Filter activated");
      #endif
      if(!(((abs(scaledRoll) - abs(roll)) > FILTER_VALUE) || ((abs(roll) - abs(scaledRoll)) > FILTER_VALUE))) 
      {
        roll = scaledRoll;
      }
      else
      {
        #ifdef DEBUG
        Serial.println("Filtering roll");
        #endif
      }
      if(!(((abs(scaledPitch) - abs(pitch)) > FILTER_VALUE) || ((abs(pitch) - abs(scaledPitch)) > FILTER_VALUE))) 
      {
        pitch = scaledPitch;
        mapActualPitch = pitch + 90.0;
      }
      if(!(((abs(scaledYaw) - abs(yaw)) > FILTER_VALUE) || ((abs(yaw) - abs(scaledYaw)) > FILTER_VALUE) || (scaledYaw < 0) || (scaledYaw > 360))) 
      {
        yaw = scaledYaw;
        
      }
      else if((((yaw-scaledYaw) > 355) && ((yaw - scaledYaw) < 361)) || (((scaledYaw-yaw) > 355) && ((scaledYaw-yaw) < 361)))
      {
        yaw = scaledYaw;
      }
      else
      {
        #ifdef DEBUG
        Serial.println("Filtering yaw");
        #endif
      }

    }//end if(activateFilter)
    //this data is not filtered 
    else
    {
      roll = scaledRoll;
      pitch = scaledPitch;
      mapActualPitch = (pitch + 90.0);
      if((scaledYaw >= 0) && (scaledYaw <=360))
      {
        yaw = scaledYaw;
      }
    }
    #ifdef DEBUG2
   Serial.print("Yaw is ");
   Serial.println(yaw);
   Serial.print("yawCenter is");
   Serial.println(yawCenter);
   #endif
    //Yaw is arbritary, so we need a beginning value 
    //for when yaw is "centered"
    if(notyawCentered)
    {
      yawCenter=yaw;
      if(yawCenter < 0)
      {
        yawCenter = yawCenter + 360;
      }
      notyawCentered = false;
    }
 

    if(activateStabilize)
    {
       //calculate deltaT
       int currTime = millis();
       deltaT = currTime - lastTime;
       lastTime = currTime;
       
       //yaw stabilization
       yawServo.writeMicroseconds(yawPID());
       
      //roll stabilization 
      rollServo.writeMicroseconds(rollPID());
      mappedPitchCenter = constrain(mappedPitchCenter, 80, 120);
      //pitch stabilization      
      pitchServo.writeMicroseconds(pitchPID());
    }//end if(activateStabilize)
    //!!!!!debug comment out of final program
    #ifdef DEBUG
    PrintDebugFloatABC(roll, mapActualPitch, yaw);
    #endif
  }
  //!!!!!debug comment out of final program
  else
  {
    #ifdef DEBUG
    Serial.println("Unknown data");
    #endif
  }

  ;
}
//Reset() re-initializes packet struct
void Reset(){
  UM6_Packet.HasData = false;
  UM6_Packet.IsBatch = false;
  UM6_Packet.BatchLength = 0;
  UM6_Packet.CommFail = false;
  UM6_Packet.Address = 0;
  UM6_Packet.Checksum1 = 0;
  UM6_Packet.Checksum0 = 0;
  UM6_Packet.DataLength = 0;
}

void PrintDebugFloatABC(float a, float b, float c){
  Serial.print(" Roll = ");
  Serial.print(a,3);
  Serial.print(" Pitch = ");
  Serial.print(b,3);
  Serial.print(" Yaw = ");
  Serial.println(c,3);
}
//yawPID() PID controller for yaw, accounts for moving across zero in positive and negative directions
//as well as moving in negative and positive directions without crossing zero
int yawPID()
{
  int retMics;
    //case 1: moving clockwise less than 180 degrees and not crossing 0
  if( (yawCenter > yaw) && ((yawCenter - yaw) <= 180))
  {
    diffYaw =  (-1 * (yawCenter - yaw));
  }
  //case 2: moving clockwise less than 180 degrees but crossing zero
  else if( (yaw > yawCenter) && ((yaw - yawCenter) > 180) )
  {
    diffYaw = (-1 * ((yawCenter + 360) - yaw));
  }
  //case 3: moving counterclockwise less than 180 degrees and not crossing zero
  else if( ( yaw > yawCenter) && ((yaw - yawCenter) <= 180))
  {
    diffYaw = (yaw - yawCenter);
  }
  //case 4: moving counterclockwise less than 180 degrees but crossing zero
  else if( ( yawCenter > yaw) && ((yawCenter - yaw) > 180))
  {
    diffYaw = ((yaw + 360) - yawCenter);
  }
  //reset sumYaw if integral term is headed to windup
  if(abs(diffYaw) > INT_WIND_UP)
  {
    sumYaw = 0;
  }
  //reset sumYaw if error changes sign
  if((diffYaw*oldDiffYaw)<=0)
  {
    sumRoll = 0;
  }
  sumYaw += diffYaw;
  if(sumYaw < MIN_SUM_YAW)
  {
    sumYaw = MIN_SUM_YAW;
  }
    #ifdef DEBUG
  Serial.print("sumYaw is ");
  Serial.println(sumYaw, DEC);
  #endif
  deltaDiffYaw = diffYaw - oldDiffYaw;
  oldDiffYaw = diffYaw;
  retMics =YAW_FLAT + ((KP_YAW * diffYaw) + (KI_YAW * (sumYaw * deltaT)) + (KD_YAW * (deltaDiffYaw/deltaT)));
  
  return retMics;
}

//rollProp simple proportion controller for roll
int rollPID()
{
  int retMics;

  pvRoll = roll;
  spRoll = ZERO_ROLL;
  diffRoll = spRoll - pvRoll;
  //reset sumRoll if integral term is headed towards windup
  if(abs(diffRoll) > INT_WIND_UP_ROLL)
  {
    sumRoll = 0;
  }
  //reset sumRoll if error changes sign
  if((diffRoll*oldDiffRoll)<=0)
  {
    sumRoll = 0;
  }
  sumRoll += diffRoll;
  if(sumRoll < MIN_SUM_ROLL)
  {
    sumRoll = MIN_SUM_ROLL;
  }
    #ifdef DEBUG
  Serial.print("sumRoll is ");
  Serial.println(sumRoll, DEC);
  #endif
  deltaDiffRoll = diffRoll - oldDiffRoll;
  oldDiffRoll = diffRoll;
  retMics = ROLL_FLAT + ((KP_ROLL * diffRoll) + (KI_ROLL * (sumRoll*deltaT)) + (KD_ROLL * (deltaDiffRoll/deltaT)));
  return retMics;
}
//pitchProp simple proportion controller for pitch
//testing PICD
int pitchPID()
{
  int retMics;

  pvPitch = mapActualPitch;
  spPitch = mappedPitchCenter;
  diffPitch = spPitch - pvPitch;
  //reset sumPitch if integral term is headed towards windup
  if(abs(diffPitch) > INT_WIND_UP)
  {
    sumPitch = 0;
  }
  //reset sumPitch if error changes sign
  if((diffPitch*oldDiffPitch)<=0)
  {
    sumPitch = 0;
  }
  sumPitch += diffPitch;
  if(sumPitch < MIN_SUM_PITCH)
  {
    sumPitch = MIN_SUM_PITCH;
  }

  #ifdef DEBUG
  Serial.print("sumPitch is ");
  Serial.println(sumPitch, DEC);
  #endif
  deltaDiffPitch = diffPitch - oldDiffPitch;
  oldDiffPitch = diffPitch;
  retMics = PITCH_FLAT + ((KP_PITCH * diffPitch) + (KI_PITCH * (sumPitch*deltaT)) + (KD_PITCH * (deltaDiffPitch/deltaT)));    
 

  return retMics;

}
//sendReceiveZeroGyroCommand sends command to zero gyros
void sendReceiveZeroGyroCommand()
{
  for(int i = 0; i < ZG_COM_PCKT_SIZE; i++)
  {
    Serial1.write(zeroGyroCommand[i]);
  }
}

//timeToZeroGyros timer for sending zero gyros command
boolean timeToZeroGyros()
{
  if((millis() < 360100) && ((millis() % 90000) < 20))
  {
    return true;
  }
  else
  {
    return false;
  }
}

