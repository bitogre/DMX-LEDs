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

template <int ledsPerStrip, int numStrips>
class Chase : public IFXProc
{
private:
  const int minPeriod = 2;
  const int periodThreshold = ledsPerStrip - minPeriod;
  Chase();
  
public:
  Chase(FX &setting) : period(setting.period + minPeriod), pos((uint8_t)1), off(setting.off), virtLeds((uint8_t)ledsPerStrip)  
  {
    ColorFP priColor(setting.pri);
    ColorFP secColor(setting.sec);
    aColor = priColor - secColor;
    bColor = secColor;
    
    virtLeds /= (setting.mode & 0x0F) + 1;
    off /= 255;
    pos /= 1<<16;
    pos *= setting.pos;
    
    if (setting.period > periodThreshold)
      period += (setting.period - periodThreshold) * int(virtLeds * (numStrips - 1)) / (255 - periodThreshold);
  }
  
  virtual ColorFP color(int x, int y)
  {
    
    return aColor;
  }
  
private:
  ColorFP aColor;
  ColorFP bColor;
  uint16_t period;
  FixedPoint<24> pos;
  FixedPoint<24> off;
  FixedPoint<24> rise;
  FixedPoint<24> virtLeds;
};

#endif /* _CHASE_H_ */
