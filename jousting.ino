//********************************************************************************
// * File Name          : linefollow
// * Author             : RadioShack Corporation
// * Version            : V1.0
// * Date               : 2014/01/16
// * Description        : Optical sensors send feedback to PCB 
// *                      to make the robot follow a black line on a white background.
// ********************************************************************************
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
// ********************************************************************************

/* 
 Adding code to turn the Make:it Robotics Starter Kit into a jousting knight. 
 Works best with two kits.
 Adapted code from the public domain Debounce example sketch http://www.arduino.cc/en/Tutorial/Debounce
 and the toneMelody example sketch http://arduino.cc/en/Tutorial/Tone
  
 Final Fantasy Battle Victory tune borrowed from http://courses.ischool.berkeley.edu/i262/s13/content/jenton/lab6-chiptune-musicbox
 Taps tune developed by Arduino Forum member 'encryptor' http://forum.arduino.cc/index.php/topic,143712.0.html
    
 Each time the input pin (lance or shield) goes from LOW to HIGH (e.g. because of a push-button
 press), the melody is played to signal victory or defeat.  There's
 a minimum delay between toggles to debounce the circuit (i.e. to ignore
 noise).  
 
 The circuit:
 * pushbutton for the lance is attached from pin 4 to +5V
 * pushbutton for the shield is attached from pin 5 to +5V
 * 10K ohm resistor attached from pin 4 to ground
 * 10K ohm resistor attached from pin 5 to ground
 * 100 ohm resistor attached from pin 6 to 8 ohm speaker+
 * speaker- to ground
 
 jousting code created 06 October 2014
 by Andrew Terranova
 */

#include <MakeItRobotics.h>//include library
#include "pitches.h"
MakeItRobotics line_following;//declare object

// **************************************************************************
// *                            Define Constants
// **************************************************************************
const int lancePin = 4;     // the number of the lance/pushbutton pin
const int shieldPin = 5;     // the number of the shield/pushbutton pin
const int speakerPin = 6;  // the number of the speaker pin

// **************************************************************************
// *                            Define Variables
// **************************************************************************
int lanceState;             // the current reading from the input lancePin
int lastLanceState = LOW;   // the previous reading from the input lancePin
int shieldState;             // the current reading from the input shieldPin
int lastShieldState = LOW;   // the previous reading from the input shieldPin
int playspeed = 50;          // how fast are we going to go?

// the following variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long lastLanceDebounceTime = 0;  // the last time the output pin was toggled
long lanceDebounceDelay = 50;    // the debounce time; increase if the lance hits are not reliable
long lastShieldDebounceTime = 0;  // the last time the output pin was toggled
long shieldDebounceDelay = 50;    // the debounce time; increase if the shield hits are not reliable

// **************************************************************************
// *                            Power Up Initial
// **************************************************************************
void setup() 
{ 
  Serial.begin(10420);                     //tell the Arduino to communicate with Make: it PCB
  delay(500);                              //delay 500ms
  line_following.line_following_setup();   //initialize the status of line following robot
  line_following.all_stop();               //all motors stop
  pinMode(lancePin, INPUT);                //set lancePin as digital input
  pinMode(shieldPin, INPUT);                //set lancePin as digital input
 }
