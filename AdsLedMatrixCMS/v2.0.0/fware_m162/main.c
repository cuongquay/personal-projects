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

// 1 Wire Bus functions
#asm
   .equ __w1_port=0x12 ;PORTD
   .equ __w1_bit=5
#endasm
#include <1wire.h>
// DS1820 Temperature Sensor functions
#include <ds18b20.h>
// DS1307 Real Time Clock functions
#include <ds1307.h>
                                      
// Declare your global variables here     
static PBYTE start_mem;
static PBYTE end_mem;        
static PBYTE buffer; 
static PBYTE bkgnd_mem;     
static PBYTE org_mem;

bit is_show_bkgnd        = 0;
bit is_power_off         = 0;     
bit is_stopping          = 0;   
bit is_half_above        = 0;
bit is_half_below        = 0; 

register UINT count_row  = 0;
register UINT count_col  = 0;     
register signed horiz_idx  = 0;

static BYTE scroll_rate   = 0; 
static BYTE scroll_type   = 0;    
static BYTE frame_index   = 0;                                 

static UINT  tick_count  = 0;      
static UINT  char_count  = 0;
static UINT  text_length = 0;
static UINT  stopping_count = 0;
static UINT  flipping_count = 0;

static UINT  char_width[256];    
static UINT  columeH = 0;
static UINT  columeL = 0;
static UINT  char_index = 0;    
static UINT  next_char_width = 0;
static UINT  current_char_width = 0;  

flash char  szText[] = "** CuongQuay 0915651001 **";    
#define DATE_FORMAT_STR "Bay gio la 00 gio 00 phut 00 giay. Ngay 00 thang 00 nam 0000."
static char szDateStr[] = DATE_FORMAT_STR;
             
// Global variables for message control
BYTE  rx_message = UNKNOWN_MSG;
WORD  rx_wparam  = 0;
WORD  rx_lparam  = 0;
// [0x55][0x55][0x55][rx_message][rx_wparam][rx_lparam]
                            
extern void reset_serial();         
extern void send_echo_msg();    
extern BYTE eeprom_read(BYTE deviceID, PBYTE address);
extern void eeprom_write(BYTE deviceID, PBYTE address, BYTE data);
extern WORD eeprom_read_w(BYTE deviceID, PBYTE address);
extern void eeprom_write_w(BYTE deviceID, PBYTE address, WORD data);
extern void eeprom_write_page(BYTE deviceID, PBYTE address, PBYTE buffer, BYTE page_size);
extern void eeprom_read_page(BYTE deviceID, PBYTE address, PBYTE buffer, BYTE page_size);

static void _displayFrame();
static BOOL _doScroll(); 
                                                                                 
extern void getLunarStr(char* sz, short h, short m, short dd, short mm, int yyyy);

void LoadFrame();      
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
static void _setRow()
{
    BYTE i=0;        
    SCAN_DAT = OFF; 
    for (i=0; i<8; i++){            
        if (i==(7-count_row)){ 
            SCAN_DAT = ON;        
        }
        else{
            SCAN_DAT = OFF;        
        }
        SCAN_CLK = 1;
        SCAN_CLK = 0;            
    }
}
            
static void _powerOff()
{
    BYTE i =0;               
    SCAN_DAT = OFF;  // data scan low        
    for (i=0; i<8; i++)    
    {                                              
        SCAN_CLK = 1;    // clock scan high
        SCAN_CLK = 0;    // clock scan low            
    }                                         
    SCAN_STB = 1;    // strobe scan high
    SCAN_STB = 0;    // strobe scan low                    
}

