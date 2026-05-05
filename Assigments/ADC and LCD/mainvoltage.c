// Title: ADC and Interfacing with an LCD
//-----------------------------
//Purpose:The goal of this code is to use an accelerometer to read the x-position (based on voltage values) and be able to display on an 
//LCD the state of the circuit which can be: flat, tilt left, tilt right and flat. The reading of this state is obtained by
//the ADC calculation in the code. An IOC interrupt is also implemented in the code and when the button is pressed, the system
//enters to a halt mode for 10 seconds, after that the system is ready to read the positions again. During the halt mode an LED is
//also blinking for 10 seconds.
// Dependencies: This is for a PIC18F47K42 and the configuration words are in this c.code.
// Compiler: MPLABX IDE V6.30/XC8 V3.10.
// Author: Heybee Esquer Cheno 
// OUTPUTS: PORTB.0 Connected to D0 in LCD
//          PORTB.1 Connected to D1 in LCD
//          PORTB.2 Connected to D2 in LCD
//	        PORTB.3 Connected to D3 in LCD
//	        PORTB.4 Connected to D4 in LCD
//	        PORTB.5 Connected to D5 in LCD
//	        PORTB.6 Connected to D6 in LCD
//          PORTB.7 Connected to D7 in LCD
//          PORTB.0 Activates SYS_LED 
//          PORTD.0 Connected to RS
//          PORTD.1 Connected to EN
//          PORTC.3 LED
// INPUTS: PORTC.2 Interrupt
//         PORTA.1 Accelerometer
// Versions:
//  	V1.0: 5/3/2026 - First version 
//  	V2.0: 5/4/2026 
//  	V3.0: 5/5/2026 


// CONFIG1L
#pragma config FEXTOSC = LP     // External Oscillator Selection (LP (crystal oscillator) optimized for 32.768 kHz; PFM set to low power)
#pragma config RSTOSC = EXTOSC  // Reset Oscillator Selection (EXTOSC operating per FEXTOSC bits (device manufacturing default))

// CONFIG1H
#pragma config CLKOUTEN = OFF   // Clock out Enable bit (CLKOUT function is disabled)
#pragma config PR1WAY = ON      // PRLOCKED One-Way Set Enable bit (PRLOCK bit can be cleared and set only once)
#pragma config CSWEN = ON       // Clock Switch Enable bit (Writing to NOSC and NDIV is allowed)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor enabled)

// CONFIG2L
#pragma config MCLRE = EXTMCLR  // MCLR Enable bit (If LVP = 0, MCLR pin is MCLR; If LVP = 1, RE3 pin function is MCLR )
#pragma config PWRTS = PWRT_OFF // Power-up timer selection bits (PWRT is disabled)
#pragma config MVECEN = OFF      // Multi-vector enable bit (Multi-vector enabled, Vector table used for interrupts)
#pragma config IVT1WAY = ON     // IVTLOCK bit One-way set enable bit (IVTLOCK bit can be cleared and set only once)
#pragma config LPBOREN = OFF    // Low Power BOR Enable bit (ULPBOR disabled)
#pragma config BOREN = SBORDIS  // Brown-out Reset Enable bits (Brown-out Reset enabled , SBOREN bit is ignored)

// CONFIG2H
#pragma config BORV = VBOR_2P45 // Brown-out Reset Voltage Selection bits (Brown-out Reset Voltage (VBOR) set to 2.45V)
#pragma config ZCD = OFF        // ZCD Disable bit (ZCD disabled. ZCD can be enabled by setting the ZCDSEN bit of ZCDCON)
#pragma config PPS1WAY = ON     // PPSLOCK bit One-Way Set Enable bit (PPSLOCK bit can be cleared and set only once; PPS registers remain locked after one clear/set cycle)
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config DEBUG = OFF      // Debugger Enable bit (Background debugger disabled)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Extended Instruction Set and Indexed Addressing Mode disabled)

// CONFIG3L
#pragma config WDTCPS = WDTCPS_31// WDT Period selection bits (Divider ratio 1:65536; software control of WDTPS)
#pragma config WDTE = OFF       // WDT operating mode (WDT Disabled; SWDTEN is ignored)

// CONFIG3H
#pragma config WDTCWS = WDTCWS_7// WDT Window Select bits (window always open (100%); software control; keyed access not required)
#pragma config WDTCCS = SC      // WDT input clock selector (Software Control)

// CONFIG4L
#pragma config BBSIZE = BBSIZE_512// Boot Block Size selection bits (Boot Block size is 512 words)
#pragma config BBEN = OFF       // Boot Block enable bit (Boot block disabled)
#pragma config SAFEN = OFF      // Storage Area Flash enable bit (SAF disabled)
#pragma config WRTAPP = OFF     // Application Block write protection bit (Application Block not write protected)

// CONFIG4H
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block not write-protected)
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)
#pragma config WRTSAF = OFF     // SAF Write protection bit (SAF not Write Protected)
#pragma config LVP = ON         // Low Voltage Programming Enable bit (Low voltage programming enabled. MCLR/VPP pin function is MCLR. MCLRE configuration bit is ignored)

