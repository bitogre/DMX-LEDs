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

#include "dmx.h"

const int debugPin = 22;

#define DMX_BAUD 250000
#define DMX_DIV  (((F_CPU * 2) + (DMX_BAUD >> 1)) / DMX_BAUD)

#define IRQ_PRIORITY            16

#define C2_DISABLE		UART_C2_TE | UART_C2_RIE | UART_C2_ILIE
#define C2_ENABLE		UART_C2_TE | UART_C2_RE | UART_C2_RIE | UART_C2_ILIE
#define C2_TX_ACTIVE		C2_ENABLE | UART_C2_TIE
#define C2_TX_COMPLETING	C2_ENABLE | UART_C2_TCIE
#define C2_TX_INACTIVE		C2_ENABLE
#define C3_2STOP_BITS           0x40
#define C3_OEIE                 0x08
#define C3_NEIE                 0x04
#define C3_FEIE                 0x02
#define C3_PEIE                 0x01
#define C5_TDMAS                0x80
#define C5_RDMAS                0x20
#define UART_S2_LBKDE           0x02
#define UART_S2_BRK13           0x04
#define UART_S2_LBKDIF          0x80
#define UART_S2_W1TOCLR         0xC0

uint16_t Dmx::_dumped = 0;

DMAChannel Dmx::dma;
uint8_t Dmx::rxBuffer[Dmx::channels];
DMAMEM uint8_t Dmx::rxDmaBuffer[Dmx::channels];

void Dmx::begin(void)
{
  pinMode(debugPin, OUTPUT);

  SIM_SCGC4 |= SIM_SCGC4_UART0;	// turn on clock
  CORE_PIN0_CONFIG = PORT_PCR_PE | PORT_PCR_PS | PORT_PCR_PFE | PORT_PCR_MUX(3);
  CORE_PIN1_CONFIG = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(3);
  UART0_BDH = (DMX_DIV >> 13) & 0x1F;
  UART0_BDL = (DMX_DIV >>  5) & 0xFF;
  UART0_C1 = UART_C1_ILT | UART_C1_M;
  UART0_C2 = C2_TX_INACTIVE;
  UART0_C3 = C3_2STOP_BITS | C3_OEIE | C3_NEIE | C3_FEIE | C3_PEIE;
  UART0_C4 = DMX_DIV & 0x1F;
  UART0_C5 = C5_RDMAS;
  UART0_S2 = 0; //UART_S2_LBKDE | UART_S2_BRK13;
  UART0_TWFIFO = 2; // tx watermark, causes S1_TDRE to set
  UART0_RWFIFO = 1; // rx watermark, causes S1_RDRF to set
  UART0_PFIFO = UART_PFIFO_TXFE | UART_PFIFO_RXFE;
  attachInterruptVector(IRQ_UART0_STATUS, uartStatusIsr);
  attachInterruptVector(IRQ_UART0_ERROR, uartStatusIsr);
  NVIC_SET_PRIORITY(IRQ_UART0_STATUS, IRQ_PRIORITY);
  NVIC_SET_PRIORITY(IRQ_UART0_ERROR, IRQ_PRIORITY);
  NVIC_ENABLE_IRQ(IRQ_UART0_STATUS);
  NVIC_ENABLE_IRQ(IRQ_UART0_ERROR);
  

  dma.begin();
  // DMA copies data from RX FIFO to buffer
  dma.TCD->SADDR = &UART0_D;
  dma.TCD->SOFF = 0;
  dma.TCD->ATTR = DMA_TCD_ATTR_SSIZE(0) | DMA_TCD_ATTR_DSIZE(0);
  dma.TCD->NBYTES_MLNO = 1;
  dma.TCD->SLAST = 0;
  dma.TCD->DADDR = rxDmaBuffer;
  dma.TCD->DOFF = 1;
  dma.TCD->CITER_ELINKNO = channels;
  dma.TCD->DLASTSGA = -channels;
  dma.TCD->CSR = 0;
  dma.TCD->BITER_ELINKNO = channels;
  
  // Connect the DMA to UART0
  dma.triggerAtHardwareEvent(DMAMUX_SOURCE_UART0_RX);

  // enable a done interrupts when complete
  dma.attachInterrupt(dmaIsr);
  dma.interruptAtCompletion();
  dma.disableOnCompletion();
}

void Dmx::dmaIsr(void)
{
  dma.clearInterrupt();
  digitalWrite(debugPin, LOW);
  __enable_irq();
  memcpy(rxBuffer, rxDmaBuffer, channels);
}

void Dmx::uartStatusIsr(void)
{
  uint8_t avail, c;

  uint8_t s1 = UART0_S1;
  if (s1 & UART_S1_FE)
  {
    c = UART0_D;
    dma.TCD->DADDR = rxDmaBuffer;
    dma.TCD->CITER_ELINKNO = channels;
    dma.TCD->BITER_ELINKNO = channels;
    dma.enable();
    digitalWrite(debugPin, HIGH);
  }

  if (s1 & UART_S1_IDLE) 
  {
    Serial.println(" __IDLE__");
    dma.disable();
  }

  if (s1 & (UART_S1_RDRF | UART_S1_IDLE)) 
  {
    avail = UART0_RCFIFO;
    if (avail == 0) 
    {
      // The only way to clear the IDLE interrupt flag is
      // to read the data register.  But reading with no
      // data causes a FIFO underrun, which causes the
      // FIFO to return corrupted data.  If anyone from
      // Freescale reads this, what a poor design!  There
      // write should be a write-1-to-clear for IDLE.
      c = UART0_D;
      // flushing the fifo recovers from the underrun,
      // but there's a possible race condition where a
      // new character could be received between reading
      // RCFIFO == 0 and flushing the FIFO.  To minimize
      // the chance, interrupts are disabled so a higher
      // priority interrupt (hopefully) doesn't delay.
      // TODO: change this to disabling the IDLE interrupt
      // which won't be simple, since we already manage
      // which transmit interrupts are enabled.
      UART0_CFIFO = UART_CFIFO_RXFLUSH;
    } 
    else 
    {
      _dumped += avail;
      do {
        c = UART0_D;
      } while (--avail > 0);
    }
  }
  
  c = UART0_S2;
  c &= UART_S2_W1TOCLR;
  UART0_S2 = c;
  
  c = UART0_C2;
  if ((c & UART_C2_TIE) && (UART0_S1 & UART_S1_TDRE)) {
    if (UART0_S1 & UART_S1_TDRE) UART0_C2 = C2_TX_COMPLETING;
  }
  if ((c & UART_C2_TCIE) && (UART0_S1 & UART_S1_TC)) {
    UART0_C2 = C2_TX_INACTIVE;
  }
}

