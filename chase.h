/*  DMX Slave
 *  Copyright (c) 2014 Devin Crumb, Church by the Glades (cbglades.com)
 *   
 * Based on serial1.c in the Teensyduino Core Library converted to use DMA
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2013 PJRC.COM, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * 1. The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * 2. If the Software is incorporated into a build system that allows 
 * selection among a list of target devices, then similar target
 * devices manufactured by PJRC.COM must be included in the list of
 * target devices and selectable in the same manner.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _CHASE_H_
#define _CHASE_H_

#include <OctoWS2811.h>
#include "cbg_dmx.h"

class FXProc
{
public:
  uint8_t startPos[NUM_STRIPS];
  uint16_t pos;
  FixedPoint inc;
  FixedPoint currPos;
  ColorFP color[256];
  uint16_t revPerLeds;
  uint16_t revAt;
  bool revStrip;
};

inline uint8_t chaseStartPos(int x, FX &setting, FXProc &out)
{
  uint16_t pos = (out.pos >> 2) + x * setting.off;
  if ((setting.mode & MODE_BOUNCE) && (pos & 0x0100))
    pos = 0x00ff - (pos & 0x00ff);
  return pos;
}

void chase(FX &setting, FXProc &out)
{
  if (setting.pos & 0x80)
    out.pos += (uint16_t)((int8_t)setting.pos - (int8_t)0xC0);
  else
    out.pos = setting.pos << 3;

  if (setting.mode & MODE_CEMETRIC)
  {
    for (int x = 0; x < (NUM_STRIPS/2); ++x)
    {
      out.startPos[(NUM_STRIPS/2) + x] = out.startPos[(NUM_STRIPS/2) - 1 - x] = chaseStartPos(x, setting, out);
    }
  }
  else
  {
    for (int x = 0; x < NUM_STRIPS; ++x)
    {
      out.startPos[x] = chaseStartPos(x, setting, out);
    }
  }

  out.inc = 256;
  out.inc /= 2 + setting.period;
  out.revPerLeds = LEDS_PER_STRIP / ((setting.mode & 0x07) + 1);
  out.revAt = out.revPerLeds;
  
  if (setting.mode & MODE_STRIP_REV)
    out.revStrip = (setting.mode & 0x01) ? false : true;
  else
    out.revStrip = (setting.mode & 0x01) ? true : false;
}

void chaseTri(FX &setting, FixedPoint &level, FXProc &out)  
{
  chase(setting, out);
  
  ColorFP current(setting.pri);
  ColorFP delta(setting.sec);
  delta = (delta - current) * level;
  current *= level;
  ColorFP loopDelta(delta);
  loopDelta /= setting.dutyCycle + 1;
  
  int i = 0;
  while(i <= setting.dutyCycle)
  {
    out.color[i++] = current;
    current += loopDelta;
  }

  loopDelta = delta / (256 - setting.dutyCycle);
  while(i <= 255)
  {
    out.color[i++] = current;
    current -= loopDelta;
  }
}

void chaseSquare(FX &setting, FixedPoint &level, FXProc &out)  
{
  chase(setting, out);
  
  ColorFP priColor(setting.pri);
  ColorFP secColor(setting.sec);
  priColor *= level;
  secColor *= level;
  
  int i = 0;
  while(i < setting.dutyCycle)
    out.color[i++] = priColor;

  while(i <= 255)
    out.color[i++] = secColor;
}

void chaseRainbow(FX &setting, FixedPoint &lvl, FXProc &out)  
{
  chase(setting, out);
  
  ColorFP current;
  FixedPoint level(lvl);
  FixedPoint delta(level);
  level *= 255;
  delta *= 6;
  
  int i = 0;
  current.red   = level;
  current.green = 0;
  current.blue  = 0;
  while(i < (1*256/6))
  {
    out.color[i++] = current;
    current.green += delta;
  }

  current.red   = level;
  current.green = level;
  current.blue  = 0;
  while(i < (2*256/6))
  {
    out.color[i++] = current;
    current.red -= delta;
  }

  current.red   = 0;
  current.green = level;
  current.blue  = 0;
  while(i < (3*256/6))
  {
    out.color[i++] = current;
    current.blue += delta;
  }

  current.red   = 0;
  current.green = level;
  current.blue  = level;
  while(i < (4*256/6))
  {
    out.color[i++] = current;
    current.green -= delta;
  }

  current.red   = 0;
  current.green = 0;
  current.blue  = level;
  while(i < (5*256/6))
  {
    out.color[i++] = current;
    current.red += delta;
  }

  current.red   = level;
  current.green = 0;
  current.blue  = level;
  while(i < (6*256/6))
  {
    out.color[i++] = current;
    current.blue -= delta;
  }
}

#endif /* _CHASE_H_ */