static void _displayFrame()
{                                   
    count_col = 0;
    count_row = 0;         

    if (is_power_off ==1){
        _powerOff();
        return;
    }
             
    columeL = current_char_width + (START_RAM_TEXT + SCREEN_WIDTH) - (WORD)start_mem;
    columeH = next_char_width + (START_RAM_TEXT + SCREEN_WIDTH) - (WORD)start_mem;         	                                           

    // display one frame in the screen at the specific time 
    for (buffer = start_mem;buffer < (END_RAM); buffer++)  
    {      
        if (scroll_type == FLYING_TEXT && current_char_width < SCREEN_WIDTH)
		{                                  		        
			if (count_col < current_char_width){				
				if (is_show_bkgnd){
                    DATA_PORT = org_mem[count_row*DATA_LENGTH + SCREEN_WIDTH + count_col]&
                            (bkgnd_mem[count_col+SCREEN_WIDTH*count_row]);	
                }                               
                else{
                    DATA_PORT = org_mem[count_row*DATA_LENGTH + SCREEN_WIDTH + count_col];
                }
			}
			else if ((count_col > columeL) && (count_col < columeH)){				
				if (is_show_bkgnd){
				    DATA_PORT = (*buffer)&(bkgnd_mem[count_col+SCREEN_WIDTH*count_row]);								
				}                                                    
				else{
				    DATA_PORT = (*buffer);
				}
			}
			else{                  
			    if (is_show_bkgnd){
    	    		DATA_PORT = (bkgnd_mem[count_col+SCREEN_WIDTH*count_row]);                				
    	        }
    	        else{
    	            DATA_PORT = 0xFF;
    	        }
			}                                                               			
        }
        else{     	        
            if ( is_half_above ==1){                            
                if (horiz_idx < 8){
                    if (count_row > 7-horiz_idx){
                        DATA_PORT = *(buffer+horiz_idx*DATA_LENGTH-8*DATA_LENGTH)>>2;
                    }
                    else{                             
                        DATA_PORT = *(buffer+horiz_idx*DATA_LENGTH);
                    }  
                }
                else{                                                        
                    if (count_row < (16-horiz_idx)){
                        DATA_PORT = *(buffer+(horiz_idx-8)*DATA_LENGTH)>>2 | 0xCC;
                    }
                    else{
                        DATA_PORT = 0xFF;
                    }
                }                    
            }
            else if ( is_half_below ==1){         
                if (horiz_idx < 8){
                    if (count_row < horiz_idx){
                        DATA_PORT = *(buffer+8*DATA_LENGTH-horiz_idx*DATA_LENGTH)<<2 | 0x33;
                    }
                    else{
                        DATA_PORT = *(buffer-horiz_idx*DATA_LENGTH);
                    }   
                }                    
                else{                                             
                    if (count_row > horiz_idx-8){
                        DATA_PORT = *(buffer+8*DATA_LENGTH-horiz_idx*DATA_LENGTH)<<2 | 0x33;
                    }
                    else{
                        DATA_PORT = 0xFF;
                    }
                }
            }                
            else{
                if (is_show_bkgnd){           
                    if (count_col < 32){     
                        DATA_PORT = (bkgnd_mem[count_col+SCREEN_WIDTH*count_row]);
                    }
                    else{
                        DATA_PORT = (*buffer);
                    }
                }                     
                else{                     
                    DATA_PORT = (*buffer);
                }
            }            
                         
        }              
     
        #ifdef ENABLE_MASK_ROW  
            DATA_PORT |= ENABLE_MASK_ROW;
        #endif //ENABLE_MASK_ROW

        DATA_CLK = 1;    // clock high
        DATA_CLK = 0;    // clock low   
        if ( ++count_col >= SCREEN_WIDTH)
        {                                            
            count_col = 0;      // reset for next                                                                                                                       
            
            _powerOff();        // turn all led off            
#ifdef __CD4094_
            SCAN_STB = 0;       // strobe low 
#endif            
            _setRow();          // turn row-led on                                                           
                                                                                              
            SCAN_STB = 1;       // strobe high                                                    
#ifndef __CD4094_            
            SCAN_STB = 0;       // strobe low 
#endif              
            DATA_STB = 1;       // strobe high            
            DATA_STB = 0;       // strobe low                                                 
                                    
            if (++count_row >= 8){ 
                count_row = 0;
            }                                             
            delay_ms(1);                                            
            buffer += (DATA_LENGTH - SCREEN_WIDTH);                 
        }                   
    }                         	
}     
                                  
