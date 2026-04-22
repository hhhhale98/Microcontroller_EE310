// Title: Interfacing with sensors
//-----------------------------
//Purpose:The goal of this program is to use the inputs from two photoresistors and check if they match a secret code. It simulates a safebox.
//Every time the sensor PR1 detects darkness (put hand over sensor) it will count a number and display that number
//on the 7-segment display. The input numbers are in increasing order from 0 to maximum 3,
//and starts at zero again. The program waits a little each time a number has been displayed
//If the last input exceeds the little waiting time (around 3 seconds), that means that last input is the number the user has decided
//on and that will be the first digit of the user code. When the program has saved the digit from PR1, the LED will blink
//indicating that photoresistor two (PR2) is ready to read inputs and it does the same procedure as PR1 in order to save the second digit.
//The program adds digit one and digit two and compare the whole number with a SECRET CODE already established in the program.
//When the entered code matches the SECRET CODE, then the motor will turn on for a couple of seconds. If the code doesn't match the
//SECRET CODE then the buzzer will make a short sound. At any moment that the program is running, an emergency button (interrupt) can
//be pressed and reset all variables (start all inputs again), clear 7 display and activate a melody (in ISR) coming from the buzzer.
// Dependencies: configfake.h (Header File). initfake.h (Header File). functionfilefake.h (Header file). This is for a PIC18F47K42. 
// Compiler: MPLABX IDE V6.30/XC8 V3.10.
// Author: Heybee Esquer Cheno 
// OUTPUTS: PORTD.0 Activates Segment A of the 7 Segment Display (common cathode)
//          PORTD.1 Activates Segment B of the 7 Segment Display
//          PORTD.2 Activates Segment C of the 7 Segment Display
//	        PORTD.3 Activates Segment D of the 7 Segment Display
//	        PORTD.4 Activates Segment E of the 7 Segment Display
//	        PORTD.5 Activates Segment F of the 7 Segment Display
//	        PORTD.6 Activates Segment G of the 7 Segment Display
//          PORTA.0 Activates SYS_LED 
//          PORTB.0 Activates Buzzer
//          PORTB.1 Activates relay (NO) that activates motor
// INPUTS: PORTC.2 Photoresistor 1 (PR1)
//         PORTC.3 Photoresistor 2 (PR2)
//         PORTC.4 Emergency button (Interrupt)
// Versions:
//  	V1.0: 4/16/2026 - First version 
//  	V2.0: 4/18/2026 
//  	V3.0: 4/21/2026 
#define _XTAL_FREQ 4000000
#include <xc.h>
#include "configfake.h"
#include "initfake.h"
#include "functionfilefake.h"
void main(void)
{
    TRISB=0b00000000; //Enable port A,B,C,D as outputs or inputs
    TRISC=0b11111111; 
    TRISD=0b00000000; 
    TRISA=0b00000000;      
    ANSELB=0b00000000;
    ANSELC=0b00000000;
    ANSELD=0b00000000;
    ANSELA=0b00000000;     
    LATB=0b00000000;
    LATC=0b11111111;
    LATD=0b00000000;
    LATA=0b00000000; 
    LATAbits.LA0 = 1; //Turn on LED when powered
    IOCCNbits.IOCCN4 = 1; //Enable Interrupt RC4
    PIE0bits.IOCIE = 1;
    INTCON0bits.GIE = 1;
    WPUCbits.WPUC4 = 1;

    while(1)
    {
        if(emergency)  //if emergency pressed reset
        {
            emergency = 0;
            continue;
        }
        oneD = firstNumb();   //store first digit
        if(emergency) continue;
        for(int i=0;i<3;i++) //blinking LED transition
        {
            LATAbits.LA0 = 0;
            __delay_ms(200);
            LATAbits.LA0 = 1;
            __delay_ms(200);
        }
        if(emergency) continue;
        twoD = 16 * secondNumb(); //store second digit
        if(emergency) continue;
        fullN = oneD + twoD;     //get full code
        if(fullN == SECRET_CODE)  //compare real with input
        {
            LATBbits.LB1 = 1; //If code is correct activate the motor for a short moment and then stop
            __delay_ms(2000);
            LATBbits.LB1 = 0; //make sure buzzer still is off
            LATBbits.LB0 = 0;
        }
        else
        {
            LATBbits.LB0 = 1; //Buzzer activates for a short moment and then stop
            __delay_ms(300);
            LATBbits.LB0 = 0;
        }
        oneD = 0; //after finishing one cycle of operation, reset all values
        twoD = 0;
        fullN = 0;
        LATD = 0; //Clear 7 segment-display
    }
}
void __interrupt(irq(IOC), high_priority) IOC_ISR(void) 
{
    if (IOCCFbits.IOCCF4 == 1) 
    { 
        IOCCFbits.IOCCF4 = 0; 
        if (PORTCbits.RC4 == 0)
        {
            oneD =0;
            twoD = 0;
            fullN = 0;
            emergency = 1;
            LATD = 0x00;

            LATB = 0x00;
            LATBbits.LB0 = 1;
            __delay_ms(300);
            LATBbits.LB0 = 0;
            __delay_ms(200);
            LATBbits.LB0 = 1;
            __delay_ms(500);
            LATBbits.LB0 = 0;
            __delay_ms(200);
            LATBbits.LB0 = 1;
            __delay_ms(300);
            LATBbits.LB0 = 0;
            __delay_ms(200);
            LATBbits.LB0 = 1;
            __delay_ms(500);
            LATBbits.LB0 = 0;
            __delay_ms(200);
            LATBbits.LB0 = 1;
            __delay_ms(300);
            LATBbits.LB0 = 0;
            __delay_ms(200);
            LATBbits.LB0 = 1;
            __delay_ms(500);
            LATBbits.LB0 = 0;
        }
    }
}