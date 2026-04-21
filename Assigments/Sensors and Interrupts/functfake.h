#ifndef FUNCTIONFILEFAKE_H
#define	FUNCTIONFILEFAKE_H
#include <xc.h>
   void reset(void)
{
    oneD = 0x00;
    twoD = 0x00;
    fullN = 0x00;
    LATD = 0x00;
}

void contfirst(int digit, int number)
{
    while(1)
    {
        while(PORTCbits.RC2 == 0)
        {
            LATD = digit;
            check = 1;
        }
        if(check == 1) 
        {
            check = 0;
            break;
        }
    }
    int i = 0;
    __delay_ms(500); 
    while (i < 15)
    {
        i++;
        __delay_ms(200);
        if(PORTCbits.RC2 == 0)
        {
            break;
        }
    }
    if(i == 15)
    {
        check = 1;
        oneD = number;
        return;
    }     
}
void firstNumb(void)
{
    while(1)
    {
        contfirst(0x3F, 0); //count and display 0 on 7-segment
        if(check == 1) //if waited for 0, register input
        {
            check = 0;
            return;
        }
        contfirst(0x06, 1); //count and display 1 on 7-segment
        if(check == 1) //register 1 as input
        {
            check = 0;
            return;
        }    
        contfirst(0x5B, 2); //count and display 2 on 7-segment
        if(check == 1) //register 2
        {
            check = 0;
            return;
        }  
        contfirst(0x4F, 3); //count and display 3 on 7-segment
        if(check == 1) //register 3
        {
            check = 0;
            return;
        } 
    }
}
void contsec(int digit, int number)
{
    while(1)
    {
        while(PORTCbits.RC3 == 0)
        {
            LATD = digit;
            check = 1;
        }
        if(check == 1)
        {
            check = 0;
            break;
        }
    }

    int i = 0;
    __delay_ms(500); 

    while (i < 15)
    {
        i++;
        __delay_ms(200);

        if(PORTCbits.RC3 == 0)
        {
            break;
        }
    }

    if(i == 15)
    {
        check = 1;
        twoD = 16 * number;
        return;
    }     
}

void secondNumb(void) //same logic as input1, but for 2nd digit
{
    while(1)
    {
        contsec(0x3F, 0);
        if(check == 1)
        {
            check = 0;
            return;
        }
        contsec(0x06, 1);
        if(check == 1)
        {
            check = 0;
            return;
        }    
        contsec(0x5B, 2);
        if(check == 1)
        {
            check = 0;
            return;
        }  
        contsec(0x4F, 3);
        if(check == 1)
        {
            check = 0;
            return;
        } 
    }
}

void __interrupt(irq(IOC), high_priority) IOC_ISR(void) 
{
    if (IOCCFbits.IOCCF4 == 1) 
    { 
        IOCCFbits.IOCCF4 = 0; 
        if (PORTCbits.RC4 == 0)
        {
            oneD = 0;
            twoD = 0;
            fullN = 0;
            emergency = 1;

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

            LATD = 0x00;
        }
    }
}

#endif	/* FUNCTFAKE_H */