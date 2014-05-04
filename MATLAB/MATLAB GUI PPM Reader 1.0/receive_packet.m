function [data_is_good, packet] = receive_packet(s)

%Function outputs
%data_is_good is a boolean to tell if the packet data is corrupted or not; it is true if the data is NOT corrupted
%packet is a structure containing the received data

%Function input
%s is the handle to the serial object


%Begin the Function

%wait for entire packet to arrive
packet_length = 42; %bytes
while s.BytesAvailable < packet_length
    %do nothing, just loop until the whole packet comes in
end

%once s.BytesAvailable >= packet_length (ie: once the whole packet has come in), we do the following

%read in the packet
for i = 1:1:packet_length
    packet_byte_array(i) = uint8(fread(s,1,'uint8'));
end

%convert packet bytes to values

%Note: the packet structure is as follows:
% //place data into the structure
% unsigned int packet_start; //an arbitrary, but fixed number to ensure data integrity
% unsigned long packet_num; //The packet number
% unsigned long t; //ms; a time-stamp
% unsigned int ch1; //us; channel 1 value, read out of the PPM signal
% unsigned int ch2; //us; channel 2 value, read out of the PPM signal
% unsigned int ch3; //us; channel 3 value, read out of the PPM signal
% unsigned int ch4; //us; channel 4 value, read out of the PPM signal
% unsigned int ch5; //us; channel 5 value, read out of the PPM signal
% unsigned int ch6; //us; channel 6 value, read out of the PPM signal
% unsigned int ch7; //us; channel 7 value, read out of the PPM signal
% unsigned int ch8; //us; channel 8 value, read out of the PPM signal
% unsigned int PPM_gap; //us; the gap between PPM pulse trains
% unsigned int PPM_pd; //us; the total period of each PPM pulse train
% float PPM_freq; //Hz; the frequency of the PPM trains
% byte checksum; //a byte-by-byte XOR checksum of all of the above information, including the packet_start
% unsigned int packet_end; //an arbitray, but fixed number to ensure data integrity

%First, let's check the starting and ending #'s to see if they are what we
%expect; if they are not what we expect, we can automatically know the data
%is bad, and we will throw it away
packet_start = typecast(packet_byte_array(1:2),'uint16');
packet_end = typecast(packet_byte_array(packet_length-1:packet_length),'uint16');
%we also need to read the checksum value, perform our own XOR checksum, and
%compare the two.  If they are the same, the data is most likely good, else
%the data is bad.
checksum_received = typecast(packet_byte_array(packet_length-2),'uint8');
%do our own checksum on the data packet
checksum = uint8(0); %initialize to zero before we start
for i = 1:1:packet_length-3
    checksum = bitxor(checksum,packet_byte_array(i),'uint8'); %this is a standard bitwise XOR checksum
end

%is the packet good?
if packet_start==8322 && packet_end==59858 && checksum==checksum_received %if the starting #, ending #, and checksum are all good, then the data is presumed to be good
    data_is_good = true;
    
    %since the data is good, read in the remaining packet values as a structure
    packet.packet_num = typecast(packet_byte_array(3:6),'uint32');
    packet.t = typecast(packet_byte_array(7:10),'uint32');
    j = 11; %starting # in the packet_byte_array
    for i = 1:1:8 %for all channels
        packet.chs(i,1) = typecast(packet_byte_array(j:j+1),'uint16'); %parse out the channel data
        j = j + 2; %increment
    end    
    packet.PPM_gap = typecast(packet_byte_array(27:28),'uint16');
    packet.pd = typecast(packet_byte_array(29:30),'uint16');
    packet.PPM_freq = typecast(packet_byte_array(31:34),'single');
    packet.t_since_interrupt = typecast(packet_byte_array(35:38),'uint32');
    packet.Tx_on = typecast(packet_byte_array(39:39),'uint8');
    
else
    data_is_good = false;
    
    %since data is bad, don't receive it in; just retain old values instead
end

%now that you've read in and processed a whole packet, request the Arduino
%to send you another packet (by sending it an 'R'), so that there can be a
%packet in the MATLAB serial in buffer waiting for you next time the code
%comes around to grab more data; note: you *could* choose to request data
%right when you need it, but then you might be inadvertently introducing
%some delay as the Arduino has to grab and send the data before MATLAB will
%have it, so automatically requesting new data is the fastest way to do it,
%even though it means you're always grabbing data from the Arduino 1 iteration old
%essentially.
% fwrite(s,'R','uint8'); %write an 'R' to the Arduino, thereby requesting that the Arduino send over another data packet to MATLAB

end %end of function