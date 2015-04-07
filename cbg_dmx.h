/*  CBG DMX Device layout
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

#ifndef _CBG_DMX_H_
#define _CBG_DMX_H_

#include "fixedPoint.h"

#define NUM_STRIPS          16
#define LEDS_PER_STRIP      120

#define MODE_STRIP_REV      0x08
#define MODE_CEMETRIC       0x10
#define MODE_BOUNCE         0x20
#define MODE_CHASE_MASK     0xC0
#define MODE_CHASE_TRI      0x40
#define MODE_CHASE_SQUARE   0x80
#define MODE_CHASE_RAINBOW  0xC0

inline bool isChaseTri(uint8_t mode)     { return ((mode & MODE_CHASE_MASK) == MODE_CHASE_TRI); }
inline bool isChaseSquare(uint8_t mode)  { return ((mode & MODE_CHASE_MASK) == MODE_CHASE_SQUARE); }
inline bool isChaseRainbow(uint8_t mode) { return ((mode & MODE_CHASE_MASK) == MODE_CHASE_RAINBOW); }

typedef struct __attribute__((packed))
{
  uint8_t  red;
  uint8_t  green;
  uint8_t  blue;
} DMX_Color;

typedef struct __attribute__((packed))
{
  uint8_t   mode;
  uint8_t   pos;
  uint8_t   off;
  uint8_t   period;
  uint8_t   dutyCycle;
  DMX_Color pri;
  DMX_Color sec;
} FX;

typedef struct __attribute__((packed))
{
  uint8_t master;
  uint8_t mix;
  FX      fx[2];
} DmxData;

class ColorFP
{
public:
  ColorFP() : red((int16_t)0), green((int16_t)0), blue((int16_t)0) {}
  ColorFP(const DMX_Color &c) : red((int16_t)c.red), green((int16_t)c.green), blue((int16_t)c.blue) {}
  ColorFP(int16_t r, int16_t g, int16_t b) : red(r), green(g), blue(b) {}
  
  int color() { return OctoWS2811::color(uint8_t(red), uint8_t(green), uint8_t(blue)); }
  
  ColorFP & operator += (const ColorFP &c) { red += c.red; green += c.green; blue += c.blue; return *this; }
  ColorFP & operator -= (const ColorFP &c) { red -= c.red; green -= c.green; blue -= c.blue; return *this; }

  template<typename T>
  ColorFP & operator += (T val) { red += val; green += val; blue += val; return *this; }
  template<typename T>
  ColorFP & operator -= (T val) { red -= val; green -= val; blue -= val; return *this; }
  template<typename T>
  ColorFP & operator *= (T val) { red *= val; green *= val; blue *= val; return *this; }
  template<typename T>
  ColorFP & operator /= (T val) { red /= val; green /= val; blue /= val; return *this; }

  FixedPoint red;
  FixedPoint green;
  FixedPoint blue;
};

template<typename T1, typename T2>
T1 operator + (const T1 &l, const T2 &r) { T1 temp(l); temp += r; return temp; }
template<typename T1, typename T2>
T1 operator - (const T1 &l, const T2 &r) { T1 temp(l); temp -= r; return temp; }
template<typename T1, typename T2>
T1 operator * (const T1 &l, const T2 &r) { T1 temp(l); temp *= r; return temp; }
template<typename T1, typename T2>
T1 operator / (const T1 &l, const T2 &r) { T1 temp(l); temp /= r; return temp; }

#endif /* _CBG_DMX_H_ */
