
 /*
****************************************************************************************
****************************************************************************************
20231121
Files Required to make a complete program - 
  CLI_V1.0, Quest_CLI.h, Quest_Flight.h, Quest_flight.cpp
      Important Functions and their use:
          cmd_takeSphot
              This function will use the serial camera (the one with the cable) to take a photo (jpg)
              on the transfer of this photo to the Host controller it will create a text file (txt)
              with the same file name as the jpg file of the conditions of when the jpg was taken.
              NOTE: add2text can be used with this command
          cmd_takeSpiphoto
              This function is for the SPI camera (the one that plugges directly on the microlab board)
              it will take a jpg the same as the serial camera, also creating a txt file.  Also NOTE: the
              add2text function can be used on the photo text file.
          nophotophoto
              if a camera is not being used, then this will simulate the camera photo operation. The text file 
              is still created, so the add2text function can be used the get data downloaded in the text file
          nophoto30K
              this dois not use a camera, instead it uses the file space for the photo the containe ascii data
              using dataappend function defined below.  The data will still have a jpg etension, but the file
              will contain ascii of the data put in buy the dataappend unction. Plus a text file is generated,
              and the add2text function will add more data.
          -----
          add2text(int value1,int value2,int value3)
              this function will take the three varables and place them at the end of the text file (the file is
              limited the 1024 charators),  if you look at the function the text output can be formatted is almost
              any way.  Look at it and change it to acccomadate your data...
          dataappend(int counts,int ampli,int SiPM,int Deadtime)
              this function will add ascii data to the 30K byte data buffer that was used a a jpg photo.  look at
              the data formating and change it necessary for you project information.  To view the data, you can
              use a hex/ascii file viewer or change the ext to a txt.  then it should be viewable with a text exitor.
.
******************************************************************************************                                   
******************************************************************************************                  
*/

#include "Quest_Flight.h"
#include "Quest_CLI.h"
#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_AS726x.h"
Adafruit_AS726x ams;

//////////////////////////////////////////////////////////////////////////
//    This defines the timers used to control flight operations
//////////////////////////////////////////////////////////////////////////
//  Fast clock --- 1 hour = 5 min = 1/12 of an  hour
//     one millie -- 1ms
//
#define SpeedFactor 60    // = times faster
//
//
//////////////////////////////////////////////////////////////////////////
//
#define one_sec   1000                       //one second = 1000 millis
#define one_min   60*one_sec                 // one minute of time
#define one_hour  60*one_min                 // one hour of time
//
//
#define TimeEvent1_time     ((one_min * 60) / SpeedFactor)      //take a photo time
#define Sensor1time         ((one_min * 15) / SpeedFactor)      //Time to make Sensor1 readings 
#define Sensor2time         ((one_sec * 20)  / SpeedFactor) 
//
  int sensor1count = 0;     //counter of times the sensor has been accessed
  int sensor2count = 0;     //counter of times the sensor has been accessed
  int State =   0;          //FOR TESTING ONLY WILL SWITCH FROM SPI CAMERA TO SERIAL CAMERA EVERY HOUR
  int valve = 0;
