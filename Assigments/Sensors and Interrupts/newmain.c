#define _XTAL_FREQ 4000000
#include <xc.h>
#include "configfake.h"
#include "initfake.h"
#include "functfake.h"

void main(void) 
{
    TRISB = 0x00;
    TRISC = 0xFF; 
    TRISD = 0x00; 
    TRISA = 0x00;      
    ANSELB = 0x00;
    ANSELC = 0x00;
    ANSELD = 0x00;
    ANSELA = 0x00;     
    LATB = 0x00;
    PORTC = 0xFF;
    LATD = 0x00;
    LATA = 0x00;       // <-- added
    IOCCNbits.IOCCN4 = 1;    // trigger on falling edge
    IOCCPbits.IOCCP4 = 0;    //disable rising edge
    PIE0bits.IOCIE = 1;      // Enable IOC interrupts
    INTCON0bits.GIE = 1;     //enable global interrupt
    WPUCbits.WPUC4 = 1;     //pull up for interrupt
    LATAbits.LA0 = 1;
    
    while(1)
    {
        emergency = 0;
        firstNumb();
        if (emergency) continue;
        for(int i=0;i<3;i++) 
        {
            LATAbits.LA0 = 0;
            __delay_ms(200);
            LATAbits.LA0 = 1;
            __delay_ms(200);
        }

        secondNumb();
        if (emergency) continue;
        fullN = twoD + oneD;
        if(fullN == SECRET_CODE)
        {
            LATBbits.LB1 = 1;
            __delay_ms(3000);
            break;
        }
        else
        {
            LATBbits.LB0 = 1;
            __delay_ms(100);
            LATBbits.LB0 = 0;
        }
        reset();
    }
    return;
}