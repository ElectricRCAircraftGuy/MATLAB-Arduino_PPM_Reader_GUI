//Gabriel Staples
//21 Feb. 2014
//PPM_Reader.h

/*
===================================================================================================
  LICENSE & DISCLAIMER
  Copyright (C) 2014 Gabriel Staples.  All right reserved.
  
  ------------------------------------------------------------------------------------------------
  License: GNU General Public License Version 3 (GPLv3) - http://www.gnu.org/licenses/gpl.html
  ------------------------------------------------------------------------------------------------

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see http://www.gnu.org/licenses/
===================================================================================================
*/

#include <Arduino.h> //This is mandatory to put here, as this command allows you to use the standard Arduino functions and core library.
                     //Though this include statement is not explicitly required in .ino files (since it is automatically included in those), it IS
                     //explicitly required in .h header files, such as this.  
                     //Note: the uint8_t data type, used below for example, is not a valid data type without this include statement, as it IS part of the
                     //Arduino core library, and is NOT part of the standard c library.  Without this Arduino.h include statement, you only have access
                     //to the standard c library, and you'd have to use "unsigned char" instead of "uint8_t"
                     
//Set up PPM_Reader-specific Global variables

//set up constants & variables
const uint8_t input_pin = 2; //Pin 2 is the interrupt 0 pin, see here: http://arduino.cc/en/Reference/attachInterrupt
const uint8_t max_num_chs = 16; //the maximum # of channels you ever expect to see in a single PPM signal that this device will read 

//volatile variables for use in the ISR (Interrupt Service Routine)
volatile unsigned int chs[max_num_chs]; //units of 0.5us; a buffer to contain the channel width values
volatile unsigned int gaps[2]; //units of 0.5us; a buffer to contain 1) the gap time between channels [UNUSED IN THIS "SIMPLE" VERSION OF THE CODE],
                               //and 2) the gap between sets of PPM channel pulses
volatile unsigned int PPM_pd = 0; //units of 0.5us; contains the PPM period as determined by the external interrupt code
volatile unsigned char num_chs = 4; //How many channels of data are coming in???; initialize as 4 until the code actually determines the correct # of channels coming in
volatile boolean output_data = false; //time to output data?

volatile unsigned long Tx_on_check = millis(); //ms; this is a time value which will be used to detect whether or not the Tx is even on