static BOOL _doScroll()
{
    // init state                         
    DATA_STB = 0;
    DATA_CLK = 0;                
    // scroll left with shift_step bit(s)
    if(tick_count >= scroll_rate)
    {                             
        tick_count = 0; 
        is_show_bkgnd = 0;          
        switch (scroll_type)
        {
        case FLYING_TEXT:        
            if (current_char_width > SCREEN_WIDTH){
                if (is_stopping==0){
                    if (stopping_count < MAX_STOP_TIME){
                        is_stopping = 1;
                        //start_mem = (PBYTE)(START_RAM_TEXT+SCREEN_WIDTH);
                    }
                    else{
                        start_mem++;
                    }
                }
                else{
                    if (++stopping_count > MAX_STOP_TIME){
                        is_stopping = 0;
                    }
                }
                                  
            }
		    else{
			    start_mem += (2);
    			if (start_mem > (START_RAM_TEXT + SCREEN_WIDTH) - current_char_width){
		    		char_index++;				 
			    	current_char_width = char_width[char_index];
				    next_char_width = char_width[char_index+1];				
    				start_mem = (PBYTE)START_RAM_TEXT + current_char_width;
	    		}
		    }  
		    if (start_mem >= (START_RAM_TEXT + SCREEN_WIDTH + SCREEN_HIDE)+ text_length){
		        LoadFrame();
		    }
            break;
        case SCROLL_TEXT:
            start_mem++;    
            if (start_mem >= (START_RAM_TEXT + SCREEN_WIDTH + SCREEN_HIDE)+ text_length){
		        LoadFrame();
		    }
            break;
        case FLIPPING_TEXT:                                  
            if (start_mem < (START_RAM_TEXT + SCREEN_WIDTH)){		    
                start_mem+=32;    
            }
            else {		    
                if (is_power_off==0){                
                    if (++stopping_count > MAX_STOP_TIME/2){
                        is_power_off=1;                    
                        stopping_count=0;                
                    }
                }
                else{                                     
                    if (++stopping_count > MAX_STOP_TIME){            
                        is_power_off=0;
                        stopping_count=0;                    
                        if(++flipping_count > MAX_FLIP_TIME){
                            flipping_count=0;
                            scroll_type = SCROLL_TEXT;
                        }
                    }
                }
            }  
            break;
        case SHOW_BKGND:      
            start_mem++;    
            is_show_bkgnd = 1;            
            if (start_mem >= (START_RAM_TEXT + SCREEN_WIDTH + SCREEN_HIDE)+ text_length){
		        LoadFrame();
		    }
            break;     
        case RIGHT_LEFT:
            if (start_mem < (START_RAM_TEXT + SCREEN_WIDTH)){		    
                start_mem+=8;    
            }   
            else if (start_mem >= (START_RAM_TEXT + SCREEN_WIDTH) && start_mem < (START_RAM_TEXT + SCREEN_WIDTH)+8){
                if (is_stopping==0){
                    is_stopping =1;
                }
                else{
                    if (++stopping_count > MAX_STOP_TIME){                       
                        is_stopping = 0; 
                        start_mem +=8;
                    }
                }
            }
            else{
                if (is_stopping==0){
                    start_mem++; 
                }
            }                   
            if (start_mem >= (START_RAM_TEXT + SCREEN_WIDTH + SCREEN_HIDE)+ text_length){
		        LoadFrame();
		    }
            break;      
#ifdef  __SCROLL_TOP_            
        case SCROLL_TOP: 
            if (is_half_above==1){ 
                if (is_stopping ==0){            
                    if (++horiz_idx >=SCREEN_HEIGHT){                
                        is_half_above = 0;
                        is_half_below = 1;        
                        horiz_idx  = SCREEN_HEIGHT;
                        start_mem += SCREEN_WIDTH;                                  
                        if (start_mem >= (START_RAM_TEXT +(text_length+SCREEN_WIDTH))){ 
                            is_half_above = 0;
                            is_half_below = 0;
                            LoadFrame();
                        } 
                    }
                }
                else{                  
                    if (++stopping_count > MAX_STOP_TIME){                       
                        is_stopping = 0;
                    }
                }
            }
            else if (is_half_below==1){                
                if (--horiz_idx <0){
                    horiz_idx =0; 
                    is_half_below = 0;
                    is_half_above = 1;
                    is_stopping =1;
                    stopping_count = 0; 
                }   
            }                                
            break;        
#endif
#ifdef  __SCROLL_BOTTOM_            
        case SCROLL_BOTTOM:
            if (is_half_below==1){       
                if (is_stopping ==0){      
                    if (++horiz_idx >=SCREEN_HEIGHT){                
                        is_half_above = 1;
                        is_half_below = 0;   
                        horiz_idx  = SCREEN_HEIGHT;                
                        start_mem += SCREEN_WIDTH;                                  
                        if (start_mem >= (START_RAM_TEXT +(text_length+SCREEN_WIDTH))){
                            is_half_above = 0;
                            is_half_below = 0;
                            LoadFrame();
                        }              
                    }
                }
                else{
                    if (++stopping_count > MAX_STOP_TIME){                       
                        is_stopping = 0;
                    }
                }
            }
            else if (is_half_above==1){
                if (--horiz_idx <0){
                    horiz_idx =0; 
                    is_half_below = 1;
                    is_half_above = 0;     
                    is_stopping =1;
                    stopping_count = 0; 
                }  
            }                       
            break;              
#endif                                
        case TEARS_DROPPED:                        
            if (is_half_above==1){
                if (--horiz_idx <0){
                    horiz_idx =0; 
                    is_half_below = 0;
                    is_half_above = 0;     
                    is_stopping =1;
                    stopping_count = 0; 
                }  
            }        
            start_mem++;                  
            if (start_mem >= (START_RAM_TEXT +(text_length+SCREEN_WIDTH))){
                is_half_above = 0;
                is_half_below = 0;
                LoadFrame();
            }            
            break;
#ifdef __SHOW_DATE_STR_            
        case SHOW_DATE_STR:  
        #ifdef __DIGITAL_CLOCK_  
            if (is_half_below==1){       
                if (is_stopping ==0){      
                    if (++horiz_idx >=SCREEN_HEIGHT){                
                        is_half_above = 1;
                        is_half_below = 0;                             
                        char_index = 0xFF;        
                        horiz_idx  = SCREEN_HEIGHT;
                        start_mem+=SCREEN_WIDTH;    
                        if (start_mem >= (START_RAM_TEXT + 4*SCREEN_WIDTH)){
            		        is_half_below = 0;
                            is_half_above = 0; 
            		        LoadFrame();
		                }               
                    }
                }
                else{
                    if (++stopping_count > MAX_STOP_TIME){                       
                        is_stopping = 0;
                    }
                }
            }
            else if (is_half_above==1){
                if (--horiz_idx <0){
                    horiz_idx =0; 
                    is_half_below = 1;
                    is_half_above = 0;     
                    is_stopping =1;
                    stopping_count = 0; 
                }  
            }       
        #else          
            start_mem++;    
            if (start_mem >= (START_RAM_TEXT + SCREEN_WIDTH + SCREEN_HIDE)+ text_length){
		        LoadFrame();
		    }
		#endif            
            break; 
#endif            
        default:
            break;
        }          
    } 
    return TRUE;       
}                 

                                          
////////////////////////////////////////////////////////////////////
// General functions
//////////////////////////////////////////////////////////////////// 
#define RESET_WATCHDOG()    #asm("WDR");