//
///////////////////////////////////////////////////////////////////////////
/**
   @brief Flying function is used to capture all logic of an experiment during flight.
*/
//************************************************************************
//   Beginning of the flight program setup
//
//
void Flying() {
  //
  Serial.println("\n\rRun flight program\n\r");
  //
  uint32_t TimeEvent1 = millis();               //set TimeEvent1 to effective 0
  uint32_t Sensor1Timer = millis();             //clear sensor1Timer to effective 0
  uint32_t Sensor2Timer = millis();             //clear sensor1Timer to effective 0
  uint32_t Sensor2Deadmillis = millis();        //clear mills for difference
  //
  uint32_t one_secTimer = millis();             //set happens every second
  uint32_t sec60Timer = millis();               //set minute timer

  //*****************************************************************
  //   Here to set up flight conditions i/o pins, atod, and other special condition
  //   of your program
  pinMode(IO0,OUTPUT);
  pinMode(IO1,OUTPUT);
  pinMode(IO3, OUTPUT);
  digitalWrite(IO0, HIGH);
  delay(1000);
  digitalWrite(IO0, LOW);
  digitalWrite(IO1, LOW);
  //digitalWrite(IO3,LOW);
//  Wire.begin();
  uint16_t sensorValues[AS726x_NUM_CHANNELS];
//  while(!Serial);
//  
//  // initialize digital pin LED_BUILTIN as an output.
//  pinMode(LED_BUILTIN, OUTPUT);
//
//  //begin and make sure we can talk to the sensor
//  if(!ams.begin()){
//    Serial.println("software issue (please check your wiring)");
//    while(1);
//  }
//
//  Serial.println("Set mode to command.");
  
  //
  //
  //******************************************************************

  //------------ flying -----------------------

  Serial.println("Flying NOW  -  x=abort");                 //terminal output for abort back to test
  Serial.println("Terminal must be reset after abort");     //terminal reset requirement upon soft reset

  missionMillis = millis();     //Set mission clock millis, you just entered flight conditions
  //
  //
  /////////////////////////////////////////////////////
  //----- Here to start a flight from a reset ---------
  /////////////////////////////////////////////////////
  //
  DateTime now = rtc.now();                   //get time now
  currentunix = (now.unixtime());             //get current unix time, don't count time not flying
  writelongfram(currentunix, PreviousUnix);   //set fram Mission time start now counting seconds unix time
  //
  //***********************************************************************
  //***********************************************************************
  //  All Flight conditions are now set up,  NOW to enter flight operations
  //
  //***********************************************************************
  //***********************************************************************
  //
  while (1) {
    //
    //----------- Test for terminal abort command (x) from flying ----------------------
    //
    while (Serial.available()) {      //Abort flight program progress
      byte x = Serial.read();         //get the input byte
      if (x == 'x') {                 //check the byte for an abort x
        return  ;                     //return back to poeration sellection
      }                               //end check
    }                                 //end abort check
//------------------------------------------------------------------
//
//*********** Timed Event 1 test ***************************************
//
    //  this test if TimeEvent1 time has come
    //  See above for TimeEvent1_time settings between this event
    //
    if ((millis() - TimeEvent1) > TimeEvent1_time) {
      TimeEvent1 = millis();                    //yes is time now reset TimeEvent1
          //  Take a photo using the serial c329 camera and place file name in Queue
      if (State == 0){      //which state ?             
          cmd_takeSphoto();            //Take serial photo and send it
      }
          //  Take a photo using the SPI c329 camera and place file name in Queue
          //  Hardware Note: to use the Spi camera - a jumper must be connected from IO0
          //  the the hold pin on J6.......
      if (State == 1){
          cmd_takeSpiphoto();         //Take SPI photo and send it
      }
          //  no camera - Send a 30k of buffer datta in place of a photo to the output Queue
      if (State == 2){
          nophoto30K();               //Use photo buffer for data
      }
          //  no camera - send just text appended with data to the output Queue
      if (State == 3){
          nophotophoto();               //photo event with no photo just to transfer data
      }
      State++;                          //go to the next state
      if (State == 4){                  //reset the state back to 0
        State = 0;                      //state to 0
      }
    }                                               //end of TimeEvent1_time
    //------------------------------------------------------------------
    //
//*******************************************************************************
//*********** One second counter timer will trigger every second ****************
//*******************************************************************************
    //  Here one sec timer - every second
    //
    if ((millis() - one_secTimer) > one_sec) {      //one sec counter
      one_secTimer = millis();                      //reset one second timer
      
      DotStarYellow();                              //turn on Yellow DotStar to Blink for running

      Serial.println("HIII");
      //
//****************** NO_NO_NO_NO_NO_NO_NO_NO_NO_NO_NO_ *************************
// DO NOT TOUCH THIS CODE IT IS NECESARY FOR PROPER MISSION CLOCK OPERATIONS
//    Mission clock timer
//    FRAM keep track of cunlitive power on time
//    and RTC with unix seconds
//------------------------------------------------------------------------------
      DateTime now = rtc.now();                           //get the time time,don't know how long away
      currentunix = (now.unixtime());                     //get current unix time
      Serial.print(currentunix); Serial.print(" ");      //testing print unix clock
      uint32_t framdeltaunix = (currentunix - readlongFromfram(PreviousUnix)); //get delta sec of unix time
      uint32_t cumunix = readlongFromfram(CumUnix);       //Get cumulative unix mission clock
      writelongfram((cumunix + framdeltaunix), CumUnix);  //add and Save cumulative unix time Mission
      writelongfram(currentunix, PreviousUnix);           //reset PreviousUnix to current for next time
      
//
//********* END_NO_END_NO_END_NO_END_NO_END_NO_END_NO_ **************************
      //
      //  This part prints out every second
      //

      
      Serial.print(": Mission Clock = ");      //testing print mission clock
      Serial.print(readlongFromfram(CumUnix));        //mission clock
      Serial.print(" is ");                        //spacer
      //
      //------Output to the terminal  days hours min sec
      //
      getmissionclk();
      Serial.print(xd); Serial.print(" Days  ");
      Serial.print(xh); Serial.print(" Hours  ");
      Serial.print(xm); Serial.print(" Min  ");
      Serial.print(xs); Serial.println(" Sec");
      //
      //
       DotStarOff();
    }  // end of one second routine
//
//**********************************************************************
//*********** Read Sensor1 Event read and add to text buffer************
//**********************************************************************
    
    if ((millis() - Sensor1Timer) > Sensor1time) {    //Is it time to read?
      Sensor1Timer = millis();                        //Yes, lets read the sensor1
      

      
      //read the device temperature
      

      digitalWrite(IO1, HIGH);
      Serial.println("Turned On");
      Wire.beginTransmission();
      
      
      
//      while(!Serial);
      
      // initialize digital pin LED_BUILTIN as an output.
//      pinMode(LED_BUILTIN, OUTPUT);
    
      //begin and make sure we can talk to the sensor
      delay(1000);
      
      while(!ams.begin()){
        Serial.println("could not connect to sensor! Please check your wiring.");
      }


      Serial.println("Set mode to command.");
//  
//      ams.drvOn(); //uncomment this if you want to use the driver LED for readings
      uint8_t temp = ams.readTemperature();
      ams.startMeasurement(); //begin a measurement
//      
//     //wait till data is available
      bool rdy = false;
      while(!rdy){

        Serial.println("STUCK!!!");
        delay(5);
        rdy = ams.dataReady();
      }
//      ams.drvOff(); //uncomment this if you want to use the driver LED for readings
//    
//      //read the values!
      ams.readRawValues(sensorValues);
//      //ams.readCalibratedValues(calibratedValues);//Yes, lets read the sensor1
      Serial.print("Temp: "); Serial.print(temp);
      Serial.print(" Violet: "); Serial.print(sensorValues[AS726x_VIOLET]);
      Serial.print(" Blue: "); Serial.print(sensorValues[AS726x_BLUE]);
      Serial.print(" Green: "); Serial.print(sensorValues[AS726x_GREEN]);
      Serial.print(" Yellow: "); Serial.print(sensorValues[AS726x_YELLOW]);
      Serial.print(" Orange: "); Serial.print(sensorValues[AS726x_ORANGE]);
      Serial.print(" Red: "); Serial.print(sensorValues[AS726x_RED]);
      Serial.println();
      Serial.println();
      
      Wire.endTransmission();
      digitalWrite(IO1, LOW);

      delay(2000);

      // turn on the other sensor

      digitalWrite(IO3, HIGH);              
      Wire.beginTransmission();
      
//      while(!Serial);
      
      // initialize digital pin LED_BUILTIN as an output.
//      pinMode(LED_BUILTIN, OUTPUT);
    
      //begin and make sure we can talk to the sensor
      delay(1000);
      
      while(!ams.begin()){
        Serial.println("could not connect to sensor! Please check your wiring.");
      }


      Serial.println("Set mode to command.");

      
      temp = ams.readTemperature();
      ams.startMeasurement(); //begin a measurement
    //      
    //     //wait till data is available
      rdy = false;
      while(!rdy){
        delay(5);
        rdy = ams.dataReady();
      }
//      ams.drvOff(); //uncomment this if you want to use the driver LED for readings
//    
//      //read the values!
      ams.readRawValues(sensorValues);
//      //ams.readCalibratedValues(calibratedValues);//Yes, lets read the sensor1
      Serial.print("Temp2: "); Serial.print(temp);
      Serial.print(" Violet2: "); Serial.print(sensorValues[AS726x_VIOLET]);
      Serial.print(" Blue2: "); Serial.print(sensorValues[AS726x_BLUE]);
      Serial.print(" Green2: "); Serial.print(sensorValues[AS726x_GREEN]);
      Serial.print(" Yellow2: "); Serial.print(sensorValues[AS726x_YELLOW]);
      Serial.print(" Orange2: "); Serial.print(sensorValues[AS726x_ORANGE]);
      Serial.print(" Red2: "); Serial.print(sensorValues[AS726x_RED]);
      Serial.println();
      Serial.println();
      

      delay(1000);

      Wire.endTransmission();
      digitalWrite(IO3, LOW);
      Serial.println("SENSOR READ DONEEEEE:)"); 
       sensor1count++;

      //cmd_rtcreg();
          
    }     // End of Sensor1 time event
    //
//**********************************************************************
//*********** Read Sensor2 Event read and add to text buffer************
//*********** Test of filling the 30K data buffer for lots of data *****
//********************************************************************** 
    //  If it is event driven then remove the Sensor2Timer evvvvvend 
    //  here to get the  data for the event 
    //
//    if ((millis() - Sensor2Timer) > Sensor2time) {    //Is it time to read?
//      Sensor2Timer = millis();                        //Yes, lets read the sensor1
//      sensor2count++;
//
//      if (valve == 0) {
//        digitalWrite(IO0, HIGH);
//        valve = 1;
//      } else {
//        digitalWrite(IO0, LOW);
//        valve = 0;
//      }
//      sensor2count++;
//      //
//      
//    }     // End of Sensor2Timer          
  }       // End of while 
}         //End nof Flighting
//
//
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
//    This is a function to adds three values to the user_text_buffer
//    Written specificy for 2023-2024 Team F, Team B,
//    Enter the function with "add2text(1st interger value, 2nd intergre value, 3rd intergervalue);
//    the " - value1 " text can be changed to lable the value or removed to same space
//    ", value2 " and ", value 3 " masy also be removed or changed to a lable.
//    Space availiable is 1024 bytes, Note- - each Data line has a ncarrage return and a line feed
//
//example of calling routine:
//       //
//      int value1 = 55;
//      int value2 = 55000;
//      int value3 = 14;
//      add2text(value1, value2, value3);
//      //
//EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE     
//
void add2text(int value1,int value2,int value3){                 //Add value to text file
        if (strlen(user_text_buf0) >= (sizeof(user_text_buf0)-100)){    //Check for full
          Serial.println("text buffer full");                           //yes, say so
          return;                                                       //back to calling
        }
        char temp[11];                  // Maximum number of digits for a 32-bit integer 
        int index = 10;                 //Start from the end of the temperary buffer  
        char str[12];                   //digits + null terminator   
//--------- get time and convert to str for entry into text buffer ----
        DateTime now = rtc.now();                   //get time of entry
        uint32_t value = now.unixtime();            //get unix time from entry
        do {
            temp[index--] = '0' + (value % 10);     // Convert the least significant digit to ASCII
            value /= 10;                            // Move to the next digit
        } while (value != 0);
        strcpy(str, temp + index +1);               // Copy the result to the output string
//---------- end of time conversion uni time is now in str -------------       
        strcat(user_text_buf0, (str));              //write unix time
        //
        // unit time finish entry into this data line
        //
        strcat(user_text_buf0, (" - count= "));            // seperator
        strcat(user_text_buf0, (itoa(value1, ascii, 10)));
        strcat(user_text_buf0, (", value2= "));
        strcat(user_text_buf0, (itoa(value2, ascii, 10)));
        strcat(user_text_buf0, (", value3= "));
        strcat(user_text_buf0, (itoa(value3,  ascii, 10)));
        strcat(user_text_buf0, ("\r\n"));

        //Serial.println(strlen(user_text_buf0));  //for testing
 }
