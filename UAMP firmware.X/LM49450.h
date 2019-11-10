/* 
 * File:   LM49450_H
 * Author: Gunnar Turnquist
 *
 * Created on May 31, 2019, 10:27 AM
 */

#ifndef LM49450_H
#define LM49450_H

#include <xc.h>

void LM49450_write(unsigned char reg, char data);
uint8_t LM49450_DC_init();
uint8_t LM49450_Wii_init();
uint8_t LM49450_PS2_init();
uint8_t LM49450_analog_init();

#endif