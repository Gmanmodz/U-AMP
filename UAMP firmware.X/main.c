/*
 * File:   main.c
 * Author: Gunnar
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

uint8_t volume = 15;    //volume to be sent to amp
uint8_t set_volume = 0; //bit to tell program when to send new data over i2c to amp. it is self clearing

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
    
    //check jumpers and initialize audio mode
    if(dat0 && dat1) {
        LM49450_Wii_init();
    }
    else if(dat0 && !dat1) {
        LM49450_PS2_init();
    }
    else if(!dat0 && !dat1) {
        LM49450_analog_init();
    }
    else if(!dat0 && dat1) {
        LM49450_DC_init();
    }
    
    //set volume over i2c
    LM49450_write(0x08, volume); //speaker volume
    LM49450_write(0x07, volume); //headphone volume
    
    while(1) {
        
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
                        volume++;
                        set_volume = 1;
                    }                         
                break; 
                case 2:
                    //button is pressed, every X time, increment volume
                    if(timer_diff(vol_plus_time_hold) >= 20) {
                        vol_plus_time_hold = get_time();
                        volume++;
                        set_volume = 1;
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
                        volume--;
                        set_volume = 1;
                    }                         
                break; 
                case 2:
                    if(timer_diff(vol_minus_time_hold) >= 20) {
                        vol_minus_time_hold = get_time();
                        volume--;
                        set_volume = 1;
                    }
                break;
                default:
                    vol_minus_state = 0;
            }
        }
        else {
            vol_minus_state = 0;
        }        
        
        //if both buttons held down, then volume is 0
        if((vol_plus_state == 2) && (vol_minus_state == 2)) {
            volume = 0;
            set_volume = 1;
        }
        
        if(set_volume) {
            if(volume < 1 || volume > 250) {    //since volume is unsigned, it needs to account when it goes past 0 
                volume = 0;
            }
            if(volume > 31) {
                volume = 31;
            }       
            //set volume over i2c
            LM49450_write(0x08, volume); //speaker volume
            LM49450_write(0x07, volume); //headphone volume

            set_volume = 0;
        }
    }
}