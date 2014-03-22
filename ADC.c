/*
 * File:   ADC.c
 * Author: Jordan Ross
 *
 * Created on February 12, 2014, 8:17 AM
 */

void InitADC()
{
    AD1PCFG = 0xFFEF;
    AD1CON1 = 0x00E0;
    AD1CHS = 0;
    AD1CSSL = 0;
    AD1CON3 = 0x0F00; // Sample time = 15Tad, Tad = Tcy
    AD1CON2 = 0x003C; // Set AD1IF every 16 samples.
    AD1CON1bits.ADON = 1;
}

int ReadADC(int ch)
{
    int ADCValue, count;
    int* ADC16Ptr;

    AD1CHS = ch;
    ADCValue = 0;
    ADC16Ptr = &ADC1BUF0;
    IFS0bits.AD1IF = 0;
    AD1CON1bits.ASAM = 1;

    while(!IFS0bits.AD1IF){};
    AD1CON1bits.ASAM = 0;
    for (count = 0; count < 16; count++)
    {
        ADCValue = ADCValue + *ADC16Ptr++;
    }
    ADCValue = ADCValue >> 4;
    return ADCValue;
}