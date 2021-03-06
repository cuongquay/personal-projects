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

// DS1307 Real Time Clock functions
#include <ds1307.h>
                                      
// Declare your global variables here     
static PBYTE start_mem;         

bit data_bit = 0;       
bit power_off = 0;
bit is_stopping = 0;      
bit is_half_top = 1;

unsigned long temp[4];
unsigned long dw_temp; 

register UINT x=0;
register UINT y=0;   
                                
static UINT  scroll_count = 0;  
static UINT  scroll_updown = 0; 
static UINT  tick_count  = 0;       
static UINT  stopping_count = 0;       
static BYTE  frame_index = 0;   

static UINT  text_length = 0; 
static BYTE  scroll_step = 0;             
static BYTE  scroll_rate = 20;
static BYTE  scroll_type = LEFT_RIGHT;            
             
// Global variables for message control
BYTE  rx_message = UNKNOWN_MSG;
WORD  rx_wparam  = 0;
WORD  rx_lparam  = 0;
// [0x55][0x55][0x55][rx_message][rx_wparam][rx_lparam]
                            
extern void reset_serial();         
extern void send_echo_msg();    
extern BYTE eeprom_read(BYTE deviceID, WORD address);
extern void eeprom_write(BYTE deviceID, WORD address, BYTE data);
extern WORD eeprom_read_w(BYTE deviceID, WORD address);
extern void eeprom_write_w(BYTE deviceID, WORD address, WORD data);
extern void eeprom_write_page(BYTE deviceID, WORD address, PBYTE buffer, BYTE page_size);
extern void eeprom_read_page(BYTE deviceID, WORD address, PBYTE buffer, BYTE page_size);

static void _displayFrame();
static void _doScroll();   
void LoadFrame(BYTE index);
void BlankRAM(PBYTE start_addr,PBYTE end_addr);

///////////////////////////////////////////////////////////////
// Timer 0 overflow interrupt service routine , 40.5 us per tick                 
///////////////////////////////////////////////////////////////
interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{       
    TCNT0 = 0x05;       // 1ms = 4*250 = 1000 us
    ++tick_count;    
}

///////////////////////////////////////////////////////////////
// static function(s) for led matrix display panel
///////////////////////////////////////////////////////////////

#define __DATA_SHIFT()    \
{\
    temp[0] = (unsigned long)start_mem[((unsigned int)4)*(x) + 0];\
    temp[1] = (unsigned long)start_mem[((unsigned int)4)*(x) + 1];\
    temp[2] = (unsigned long)start_mem[((unsigned int)4)*(x) + 2];\
    temp[3] = (unsigned long)start_mem[((unsigned int)4)*(x) + 3];\
    dw_temp = ~((temp[0]&0x000000FF) | ((temp[1]<<8)&0x0000FF00) | \
              ((temp[2]<<16)&0x00FF0000) | ((temp[3]<<24)&0xFF000000));\
    if (is_half_top){\
        dw_temp = dw_temp>>scroll_updown;\
    }\
    else{\
        dw_temp = dw_temp<<scroll_updown;\
    }\
}

static void _putData()
{                   
    BYTE buff1 = 0;  
    BYTE buff2 = 0;                               
    __DATA_SHIFT();                     
     
    buff2 = ~(BYTE)(dw_temp>>24)&0x000000FF;        
    for (y=0; y< 8; y++){                      					
        if (power_off) data_bit =OFF_LED;                            
        data_bit = (buff2<<y) & 0x80;
        DATA_OUT = data_bit;      
        DATA_CLK = 1;
        DATA_CLK = 0;	    		
    }                           
    buff2 = ~(BYTE)(dw_temp>>16)&0x000000FF;
    for (y=0; y< 8; y++){                     					
        if (power_off) data_bit =OFF_LED; 
        data_bit = (buff2<<y) & 0x80;                            
        DATA_OUT = data_bit;  
        DATA_CLK = 1;
        DATA_CLK = 0;	    		
    }             
             
    buff1 = ~(BYTE)(dw_temp>>8)&0x000000FF;
    for (y=0; y< 8; y++){                      					
        if (power_off) data_bit =OFF_LED;                            
        data_bit = (buff1<<y) & 0x80;
        DATA_OUT = data_bit;      
        DATA_CLK = 1;
        DATA_CLK = 0;	    		
    }                           
    buff1 = ~(BYTE)(dw_temp)&0x000000FF;
    for (y=0; y< 8; y++){                     					
        if (power_off) data_bit =OFF_LED; 
        data_bit = (buff1<<y) & 0x80;                            
        DATA_OUT = data_bit;                         
        DATA_CLK = 1;
        DATA_CLK = 0;	    		
    }
}

