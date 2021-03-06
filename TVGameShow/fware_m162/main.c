/*****************************************************
This program was produced by the
CodeWizardAVR V1.24.6 Professional
Automatic Program Generator
� Copyright 1998-2005 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com
e-mail:office@hpinfotech.com

Project : 
Version : 
Date    : 11/10/2005
Author  : CUONGDD                            
Company : 3iGROUP                           
Comments: 


Chip type           : ATmega16/162/8515
Program type        : Application
Clock frequency     : 8.000000 MHz
Memory model        : Small
External SRAM size  : 0
Data Stack size     : 256
*****************************************************/

#include "define.h"      
                                      
// Global variables for message control
BYTE  rx_message = UNKNOWN_MSG;
WORD  rx_wparam  = 0;
WORD  rx_lparam  = 0;                

// [0x55][0x55][0x55][rx_message][rx_wparam][rx_lparam]

// display code area //
/* station 1 [ 888 ] */
/* station 2 [ 888 ] */
/* station 3 [ 888 ] */
/* station 4 [ 888 ] */

BYTE  buffer[MAX_DIGIT] = {0x84,0xFC,0x91,0xB0,0xE8,0xA2,
                           0x82,0xF4,0x80,0xA0,0xFB,0xFF};
                            
extern void reset_serial();         
extern void send_echo_msg();    

static void _displayFrame();
static void _doScroll();   

///////////////////////////////////////////////////////////////
// Timer 0 overflow interrupt service routine , 40.5 us per tick                 
///////////////////////////////////////////////////////////////
interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{       
    TCNT0 = 0x05;       // 1ms = 4*250 = 1000 us
}

///////////////////////////////////////////////////////////////
// static function(s) for led matrix display panel
///////////////////////////////////////////////////////////////

static void _displayFrame()
{                
    BYTE mask=0;                  
    BYTE i=0,k=0;     
    CTRL_CLK = 0;
    CTRL_STB = 0;
    CTRL_DAT = 0;
    for (i=0; i< MAX_DIGIT; i++){      
        mask = 0x01;                      
        for (k=0; k< 8; k++){
            if ((buffer[i]&mask)>>(k)){
                CTRL_DAT = 1;
            }
            else{
                CTRL_DAT = 0;
            }
            CTRL_CLK = 1;delay_us(250);
            CTRL_CLK = 0;delay_us(250);        
            mask = mask<<1;  
        }        
    }	             
    CTRL_STB = 1;delay_us(250);
    CTRL_STB = 0;delay_us(250);
}                                                                                       
static void _doScroll()
{                          
    delay_ms(1);
}          
////////////////////////////////////////////////////////////////////
// General functions
//////////////////////////////////////////////////////////////////// 
#define RESET_WATCHDOG()    #asm("WDR");
                                                                            
void LoadConfig()
{
 
}
                       
void SaveConfig()
{                     

}
               
void SerialToBuffer(BYTE length)
{        
    BYTE i=0;
    for (i=0; i< length; i++){
        buffer[i] = getchar();
    }               
    
    printf("\r\n");
    for (i=0; i< MAX_DIGIT; i++){  
        printf("%02X ",buffer[i]);
    }
    printf("\r\n");         
    
}

///////////////////////////////////////////////////////////////
// END static function(s)
///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////           

void InitDevice()
{
// Declare your local variables here
// Crystal Oscillator division factor: 1  
#ifdef _MEGA162_INCLUDED_ 
#pragma optsize-
CLKPR=0x80;
CLKPR=0x00;
#ifdef _OPTIMIZE_SIZE_
#pragma optsize+
#endif                    
#endif

PORTA=0x00;
DDRA=0xFF;

PORTB=0x00;
DDRB=0xFF;

PORTC=0x00;
DDRC=0xFF;

PORTD=0xFF;
DDRD=0x00; 

PORTE=0x00;
DDRE=0xFF;

// Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: 250.000 kHz
// Mode: Normal top=FFh
// OC0 output: Disconnected
TCCR0=0x03;     // 4 us per tick
TCNT0=0x05;     // 1ms = 4*250
OCR0=0x00;      // 255 -250 = TCNT0

#ifdef _MEGA162_INCLUDED_
UCSR0A=0x00;
UCSR0B=0x98;
UCSR0C=0x86;
UBRR0H=0x00;
UBRR0L=0x67;      //  16 MHz     

#else // _MEGA8515_INCLUDE_     
UCSRA=0x00;
UCSRB=0x98;
UCSRC=0x86;
UBRRH=0x00;
UBRRL=0x67;       // 16 MHz
#endif

// Lower page wait state(s): None
// Upper page wait state(s): None
MCUCR=0x80;
EMCUCR=0x00;     

// Timer(s)/Counter(s) Interrupt(s) initialization
TIMSK=0x02;
ETIMSK=0x00;

// Watchdog Timer initialization
// Watchdog Timer Prescaler: OSC/2048k     
#ifdef __WATCH_DOG_
WDTCR=0x1F;
WDTCR=0x0F;
#endif
}

void PowerReset()
{      
    InitDevice();    
    LED_STATUS = 0;  
    delay_ms(50);
    LED_STATUS = 1;
    delay_ms(50);
    LED_STATUS = 0;
    delay_ms(500);    
    LED_STATUS = 1;
                
    printf("GameShow v1.00 Designed by CuongQuay\r\n");  
    printf("cuong3ihut@yahoo.com - 0915651001\r\n");
    printf("Started on: 06.03.2007\r\n");
}

void ProcessCommand()
{
   	#asm("cli"); 
    RESET_WATCHDOG();

    // serial message processing     
    switch (rx_message)
    {                  
    case LOAD_DATA_MSG:
        {                
            SerialToBuffer(rx_lparam);    
        }				
        break;           
    default:
        break;
    }                 
    send_echo_msg();            
    rx_message = UNKNOWN_MSG;
    #asm("sei");        
}           
////////////////////////////////////////////////////////////////////////////////
// MAIN PROGRAM
////////////////////////////////////////////////////////////////////////////////
void main(void)
{         
    if (MCUCSR & 8){
        // Watchdog Reset
        MCUCSR&=0xE0;  
        reset_serial(); 
    }
    else {      
        MCUCSR&=0xE0;
    }                                     
     
    PowerReset();                        
    #asm("sei");     

    while (1){         
        if (rx_message != UNKNOWN_MSG){   
            ProcessCommand();   
        }
        else{           
            _displayFrame();
            _doScroll();            
        }
        RESET_WATCHDOG();
    };

}
                         
