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

#ifndef _TEENSY_DMA_DMX_H_
#define _TEENSY_DMA_DMX_H_

#include <Arduino.h>
#include "DMAChannel.h"

class Dmx
{
protected:
  uint16_t _startChannel;

public:
  static const uint16_t channels = 512;

  Dmx(uint16_t startChannel) : _startChannel(startChannel) {}
  void begin(void);
  
  static void debugDma()
  {
    Serial.print("DMA");
    Serial.print(dma.channel);
    Serial.print(": ");
    uint8_t *p = (uint8_t *)dma.TCD;
    for (int i = 0; i < sizeof(DMAChannel::TCD_t); ++i)
    {
      Serial.print(p[i], HEX);
      Serial.print(' ');
    }
  }
  
  void dumpBuffer()
  {
    Serial.println(dumped());
    for (int i = 0; i < channels; ++i)
    {
      Serial.print(rxBuffer[i], HEX);
      Serial.print(' ');
      if (((i + 1) & 0x3F) == 0)
        Serial.println();
    }
  }
  
  static uint16_t _dumped;
  uint16_t dumped()
  {
    __disable_irq();
    uint16_t ret = _dumped;
    _dumped = 0;
    __enable_irq();
    return ret;
  }
  
  bool complete()
  {
    if (!dma.complete())
      return false;

    dma.clearComplete();
    return true;
  }

  bool error()
  {
    if (!dma.error())
      return false;

    dma.clearError();
    return true;
  }

  static uint8_t rxBuffer[channels];
private:
  static DMAChannel dma;
  static DMASetting nextDma;
  static DMAMEM uint8_t rxDmaBuffer[channels];
  
  static void dmaIsr(void);
  static void uartStatusIsr(void);
};

#endif /* _TEENSY_DMX_H_ */
