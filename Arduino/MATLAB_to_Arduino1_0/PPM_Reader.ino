/*
PPM_Reader1
By Gabriel Staples
http://electricrcaircraftguy.blogspot.com/
Written: 10 Feb. 2014
Last Updated: 12 Feb. 2014

-This code will read in a PPM signal from an RC transmitter.  I am using my custom Timer2_Counter code to get a precision of up to 0.5us.
--This code works equally well on positive *and* negative shift PPM trains

This code was written entirely at home, during my own personal time, and is neither a product of work nor my employer.
It is owned entirely by myself.

Hardware Setup: 
-Plug your PPM output signal from your radio transmitter (Tx) into the Arduino digital pin 2
--I recommend you go through a 1K resistor for safety. (ie: if you're wrong about which pin is which, it will help protect your Arduino and Tx)
-Plug PPM ground from your Tx into Arduino GND

*/

/*
===================================================================================================
  LICENSE & DISCLAIMER
  Copyright (C) 2014 Gabriel Staples.  All right reserved.
  
  This code was written entirely at home, during my own personal time, and is neither a product of work nor my employer.
  Furthermore, unless otherwise stated, it is owned entirely by myself.
  
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


void set_up_PPM_Reader()
{
  //initialize some of the PPM_Reader global variables with zeros
  
  //fill in chs with zeros
  for (int i=0; i < max_num_chs; i++){
    chs[i] = 0;
  } 
  //fill in gaps with zeros
  gaps[0] = 0;
  gaps[1] = 0;
  
  //configure Timer2 for 0.5us precision using my code found here: http://electricrcaircraftguy.blogspot.com/2014/02/Timer2Counter-more-precise-Arduino-micros-function.html
  setup_T2();
  
  //set up interrupt
  attachInterrupt(0,PPM_interrupt_triggered,FALLING); //PPM_Reader uses External Interrupt 0, which is on Arduino Digital Pin 2
}


//External Interrupt Function, on Arduino Digital Pin 2
//This is the main function which reads in the PPM signal
void PPM_interrupt_triggered()
{
  static unsigned long start_t = 0; //Timer2_Count (0.5us increments); initialize start time
  static unsigned long end_t = 0; //Timer2_Count (0.5us increments); initialize end time
  static char index = 0; //initialize the channel index
  static unsigned long PPM_start_t = 0; //Timer2_Count (0.5us increments); initialize start time of PPM train
  
  //update time
  end_t = get_T2_count(); //do first since T2_count WILL increment even in interrupts, and we want the time as near as possible to when the interrupt was called.
  
  //update Tx_on_check
  Tx_on_check = millis(); //ms; this is a time value which will be used to detect whether or not the Tx is even on
  
  //verify pin is low, to ensure it wasn't just low noise (<~4us in length) that triggered the interrupt
  if (!digitalRead(input_pin)) //if pin is low
  {
    unsigned int dt = end_t - start_t; //Timer2_Count (0.5us increments); channel length
    if (dt/2 >= 2500){ //if it is a large gap (>= 2500us) between sets of outputting all channels; remember, dt has units of 0.5us, hence the factor of 2
                       //a large gap (> ~2500us) means that the last channel in the set has just been read, and the next channel to be read is channel 1 again
      
      //find the PPM period
      PPM_pd = end_t - PPM_start_t; //0.5us counts; store the PPM period
      PPM_start_t = end_t; //0.5us counts; update the start time as a new PPM cycle begins
      
      num_chs = index; //store the index value (which is now the # of channels read in during the last full PPM read)
      index = 0; //reset the channel index
      gaps[1] = dt; //store the width BETWEEN sets of PPM channel pulses 
                    //Be careful w/arrays!!! - putting in an index value out of bounds broke my code for quite some time & was hard to catch...:(
      
    }
    else { //the gap is normal, so it is channel data, so store the data!
      chs[index] = dt; //store the channel value
      index++; //increment
      
      //in case no PPM end gap is found, force index to remain in bounds
      if (index >= max_num_chs){
        index = 0; //reset
      }
    }
  }
  //update times
  start_t = end_t;
}
