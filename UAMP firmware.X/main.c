/*
 * File:   main.c
 * Author: Gunnar T. 
 *
 * Created on October 9, 2019, 5:44 PM
 */

#include <xc.h>
#include <stdint.h>
#include "PICCONFIG_UAMP.h"
#include "PPS.h"
#include "I2C.h"
#include "time.h"
#include "LM49450.h"

void PIC_SETUP(){
    
    //IO setup
    TRISA = 0xFF;
    TRISC = 0xFF;
    ANSELA = 0;
    ANSELC = 0;
    WPUA = 0b00110000;      //WPU on RA5 and RA4, volume plus and minus
    WPUC = 0b00011000;      //WPU on RC4 and RC3, jumpers
    
    //TIMER1 setup
    T1CONbits.CKPS = 0b00;  //1:1 prescale
    T1CONbits.nSYNC = 0;
    T1CONbits.RD16 = 1;     //16-bit read
    T1GCONbits.GE = 0;      //Gate OFF
    T1CLK = 0b00000100;     //CLK is LFINTOSC
    TMR1 = TMR1_RST;  
    PIE4bits.TMR1IE = 1;    //enable tmr1 interrupt     
    T1CONbits.ON = 1;
    
    INTCONbits.GIE = 1;     //enable active interrupts
    INTCONbits.PEIE = 1;    //enable peripheral interrupts
}

void interrupt ISR(){

    if(TMR1IF) {
        TMR1IF = 0;
        TMR1 = TMR1_RST;
        timer_counter++;
    }
    
} 

uint8_t mute_config = 0;    //sets mute to 0 and saves default reg0 config
uint8_t mute_state = 0;

uint8_t volume_sp = 10;    //speaker volume to be sent to amp
uint8_t volume_sp_prev = 10;

uint8_t volume_hp = 10;    //headphone volume to be sent to amp
uint8_t volume_hp_prev = 10;
uint32_t HPS_time_start = 0;   //time when HPS is first signaled
uint8_t HPS_state = 0;

uint32_t vol_plus_time_start = 0;   //time when the button is first pressed
uint32_t vol_plus_time_hold = 0;    //a timer to increment volume after button is pressed and held

uint32_t vol_minus_time_start = 0;
uint32_t vol_minus_time_hold = 0;

uint8_t vol_plus_state = 0;     //state of button presses
uint8_t vol_minus_state = 0;
/*
 state 0: button is not pressed
 state 1: button is being debounced
 state 2: button is pressed and held
*/