void SerialToRAM(PBYTE address,WORD length, BYTE type)                                             
{
    PBYTE temp = 0;          
    UINT i =0, row =0;     				
    temp   = (PBYTE)address;    
    LED_STATUS = 0;
    for (row =0; row < 8; row++)
    {
        for (i =0; i< length; i++) 
        {
            *temp++ = ~getchar();
            RESET_WATCHDOG();
        }                               
        if (type == FRAME_TEXT)   
            temp += DATA_LENGTH - length;        
    }              
    LED_STATUS = 1;
}

void LoadCharWidth(WORD length)
{                               
    UINT i =0;  
    LED_STATUS = 0;   
    for (i =0; i < length; i++)
    {                           
        WORD data = 0;
        data = getchar();                       // LOBYTE 
        RESET_WATCHDOG();       
        char_width[i] = data;        
        data = getchar();                       // HIBYTE
        RESET_WATCHDOG();
        char_width[i] |= (data<<8)&0xFF00;
    }                  
    current_char_width = 0xFFFF;
    
    LED_STATUS = 1;
}

void SaveCharWidth(WORD length, BYTE index)
{
    UINT i =0;                 
    BYTE devID = EEPROM_DEVICE_BASE;
    PBYTE base  = 0x0A;   
    devID += index<<1;
    i2c_init();
    LED_STATUS = 0;   
    for (i =0; i < length; i++)
    {                           
        eeprom_write_w(devID,base+(i<<1),char_width[i]);
        RESET_WATCHDOG();
    }              
    LED_STATUS = 1; 
    PORTD = 0x00;
    DDRD = 0x3F; 
}

