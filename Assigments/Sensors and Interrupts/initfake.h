//Author: Heybee Esquer Cheno 
//This is "initfake.h" and it is a header that contains the initializations of my C program.
// Versions:
//  	V1.0: 4/16/2026 - First version 
//  	V2.0: 4/18/2026 
//  	V3.0: 4/21/2026 
#ifndef INITFAKE_H
#define INITFAKE_H
#include <stdint.h>
volatile uint8_t emergency = 0; //Trigger button

volatile uint8_t oneD = 0;  //Input value operation (adding)
volatile uint8_t twoD = 0;  //Input value operation (adding)
volatile uint8_t fullN = 0; //Input code by user

const uint8_t dispZero = 0x3F;//Hex to decimal in 7 segment display from 0-3
const uint8_t dispOne  = 0x06;
const uint8_t dispTwo  = 0x5B;
const uint8_t dispThree= 0x4F;

#define SECRET_CODE 0x23 //Actual secret code

#endif /* INITFAKE_H */




