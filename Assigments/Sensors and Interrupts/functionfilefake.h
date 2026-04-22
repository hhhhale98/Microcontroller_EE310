//Author: Heybee Esquer Cheno 
//This is "functionfilefake.h" and it is a header that contains the functions that I use in my C program.
// Versions:
//  	V1.0: 4/16/2026 - First version 
//  	V2.0: 4/18/2026 
//  	V3.0: 4/21/2026 
#ifndef FUNCTIONFILEFAKE_H
#define FUNCTIONFILEFAKE_H

#include <xc.h>
#include "initfake.h"

static void debounce(void)  //function debounce
{
    __delay_ms(50);
}

uint8_t firstNumb(void)  //function first digit
{                        //clear variables to use
    uint8_t value = 0;   //value to display
    uint16_t timeout = 0;  //timeout is for waiting between digits
    uint8_t ready = 0;   //are we starting?
    LATD = 0x00;   // starting with 7 segment off
    while(1)
    {
        if(emergency) return 0; //If emergency pressed

        if(!ready)    //If we are starting with sensors
        {
            if(PORTCbits.RC2 == 0)  //first photoresistor
            {
                debounce();
                if(PORTCbits.RC2 == 0)  //If shaded...
                {
                    ready = 1;     //start lecture
                    value = 0;     //first lecture is 0
                    LATD = dispZero;   //Display zero 
                    while(PORTCbits.RC2 == 0);
                }
            }
            continue;
        } //below values 0,1,2,3 for each input lecture in PR1
        LATD = (value == 0) ? dispZero : //first lecture, show zero
               (value == 1) ? dispOne  : //second lecture, show one
               (value == 2) ? dispTwo  : dispThree;
        __delay_ms(120);   //wait time between digits
        if(PORTCbits.RC2 == 0)
        {
            debounce();
            if(PORTCbits.RC2 == 0)
            {
                timeout = 0;
                value++;  //be incrementing by one each time put hand on sensor
                if(value > 3) value = 0; //stop at 3 and start again
                while(PORTCbits.RC2 == 0);
            }
        }
        else
        {
            timeout++;
            if(timeout > 20) //If user takes to long to put hand in PR1
            {                //save it as first digit
                while(PORTCbits.RC2 == 0);
                return value;   //last value will be firstNumb
            }
        }
    }
}

uint8_t secondNumb(void) //function second digit
{
    uint8_t value = 0;  //clear all variables to use
    uint16_t timeout = 0;
    uint8_t ready = 0;
    LATD = 0x00;
    while(1)
    {
        if(emergency) return 0;
        if(!ready) //If we are starting with sensors
        {
            if(PORTCbits.RC3 == 0) //second photoresistor
            {
                debounce();
                if(PORTCbits.RC3 == 0) //If shaded...
                {
                    ready = 1;  //start lecture
                    value = 0;  //first lecture is 0
                    LATD = dispZero;  //display zero
                    while(PORTCbits.RC3 == 0);
                }
            }
            continue;
        }//below values 0,1,2,3 for each input lecture in PR2
        LATD = (value == 0) ? dispZero :
               (value == 1) ? dispOne  :
               (value == 2) ? dispTwo  : dispThree;
        __delay_ms(120); //wait time between digits
        if(PORTCbits.RC3 == 0)
        {
            debounce();
            if(PORTCbits.RC3 == 0)
            {
                timeout = 0;
                value++; //be incrementing by one each time put hand on sensor
                if(value > 3) value = 0; //stop at 3 and start zero
                while(PORTCbits.RC3 == 0);
            }
        }
        else
        {
            timeout++;
            if(timeout > 20) //If user takes to long to put hand in PR2
                             //save it as second digit
            {
                while(PORTCbits.RC3 == 0);
                return value; //last value will be secondNumb
            }
        }
    }
}

#endif