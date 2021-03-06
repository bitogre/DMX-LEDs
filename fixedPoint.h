/*  Fixed Point Code
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

#ifndef _FIXED_POINT_H_
#define _FIXED_POINT_H_

#define Q 22

class FixedPoint
{
public:
  FixedPoint() : val(0) {}
  FixedPoint(int16_t x) : val(x << Q) {}
  explicit FixedPoint(float x) : val(x * (1 << Q)) {}

  operator float() { return (float)val / (float)(1 << Q); }
  explicit operator uint8_t() { return (val >> Q) & 0x00FF; }
  int32_t raw() { return val; }
  
  FixedPoint & operator += (int8_t x) { val += ((int32_t)x) << Q; return *this; }
  FixedPoint & operator -= (int8_t x) { val -= ((int32_t)x) << Q; return *this; }

  template<typename T>
  FixedPoint & operator *= (T x) { val *= x; return *this; }

  template<typename T>
  FixedPoint & operator /= (T x) { val /= x; return *this; }

  template<typename T>
  FixedPoint & operator >>= (T x) { val >>= x; return *this; }

  FixedPoint & operator += (const FixedPoint &x)  { val += x.val; return *this; }
  FixedPoint & operator -= (const FixedPoint &x)  { val -= x.val; return *this; }
  
  FixedPoint & operator *= (const FixedPoint &x) 
  {
    val >>= Q / 2;
    val *= x.val >> (Q / 2);
    return *this;
  }

protected:
  int32_t val;
};

#endif /* _FIXED_POINT_H_ */