void GetCharWidth(WORD length, BYTE index)
{                               
    UINT i =0;   
    BYTE devID = EEPROM_DEVICE_BASE;
    PBYTE base  = 0x0A;              
    devID += index<<1;
    i2c_init();
    LED_STATUS = 0;              

    for (i =0; i < length; i++)
    {                           
        char_width[i] = eeprom_read_w(devID,base+(i<<1));    
        RESET_WATCHDOG();
    }                      
    LED_STATUS = 1;
    PORTD = 0x00;
    DDRD = 0x3F; 
}

void SaveToEEPROM(PBYTE address, BYTE type, BYTE index)
{                             
    PBYTE temp = 0;    
    PBYTE end  = START_RAM_TEXT;     
    BYTE devID = EEPROM_DEVICE_BASE;      				
    devID += index<<1;
    temp   = address;         
           
    i2c_init();
    LED_STATUS = 0;        
    
    if (type == FRAME_TEXT) end = (PBYTE)END_RAM;
    
    for (temp = address; temp < end; temp+= EEPROM_PAGE) 
    {   
        RESET_WATCHDOG();               
        eeprom_write_page( devID, temp, temp, EEPROM_PAGE);	      
    }       
    LED_STATUS = 1;
    PORTD = 0x00;
    DDRD = 0x3F;    
}
                      
void LoadToRAM(PBYTE address, WORD length, BYTE type, BYTE index)
{                         
    PBYTE temp = 0;          
    UINT i =0, row =0;    
    BYTE devID = EEPROM_DEVICE_BASE;
    devID += index<<1;      				
    temp   = address;                 
    
    if (length > DATA_LENGTH)    
        return; // invalid param
    if (length%EEPROM_PAGE)
        length = EEPROM_PAGE*(length/EEPROM_PAGE) + EEPROM_PAGE;        

    i2c_init();
    LED_STATUS = 0;             

    for (row =0; row < 8; row++)            
    {                           
        for (i =0; i< length; i+=EEPROM_PAGE) 
        {                                 
            eeprom_read_page( devID, temp, temp, EEPROM_PAGE );	                                   
            temp += EEPROM_PAGE;
            RESET_WATCHDOG();     
        }         
        if (type == FRAME_TEXT)  
            temp += DATA_LENGTH - length;  
    }             
    
    LED_STATUS = 1; 
    PORTD = 0x00;
    DDRD = 0x3F;  
}


void LoadConfig(BYTE index)
{   
    BYTE devID = EEPROM_DEVICE_BASE;
    devID += index<<1; 
    i2c_init();
    LED_STATUS = 1;                  
    RESET_WATCHDOG();             
    scroll_type   = eeprom_read(devID,0);
    scroll_rate = eeprom_read(devID,(PBYTE)1);
    text_length =  eeprom_read_w(devID,(PBYTE)2); 
    char_count =  eeprom_read_w(devID,(PBYTE)4);         
    printf("line=%d rate=%d type=%d len=%d\r\n",index,scroll_rate,scroll_type,text_length);
    if (text_length > DATA_LENGTH){
        text_length= 0;            
    }
    if (char_count > 255){
        char_count = 0;
    }
    LED_STATUS = 0; 
    PORTD = 0x00;
    DDRD = 0x3F;   
}

void SaveConfig(BYTE index)
{     
    BYTE devID = EEPROM_DEVICE_BASE;
    devID += index<<1; 
    i2c_init();
    LED_STATUS = 1;  
    eeprom_write(devID,(PBYTE)0,scroll_type);
    eeprom_write(devID,(PBYTE)1,scroll_rate);   
    LED_STATUS = 0; 
    PORTD = 0x00;
    DDRD = 0x3F;   
}