//------end of Function to add to user text buffer ------       
//
//=============================================================================
//
////FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
//  Function to write into a 30K databuffer
//    char databuffer[30000];         // Create a character buffer with a size of 2KB
//    int databufferLength = 0;       // Initialize the buffer length
//  Append data to the large data buffer buffer always enter unit time of data added
//  enter: void dataappend(int counts, int ampli, int SiPM, int Deadtime) (4 values)
//
void dataappend(int counts,int ampli,int SiPM,int Deadtime) {          //entry, add line with values to databuffer
  //----- get and set time to entry -----
  DateTime now = rtc.now();                                               //get time of entry
  String stringValue = String(now.unixtime());                            //convert unix time to string
  const char* charValue = stringValue.c_str();                            //convert to a C string value
  appendToBuffer(charValue);                                              //Sent unix time to databuffer
  //----- add formated string to buffer -----
  String results = " - " + String(counts) + " " + String(ampli) + " " + String(SiPM) + " " + String (Deadtime) + "\r\n";  //format databuffer entry
  const char* charValue1 = results.c_str();                               //convert to a C string value
  appendToBuffer(charValue1);                                             //Send formated string to databuff
  //
  //  Serial.println(databufferLength);                                   //print buffer length for testing only
}
//-----------------------                                               //end dataappend
//----- sub part od dataappend -- append to Buffer -----
//-----------------------
void  appendToBuffer(const char* data) {                                   //enter with charator string to append
  int dataLength = strlen(data);                                          //define the length of data to append
      // ----- Check if there is enough space in the buffer                           //enough space?
  if (databufferLength + dataLength < sizeof(databuffer)) {               //enouth space left in buffer
      // ----- Append the data to the buffer
    strcat(databuffer, data);                                             //yes enough space, add data to end of buffer
    databufferLength += dataLength;                                       //change to length of the buffer
  } else {
    Serial.println("Buffer is full. Data not appended.");                 //Not enough space, say so on terminal
  }       //end not enough space
}         //end appendToBuffer
//

//=================================================================================================================
//
