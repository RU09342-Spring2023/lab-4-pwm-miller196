#define PERIOD 1000
#define RED_TO_GREEN 0
#define GREEN_TO_BLUE 1
#define BLUE_TO_RED 2

#include <msp430.h>


char LEDstate = RED_TO_GREEN;

void LEDSetup();
void TimerSetup();

void main()
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

    LEDSetup();     
    TimerSetup();   

    PM5CTL0 &= ~LOCKLPM5;       // Unlock LPM5

    __bis_SR_register(LPM0_bits | GIE); // Enter LPM0 with interrupts enabled
  //  __no_operation();           // For debugger
}


void LEDSetup(){
    P6DIR |= BIT0 | BIT1 | BIT2; 

    P6OUT &= ~(BIT0 | BIT1 | BIT2); 

    P6SEL0 |= BIT0 | BIT1 | BIT2; 
    P6SEL1 &= ~(BIT0 | BIT1 | BIT2);    
    P6IE |= BIT0 | BIT1 | BIT2;  
}


void TimerSetup(){
    TB3CCR0 = PERIOD - 1;          // Set PWM period
    TB3CTL = TBSSEL__SMCLK | MC__UP | TBCLR; // Configure timer: SMCLK, up mode, clear TBR

    TB3CCTL1 = OUTMOD_7;           // RED
    TB3CCTL2 = OUTMOD_7;           // GREEN 
    TB3CCTL3 = OUTMOD_7;           // BLUE

    TB3CCR1 = PERIOD - 1;         
   
    TB3CCR2 = 0;   
    TB3CCR3 = 0;    

    TB1CCTL0 |= CCIE;              
    TB1CCR0 = 1;                  
    TB1CTL = TBSSEL_1 | MC_2 | ID_3 | TBCLR | TBIE; 
}


#pragma vector = TIMER1_B0_VECTOR
__interrupt void Timer1_B0_ISR(void)
{
    switch(LEDstate){
    case RG:
        TB3CCR1--;                // Decrease RED to 0%
        TB3CCR2++;                // Increase GREEN to 50%
        if (TB3CCR1 == 0) LEDstate = GREEN_TO_BLUE;
        break;
    case GB:
        TB3CCR2--;                // Decrease GREEN to 0%
        TB3CCR3++;                // Increase BLUE to 50%
        if (TB3CCR2 == 0) LEDstate = BLUE_TO_RED;
        break;
    case BR:
        TB3CCR3--;                // Decrease BLUE to 0%
        TB3CCR1++;                // Increase RED to 50%
        if (TB3CCR3 == 0) LEDstate = RED_TO_GREEN;
        break;
    }

    
    if(TB3R >= 60000)
        TB3R = 1;                           // If so, reset TB3R to 1 to avoid overflow (max value is 65535)

     // Increment TB1CCR0 by 20 to adjust the timer compare value
  
    TB1CCR0 += 20;
}