void SaveTextLength(WORD length, BYTE index)
{
    BYTE devID = EEPROM_DEVICE_BASE;
    devID += index<<1; 
    i2c_init();
    LED_STATUS = 1;   
    eeprom_write_w(devID, (PBYTE)2,length); 
    eeprom_write_w(devID, (PBYTE)4,char_count); 
    LED_STATUS = 0;
    PORTD = 0x00;
    DDRD = 0x3F;        
}

void BlankRAM(PBYTE start_addr,PBYTE end_addr)
{        
    PBYTE temp = START_RAM;
    for (temp = start_addr; temp<= end_addr; temp++)    
        *temp = 0xFF;             
}

void SetRTCDateTime()
{
    i2c_init();
    LED_STATUS = 0;   
    rtc_set_time(0,0,0);    /* clear CH bit */
    rtc_set_date(getchar(),getchar(),getchar());
    rtc_set_time(getchar(),getchar(),getchar());    
    LED_STATUS = 1;
    PORTD = 0x00;
    DDRD = 0x3F; 
}

static void TextFromFont(char *szText, BYTE nColor, BYTE bGradient, PBYTE pBuffer, BYTE index)
{
	int pos = 0,x=0,y=0;     
	BYTE i =0, len;
	BYTE ch = 0;
	UINT nWidth = 0;   
	UINT nCurWidth = 0, nNxtWidth = 0;		
    BYTE mask = 0x00, data = 0;
	BYTE mask_clr[2] = {0x00};
    BYTE devID = EEPROM_DEVICE_BASE;
    devID += index<<1; 
    	
	switch (nColor)
	{
	case 0:
		mask = 0xFF;		// BLANK
		mask_clr[0] = 0xFF;
		mask_clr[1] = 0xFF;
		break;
	case 1:
		mask = 0xAA;		// RED			RRRR	
		mask_clr[0] = 0x99;	// GREEN		RGRG
		mask_clr[1] = 0x88;	// YELLOW		RYRY
		break;
	case 2:
		mask = 0x55;		// GREEN		GGGG
		mask_clr[0] = 0x44;	// YELLOW		GYGY
		mask_clr[1] = 0x66;	// RED			GRGR	
		break;
	case 3:
		mask = 0x00;		// YELLOW		YYYY
		mask_clr[0] = 0x22;	// RED			YRYR	
		mask_clr[1] = 0x11;	// GREEN		YGYG
		break;
	default:
		break;
	}	
                           
	LED_STATUS = 0;
	i2c_init();
	len = strlen(szText);
	for (i=0; i< len; i++){				                                     
        ch = szText[i];             
		nCurWidth = char_width[ch];
		nNxtWidth = char_width[ch+1];    		
		nWidth = (nNxtWidth - nCurWidth); 		
		if ((pos + nWidth) >= DATA_LENGTH)  break;		
		for (y=0; y< 8 ; y++){    		            
		    if (bGradient) {
				if (y >=0 && y <4)	mask = mask_clr[0];
				if (y >=4 && y <8)	mask = mask_clr[1];	
			}			
			for (x=0; x< nWidth; x++){                                 
			    RESET_WATCHDOG();       
			    data = eeprom_read(devID, (PBYTE)(START_RAM_TEXT + SCREEN_WIDTH + y*DATA_LENGTH + nCurWidth + x));
			    data = (~data) & (~mask); 
   				pBuffer[y*DATA_LENGTH + x + pos] = ~data;   				
			}					
		}
		pos += nWidth;	 		
	}	                           
    
    text_length = pos;    
    LED_STATUS = 1;
    PORTD = 0x00;
    DDRD = 0x3F;     
}