static void _displayFrame()
{                                    
    if (scroll_type==SCROLLING){
/*      if (tick_count > scroll_rate){            
            _putData();
            DATA_STB = 1;
            DATA_STB = 0;                              
        }
*/        
    }
    else{
	    for (x=0; x< SCREEN_WIDTH; x++){
		    _putData();					
    	}   
        DATA_STB = 1;
        DATA_STB = 0; 
	}       
}     
                                                                                  
static void _doScroll()
{
  if (tick_count > scroll_rate){    
    switch (scroll_type)
    {
    case LEFT_RIGHT:                
        if (is_stopping==0){   
            if (scroll_rate > MIN_RATE)
       	        start_mem += 4;
       	    else 
       	        start_mem += 8;
   	    }
   	    if (start_mem == START_RAM_TEXT + (SCREEN_WIDTH<<2))
   	        is_stopping = 1;
   	    if (is_stopping ==1)
   	    {
   	        if (stopping_count++>MAX_STOP_TIME)
   	        {
   	            is_stopping=0;
   	            stopping_count = 0;
   	        }
   	    }                                  
       	if (start_mem > START_RAM_TEXT +4*(text_length+SCREEN_WIDTH+SCREEN_HIDE))             
   	    {                  
   	        LoadFrame(++frame_index);
       	}           
   	    break;
    case RIGHT_LEFT:
       	if (is_stopping==0){
   	        if (scroll_rate > MIN_RATE)
   	            start_mem -= 4;      
   	        else
   	            start_mem -= 8;
   	        
   	    }
   	    if (start_mem == START_RAM_TEXT + (SCREEN_WIDTH<<2))
   	        is_stopping = 1;
   	    if (is_stopping ==1)
   	    {
   	        if (stopping_count++ >MAX_STOP_TIME)
   	        {
   	            is_stopping=0;
   	            stopping_count = 0;
   	        }
   	    }
   	    else if (start_mem < START_RAM_TEXT)             
       	{
       	    scroll_count = 0;             
   	        LoadFrame(++frame_index);
       	}
       	break;
    case BOTTOM_TOP:               
         if (scroll_step==0){
            if (--scroll_updown <= 0){                
                scroll_step++;                
                is_half_top =1;   
            }
        }
        else if (scroll_step==1){  
            if (++scroll_count > MAX_STOP_TIME){   
                scroll_step++;
                scroll_count = 0;               
            }
        } 
        else if (scroll_step==2){
            if (++scroll_updown >SCREEN_HEIGHT){                
                scroll_step++;                
                is_half_top =0;   
            }
        }
        else{                   
            is_half_top =0;
            scroll_step =0;
            scroll_count =0;
            scroll_updown = SCREEN_HEIGHT;  
            start_mem += SCREEN_WIDTH;
            if (start_mem > START_RAM_TEXT +4*(text_length+SCREEN_WIDTH+SCREEN_HIDE)){                  
       	        LoadFrame(++frame_index);
           	}
        }  
        break;
    case TOP_BOTTOM:
        if (scroll_step==0){
            if (--scroll_updown <=0){                
                scroll_step++;                
                is_half_top =0;   
            }
        }
        else if (scroll_step==1){                  
            if (++scroll_count > MAX_STOP_TIME){   
                scroll_step++;   
                scroll_count = 0;             
                scroll_updown = 0;
            }
        }               
        else if (scroll_step==2){
            if (++scroll_updown > SCREEN_HEIGHT){                
                scroll_step++;                
                is_half_top =1;   
            }
        }                 
        else{
            is_half_top =1;
            scroll_step =0;
            scroll_count =0;
            scroll_updown =SCREEN_HEIGHT; 
            start_mem += SCREEN_WIDTH;
            if (start_mem > START_RAM_TEXT +4*(text_length+SCREEN_WIDTH+SCREEN_HIDE)){                  
       	        LoadFrame(++frame_index);
           	}
        }     
        break;  
    case SCROLLING:            
        /* shift data out step by step */
            _putData();
            DATA_STB = 1;
            DATA_STB = 0;    
            
        if (scroll_rate > MIN_RATE)
   	        start_mem += 4;
   	    else 
   	        start_mem += 8;
        if (start_mem > START_RAM_TEXT +4*(text_length+SCREEN_WIDTH+SCREEN_HIDE))             
   	    {            
   	        LoadFrame(++frame_index);
       	}       	  
        break;     
    case TOP_SCROLL:
        if (scroll_step==0){
            if (--scroll_updown <=0){                
                scroll_step++;                
                is_half_top =0;   
            }
        }
        else if (scroll_step==1){                  
            if (++scroll_count > MAX_STOP_TIME){   
                scroll_step++;   
                scroll_count = 0;             
                scroll_updown = 0;
            }
        }               
        else{            
            if (start_mem > START_RAM_TEXT +4*(text_length+SCREEN_WIDTH+SCREEN_HIDE)){                  
       	        LoadFrame(++frame_index);
           	}
        }    
        if (scroll_rate > MIN_RATE){
   	        start_mem += 4;         
   	    }
   	    else{ 
   	        start_mem += 8;
   	    }
        break;        
    case NOT_USE:
        LoadFrame(++frame_index);
        break;  
    default:
        break;
    }
	tick_count = 0;
	if (frame_index >= MAX_FRAME) frame_index=0;
    		
  }
             
}          
////////////////////////////////////////////////////////////////////
// General functions
//////////////////////////////////////////////////////////////////// 
#define RESET_WATCHDOG()    #asm("WDR");
                                                                            
