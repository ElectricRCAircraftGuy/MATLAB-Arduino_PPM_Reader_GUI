//MATLAB_to_Arduino
//By Gabriel Staples, using code snippets from Rich Roberts, who got a lot of information and examples online
//19 Feb. 2014

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

#include "PPM_Reader.h" //mandatory to put here; this makes the PPM Reader's global variables instantiated in the right spot (here at the top)
                        //so that they can be visible to everybody, including the code below


//Constants & Global Variables
const int LED = 13; //this is the Arduino built-in LED
char a = 'b'; //this is the character used to receive incoming serial commands from MATLAB; initialize to anything but 'a'


//--------------------------------------------------------------------------------------
//Packet Building
//--------------------------------------------------------------------------------------

const unsigned int packet_start = 8322; //make a unique number to begin our packet (for error checking)
const unsigned int packet_end = 59858; //make a unique number to end our packet (for error checking)

//define a structure data type, typedefined as a "packet_struct_t" type (note: the t at the end stands for "typedef")
typedef struct __attribute__((__packed__)) packet_struct //create a structure datatype called "packet_struct"; use the "packed" attribute, which prevents it from aligning the structure datatypes 
                                                           //with physical memory boundaries and padding it with zeros between alignments
{
  //place data into the structure
  unsigned int packet_start; //an arbitrary, but fixed number to ensure data integrity
  unsigned long packet_num; //The packet number
  unsigned long t; //ms; a time-stamp
  unsigned int ch1; //us; channel 1 value, read out of the PPM signal
  unsigned int ch2; //us; channel 2 value, read out of the PPM signal
  unsigned int ch3; //us; channel 3 value, read out of the PPM signal
  unsigned int ch4; //us; channel 4 value, read out of the PPM signal
  unsigned int ch5; //us; channel 5 value, read out of the PPM signal
  unsigned int ch6; //us; channel 6 value, read out of the PPM signal
  unsigned int ch7; //us; channel 7 value, read out of the PPM signal
  unsigned int ch8; //us; channel 8 value, read out of the PPM signal
  unsigned int PPM_gap; //us; the gap between PPM pulse trains
  unsigned int PPM_pd; //us; the total period of each PPM pulse train
  float PPM_freq; //Hz; the frequency of the PPM trains
  unsigned long t_since_interrupt; //ms; the total elapsed time since the last interrupt occured; this is useful to be able to tell if the Tx is even on
  byte Tx_on; //a single value, acting like a boolean, to tell whether or not the Tx is on (ie: whether or not the Arduino is even receiving a PPM signal)
  byte checksum; //a byte-by-byte XOR checksum of all of the above information, including the packet_start
  unsigned int packet_end; //an arbitray, but fixed number to ensure data integrity
  
} packet_struct_t; //set the type definition NAME for this "packet_struct" datatype to be "packet_struct_t."  Note: t is a conventional way to mean data "Type", so I add the t here to help me remember that 
                   //"packet_struct_t" is now a type-defined datatype, or a "typedef"

//no need to instantiate the above structure at all, as we will be using it within a union, and instantiating a union only; so, on to the definition of the union we want to use! 

//define a union
//this is a type definition for a union; note: a union allows us to read the same memory location in different ways (ie: interpreting the same data in different ways; for ex, we can read a 4-byte array as 
//4 individual bytes, or as 1 float)
typedef union packet_union //create a union datatype called "packet_union"
{
  byte byte_array[]; //make a byte array with the same # of bytes as what are contained in a data packet
  packet_struct_t packet; //having the structure in the union in parallel with the byte_array allows us to interpret the data in the byte_array as the structure dictates
} packet_union_t; //set the type definition NAME for this "packet_union" datatype to be "packet_union_t."

//instantiate an instance of the above union
packet_union_t union1; //instantiate an instance of this new packet_union_t datatype
//get the size, in bytes, of our data packet
const uint8_t packet_length = sizeof(union1); //this is the size of a data packet, in bytes


//--------------------------------------------------------------------------------------
//Start of Primary Functions, such as setup() and loop()
//--------------------------------------------------------------------------------------