// CONFIG5L
#pragma config CP = OFF         // PFM and Data EEPROM Code Protection bit (PFM and Data EEPROM code protection disabled)

#include <xc.h> // must have this
#include <stdio.h>
#include <string.h>
#include <math.h>

#define _XTAL_FREQ 4000000                 // Fosc  frequency for _delay()  library
#define FCY    _XTAL_FREQ/4
#define Vref 5.0                  //reference voltage
#define RS LATD0                  // PORTD 0 pin is used for Register Select
#define EN LATD1                  // PORTD 1 pin is used for Enable
#define ldata LATB                // PORTB is used for transmitting data to LCD
#define LCD_Port TRISB
#define LCD_Control TRISD
#define LED LATCbits.LATC3        // LED connected to RC3

int digital;                      // holds the digital value 
float voltage;                    // hold the analog value (volt))
float recentV = 0;               // holds recent voltage 
float acceleration;              // holds acceleration value
volatile unsigned char halt = 0;   // Interrupt stored
void LCD_Init(void); 
void ADC_Init(void); 
void LCD_Command(char);                      
void LCD_Char(char);                         
void LCD_String(const char *);              
void LCD_String_xy(char, char, const char *); 
void MSdelay(unsigned int);                  
void IOC_Init(void);                         //Interrupt function
char row2[17];                   // row 2 LCD
char state[17];                   // holds state

void __interrupt() ISR(void) {
    if(IOCCFbits.IOCCF2)    //Check RC2
    {
        halt = 1;                             // HALT STATE
        IOCCFbits.IOCCF2 = 0;                // clears RC2
        PIR0bits.IOCIF = 0;                  // clears IOC
    }
}
void IOC_Init(void)
{
    TRISCbits.TRISC2 = 1;
    ANSELCbits.ANSELC2 = 0;
    IOCCPbits.IOCCP2 = 1;
    IOCCNbits.IOCCN2 = 0;
    IOCCFbits.IOCCF2 = 0;
    PIR0bits.IOCIF = 0;
    PIE0bits.IOCIE = 1;
    INTCON0bits.GIE = 1;
}

void ADC_Init(void)
{    //Setup ADC
    ADCON0bits.FM = 1;                       //right justify
    ADCON0bits.CS = 1;                       //ADCRC Clock
    TRISAbits.TRISA1 = 1;                    //Set RA1 to input
    ANSELAbits.ANSELA1 = 1;                  //Set RA1 to analog
    ADPCH = 0x01;                            //Set RA1 as Analog channel in ADC ADPCH
    ADCLK = 0x00;                            //set ADC CLOCK Selection register to zero
    ADRESH = 0;                              // Clear ADC Result registers
    ADRESL = 0;                              
    ADPREL = 0;                              // set precharge select to 0 in register ADPERL & ADPERH
    ADPREH = 0;                             
    ADACQL = 0;                              //set acquisition low and high byte to zero 
    ADACQH = 0;                              
    ADREF = 0x00;                            //ADC reference
    ADCON0bits.ON = 1;                       //Turn ADC On 
}

void LCD_Init(void)
{
    MSdelay(15);                             // LCD power-on delay
    LCD_Port = 0x00;                         // Set PORTB as output PORT for LCD data(D0-D7) pins
    LCD_Control = 0x00;                      // Set PORTD as output PORT LCD Control(RS,EN) Pins
    LCD_Command(0x01);                       // clear display screen
    LCD_Command(0x38);                       // uses 2 line and initialize 5*7 matrix of LCD
    LCD_Command(0x0C);                       // display on cursor off 
    LCD_Command(0x06);                       // increment cursor (shift cursor to right)
}

void LCD_Command(char cmd)
{
    ldata = cmd;                             // Send data to PORT as a command for LCD
    RS = 0;                                  // Command Register is selected
    EN = 1;                                  // High-to-Low pulse on Enable pin to latch data
    NOP();                                   
    EN = 0;                                  
    MSdelay(3);                              
}

void LCD_Char(char dat)
{
    ldata = dat;                             // Send data to LCD
    RS = 1;                                  // Data Register is selected
    EN = 1;                                  // High-to-Low pulse on Enable pin to latch data
    NOP();                                   
    EN = 0;                                  
    MSdelay(1);                              
}

void LCD_String(const char *msg)
{
    while((*msg)!=0)                           
    {
        LCD_Char(*msg);
         msg++;  
    }
}

void LCD_String_xy(char row, char pos, const char *msg)
{
    char location=0;
    if(row<=1)
    {
        location=(0x80) | ((pos) & 0x0f); /*Print message on 1st row and desired location*/
        LCD_Command(location);
    }
    else
    {
        location=(0xC0) | ((pos) & 0x0f); /*Print message on 2nd row and desired location*/
        LCD_Command(location);    
    }  
    LCD_String(msg);
}