void LoadFrame()
{  
    is_stopping = 0;
    stopping_count = 0;
    if (++frame_index >= MAX_FRAME){
        frame_index = 0;
    }                   
    _powerOff();
#ifdef __DIGITAL_CLOCK_ 
    frame_index = 0;
#endif 
    LoadConfig(frame_index);              
    BlankRAM((PBYTE)START_RAM,(PBYTE)END_RAM);
    GetCharWidth(char_count,frame_index);                               
    if (scroll_type != SHOW_DATE_STR){
        LoadToRAM((PBYTE)START_RAM_BK,SCREEN_WIDTH,FRAME_BKGND,frame_index);    	 
        LoadToRAM((PBYTE)START_RAM_TEXT + SCREEN_WIDTH,text_length,FRAME_TEXT,frame_index);    
    }

    switch (scroll_type)
    {
    case FLYING_TEXT: 
        {
            char_index = 0;
            start_mem = (PBYTE)START_RAM_TEXT;        
        }
        break;   
    case SHOW_BKGND:     
        is_show_bkgnd = 1;
    case FLIPPING_TEXT:          
    case SCROLL_TEXT:            
    case RIGHT_LEFT:
        {
            char_index = 0xFF;                     
            start_mem = (PBYTE)START_RAM_TEXT;        
        }
        break;     
#ifdef __SCROLL_TOP_                                      
    case SCROLL_TOP:
        {
            is_half_above = 0;
            is_half_below = 1;
            char_index = 0xFF; 
            horiz_idx  = SCREEN_HEIGHT;
            start_mem = (PBYTE)START_RAM_TEXT + SCREEN_WIDTH;
        }
        break;   
#endif
#ifdef __SCROLL_BOTTOM_        
    case SCROLL_BOTTOM:                                                       
        {
            is_half_above = 1;
            is_half_below = 0;                             
            char_index = 0xFF;        
            horiz_idx  = SCREEN_HEIGHT;
            start_mem = (PBYTE)START_RAM_TEXT + SCREEN_WIDTH;        
        }
        break;         
#endif
        case TEARS_DROPPED:            
        {
            is_half_above = 1;
            is_half_below = 0;                             
            char_index = 0xFF;        
            horiz_idx  = SCREEN_HEIGHT;
            start_mem = (PBYTE)START_RAM_TEXT;                
        }
        break;
#ifdef __SHOW_DATE_STR_        
    case SHOW_DATE_STR:
        {        
            int temp=0;                                                                                    
            BYTE hh=0,mm=0,ss=0;
            BYTE DD=0,MM=0,YY=00;
            i2c_init();                           
            rtc_get_date(&DD,&MM,&YY);
            rtc_get_time(&hh,&mm,&ss);                                                                                                                  
        #ifdef __DIGITAL_CLOCK_     
            if (!ds18b20_init(0,10,40,DS18B20_9BIT_RES)){
                printf("INIT DS18B20 ERROR!! \r\n");
            }
            temp=(int)ds18b20_temperature(0);        
            if (temp<0 || temp>100) temp = 0;
            sprintf(szDateStr,"%02d:%02d%02d/%02d%02d'C",hh,mm,DD,MM,temp);              
        #else       
            getLunarStr(szDateStr,hh,mm,DD,MM,2000+YY);                                      
        #endif            
            BlankRAM((PBYTE)START_RAM,(PBYTE)END_RAM);
            TextFromFont(szDateStr, 1, 0, (PBYTE)START_RAM_TEXT + SCREEN_WIDTH, frame_index);
            char_index = 0xFF;      
        #ifdef __DIGITAL_CLOCK_      
            is_half_above = 1;
            is_half_below = 0;                             
            char_index = 0xFF;        
            horiz_idx  = SCREEN_HEIGHT;
            start_mem = (PBYTE)START_RAM_TEXT + SCREEN_WIDTH; 
        #else           
            start_mem = (PBYTE)START_RAM_TEXT;                            
        #endif
        }
        break;         
#endif        
    default:        
        break;
    }       
    reset_serial();    
    current_char_width = char_width[char_index];
    next_char_width = char_width[char_index+1];
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
DDRA=0x00;

PORTB=0x00;
DDRB=0xFF;

PORTC=0x00;
DDRC=0x00;

PORTD = 0x00;
DDRD = 0x00;

PORTE=0x00;
DDRE=0x05;

TCCR0=0x03; 
TCNT0=0x05; 
OCR0=0x00;  

UCSR0A=0x00;
UCSR0B=0x98;
UCSR0C=0x86;
UBRR0H=0x00;
UBRR0L=0x67;

MCUCR=0x80;
EMCUCR=0x00;     

TIMSK=0x02;
ETIMSK=0x00;

// I2C Bus initialization
i2c_init();

// DS1307 Real Time Clock initialization
// Square wave output on pin SQW/OUT: Off
// SQW/OUT pin state: 1
rtc_init(0,0,1);   

#ifdef __WATCH_DOG_
WDTCR=0x1F;
WDTCR=0x0F;
#endif 

    printf("LCMS v2.02 Designed by CuongQuay\r\n");  
    printf("cuong3ihut@yahoo.com - 0915651001\r\n");
    printf("Release date: 09.01.2007\r\n");
#ifdef _TEST_RTC_
{                                
    BYTE hh=0,mm=0,ss=0,DD=0,MM=0,YY=0;
    rtc_get_date(&DD,&MM,&YY);
    rtc_get_time(&hh,&mm,&ss);
    printf("%02d:%02d:%02d %02d-%02d-%02d\r\n",hh,mm,ss,DD,MM,YY);
}           
#endif                   
}

