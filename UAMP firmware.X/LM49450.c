/* 
 * File:   LM49450.cpp
 * Author: Gunnar Turnquist
 *
 * Created on May 31, 2019, 10:27 AM
 */

#include <xc.h>
#include "I2C.h"
#include <stdint.h>

#define LM49450_ADDR    0b1111101 
            
void LM49450_write(unsigned char reg, char data) {
    I2C_Master_Start();
    I2C_Master_Write(LM49450_ADDR << 1);            
    I2C_Master_Write(reg);
    I2C_Master_Write(data);   
    I2C_Master_Stop();
}

uint8_t LM49450_DC_init() {
    uint8_t reg0_DC = 0b00101001;
    LM49450_write(0x00, reg0_DC); 
    LM49450_write(0x01, 0b00000001);
    LM49450_write(0x02, 0x44);   //68 for 11.28MHz
    LM49450_write(0x03, 0b00000010);     //16bit word size, Right-Justified  
    LM49450_write(0x04, 0b00000000);     
    
    return reg0_DC;
}

uint8_t LM49450_Wii_init() {
    uint8_t reg0_Wii = 0b00101001;
    LM49450_write(0x00, reg0_Wii); 
    LM49450_write(0x01, 0b00000001); // /1
    LM49450_write(0x02, 0x4B);      // 75 recommended value for 12.228MHz
    LM49450_write(0x03, 0b01110001);
    LM49450_write(0x04, 0b00000000);  

    return reg0_Wii;
}

uint8_t LM49450_PS2_init() {
    uint8_t reg0_PS2 = 0b00101001;
    LM49450_write(0x00, reg0_PS2);
    LM49450_write(0x01, 0b00000010); 
    LM49450_write(0x02, 0x4B);   
    LM49450_write(0x03, 0b01110000);
    LM49450_write(0x04, 0b00000000);       
    
    return reg0_PS2;
}

uint8_t LM49450_analog_init() {
    uint8_t reg0_analog = 0b00101011;
    LM49450_write(0x00, reg0_analog);     
    
    return reg0_analog;
}