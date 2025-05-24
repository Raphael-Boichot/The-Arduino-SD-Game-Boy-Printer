clc;
clear;

disp('-----------------------------------------------------------')
disp('|Beware, this code is for GNU Octave ONLY !!!             |')
disp('-----------------------------------------------------------')

pkg load instrument-control
pkg load image

%-------------------------------------------------------------
palette=0xE4;%any value is possible
intensity=0x40;%0x00->0x7F, 0x40 is default
margin=1;%0 before margin, 3 after margins, used between images

packet_lenght=640;
DATA = uint8(rand*0xFF*ones(1,packet_lenght));
global arduinoObj
list = serialportlist;
valid_port=[];
protocol_failure=1;
for i =1:1:length(list)
    disp(['Testing port ',char(list(i)),'...'])
    arduinoObj = serialport(char(list(i)),'BaudRate',115200);
    set(arduinoObj, 'timeout',2);
    flush(arduinoObj);
    response=char(read(arduinoObj, 100));
    if ~isempty(response)
        disp(['Arduino detected on port ',char(list(i))])
        valid_port=char(list(i));
        beep ()
        protocol_failure=0;
    end
    arduinoObj=[];
end
if protocol_failure==0
    arduinoObj = serialport(valid_port,'baudrate',115200,'parity','none','timeout',255); %set the Arduino com port here
    s.Terminator = 'CR';  % Carriage return
    response='';
    while (isempty(strfind(response,'connected')))
        response=char(ReadToTermination(arduinoObj));
    end
    disp('Proooouuuuut')
    fwrite(arduinoObj,'P');
    %fwrite(arduinoObj,margin);
    %fwrite(arduinoObj,palette);
    %fwrite(arduinoObj,intensity);
    arduinoObj=[];
else
    disp('No device found, check connection with the Arduino !')
    disp('// If you''re using the Game Boy Printer emulator at:')
    disp('// https://github.com/mofosyne/arduino-gameboy-printer-emulator')
    disp('// switch the printer ON before connecting the Arduino')
    disp('// It has to detect a valid printer to boot in printer interface mode')
end
