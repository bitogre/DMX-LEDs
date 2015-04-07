/*  DMX LED Strip Controller
    Copyright (c) 2014 Devin Crumb, Church by the Glades (cbglades.com)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.

  Required Connections
  --------------------
    pin 2:  LED Strip #1    OctoWS2811 drives 8 LED Strips.
    pin 14: LED strip #2    All 8 are the same length.
    pin 7:  LED strip #3
    pin 8:  LED strip #4    A 100 ohm resistor should used
    pin 6:  LED strip #5    between each Teensy pin and the
    pin 20: LED strip #6    wire to the LED strip, to minimize
    pin 21: LED strip #7    high frequency ringining & noise.
    pin 5:  LED strip #8
    pin 15 & 16 - Connect together, but do not use
    pin 4 - Do not use
    pin 3 - Do not use as PWM.  Normal use is ok.

*/

#include <OctoWS2811.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "dmx.h"

#define DMX_ADDR        0
#define LEDS_PER_STRIP  120
#define NUM_STRIPS      16

#if (NUM_STRIPS % 8) != 0
#error Number of Strips must be a multiple of 8.
#endif

const int ledsPerPin = LEDS_PER_STRIP * NUM_STRIPS / 8;
const int bufSize = LEDS_PER_STRIP * NUM_STRIPS * 3;
const int arraySize = bufSize / sizeof(int);

DMAMEM int displayMemory[arraySize];
int drawingMemory[arraySize];

const int config = WS2811_GRB | WS2811_800kHz;
const int ledPin = 13;

Dmx dmx(DMX_ADDR);
OctoWS2811 leds(ledsPerPin, displayMemory, drawingMemory, config);

// Audio library objects
AudioInputAnalog         adc1(A3);       //xy=99,55
AudioAnalyzeFFT1024      fft;            //xy=265,75
AudioConnection          patchCord1(adc1, fft);

void setup() {
  dmx.begin();
  pinMode(ledPin, OUTPUT);

  // the audio library needs to be given memory to start working
  AudioMemory(12);

  Serial.begin(57600);
  leds.begin();
  leds.show();
}

#define RED    0xFF0000
#define GREEN  0x00FF00
#define BLUE   0x0000FF
#define YELLOW 0xFFFF00
#define PINK   0xFF1088
#define ORANGE 0xE05800
#define WHITE  0xFFFFFF

unsigned long lastFft = 0;
unsigned long lastOut = 0;

void loop() 
{
  unsigned long time = millis();
  if (fft.available()) 
  {
    Serial.print("FFT Ready ");
    Serial.println(time - lastFft);
    lastFft = time;
  }
  
  if (dmx.error())
  {
    digitalWrite(ledPin, HIGH);
    Serial.println(" >> ERROR << ");
    dmx.debugDma();
  }
  if (dmx.complete())
  {
    unsigned long delta = time - lastOut;
    digitalWrite(ledPin, HIGH);
    Serial.print("Start: ");
    Serial.println(delta);
    dmx.dumpBuffer();
    leds.show();
    lastOut = time;
    delta = millis() - time;
    Serial.print("end: ");
    Serial.println(delta);
    dmx.debugDma();
  }
  digitalWrite(ledPin, LOW);
}

