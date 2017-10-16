/*
 * Milestone 1: Contacting Will Byers
 * Authors: Brandon Salamone and Brad Anderson
 * Date Submitted: 10/16/17
 */
#include <msp430f5529.h>

int BIP = 0;                                  // Number of bytes in packet
int NRB = 0;                                  // Number of Received Bytes
int R,G,B = 0;                                // Stores received UART bytes for RGB LED


void main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer

  //UART Initialization
  P3SEL = BIT3+BIT4;                        // P3.3, P3.4 transmit/receive
  UCA0CTL1 |= UCSWRST;                      // Put state machine in reset
  UCA0CTL1 |= UCSSEL_2;                     // SMCLK
  UCA0BR0 = 6;                              // 1MHz 9600 baud
  UCA0BR1 = 0;                              // 1MHz 9600
  UCA0MCTL = UCBRS_0 + UCBRF_13 + UCOS16;   // Sets m control register
  UCA0CTL1 &= ~UCSWRST;                     // Sets control register
  UCA0IE |= UCRXIE;                         // Enable UART interrupt

  //RGB LED Hardware PWM
  P1DIR |= BIT2+BIT3+BIT4;                  // P1.2 , P1.3, P1.4 output
  P1SEL |= BIT2+BIT3+BIT4;                  // P1.2 and P1.3, P1.4 options select GPIO

  TA0CCR0 = 255;                           // PWM Period about 1khz
  TA0CCTL1 = OUTMOD_7;                      // CCR1 reset/set
  TA0CCTL2 = OUTMOD_7;                      // CCR2 reset/set
  TA0CCTL3 = OUTMOD_7;                      // CCR3 reset/set

  TA0CTL = TASSEL_2 + MC_1 + TACLR;         // SMCLK, up mode, clear TAR

  __bis_SR_register(LPM0_bits + GIE);       // Low power mode

  __no_operation();                         // For debugger
}

//UART Interrupt Service Routine
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
  switch(__even_in_range(UCA0IV,4))
  {
  case 0:break;                             // Interrupt Vector 0 - no interrupt
  case 2:{
      while (!(UCA0IFG&UCTXIFG));           // USCI_A0 TX buffer check
          if(NRB == 0)                      // If number of bytes is zero
          {
             BIP = UCA0RXBUF;               // First byte in UART is number of bytes in the packet
             NRB++;                         // 1 byte is received
          }

          else if ((NRB >0 & NRB <4))              // If number of received bytes is between zero and 4
             {
               switch(NRB)                        // Switch statement for number of bytes received
               {
                  case 1:                         // If 1 byte is received
                  {
                     R = UCA0RXBUF;               // R storage variable gets next byte from UART
                     TA0CCR1 = R;             // CCR1 gets PWM duty cycle for red LED
                     break;
                  }
                   case 2:                        // If 2 bytes are received
                   {
                     G = UCA0RXBUF;               // G storage variable gets next byte from UART
                     TA0CCR2 = G;             // CCR2 gets PWM duty cycle for green LED
                     break;
                   }
                   case 3:                        // If 3 bytes are received
                   {
                     B = UCA0RXBUF;               // B storage variable gets next byte from UART
                     TA0CCR3 = B;             // CCR3 gets PWM duty cycle for blue LED
                     UCA0TXBUF = BIP-3;           // Beginning of new transmit message
                     break;
                   }
                     default: break;
                  }
                     NRB++;                        // Number of bytes received goes up to 4
                }

                else if (NRB > 3  & NRB <= BIP-1)  // If number of bytes is between 4 and rest of byte string
                  {
                     if (NRB != BIP-1)             // If the number of bytes received is not equal to the end
                                                   // of the string
                     {
                        UCA0TXBUF = UCA0RXBUF;     // Tx buffer transmits back the next Rx buffer value
                        NRB++;                     // Increment number of bytes received and loop through
                                                   // until the last byte in the packet
                     }
                     else
                     {
                        UCA0TXBUF = 0x0D;            //End of new message
                        NRB = 0;
                     }
                   }
          break;
  }
  case '4':break;
  default: break;
  }
}
