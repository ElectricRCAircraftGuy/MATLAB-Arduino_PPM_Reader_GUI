function varargout = MATLAB_to_Arduino_GUI_driver(varargin)
% MATLAB_TO_ARDUINO_GUI_DRIVER MATLAB code for MATLAB_to_Arduino_GUI_driver.fig
%      MATLAB_TO_ARDUINO_GUI_DRIVER, by itself, creates a new MATLAB_TO_ARDUINO_GUI_DRIVER or raises the existing
%      singleton*.
%
%      H = MATLAB_TO_ARDUINO_GUI_DRIVER returns the handle to a new MATLAB_TO_ARDUINO_GUI_DRIVER or the handle to
%      the existing singleton*.
%
%      MATLAB_TO_ARDUINO_GUI_DRIVER('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in MATLAB_TO_ARDUINO_GUI_DRIVER.M with the given input arguments.
%
%      MATLAB_TO_ARDUINO_GUI_DRIVER('Property','Value',...) creates a new MATLAB_TO_ARDUINO_GUI_DRIVER or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before MATLAB_to_Arduino_GUI_driver_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop_btn.  All inputs are passed to MATLAB_to_Arduino_GUI_driver_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help MATLAB_to_Arduino_GUI_driver

% Last Modified by GUIDE v2.5 25-Feb-2014 10:14:12

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @MATLAB_to_Arduino_GUI_driver_OpeningFcn, ...
                   'gui_OutputFcn',  @MATLAB_to_Arduino_GUI_driver_OutputFcn, ...
                   'gui_LayoutFcn',  [] , ...
                   'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT


% --- Executes just before MATLAB_to_Arduino_GUI_driver is made visible.
function MATLAB_to_Arduino_GUI_driver_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to MATLAB_to_Arduino_GUI_driver (see VARARGIN)

% Choose default command line output for MATLAB_to_Arduino_GUI_driver
handles.output = hObject;

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes MATLAB_to_Arduino_GUI_driver wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = MATLAB_to_Arduino_GUI_driver_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;


% --- Executes on button press in start_btn.
function start_btn_Callback(hObject, eventdata, handles)
% hObject    handle to start_btn (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

%%% My Code Begins Here %%%
clc; %clear the screen

%clear the plot, in case anything remains from prev. run
cla(handles.PPM_plot,'reset');

%initialize some values for teh stop button
stop_btn_data.stop = false; %set stop = false since the button has NOT yet been pressed
set(handles.stop_btn,'UserData',stop_btn_data) %store the stop_btn_data into UserData
set(handles.text_output,'String','PPM Data Output') %clear the output text string

%since the button has been pushed, make it red and have the words "running" on it now
start_btn_data.color = get(handles.start_btn,'BackgroundColor'); %copy the initial color value
set(handles.start_btn,'UserData',start_btn_data); %store the color value before losing it
set(handles.start_btn,'BackgroundColor',[1 .5 .5]); %make the start_btn light red
set(handles.start_btn,'String','Running','Enable','off'); %make it change text to "Running", and disable the button
set(handles.stop_btn,'Enable','on'); %enable the stop button
set(handles.serial_info,'String','Connecting to the Arduino PPM Reader...');
drawnow; %force button state to visually update

%Call the main code script (m-file)
MATLAB_to_Arduino



% --- If Enable == 'on', executes on mouse press in 5 pixel border.
% --- Otherwise, executes on mouse press in 5 pixel border or over start_btn.
function start_btn_ButtonDownFcn(hObject, eventdata, handles)
% hObject    handle to start_btn (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --- Executes on button press in stop_btn.
function stop_btn_Callback(hObject, eventdata, handles)
% hObject    handle to stop_btn (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

stop_btn_data.stop = true; %set stop = true since the button has been pressed
set(handles.stop_btn,'UserData',stop_btn_data) %store the stop_btn_data into UserData

%turn the start button back to normal, re-enabling it, while disabling the stop button
start_btn_data = get(handles.start_btn,'UserData');
set(handles.start_btn,'BackgroundColor',start_btn_data.color);
set(handles.start_btn,'String','Start','Enable','on');
set(handles.stop_btn,'Enable','off');
drawnow; %force button redraw


% --- Executes during object creation, after setting all properties.
function text_output_CreateFcn(hObject, eventdata, handles)
% hObject    handle to text_output (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called



function com_port_Callback(hObject, eventdata, handles)
% hObject    handle to com_port (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of com_port as text
%        str2double(get(hObject,'String')) returns contents of com_port as a double


% --- Executes during object creation, after setting all properties.
function com_port_CreateFcn(hObject, eventdata, handles)
% hObject    handle to com_port (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end
