/*****************************************************
This program was produced by the
CodeWizardAVR V1.25.9 Professional
Automatic Program Generator
� Copyright 1998-2008 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project : 
Version : 
Date    : 11/17/2009
Author  : CuongQuay                       
Company : HOME                            
Comments: 


Chip type           : ATmega32
Program type        : Boot Loader - Size:2048words
Clock frequency     : 8.000000 MHz
Memory model        : Small
External SRAM size  : 0
Data Stack size     : 512
*****************************************************/

#include <stdio.h>            
#include <delay.h>
#include <mega32.h>

#include "bootloader.h"

// Declare your global variables here            
void Initialize()
{                                        
    PORTA=0x00;
    DDRA=0xFF;
    PORTB=0xF0;
    DDRB=0x0F;
    PORTC=0xFF;
    DDRC=0xFF;
    PORTD=0x00;
    DDRD=0xF8;

    GICR|=0x40;
    MCUCR=0x03;
    MCUCSR=0x00;
    GIFR=0x40;

    TIMSK=0x00;

    UCSRA=0x00;
    UCSRB=0x98;
    UCSRC=0x86;
    UBRRH=0x00;
    UBRRL=0x2F;    
    UBRRL=0x33;    

    ACSR=0x80;
    SFIOR=0x00;

    OSCCAL=0xAD;

    if (1){
        printf("                                          \r\n");
        printf("|=========================================|\r\n");
        printf("|       LEDSign AVR Firmware v2.0.0       |\r\n");
        printf("|_________________________________________|\r\n");
    }
}
              
extern BYTE ubPageBuffer[];  

void GetDataFromHost()
{    
    WORD wFlashAddr = 0xFFFF;        
    
    while (1)
    {                                    
        wFlashAddr = getchar() & 0x0F;     // get MSB address
        wFlashAddr = wFlashAddr<<8; // store into word
        wFlashAddr |= getchar() & 0x0F;    // get LSB address                          
        printf("ADDRESS: %0004X \r\n", wFlashAddr);        
        
        if (wFlashAddr == 0xFFFF)
            break;
        
        if (GetPageBuffer())
        {   
            int i =0;                 
            for (i=0; i<128; i++)
            {        
                printf("%02X ",ubPageBuffer[i]);
            }                                 
            FLWritePage(wFlashAddr);            
            if (FLCheckPage())
            {
                printf("\r\nNEXT PAGE \r\n");
            }
        }
    }                  
}
                                     
/*                  
struct _strLEDDataStruct {
   UWord16 ubNumOfRow;
   UWord16 ubNumOfCol;
   UWord16 ubNumOfPage;   
} ;

    if (++nTickCount > (int)tbuwDataBuffer[nCurrentRow])
	{		
		nTickCount = 0;
		nCurrentRow++;		
		SetColumnData(tbuwDataBuffer[nCurrentRow], pData);			

		WPARAM wParam = ((nCurrentRow/2)%(int)pData->ubNumOfRow)<<8;		
		wParam |= ((nCurrentRow/2)/(int)pData->ubNumOfRow & 0x00FF);		

		if (m_hwndDest != NULL)
			::PostMessage(m_hwndDest,mg_nTextCtrlMsg,WPARAM(wParam),LPARAM(tbuwDataBuffer[nCurrentRow]));

		if (++nCurrentRow > 2*(int)pData->ubNumOfRow*(int)pData->ubNumOfPage)
		{								
			nCurrentRow = 0;			
		}			
		
	}
*/
void SendToDisplay()
{    
    BYTE ubTempData;
    WORD wTempData = 0;                   
    WORD wCurrentAddr = 0;                       
    WORD wNumOfRow = 0;    
    WORD wNumOfPage = 0;      		
	    
    wNumOfRow = FLReadWord(wCurrentAddr++);    
    wCurrentAddr++;//wNumOfCol = FLReadWord(wCurrentAddr++);
    wNumOfPage = FLReadWord(wCurrentAddr++);
    
    while (1)
    {        
        wTempData = FLReadWord(wCurrentAddr++);
        delay_ms(wTempData);
        wTempData = FLReadWord(wCurrentAddr++);
                
        ubTempData = wTempData & 0x00FF;
        
        PORTC.5 = (ubTempData&0x01)>>0;
        PORTC.4 = (ubTempData&0x02)>>1;
        PORTC.3 = (ubTempData&0x04)>>2;
        PORTC.2 = (ubTempData&0x08)>>3;
        PORTC.1 = (ubTempData&0x10)>>4;
        PORTC.0 = (ubTempData&0x20)>>5;
        PORTB.5 = (ubTempData&0x40)>>6;
        PORTB.4 = (ubTempData&0x80)>>7;
        
        ubTempData = wTempData >> 8;
             
        PORTB.3 = (ubTempData&0x01)>>0;
        PORTB.2 = (ubTempData&0x02)>>1;
        PORTB.1 = (ubTempData&0x04)>>2;
        PORTB.0 = (ubTempData&0x08)>>3;
        PORTD.7 = (ubTempData&0x10)>>4;
        PORTB.7 = (ubTempData&0x20)>>5;
        PORTD.5 = (ubTempData&0x40)>>6;
        PORTD.6 = (ubTempData&0x80)>>7;                        
                                			        
        if (++wCurrentAddr > 2*wNumOfRow*wNumOfPage)
        {								
            wCurrentAddr = 0;			
        }
    }
}

void main(void)
{     
    Initialize();                   
    GetDataFromHost();
}