void main(void) {
    
    PIC_SETUP();
 
    //Initialize I2C Master
    PPS_unlock();
    SSP1DATPPS = 0x11;  //SDA INPUT
    RC1PPS = 0x16;      //SDA OUTPUT
    SSP1CLKPPS = 0x10;  //SDA INPUT
    RC0PPS = 0x15;      //SCL OUTPUT
    PPS_lock();
    I2C_Master_Init(100000);      
    
    //delay before beginning I2C to make sure voltage is stable
    __delay_ms(50);
    
    //check jumpers and initialize audio mode
    if(dat0 && dat1) {
        mute_config = LM49450_Wii_init();
    }
    else if(dat0 && !dat1) {
        mute_config = LM49450_PS2_init();
    }
    else if(!dat0 && !dat1) {
        mute_config = LM49450_analog_init();
    }
    else if(!dat0 && dat1) {
        mute_config = LM49450_DC_init();
    }
    
    //set volume over i2c
    LM49450_write(0x08, volume_sp); //speaker volume
    LM49450_write(0x07, volume_hp); //headphone volume
    
    while(1) {
        
        //debouncing headphone sense
        if(HPS) {
            if(HPS_state == 0) {
                HPS_time_start = get_time();
                HPS_state = 1;
            }
            else if(HPS_state == 1) {
                if(timer_diff(HPS_time_start) >= 4) {
                    HPS_state = 2;
                    if(volume_sp == 0 && mute_state == 0) {
                        //if sp volume is 0, then amp has been muted. it should be unmuted if it is not in mute state
                        LM49450_write(0x00, mute_config);   //un mute amp
                    }
                }
            }
        }
        else {
            if(HPS_state == 2) {
                if(volume_hp == 0 && mute_state == 0) {
                    //if hp volume is 0, then amp has been muted. it should be unmuted if it is not in mute state
                    LM49450_write(0x00, mute_config);   //un mute amp  
                }
            }
            HPS_state = 0;
        }
        
        if(!vol_plus) {
            switch(vol_plus_state) {
                case 0:
                    //button has been pressed first time
                    vol_plus_time_start = get_time();
                    vol_plus_state = 1;
                break;
                case 1:
                    //wait to see if button is debounced
                    if(timer_diff(vol_plus_time_start) >= 4) {
                        vol_plus_time_hold = get_time();
                        vol_plus_state = 2;
                        if(HPS_state == 2) {
                             if((volume_hp < 31) && mute_state == 0) volume_hp++;
                        }
                        else if((volume_sp < 31) && mute_state == 0) volume_sp++;
                    }                         
                break; 
                case 2:
                    //button is pressed, every X time, increment volume
                    if(timer_diff(vol_plus_time_hold) >= 20) {
                        vol_plus_time_hold = get_time();
                        if(HPS_state == 2) {
                             if((volume_hp < 31) && mute_state == 0) volume_hp++;
                        }
                        else if((volume_sp < 31) && mute_state == 0) volume_sp++;
                    }
                break;
                default:
                    vol_plus_state = 0;
            }
        }
        else {
            vol_plus_state = 0;
        }
              
        if(!vol_minus) {
            switch(vol_minus_state) {
                case 0:
                    vol_minus_time_start = get_time();
                    vol_minus_state = 1;
                break;
                case 1:
                    if(timer_diff(vol_minus_time_start) >= 4) {
                        vol_minus_time_hold = get_time();
                        vol_minus_state = 2;
                        if(HPS_state == 2) {
                             if((volume_hp > 0) && mute_state == 0) volume_hp--;
                        }
                        else if((volume_sp > 0) && mute_state == 0) volume_sp--;
                    }                         
                break; 
                case 2:
                    if(timer_diff(vol_minus_time_hold) >= 20) {
                        vol_minus_time_hold = get_time();
                        if(HPS_state == 2) {
                             if((volume_hp > 0) && mute_state == 0) volume_hp--;
                        }
                        else if((volume_sp > 0) && mute_state == 0) volume_sp--;
                    }
                break;
                default:
                    vol_minus_state = 0;
            }
        }
        else {
            vol_minus_state = 0;
        }        
        
        //if both buttons held down, then mute amp
        if((vol_plus_state == 2) && (vol_minus_state == 2)) {
            switch(mute_state) {
                case 0:
                    //both buttons are held down first time
                    LM49450_write(0x00, (mute_config | 0b100));    //mute amp
                    mute_state = 1;
                break;
                case 1:
                    //amp is now muted, wait for both buttons to be released
                break;    
                case 2:
                    LM49450_write(0x00, mute_config);   //un mute amp
                    mute_state = 3;
                break;    
                case 3:
                    //wait for both buttons to be released
                break;
                default:
                    mute_state = 0;
            }
        }
        else {
            if(mute_state == 1) {
                mute_state = 2;
            }
            if(mute_state == 3) {
                mute_state = 0;
            }
        }
        
        //setting the volume to the amp
        if((volume_sp != volume_sp_prev) && mute_state == 0) {   
            LM49450_write(0x08, volume_sp); //speaker volume
            if(volume_sp == 0) {
                LM49450_write(0x00, (mute_config | 0b00000100));    //mute amp
            }
            else {
                LM49450_write(0x00, mute_config);   //un mute amp
            }
        }
        
        if((volume_hp != volume_hp_prev) && mute_state == 0) {     
            LM49450_write(0x07, volume_hp); //headphone volume
            if(volume_hp == 0) {
                LM49450_write(0x00, (mute_config | 0b00000100));    //mute amp
            }
            else {
                LM49450_write(0x00, mute_config);   //un mute amp
            }
        }        
        
        volume_sp_prev = volume_sp;
        volume_hp_prev = volume_hp;
    }
}