void setup()
{
  //initialize the union we instantiated above
  union1.packet.packet_start = packet_start; //the arbitrary, unique number to begin our packet (for error checking)
  
  union1.packet.packet_num = 0; //initialize the packet number, which we will increment each time we send a data packet
  union1.packet.t = millis(); //ms; current time stamp for when packet was assembled
  union1.packet.ch1 = 0; //us; channel 1 value, read out of the PPM signal
  union1.packet.ch2 = 0; //us; channel 2 value, read out of the PPM signal
  union1.packet.ch3 = 0; //us; channel 3 value, read out of the PPM signal
  union1.packet.ch4 = 0; //us; channel 4 value, read out of the PPM signal
  union1.packet.ch5 = 0; //us; channel 5 value, read out of the PPM signal
  union1.packet.ch6 = 0; //us; channel 6 value, read out of the PPM signal
  union1.packet.ch7 = 0; //us; channel 7 value, read out of the PPM signal
  union1.packet.ch8 = 0; //us; channel 8 value, read out of the PPM signal
  union1.packet.PPM_gap = 0; //us; the gap time after the last channel, and between full PPM trains
  union1.packet.PPM_pd = 0; //us; the period of the entire PPM train (which contains pulse width data for ALL channels)
  union1.packet.PPM_freq = 0; //Hz; the frequency of the PPM trains
  union1.packet.t_since_interrupt = 0; //ms; the total elapsed time since the last interrupt occured; this is useful to be able to tell if the Tx is even on
  union1.packet.Tx_on = false; //a single value, acting like a boolean, to tell whether or not the Tx is on (ie: whether or not the Arduino is even receiving a PPM signal)
  
  union1.packet.checksum = 0; //a byte-by-byte XOR checksum of all of the above information, including the packet_start; just initialize as zero
  union1.packet.packet_end = packet_end; //the arbitrary, unique number to end our packet (for error checking)
  
  //Set up PPM_Reader code and Serial communication
  set_up_PPM_Reader(); //Note: this function sets up my Timer2_Counter code by calling "setup_T2()"--so that I can get 0.5us precision on a custom timer
                       //which I use to read in the PPM signal.
  //open serial
  Serial.begin(250000); //According to Table 20-7 in the ATmega328 datasheet, pg. 193, the max. synchronous (U2Xn=0) serial baud rate that an Arduino (w/16MHz clock) can do is 1Mbps
  
  //Establish Serial Communication w/MATLAB
  
  //First, send a single 'a' character to MATLAB
  Serial.write('a');//send character via serial port to MATLAB, to begin verification of connectivity; I choose not to use Serial.println() here because it adds a 
  //carriage return character (ASCII 13, or '\r') and a newline character (ASCII 10, or '\n') to the end of the data, which I don't want.
  //See reference page on Serial.println() for details.
  while(a!='a'){ //keep looping until the Arduino receives an 'a' back from MATLAB; once an 'a' is received, continue on
    a = Serial.read(); //read a single byte from Arduino's hardware serial buffer; note: if nothing is in the buffer, this will return -1
  }  
  
  //Setup LED 13
  pinMode(LED,OUTPUT);
  
  //TEST CODE BELOW, COMMENT OUT OR DELETE WHEN DONE
//  unsigned long t = millis(); //ms; time now
//  t = 13123455; //define to a known number just for testing; COMMENT OUT WHEN DONE.
//  byte* p_t = (byte*)&t; //establish a byte pointer, p_t, to t
//  for (int i=0; i < 4; i++){
//    Serial.write(p_t[i]);
//  }
    
//  Serial.write(t); //send the current ms time to MATLAB (4 bytes, unsigned long); THIS DOESN'T WORK; RATHER, I MUST USE A FOR LOOP TO SEND ONE BYTE AT A TIME, EITHER WITH
//                   //THE HELP OF A POINTER, OR WITH THE HELP OF STRUCTURES & UNIONS! (ie: manually-built packets)
}


