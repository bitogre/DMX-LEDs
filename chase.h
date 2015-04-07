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
  uint8_t startPos;
  uint8_t stripOff;
  FixedPoint pos;
  FixedPoint inc;
  ColorFP color[256];
};

void chaseTri(FX &setting, FixedPoint &level, FXProc &out)  
{
  ColorFP current(setting.pri);
  ColorFP delta(setting.sec);
  delta = (delta - current) * level;
  current *= level;
  ColorFP loopDelta(delta);
  loopDelta /= setting.dutyCycle + 1;
  
  out.startPos = setting.pos;
  out.stripOff = setting.off;
  
  out.inc = 256;
  out.inc /= 2 + setting.period;
  
  int i = 0;
  while(i < setting.dutyCycle)
  {
    out.color[i++] = current;
    current += loopDelta;
  }

  loopDelta = delta / (setting.dutyCycle - 256);
  while(i <= 255)
  {
    out.color[i++] = current;
    current += loopDelta;
  }
}

void chaseSquare(FX &setting, FixedPoint &level, FXProc &out)  
{
  ColorFP priColor(setting.pri);
  ColorFP secColor(setting.sec);
  priColor *= level;
  secColor *= level;
  
  out.startPos = setting.pos;
  out.stripOff = setting.off;
  
  out.inc = 256;
  out.inc /= 2 + setting.period;
  
  int i = 0;
  while(i < setting.dutyCycle)
    out.color[i++] = priColor;

  while(i <= 255)
    out.color[i++] = secColor;
}

#endif /* _CHASE_H_ */
