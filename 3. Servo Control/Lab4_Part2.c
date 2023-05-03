#include <msp430.h>

void pinSetup();
void ButtonSetup();
void TimerB3Setup();

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop WDT
    PM5CTL0 &= ~LOCKLPM5;  // Disable the GPIO power-on default high-impedance mode to activate previously configured port settings


    pinSetup();                                  // servo motor
    ButtonSetup();                               
    TimerB3Setup();                              
    __bis_SR_register(LPM0_bits + GIE);          
}

void pinSetup(){
    P6OUT &= ~BIT0;     // LOW
    P6DIR |= BIT0;      //  output
    P6SEL0 |= BIT0;     //  peripheral function 
    P6SEL1 &= ~BIT0;    //  peripheral function 
}

void ButtonSetup (){
    // Configure Button on P2.3 
             P2OUT |= BIT3;                          
             P2REN |= BIT3;                          
             P2IES &= ~BIT3;                      
             P2IE |= BIT3;                          
    // Configure Button on P4.1 as input with pullup resistor
             P4OUT |= BIT1;                         
             P4REN |= BIT1;                      
             P4IES &= ~BIT1;                     
             P4IE |= BIT1;                        
}

void TimerB3Setup(){
    // Configure Timer_B3
    TB3CTL = TBSSEL_2 | MC__UP | TBCLR | TBIE;    
    TB3CCTL0 = OUTMOD_7;                        
    TB3CCTL1 |= CCIE;                          
    TB3CCR0 = 20000;                             // 50Hz frequency
    TB3CCR1 = 1500;                              // Initial duty cycle of 1.5 ms
}

// Port 2 ISR
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    P2IFG &= ~BIT3;                         // Clear P2.3 interrupt flag

    if(TB3CCR1 > 1000)
        TB3CCR1 -= 100;                     // Decrease the duty cycle by 5% to move the servo left
    else
        TB3CCR1 = TB3CCR1;                  // If the servo is already at its minimum position, do not move it further

}                                          // End of Port_2 ISR

// Port 4 interrupt service routine
#pragma vector=PORT4_VECTOR
__interrupt void Port_4(void)
{
    P4IFG &= ~BIT1;                         // Clear P4.1 interrupt flag

    if(TB3CCR1 < 2000)
        TB3CCR1 += 100;                     // Increase the duty cycle by 5% to move the servo right
    else
        TB3CCR1 = TB3CCR1;                  

}                                          

// TimerB3 interrupt service routine
#pragma vector = TIMER3_B1_VECTOR
__interrupt void Timer3_B1_ISR(void)
{
    switch(__even_in_range(TB3IV,TB3IV_TBIFG))
    {
        case  TB3IV_TBCCR1:                 // CCR1 interrupt
            P6OUT &= ~BIT0;                 // Set P6.0 to low to end the PWM cycle
            break;
        default:
            break;
    }

}                                          // End of TimerB3 ISR