void LoadConfig(BYTE index)
{
    BYTE devID = EEPROM_DEVICE_BASE;    
    WORD base = 0;   // base address
    devID += index<<1;                 
    if (index >=4){
        base = 0x8000;    
    }                 
    devID &= 0xF7;      // clear bit A3 
    
    // init I2C bus
    i2c_init();
    LED_STATUS = 1;
    scroll_rate = eeprom_read(devID, (WORD)base+0);    
    scroll_type = eeprom_read(devID, (WORD)base+1);    
    text_length = eeprom_read_w(devID, (WORD)base+2); 
    printf("line=%d rate=%d type=%d len=%d\r\n",index,scroll_rate,scroll_type,text_length);
    if (text_length > DATA_LENGTH){
        text_length= 0;            
    }
    if (scroll_type > NOT_USE){
        scroll_type = NOT_USE;
    }          
    if (scroll_rate > MAX_RATE){
        scroll_rate = 0;
    }
    LED_STATUS = 0;   
}
                       
void SaveTextLength(BYTE index)
{
    BYTE devID = EEPROM_DEVICE_BASE;    
    WORD base = 0;   // base address
    devID += index<<1;                 
    if (index >=4){
        base = 0x8000;    
    }                 
    devID &= 0xF7;      // clear bit A3 
    
    i2c_init();
    LED_STATUS = 1;   
    eeprom_write_w(devID, base+2,text_length); 
    LED_STATUS = 0;   
}

void SaveConfig(BYTE rate,BYTE type, BYTE index)
{                     
    BYTE devID = EEPROM_DEVICE_BASE;    
    WORD base = 0;   // base address
    devID += index<<1;                 
    if (index >=4){
        base = 0x8000;    
    }                 
    devID &= 0xF7;      // clear bit A3  
    i2c_init();
    LED_STATUS = 1;  
    eeprom_write(devID, base+0,rate);    
    eeprom_write(devID, base+1,type);    
    LED_STATUS = 0;       
}

