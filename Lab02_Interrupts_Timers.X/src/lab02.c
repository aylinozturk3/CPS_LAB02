#include "lab02.h"

#include <xc.h>
#include <p33Fxxxx.h>
//do not change the order of the following 2 definitions
#define FCY 12800000UL
#include <libpic30.h>

#include "types.h"
#include "lcd.h"
#include "led.h"

#define FCY_EXT 32768

volatile unsigned long ms_value=0;
volatile unsigned long seconds=0; 
void initialize_timer()
{
    // Enable RTC Oscillator -> this effectively does OSCCONbits.LPOSCEN = 1
    // but the OSCCON register is lock protected. That means you would have to 
    // write a specific sequence of numbers to the register OSCCONL. After that 
    // the write access to OSCCONL will be enabled for one instruction cycle.
    // The function __builtin_write_OSCCONL(val) does the unlocking sequence and
    // afterwards writes the value val to that register. (OSCCONL represents the
    // lower 8 bits of the register OSCCON)
    __builtin_write_OSCCONL(OSCCONL | 2); // Enable the Low-Power 32.768 kHz external oscillator (LPOSCEN bit in OSCCONL).
    // OSCCONL is a protected register, so we must use the built-in function
    // __builtin_write_OSCCONL() to unlock and modify it safely
    CLEARBIT(T1CONbits.TON) ;      // Disable Timer1
    SETBIT(T1CONbits.TCS);         // External clock (32.768 kHz)
    CLEARBIT(T1CONbits.TGATE);     // Gated mode off
    CLEARBIT(T1CONbits.TSYNC);      // Disable Synchronization
    T1CONbits.TCKPS = 0b11;       // 1:256 prescaler
    TMR1 = 0x00;               //Clear timer register
     // PR1 hesaplaması:
    // T_interrupt = 1s, F_timer = 32,768 Hz, prescaler = 256
    // PR1 = 1 * 32768 / 256 - 1 = 127
    PR1 = 128;                     
    IPC0bits.T1IP = 0x01;         // Interrupt priority level 1
    CLEARBIT(IFS0bits.T1IF);         // Clear flag
    SETBIT(IEC0bits.T1IE);         // Enable interrupt
    SETBIT(T1CONbits.TON);         // Start Timer1
    
    
    CLEARBIT(T2CONbits.TON) ;      // Disable Timer2
    CLEARBIT(T2CONbits.TCS);         // Internal clock (FCY = 12.8 MHz)
    CLEARBIT(T2CONbits.TGATE);     // Gated mode off
    CLEARBIT(T2CONbits.TSYNC);      // Disable Synchronization
    T2CONbits.TCKPS = 0b11;       // 1:256 prescaler
    TMR2 = 0x00;                  //Clear timer register
    // PR2 hesaplaması:
    // T_interrupt = 2ms = 0.002s, F_timer = 12.8 MHz, prescaler = 256
    // PR2 = 0.002 * 12,800,000 / 256 - 1 ≈ 99
    PR2 = 100;                   
    IPC0bits.T2IP = 0x01;         // Interrupt priority level 1
    CLEARBIT(IFS0bits.T2IF);         // Clear flag
    SETBIT(IEC0bits.T2IE);         // Enable interrupt
    SETBIT(T2CONbits.TON);         // Start Timer2
    
    CLEARBIT(T3CONbits.TON) ;      // Disable Timer3
    CLEARBIT(T3CONbits.TCS);         // Internal clock (FCY = 12.8 MHz)
    CLEARBIT(T3CONbits.TGATE);     // Gated mode off
    CLEARBIT(T3CONbits.TSYNC);      // Disable Synchronization
    T3CONbits.TCKPS = 0b00;       // 1:1 prescaler
    TMR3 = 0x00;                  //Clear timer register
    PR3 = 0xFFFF;                    // (32768 / 256 = 128 Hz) → 1 interrupt/sec
    IPC0bits.T3IP = 0x01;         // Interrupt priority level 1
    CLEARBIT(IFS0bits.T3IF);         // Clear flag
    SETBIT(IEC0bits.T3IE);         // Enable interrupt
    SETBIT(T3CONbits.TON);         // Start Timer2
}

void timer_loop()
{
    lcd_printf("Lab02: Int & Timer");
    lcd_locate(0, 1);
    lcd_printf("Group:7, Aylin, Lavinda, Ahmet");

    unsigned int iteration = 0;
    while(TRUE)
    {
        iteration++;
        if(iteration % 2000 == 0)
        {
            LED3 = !LED3;

            unsigned long total_ms = seconds * 1000 + ms_value;
            lcd_locate(0,3);
            lcd_printf("Time: %2u:%2u:%3u", total_ms/60000, (total_ms/1000)%60, total_ms%1000);

            // Timer3 measurement
            TMR3 = 0x00;
            SETBIT(T3CONbits.TON);
            for(int i=0; i<2000; i++)
            {
                Nop();
            }
            CLEARBIT(T3CONbits.TON);

            unsigned int cycles = TMR3;
            double ms_duration = (double)cycles / (FCY / 1000.0);
            lcd_locate(0,4);
            lcd_printf("T3:%u cyc and %.4f ms", cycles, ms_duration);
        }
    }
}


void __attribute__((__interrupt__, __shadow__, __auto_psv__)) _T1Interrupt(void)
{ // invoked every ??
    CLEARBIT(IFS0bits.T1IF);
    LED2=!LED2; 
    seconds+=1; // 1 sec.
}

void __attribute__((__interrupt__, __shadow__, __auto_psv__)) _T2Interrupt(void)
{ // invoked every ??
    CLEARBIT(IFS0bits.T2IF); 
    LED1=!LED1;
    ms_value+=2; // 2ms
}
