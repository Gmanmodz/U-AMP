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
    //mode control register
    //128 oversampling: B6:B5 = 0b01
    //spread spectrum
    //device enabled
    
    uint8_t reg0_DC = 0b00101001;
    LM49450_write(0x00, reg0_DC); 
    //clock control register
    //division is 1 B5:B0 = 0b000001
    LM49450_write(0x01, 0b00000001); // /1
    //charge pump clock register 
    //I2S mode control register
    //32bit word size: B6:B4 = 0b111
    //left-mode
    //I2S clock register
    LM49450_write(0x04, 0b00000000);   
    
    LM49450_write(0x02, 0x44);   //68 for 11.28MHz
    LM49450_write(0x03, 0b00000010);     //for DC. 16bit word size, Right-Justified    
    
    return reg0_DC;
}

uint8_t LM49450_Wii_init() {
    //mode control register
    //128 oversampling: B6:B5 = 0b01
    //spread spectrum
    //device enabled
    uint8_t reg0_Wii = 0b00101001;
    LM49450_write(0x00, reg0_Wii); 
    //clock control register
    //division is 1 B5:B0 = 0b000001
    LM49450_write(0x01, 0b00000001); // /1
    LM49450_write(0x02, 0x4B);      // 75 recommended value for 12.228MHz
    //charge pump clock register 
    //I2S mode control register
    //32bit word size: B6:B4 = 0b111
    //left-mode
    LM49450_write(0x03, 0b01110001);     //for Wii 
    //I2S clock register
    LM49450_write(0x04, 0b00000000);       
    
    return reg0_Wii;
}

uint8_t LM49450_PS2_init() {
    //mode control register
    //128 oversampling: B6:B5 = 0b01
    //spread spectrum
    //device enabled
    uint8_t reg0_PS2 = 0b00101001;
    LM49450_write(0x00, reg0_PS2);
    //clock control register
    //division is 1.5 B5:B0 = 0b000010
    LM49450_write(0x01, 0b00000010); 
    //charge pump clock register
    LM49450_write(0x02, 0x4B);   
    //I2S mode control register
    //32bit word size: B6:B4 = 0b111
    //normal mode
    LM49450_write(0x03, 0b01110000);
    //I2S clock register
    LM49450_write(0x04, 0b00000000);       
    
    return reg0_PS2;
}

uint8_t LM49450_analog_init() {
    //mode control register
    //128 oversampling: B6:B5 = 0b01
    //spread spectrum
    //device enabled
    //Line in enabled
    uint8_t reg0_analog = 0b00101011;
    LM49450_write(0x00, reg0_analog);     
    
    return reg0_analog;
}