void SaveToEEPROM(PBYTE address,WORD length,BYTE index)
{                             
    PBYTE temp = 0;     
    BYTE devID = EEPROM_DEVICE_BASE;
    WORD base = 0;   // base address
    devID += index<<1;                 
    if (index >=4){
        base = 0x8000;    
    }         				
    temp = address;         
        
    if (length > DATA_LENGTH)    
        return; // invalid param 
    length = (WORD)address+4*(SCREEN_WIDTH+length);         
    if (length%EEPROM_PAGE)
        length = EEPROM_PAGE*(length/EEPROM_PAGE) + EEPROM_PAGE;  
    // init I2C bus
    i2c_init();
    LED_STATUS = 1;        
    
    for (temp = address; temp < length; temp+= EEPROM_PAGE) 
    {   
        RESET_WATCHDOG();                          	                                              
        eeprom_write_page( devID, base+(WORD)temp, (PBYTE)temp, EEPROM_PAGE);	      
    }       
        
    LED_STATUS = 0;   
}
                      
void LoadToRAM(PBYTE address,WORD length,BYTE index)
{                         
    PBYTE temp = 0;          
    BYTE devID = EEPROM_DEVICE_BASE;
    WORD base = 0;   // base address
    devID += index<<1;                 
    if (index >=4){
        base = 0x8000;    
    }       				
    temp = address;                 

    if (length > DATA_LENGTH)    
        return; // invalid param
    length = (WORD)address+4*(SCREEN_WIDTH+length);         
    if (length%EEPROM_PAGE)
        length = EEPROM_PAGE*(length/EEPROM_PAGE) + EEPROM_PAGE;  
    // init I2C bus
    i2c_init();
    LED_STATUS = 1;             
 
    for (temp = address; temp < length; temp+= EEPROM_PAGE) 
    {
        eeprom_read_page( devID, base+(WORD)temp, (PBYTE)temp, EEPROM_PAGE );	                                   
        RESET_WATCHDOG();     
    }             

    LED_STATUS = 0;   
}

void LoadFrame(BYTE index)
{                 
    if (index >= MAX_FRAME){
        index=0;  
    }
    LoadConfig(index);  
    if (scroll_type==NOT_USE){
        return;           
    }                   
    BlankRAM((PBYTE)START_RAM,(PBYTE)END_RAM);
    LoadToRAM((PBYTE)START_RAM_TEXT,text_length,index);
    scroll_count = 0;
    is_stopping = 0; 
    scroll_step = 0;
    scroll_updown = 0;
    switch (scroll_type)
    {
    case LEFT_RIGHT:
        start_mem = (PBYTE)START_RAM_TEXT; 
        break;                
    case RIGHT_LEFT:
        start_mem = (PBYTE)START_RAM_TEXT + (text_length<<2); 
        break;
    case BOTTOM_TOP:  
        is_half_top =0;
        scroll_updown = SCREEN_HEIGHT;                           
        start_mem = (PBYTE)START_RAM_TEXT + (SCREEN_WIDTH<<2); 
        break;
    case TOP_BOTTOM:   
        is_half_top =1; 
        scroll_updown = SCREEN_HEIGHT;                                           
        start_mem = (PBYTE)START_RAM_TEXT + (SCREEN_WIDTH<<2); 
        break;  
    case SCROLLING:
        start_mem = (PBYTE)START_RAM_TEXT;
        break; 
    case TOP_SCROLL:
        is_half_top =1; 
        scroll_updown = SCREEN_HEIGHT;                                           
        start_mem = (PBYTE)START_RAM_TEXT; 
        break;
    default: 
        break;
    }                   
}

void SerialToRAM(PBYTE address,WORD length)                                             
{
    PBYTE temp = 0;          
    UINT i =0;     				
    temp   = address;    
    LED_STATUS = 1;
    for (i =0; i< (length<<2); i++)         
    {                          
        BYTE data = 0;
        data = ~getchar();
        *temp = data;
        temp++;
        RESET_WATCHDOG();                                     
    }              
    LED_STATUS = 0;
}
                      