void PowerReset()
{   
    start_mem = (PBYTE)START_RAM_TEXT;     
    end_mem   = (PBYTE)END_RAM;
    bkgnd_mem = (PBYTE)START_RAM_BK;
    org_mem   = (PBYTE)START_RAM_TEXT;	                   

    InitDevice();
                     
    LED_STATUS = 0;
    BlankRAM((PBYTE)START_RAM,(PBYTE)END_RAM);    
    delay_ms(500);
    LED_STATUS = 1;
    
    current_char_width = 0xFFFF;
    next_char_width = 0xFFFF;	 
    
    LED_STATUS = 0;  
    frame_index = 0; 
    LoadFrame();        
    LED_STATUS = 1;             
                       
    // reload configuration
    LED_STATUS = 0;
    delay_ms(100);
    LED_STATUS = 1;  
    PORTD = 0x00;
    DDRD = 0x3F;      
}

void ProcessCommand()
{
   	#asm("cli"); 
    RESET_WATCHDOG();
    // Turn off the scan board           
    _powerOff();
    // serial message processing     
    switch (rx_message)
    {                  
    case LOAD_TEXT_MSG:
        {
            if (rx_wparam > DATA_LENGTH) rx_wparam = DATA_LENGTH;
            BlankRAM((PBYTE)START_RAM_TEXT,(PBYTE)END_RAM);
            frame_index = rx_lparam>>8;                       
            char_count  = rx_lparam&0x00FF;                   
            text_length = rx_wparam;   
            SerialToRAM((PBYTE)START_RAM_TEXT + SCREEN_WIDTH,text_length,FRAME_TEXT);                
            LoadCharWidth(char_count);                                                              
			start_mem = (PBYTE)START_RAM_TEXT + SCREEN_WIDTH;				
	    	bkgnd_mem = (PBYTE)START_RAM_BK;		    		    		    	

            SaveCharWidth(char_count,frame_index);                           
            SaveTextLength(text_length,frame_index);
            SaveToEEPROM((PBYTE)START_RAM_TEXT,FRAME_TEXT,frame_index);            
            current_char_width = next_char_width = 0xFFFF;								  
        }				
        break;
    case LOAD_BKGND_MSG:
        {
            if (rx_wparam > SCREEN_WIDTH) rx_wparam = SCREEN_WIDTH;
            frame_index = rx_lparam>>8;  
            BlankRAM((PBYTE)START_RAM_BK,(PBYTE)START_RAM_TEXT);                    
            SerialToRAM((PBYTE)START_RAM_BK,rx_wparam,FRAME_BKGND);                                               
		    start_mem = (PBYTE)START_RAM_TEXT + SCREEN_WIDTH;				
		    bkgnd_mem = (PBYTE)START_RAM_BK;	
		    
		    SaveToEEPROM((PBYTE)START_RAM_BK,FRAME_BKGND,frame_index);			    
		}
        break; 
    case SET_CFG_MSG:
        {    
            frame_index = rx_lparam>>8;
            scroll_type  = rx_wparam&0x00FF;
            scroll_rate = rx_wparam>>8;             
            SaveConfig(frame_index);
        }
        break;   
    case SET_RTC_MSG:
        {                                
            SetRTCDateTime();
        }
        break;    
    default:
        break;
    }                 
    send_echo_msg();            
    rx_message = UNKNOWN_MSG;
    #asm("sei");
    PORTD = 0x00;
    DDRD = 0x3F;     

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
            _doScroll();
            _displayFrame();         
        }
        RESET_WATCHDOG();
    }
}
                         
