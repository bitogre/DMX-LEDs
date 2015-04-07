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
#include "cbg_dmx.h"
#include "chase.h"

#define DMX_ADDR        1

#if (NUM_STRIPS % 8) != 0
#error Number of Strips must be a multiple of 8.
#endif

const int ledsPerPin = LEDS_PER_STRIP * NUM_STRIPS / 8;
const int bufSize    = LEDS_PER_STRIP * NUM_STRIPS * 3;
const int arraySize  = bufSize / sizeof(int);

DMAMEM int displayMemory[arraySize];
int drawingMemory[arraySize];

const int config = WS2811_GRB | WS2811_800kHz;
const int ledPin = 13;

Dmx dmx;
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

unsigned long lastFft = 0;
unsigned long lastOut = 0;

void loop() 
{
  unsigned long startTime = micros();
  if (fft.available()) 
  {
    Serial.print("FFT Ready ");
    Serial.println(startTime - lastFft);
    lastFft = startTime;
  }
  
  if (dmx.error())
  {
    digitalWrite(ledPin, HIGH);
    Serial.println(" >> DMX ERROR << ");
    dmx.debugDma();
  }
  
  if (dmx.complete())
  {
    unsigned long delta = startTime - lastOut;
    digitalWrite(ledPin, HIGH);
    Serial.print("DMX ");
    Serial.println(delta);
    process((DmxData *)&dmx.rxBuffer[1 + DMX_ADDR]);
    unsigned long d2 = micros() - startTime;
    Serial.print("D2 ");
    Serial.println(d2);
    leds.show();
    digitalWrite(ledPin, LOW);
    //dmx.dumpBuffer();
    lastOut = startTime;
    unsigned long procTime = micros() - startTime;
    Serial.print("D3 ");
    Serial.println(procTime);
    //Serial.print("%CPU: ");
    //Serial.println((procTime * 100) / delta);
  }
  digitalWrite(ledPin, LOW);
}

void process(DmxData *data)
{
  static FXProc fx[2];
  
  FixedPoint master = data->master;
  master /= 255;
  FixedPoint mix[2] = { FixedPoint((int16_t)data->mix), FixedPoint((int16_t)1) };
  mix[0] /= 255;
  mix[1] -= mix[0];
  
  for(int i = 0; i < 2; ++i)
  {
    mix[i] *= master;
    if (isChaseTri(data->fx[i].mode))
      chaseTri(data->fx[i], mix[i], fx[i]);
    else if (isChaseSquare(data->fx[i].mode))
      chaseSquare(data->fx[i], mix[i], fx[i]);
    else if (isChaseRainbow(data->fx[i].mode))
      chaseRainbow(data->fx[i], mix[i], fx[i]);
    else
    {
      // Default FX is Blackout
      for (int x = 0; x < 256; ++x)
        fx[i].color[x] = ColorFP(0,0,0);
      for (int x = 0; x < NUM_STRIPS; ++x)
        fx[i].startPos[x] = 0;
      fx[i].revPerLeds = LEDS_PER_STRIP;
      fx[i].revStrip = false;
      fx[i].inc = 0;
    }
  }
  
  for(int x = 0; x < NUM_STRIPS; ++x)
  {
    fx[0].pos = fx[0].startPos[x];
    fx[1].pos = fx[1].startPos[x];
    fx[0].revAt = fx[0].revPerLeds;
    fx[1].revAt = fx[1].revPerLeds;
    for(int y = 0; y < LEDS_PER_STRIP; ++y)
    {
      ColorFP out = fx[0].color[uint8_t(fx[0].pos)] + fx[1].color[uint8_t(fx[1].pos)];
      leds.setPixel(strip(x,y), offset(x,y), out.red, out.green, out.blue);
      fx[0].pos += fx[0].inc;
      fx[1].pos += fx[1].inc;
      if (y >= fx[0].revAt)
      {
        fx[0].inc *= -1;
        fx[0].revAt += fx[0].revPerLeds;
      }
      if (y >= fx[1].revAt)
      {
        fx[1].inc *= -1;
        fx[1].revAt += fx[1].revPerLeds;
      }
    }
    if (fx[0].revStrip)
      fx[0].inc *= -1;
    if (fx[1].revStrip)
      fx[1].inc *= -1;
  }
}

inline int strip(int x, int y)
{
  return x & 0x07;
}

inline int offset(int x, int y)
{
  return y + (x >> 8) * LEDS_PER_STRIP;
}