void BlankRAM(PBYTE start_addr,PBYTE end_addr)
{        
    PBYTE temp = START_RAM;
    for (temp = start_addr; temp<= end_addr; temp++)    
        *temp = 0xFF;             
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
UBRRL=0x33;       // 8 MHz
#endif

// Lower page wait state(s): None
// Upper page wait state(s): None
MCUCR=0x80;
EMCUCR=0x00;     

// Timer(s)/Counter(s) Interrupt(s) initialization
TIMSK=0x02;
ETIMSK=0x00;

// Load calibration byte for osc.  
#ifdef _MEGA162_INCLUDED_
OSCCAL = 0x5E; 
#else
OSCCAL = 0xA7; 
#endif            

// I2C Bus initialization
// i2c_init();

// DS1307 Real Time Clock initialization
// Square wave output on pin SQW/OUT: Off
// SQW/OUT pin state: 1
// rtc_init(3,0,1);

//i2c_init(); // must be call before
//rtc_init(3,0,1); // init RTC DS1307  
//rtc_set_time(15,2,0);
//rtc_set_date(9,5,6);    
                
// Watchdog Timer initialization
// Watchdog Timer Prescaler: OSC/2048k     
#ifdef __WATCH_DOG_
WDTCR=0x1F;
WDTCR=0x0F;
#endif
}

void PowerReset()
{      
    start_mem = (PBYTE)START_RAM_TEXT;                    

    InitDevice();
       
    LED_STATUS = 0;
    BlankRAM((PBYTE)START_RAM,(PBYTE)END_RAM);  
    
    LED_STATUS = 0;  
    delay_ms(50);
    LED_STATUS = 1;
    delay_ms(50);
    LED_STATUS = 0;
    delay_ms(500);    
    LED_STATUS = 1;
                
    frame_index= 0;
    LoadFrame(frame_index);
        
#ifdef _INIT_EEPROM_ 
{
    BYTE i =0;
    for (i =0; i< MAX_FRAME; i++){   
        SaveConfig(10,0,i);
        text_length = 160;
        SaveTextLength(i);            
    }
}
#endif  
    printf("LCMS v3.10 Designed by CuongQuay\r\n");  
    printf("cuong3ihut@yahoo.com - 0915651001\r\n");
    printf("Started date: 04.04.2007\r\n");
    printf("Release date: 07.04.2007\r\n");    
}

void ProcessCommand()
{
   	#asm("cli"); 
    RESET_WATCHDOG();

    // serial message processing     
    switch (rx_message)
    {                  
    case LOAD_TEXT_MSG:
        {                
            text_length = rx_wparam;            
            frame_index = rx_lparam&0x0F;   
            BlankRAM((PBYTE)START_RAM,(PBYTE)END_RAM);
            SerialToRAM((PBYTE)START_RAM_TEXT+4*(SCREEN_WIDTH),text_length);                
			start_mem = (PBYTE)START_RAM_TEXT+4*(SCREEN_WIDTH);				
			SaveToEEPROM((PBYTE)START_RAM_TEXT,text_length,rx_lparam);
			SaveTextLength(rx_lparam);							  
        }				
        break;           
    case LOAD_BKGND_MSG:
        {
        }
        break;   
    case SET_CFG_MSG: 
        {               
            SaveConfig(rx_wparam&0x0FF,rx_wparam>>8,rx_lparam); 
        }
        break;    
    case EEPROM_SAVE_TEXT_MSG:     
    case EEPROM_SAVE_ALL_MSG:  
        {                                                          
            SaveTextLength(rx_lparam);              
            SaveToEEPROM((PBYTE)START_RAM_TEXT,text_length,rx_lparam);
        }
        break;         
    case EEPROM_LOAD_TEXT_MSG:    
    case EEPROM_LOAD_ALL_MSG:
        {
            LoadConfig(rx_lparam);                               
            LoadToRAM((PBYTE)START_RAM_TEXT,text_length,rx_lparam); 
            start_mem = (PBYTE)START_RAM_TEXT+4*(SCREEN_WIDTH);
        }
        break;  
    case POWER_CTRL_MSG:
        power_off = rx_wparam&0x01;
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
            if (!is_stopping){
                _displayFrame();
            }
            _doScroll();            
        }
        RESET_WATCHDOG();
    };

}
                         
