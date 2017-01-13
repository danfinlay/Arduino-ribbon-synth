/*
  Analog Input
 Demonstrates analog input by reading an analog sensor on analog pin 0 and
 turning on and off a light emitting diode(LED)  connected to digital pin 13.
 The amount of time the LED will be on and off depends on
 the value obtained by analogRead().

 The circuit:
 * Potentiometer attached to analog input 0
 * center pin of the potentiometer to the analog pin
 * one side pin (either one) to ground
 * the other side pin to +5V
 * LED anode (long leg) attached to digital output 13
 * LED cathode (short leg) attached to ground

 * Note: because most Arduinos have a built-in LED attached
 to pin 13 on the board, the LED is optional.


 Created by David Cuartielles
 modified 30 Aug 2011
 By Tom Igoe

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/AnalogInput

 */

#include <EEPROM.h>
#include "./PitchToNote.h"

// Input pins:
const int TOP_STRING = A0;    // select the input pin for the potentiometer
const int TOP_STR_X = A1;
const int TOP_STR_Y = A2;
const int TOP_STR_CLICK = 2;

// Hard coded calibration
const int TOP_STRING_LOW = 1021;
const int TOP_STRING_HIGH = 782;
const int TOP_STRING_NOTE = E4;
const int fretCount = 20;
const int lowNote = E4;

// Low E of a guitar:
float notes[] = {
  329.628,
  349.228,
  369.994,
  391.995,
  415.305,
  440,
  466.164,
  493.883,
  523.251,
  554.365,
  587.33,
  622.254,
  659.255,
  698.456,
  739.989,
  783.991,
  830.609,
  880,
  932.328,
  987.767
};

// Cached sensor values:
int topStringValue = 0;
int topStringXValue = 0;
int topStringYValue = 0;
int topStringClick = 0;

int stickZeroX = 0;
int stickZeroY = 0;

// Global vars
int maxBasic = 0;
int topStringNote = E4;

// Output Pins:
int ledPin = 13;      // select the pin for the LED

void setup() {
  // declare the ledPin as an OUTPUT:
  pinMode(ledPin, OUTPUT);
  
  pinMode(TOP_STRING, INPUT);
  pinMode(TOP_STR_X, INPUT);
  pinMode(TOP_STR_Y, INPUT);
  pinMode(TOP_STR_CLICK, INPUT);

  // calibrate:
  stickZeroX = analogRead(TOP_STR_X);
  stickZeroY = analogRead(TOP_STR_Y);
//  Serial.println("Centered at:");
//  Serial.println(stickZeroX);
//  Serial.println(stickZeroY);

  Serial.begin(9600);

  playMidi (0x90, 0, 127);
}

void loop() {
  
  topStringValue = analogRead(TOP_STRING);
//  Serial.println("Top string:");
//  Serial.println(topStringValue);

  // turn the ledPin on
  digitalWrite(ledPin, HIGH);
  // stop the program for <TOP_STRING> milliseconds:
  delay(topStringValue);
  // turn the ledPin off:
  digitalWrite(ledPin, LOW);
  // stop the program for for <TOP_STRING> milliseconds:
  delay(1000);
}

//note-off message
void noteOff(int cmd, int pitch, int velocity) {
  Serial.write(byte(cmd));
  Serial.write(byte(pitch));
  Serial.write(byte(0));
}

//  plays a MIDI note.  Doesn't check to see that
//  cmd is greater than 123, or that data values are  less than 123:
void playMidi(int cmd, int pitch, int velocity) {
  Serial.write(cmd);
  Serial.write(pitch);
  Serial.write(velocity);
}

void pressureChange (int noteNumber, int value) {
  playMidi(0xA4, noteNumber, value);
}

void pitchWheelChange(int value) {
   unsigned int change = 0x2000 + value;  //  0x2000 == No Change
   unsigned char low = change & 0x7F;  // Low 7 bits
   unsigned char high = (change >> 7) & 0x7F;  // High 7 bits
   playMidi(0xE0, low, high);
}

const int noteCodes[] = {
  E4, F4, G4b, G4, A4, B4b, B4, C5, D5b, E5, F5, G5b, G5, A5b, A5, B5b, B5, C6, D6b, D6, E6b, E6, F6
};

int playNoteFor (float frequency) {
  int newHighest = 0;
  
  for (int i = 0; i < 20; i++) {
    if (frequency < notes[i]) {
      newHighest = 0;
      break;
    }

    if (notes[i] > frequency) {
      newHighest = i;
      break;
    }
    
    newHighest = i;
  }

  if (topStringNote != newHighest) {
    // end old note
    noteOff(0x80 + 0, noteCodes[newHighest], 0);
    // begin new note
    playMidi(0x90 + 0, noteCodes[newHighest], 127);
    topStringNote = newHighest;
  }

  int bendValue = getBendFor(notes[newHighest], notes[newHighest + 1], frequency);
  pitchWheelChange(bendValue);

  int volume = getVolume();
  pressureChange(noteCodes[newHighest], volume);
}

// Returns bend as int up to 127, always bent up above 
int getBendFor (int base, int next, float target) {
  return 0;
}

int getVolume () {
  
  topStringXValue = analogRead(TOP_STR_X);
//  Serial.println("Top string X:");
//  Serial.println(topStringXValue - stickZeroX);
  int xOffset = topStringXValue - stickZeroX;

  topStringYValue = analogRead(TOP_STR_Y);
//  Serial.println("Top string Y:");
//  Serial.println(topStringYValue - stickZeroY);
  int yOffset = topStringYValue - stickZeroY;

  int basicVolume = abs(xOffset) + abs(yOffset);
  int normalizedVolume = int(float(basicVolume) / float(maxBasic) * 123);
//  Serial.println("Normalized is " + String(normalizedVolume) + " from " + String(basicVolume) + "/" + String(maxBasic));
//  Serial.println("Volume: ");
//  Serial.println(normalizedVolume);

  if (basicVolume > maxBasic) {
    maxBasic = basicVolume;
  }

  return normalizedVolume;
}
