function s = setup_serial(com_port,baud_rate,handles)

%Function inputs:
%com_port = the com port string, ex: 'COM24'
%baud_rate = the serial baud rate value, ex: 115200;

%Description:
%To open serial coms between the Arduino and MATLAB, we will use this
%process: 1) You click "Run" in MATLAB.  This initializes a serial object in MATLAB, which, upon trying to connect to the Arduino, 
%causes the Arduino to reset. As the Arduino resets, it enters its setup() function.  2) The Arduino then sends an 'a', ONE single time, in its setup()
%function, to MATLAB, then goes into a loop waiting to get an 'a' back. 3) MATLAB meanwhile goes into a loop waiting to get an 'a', then once it
%gets the ONE single 'a' that the Arduino sent, it sends an 'a' back to the Arduino to tell the Arduino that it received the 'a'.  4) Now that the
%Arduino has both sent an 'a' TO MATLAB *and* received an 'a' back from MATLAB, it goes into its main loop()function and begins running its 
%normal routine.  Two-way communication has been established!

%Reminder notes to self: "fread" reads in BINARY data in the way ("precision") you
%specify, whereas "fscanf" or "sscanf" read in STRING data in the numerical
%format you specify. Similarly, "fwrite" writes out BINARY data, whereas
%"fprintf" writes out a string according to the format you specify.

%Serial notes: 
%-the serial buffer in MATLAB is 512 bytes.  You can see this
% by running this code, hitting ctrl+c to kill the program, and typing "s"
% into the workspace to call up the state of the serial object "s" that we
% create just below.  "BytesAvailable" is one of the parameters it shows, and 
% I have watched this value (the serial buffer) fill up to 512 bytes
% then stop incrementing, so I know that's how big it is.
%-The BytesAvailableFcn is very useful; it is a MATLAB interrupt function
% which can be automatically called whenever a certain # of bytes are
% available.  Google it for more info. from the mathworks site.
%-where the serial object below is called s, s.BytesAvailable will display
% the # of bytes available in the serial in buffer.  Type s to see other properties
% which you can ask for too.

tic; %start a timer

%First, create a serial object
s = serial(com_port,'BaudRate',baud_rate,'Parity','none','DataBits',8,'StopBits',1,'terminator','LF'); %construct a serial port object with appropriate settings
    %Note: the 'LF' terminator is a Line Feed terminator, which is an ASCII 10, or '\n' (this is the final character after an Arduino
    %Serial.println() command -- see the Arduino reference page on Serial.println() for details).
fopen(s); %connect the serial port object to the serial port; ie: connect MATLAB (via the "s" object) to the Arduino
a = 'b'; %this is a character to receive over serial; initialize as anything but 'a'

counter = 0; %just to see, out of curiosity, how many loop counts it takes to get the 'a' from the Arduino
while (a~='a') %continue looping until we receive an 'a' from the Arduino
    a = fread(s,1,'uint8'); %read from serial port (s), a single byte, in 8 bit unsigned format
    counter = counter + 1;
end

str1 = sprintf('Loop counts required to receive the ''a'' char from the Arduino = %d\n',counter); %display it
fprintf(str1); %print to workspace

%since we are past the above loop, it means we have now received an 'a'
%from the Arduino, so we must send an 'a' back.

fwrite(s,'a','uint8'); %write an 'a' back
% fprintf(s,'%c','a'); %this line would work too; write over serial port, as a single character, the letter a

str2 = sprintf('Two-way serial communication established between Arduino & MATLAB.\n');
fprintf(str2) %print to workspace

str3 = sprintf('This took %f seconds.\n',toc);
fprintf(str3); %print to workspace

%display string in GUI Serial data text box
str2print = [get(handles.serial_info,'String'),sprintf('Done!\n'),str1,str2,str3];
set(handles.serial_info,'String',str2print);

end %end of function


