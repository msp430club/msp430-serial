//******************************************************************************
//   MSP430G2xx3 Demo - USCI_A0, 9600 UART Echo ISR, DCO SMCLK
//
//   Description: Echo a received character, RX ISR used. Normal mode is LPM0.
//   USCI_A0 RX interrupt triggers TX Echo.
//   Baud rate divider with 1MHz = 1MHz/9600 = ~104.2
//   ACLK = n/a, MCLK = SMCLK = CALxxx_1MHZ = 1MHz
//
//                MSP430G2xx3
//             -----------------
//         /|\|              XIN|-
//          | |                 |
//          --|RST          XOUT|-
//            |                 |
//            |     P1.2/UCA0TXD|------------>
//            |                 | 9600 - 8N1
//            |     P1.1/UCA0RXD|<------------
//
//   D. Dang
//   Texas Instruments Inc.
//   February 2011
//   Built with CCS Version 4.2.0 and IAR Embedded Workbench Version: 5.10
//******************************************************************************
//#include  "msp430g2553.h"

const char message[] = { "This message is being sent over serial!\r\n" };
const char endl[] = { "\r\n" };

char rxBuffer[600];
char *pRX;

char const *pTX, *pTX_end;
unsigned int mode_echo = 1;

#include "signal.h"

void main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
  DCOCTL = CALDCO_1MHZ;
  P1SEL = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
  P1SEL2 = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
  UCA0CTL1 |= UCSSEL_2;                     // SMCLK
  UCA0BR0 = 104;                            // 1MHz 9600
  UCA0BR1 = 0;                              // 1MHz 9600
  UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  
  rxBuffer[0] = 10;
  rxBuffer[1] = 13;
  pRX = &rxBuffer[2];

  IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt

  __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0, interrupts enabled
}

interrupt (USCIAB0TX_VECTOR) USCI0TX_ISR(void)
{
  if ( mode_echo == 0 )
  {
    UCA0TXBUF = *pTX++;                 // TX next character

    if ( pTX > pTX_end )              // TX over?
    {
      mode_echo = 1;
      IE2 &= ~UCA0TXIE;                       // Disable USCI_A0 TX interrupt
    }
  }
  else
  {
    IE2 &= ~UCA0TXIE;
  }
}

interrupt (USCIAB0RX_VECTOR) USCI0RX_ISR(void)
{
  char rx;
  rx = UCA0RXBUF;
  if ( 0 && UCA0RXBUF == 'a')                     // 'a' received?
  {
    mode_echo = 0;
    pTX = &message[0];
    IE2 |= UCA0TXIE;                        // Enable USCI_A0 TX interrupt
    UCA0TXBUF = *pTX++;
  }
  else if ( rx == 13 )
  {
    *pRX++ = 13;
    *pRX = 10;

    pTX_end = pRX;
    pRX = &rxBuffer[2];
    pTX = rxBuffer;

    mode_echo = 0;
  }
  else
  {
    *pRX++ = rx;
    UCA0TXBUF = UCA0RXBUF;
  }
    IE2 |= UCA0TXIE;
}