// **************************************************************************
// *                            Main Loop 
// **************************************************************************
void loop() 
{ 
  // Jousting
  // read the state of the lancePin and shieldPin into local variables:
  int lanceReading = digitalRead(lancePin);
  int shieldReading = digitalRead(shieldPin);

  // check to see if you just pressed the lance button 
  // (i.e. the input went from LOW to HIGH), and you've waited 
  // long enough since the last press to ignore any noise:  

  // If the switch changed, due to noise or pressing:
  if (lanceReading != lastLanceState) {
    // reset the debouncing timer
    lastLanceDebounceTime = millis();
  } 
  
  if ((millis() - lastLanceDebounceTime) > lanceDebounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
    lanceState = lanceReading;
    // set the action for the state of the lance button:
    if(lanceState == HIGH) {
      win();
    }
  }

  // save the reading.  Next time through the loop, it'll be the lastLanceState:
  lastLanceState = lanceReading;
  

  // check to see if you just pressed the shield button 
  // (i.e. the input went from LOW to HIGH), and you've waited 
  // long enough since the last press to ignore any noise:  

  // If the switch changed, due to noise or pressing:
  if (shieldReading != lastShieldState) {
    // reset the debouncing timer
    lastShieldDebounceTime = millis();
  } 
  
  if ((millis() - lastShieldDebounceTime) > shieldDebounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
    shieldState = shieldReading;
  // set the action for the state of the shield button:
  if(shieldState == HIGH) {
    lose();
    }
  }
  
  // save the reading.  Next time through the loop, it'll be the lastShieldState:
  lastShieldState = shieldReading;


  // Line Following 
  static int sensor_in;                  //variable to store the value of read_optical function feedback 
  static int sensorValue1 = 0;           //variable to store optical1 status
  static int sensorValue2 = 0;           //variable to store optical2 status
  static int sensorCnt = 0;              //variable to count for trigger which optical
  static long sensorTimer1 = millis();   //last triggered time
  static long sensorTimer2 = millis();   //now time
  static int action1 = 0;                //now action
  static int action2 = 0;                //last action
  //************************************************************************
  //  Trigger Left/Right optical every 15 milliseconds
  //************************************************************************ 
  sensorTimer2 = millis();                 //read now time
  if (sensorTimer2 - sensorTimer1 > 15)    //if now time minus last triggered time is greater than 15ms, then trigger another optical
  {
    sensorTimer1 = sensorTimer2;           //last triggered time = now time
    /***********************************************************************
       -> trigger optical1 -> greater than 15ms -> trigger optical2 -> greater than 15ms ->|
       |-----------------------------------------------------------------------------------|
    ***********************************************************************/
    if (sensorCnt == 0)
      line_following.trigger_optical1();
    else if (sensorCnt == 1)
      line_following.trigger_optical2();
    sensorCnt++;
    if (sensorCnt == 2)
      sensorCnt = 0;
  }            
  //***********************************************************************
  //  Read Left/Right optical status
  //***********************************************************************
  sensor_in=line_following.read_optical();  
  /************************************************************************
    read_optical()
    Description
      Reads the value from optical1(Right side) or optical2(Left side)
    Syntax
      read_optical()
    Parameters
      none
    Returns
      0x000  optical1 black
      0x0ff  optical1 white
      0x100  optical1 white
      0x1ff  optical1 black
      0x2XX  not ready; don't use this value      
  *************************************************************************/
  if((sensor_in & 0xf00)==0)
    sensorValue1=sensor_in & 0xff;
  else if((sensor_in & 0xf00)>>8==1)  
    sensorValue2=sensor_in & 0xff;
  if (sensorValue1 == 0x00)
    action1 =action1 & 0xfe;
  if (sensorValue1 == 0xFF)
    action1 = action1 | 0x01;
  if (sensorValue2 == 0x00)
    action1 = action1 | 0x02;
  if (sensorValue2 == 0xFF)
    action1 = action1 & 0xfd;  
  /************************************************************************
    action1
             left        right
    0x00    black        black
    0x01    black        white
    0x02    white        black
    0x03    white        white
  *************************************************************************/  
  /************************************************************************
     Make Robot Move
     if action1 is not equal to action2, then change motor status
     if action1 is equal to action2, then do nothing
  *************************************************************************/
  if(action1 != action2)
  {
    if (action1 == 3 )
      line_following.go_forward(playspeed);
    if (action1 == 1)
//      line_following.line_following_turn_left(playspeed);
      line_following.line_following_turn_right(playspeed);    // reversing normal behavior for jousting
    if (action1 == 2)
//      line_following.line_following_turn_right(playspeed);
      line_following.line_following_turn_left(playspeed);  // reversing normal behavior for jousting
    if (action1 == 0)
      line_following.go_forward(playspeed);  
  }
  action2=action1;  
}

void playVictory() {

// Tune borrowed from http://courses.ischool.berkeley.edu/i262/s13/content/jenton/lab6-chiptune-musicbox
//Final Fantasy Battle Victory
int victory[] ={NOTE_G4, NOTE_G4, NOTE_G4, NOTE_G4, NOTE_DS4, NOTE_F4, NOTE_G4, NOTE_F4, NOTE_G4}; 
  
int noteDurations[] = {8,8,8,4,4,4,8,8,2}; 

  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 6; thisNote++) {  // just play the first 6 notes

    // to calculate the note duration, take one second 
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000/noteDurations[thisNote];
    
    tone(speakerPin, victory[thisNote],noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(speakerPin);
  }
}

void playTaps() {

// Taps - Melody developed by Arduino Forum member 'encryptor' http://forum.arduino.cc/index.php/topic,143712.0.html
int taps[] = {NOTE_G4, NOTE_G4, NOTE_C5, NOTE_G4, NOTE_C5, NOTE_E5, NOTE_G4, NOTE_C5, NOTE_E5, NOTE_G4, 
              NOTE_C5, NOTE_E5, NOTE_G4, NOTE_C5, NOTE_E5, NOTE_C5, NOTE_E5, NOTE_G5, NOTE_E5, NOTE_C5, 
              NOTE_G4, NOTE_G4, NOTE_G4, NOTE_C5};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {2,4,1, 2,4,1, 4,4,2, 4,4,2, 4,4,1, 2,4,1, 2,2,1, 2,4,1};
  

  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 6; thisNote++) {  // just play the first 6 notes

    // to calculate the note duration, take one second 
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000/noteDurations[thisNote];
    
    tone(speakerPin, taps[thisNote],noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(speakerPin);
  }
}

void win() {
      line_following.all_stop();
      delay(200);
      line_following.go_backward(128);
      delay(1000);
      line_following.all_stop();
      delay(200);
      playVictory(); 
      line_following.turn_right(128);
      delay(2200);
      line_following.all_stop(); 
      Serial.end();      // Stop communication to the MakeItRobotics Control Board - reset to go again
}

void lose() {
      line_following.all_stop();
      delay(2000);
      line_following.go_backward(64);
      delay(2000);
      line_following.all_stop();
      delay(200);
      playTaps(); 
      line_following.turn_right(128);
      delay(1200);
      line_following.all_stop(); 
      Serial.end();      // Stop communication to the MakeItRobotics Control Board - reset to go again
    }