void MSdelay(unsigned int val)
{
    unsigned int i,j;
        for(i=0;i<val;i++)
            for(j=0;j<165;j++);   
}

void main(void)
{
    TRISB=0b00000000;                        // Enable PORTB as output
    TRISD=0b00000000;                        // Enable PORTD as output
    ANSELA=0b00000000;                       //PORTA digital
    ANSELB=0b00000000;                       //PORTB digital
    ANSELC=0b00000000;                       //PORTC digital
    ANSELD=0b00000000;                       //PORTD digital
    LATB=0b00000000;                         //clear
    LATC=0b00000000;                         //clear
    LATD=0b00000000;                         //clear
    TRISCbits.TRISC2 = 1;                    // Enable RC2 as input
    TRISCbits.TRISC3 = 0;                    // Enable RC3 as output

    LCD_Init();                              
    ADC_Init();                              
    IOC_Init();                              

    while(1)                                
    {
        if(halt == 1)                   // If interrupt pressed...
        {
            halt = 0;                   // Clears it
            PIE0bits.IOCIE = 0;              // Disables IOC
            LCD_Command(0x01);               //clear display screen
            LCD_String_xy(1, 0, "HALT STATE"); //Display string at location(row,location)
            LCD_String_xy(2, 0, "WAIT FOR 10 SEC"); //Display string at location(row,location)
            
            //LED blink for 10 seconds//
            LED = 1;                         // LED ON
            __delay_ms(500);                 // for 500 ms
            LED = 0;                         // LED OFF
            __delay_ms(500);                 // for 500 ms
            LED = 1;                         // LED ON
            __delay_ms(500);                 // for 500 ms
            LED = 0;                         // LED OFF
            __delay_ms(500);                 // for 500 ms
            LED = 1;                         // LED ON
            __delay_ms(500);                 // for 500 ms
            LED = 0;                         // LED OFF
            __delay_ms(500);                 // for 500 ms
            LED = 1;                         // LED ON
            __delay_ms(500);                 // for 500 ms
            LED = 0;                         // LED OFF
            __delay_ms(500);                 // for 500 ms
            LED = 1;                         // LED ON
            __delay_ms(500);                 // for 500 ms
            LED = 0;                         // LED OFF
            __delay_ms(500);                 // for 500 ms
            LED = 1;                         // LED ON
            __delay_ms(500);                 // for 500 ms
            LED = 0;                         // LED OFF
            __delay_ms(500);                 // for 500 ms
            LED = 1;                         // LED ON
            __delay_ms(500);                 // for 500 ms
            LED = 0;                         // LED OFF
            __delay_ms(500);                 // for 500 ms
            LED = 1;                         // LED ON
            __delay_ms(500);                 // for 500 ms
            LED = 0;                         // LED OFF
            __delay_ms(500);                 // for 500 ms
            LED = 1;                         // LED ON
            __delay_ms(500);                 // for 500 ms
            LED = 0;                         // LED OFF
            __delay_ms(500);                 // for 500 ms
            LED = 1;                         // LED ON
            __delay_ms(500);                 // for 500 ms
            LED = 0;                         // LED OFF
            __delay_ms(500);                 // for 500 ms
            LED = 0;                     
           __delay_ms(50);
           while(PORTCbits.RC2 == 1)
           {
           __delay_ms(10);
           }
            IOCCFbits.IOCCF2 = 0;            // clears RC2 
            PIR0bits.IOCIF = 0;              // clears IOC 
            PIE0bits.IOCIE = 1;              // enables IOC 
            LCD_Command(0x01);               // clears LCD
        }

        ADCON0bits.GO = 1;                   // Start conversion
        while(ADCON0bits.GO);                //Wait for conversion done
        digital = ((int)ADRESH << 8) | ADRESL; // Combine 8 bit ADRESL and ADRESH (until 12 bits total)
        voltage= digital*((float)Vref/(float)(4096)); //getting voltage value
        acceleration = (voltage - 1.65) / 0.33;   // Converting to acceleration

        if((voltage - recentV > 0.15) || (recentV - voltage > 0.15)) // is there a change in voltage?
        {
            strcpy(state, "Shake");          // shake
        }
        else if(voltage < 1.60)              // below flat? 
        {
            strcpy(state, "Tilt Left");      // Tilt Left
        }
        else if(voltage > 1.70)              // above flat?
        {
            strcpy(state, "Tilt right");     // Tilt right
        }
        else                                
        {
            strcpy(state, "Flat");           // Flat
        }
        recentV = voltage;              // stores the recent value in voltage
       
        sprintf(row2, "%.3f XDirection", acceleration); // display "value" X direction
        LCD_String_xy(1, 0, "State:");       // display on row 1 "state:"
        LCD_String_xy(1, 7, state);          // display on row 1 (state) after the first word
        LCD_String_xy(2, 0, row2);          // display on row 2 (value)
        __delay_ms(300);                     // delay 
    }
}

