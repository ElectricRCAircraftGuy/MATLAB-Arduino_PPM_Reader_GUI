function close_serial()
%% Close and delete any and all open serial ports in MATLAB
com_objs = instrfindall; %Note: "instrfindall" will display all pertinent information about any open serial port
if ~isempty(com_objs)
    fclose(com_objs); %close this serial COM port connection, but leave the serial object in existence
    delete(com_objs); %delete the serial object too
end

end %end of function