void loop(){
  //--------------------------------------------------------------------------------------
  //LED 13 Code, to blink LED 13, indicating we are in the Main loop
  //--------------------------------------------------------------------------------------
  
  static unsigned long start_t = millis(); //ms; initialize
  static boolean LED_state = LOW; //initialize
  
  //control LED 13 to blink to indicate we are in the main loop and it is working
  if (millis() - start_t >= 100){ //if 100ms has elapsed
    start_t = millis(); //update the start time
    LED_state = !LED_state; //toggle LED 13
    digitalWrite(LED,LED_state);
  }
  
  //--------------------------------------------------------------------------------------
  //Send a whole data packet to MATLAB when requested by MATLAB 
  //MATLAB sends a request command by means of an "R" to the Arduino
  //--------------------------------------------------------------------------------------
  
  //Set Up Local PPM_Reader Variables (for the main loop() function)
  static unsigned long start_time = millis();
  static unsigned int chs_cpy[max_num_chs] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //prepare a COPY buffer for channel pulse width data
  static unsigned int gaps_cpy[2] = {0, 0};
  static unsigned int PPM_pd_cpy = 0; //units of 0.5us; contains the PPM period as determined by the external interrupt code
  
  //Import the latest PPM channel values, and Print them to the Serial Monitor 
  //--do this once first to load data into MATLAB's serial buffer, then from that point on, only do it if MATLAB has requested data by sending an "R" to the Arduino
  a = Serial.read(); //read a single byte from Arduino's hardware serial buffer; note: if nothing is in the buffer, this will return -1
  if (a=='R') //if this is true, it means MATLAB has just sent a request for a data packet!
  {      
    //get and output data
    unsigned char num_chs_cpy = num_chs; //copy the # of channels variable; note: since this is a 1-byte variable, I do not need to call noInterrups() right before, 
                                         //and interrupts() right after.
    
    //detect if the Tx is even on
    noInterrupts(); //disable interrupts for critical section of code (ie: in this case, reading a multi-byte variable which can be changed at any time in the ISR)
    unsigned long Tx_on_check_cpy = Tx_on_check; //ms; this is a time value which will be used to detect whether or not the Tx is even on
    interrupts(); //re-enable interrupts
    boolean Tx_on = true; //initialize as true
    unsigned long t_since_interrupt = millis() - Tx_on_check_cpy; //ms; the time elapsed since the last time the external interrupt was triggered
    if (t_since_interrupt >= 25){ //if more than ___ms has elapsed since the last time the interrupt was triggered
      Tx_on = false; //the Tx must be off
    }
    
    //copy the channel values
    for (int i=0; i < num_chs_cpy; i++){
      noInterrupts(); //disable interrupts for critical section of code (ie: in this case, reading a multi-byte variable which can be changed at any time in the ISR)
      chs_cpy[i] = chs[i];
      interrupts(); //re-enable interrupts
    }
    
    //copy the PPM gap value    
    for (int i=0; i < 2; i++){
      noInterrupts(); //disable interrupts for critical section of code (ie: in this case, reading a multi-byte variable which can be changed at any time in the ISR)
      gaps_cpy[i] = gaps[i]; //grab the PPM gap values, which occur 1) between each channel, and 2) after the final channel and before the next PPM train
      interrupts(); //re-enable interrupts
    }
    
    //copy the PPM pd value
    noInterrupts(); //disable interrupts for critical section of code (ie: in this case, reading a multi-byte variable which can be changed at any time in the ISR)
    PPM_pd_cpy = PPM_pd;
    interrupts(); //re-enable interrupts
    
    //convert from 0.5us unit counts to us (microseconds)
    gaps_cpy[0] /= 2; //us; the gap time between channels
    gaps_cpy[1] /= 2; //us; the gap time after the last channel, and between full PPM trains
    PPM_pd_cpy /= 2; //us; the period of the entire PPM train (which contains pulse width data for ALL channels)
    for (int i=0; i < num_chs_cpy; i++){
      chs_cpy[i] = chs_cpy[i]/2; //convert from 0.5us unit counts to us
    }
    
    //get PPM_freq
    float PPM_freq = 1/(PPM_pd_cpy*1e-6); //Hz
    
    //Store the values into our union data type which contains our packet structure
    //Note: the beauty of the union is that after I store the data into it via its ".packet" member, I can read out the data byte-by-byte
    //      via its ".byte_array" member, as you will see when we send the data to MATLAB
    union1.packet.packet_num++; //increment the packet number
    union1.packet.t = millis(); //ms; current time stamp for when packet was assembled
    union1.packet.ch1 = chs_cpy[0]; //us
    union1.packet.ch2 = chs_cpy[1]; //us
    union1.packet.ch3 = chs_cpy[2]; //us
    union1.packet.ch4 = chs_cpy[3]; //us
    union1.packet.ch5 = chs_cpy[4]; //us
    union1.packet.ch6 = chs_cpy[5]; //us
    union1.packet.ch7 = chs_cpy[6]; //us
    union1.packet.ch8 = chs_cpy[7]; //us
    union1.packet.PPM_gap = gaps_cpy[1]; //us; the gap time after the last channel, and between full PPM trains
    union1.packet.PPM_pd = PPM_pd_cpy; //us; the period of the entire PPM train (which contains pulse width data for ALL channels)
    union1.packet.PPM_freq = PPM_freq; //Hz; the frequency of the PPM trains
    union1.packet.t_since_interrupt = t_since_interrupt; //ms; the total elapsed time since the last interrupt occured; this is useful to be able to tell if the Tx is even on
    union1.packet.Tx_on = Tx_on; //a single value, acting like a boolean, to tell whether or not the Tx is on (ie: whether or not the Arduino is even receiving a PPM signal)
        
    //send the data to MATLAB, over serial, byte by byte, one byte at a time
    union1.packet.checksum = 0; //reset the checksum byte before beginning
    for (int i=0; i < packet_length; i++)
    {
      //Error checking:
      //perform the checksum on the data packet (note: all but the last 3 bytes of the data packet will be included in the checksum; 
      //the last 3 bytes include the checksum itself, 1 byte, and the packet_end number, 2 bytes)
      if (i < packet_length-3)
      {
        union1.packet.checksum ^= union1.byte_array[i]; //this is a basic XOR checksum
      }
      Serial.write(union1.byte_array[i]); //this is where we actually send the data over to MATLAB, via serial, one byte at a time
    }
  } //end of if (a=='R') statement
  
  
} //end of loop() function








