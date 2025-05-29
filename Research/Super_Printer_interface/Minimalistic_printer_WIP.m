clc
clear

disp('-----------------------------------------------------------')
disp('|Beware, this code is for GNU Octave ONLY !!!             |')
disp('-----------------------------------------------------------')

pkg load instrument-control

%///////////////////////////////////////////////////////////////////////////////////
margin = 0x01;     %high nibble, upper margin, low nibble, lower margin, that simple
palette = 0x00;    %0x00 is treated as default (= 0xE4)
intensity = 0x7F;  %default intensity is 0x40, min is 0x00, max is 0x7F, values between 0x80 and 0xFF are treated as default
%///////////////////////////////////////////////////////////////////////////////////

% === Setup Serial Port ===
arduinoPort = detectArduino();
arduinoObj = serialport('COM9','baudrate',115200, 'Parity', 'none', 'Timeout', 2);
configureTerminator(arduinoObj, "CR");  % Sets terminator to CR (carriage return)
pause(2);  % Give Arduino time to initialize

% === Flush any previous welcome message ===
while arduinoObj.NumBytesAvailable > 0
  discard = readline(arduinoObj);  % Clear all startup messages
  if not(isempty(strfind(discard,"Printer connected !")))
    disp("✅ Printer connected")
  else
    disp("❌ Printer not yet connected");
  end
end

% === Function to send and confirm echo ===
function sendPacketAndConfirm(arduinoObj, packet)
  write(arduinoObj, packet, "uint8");  % Send packet
  pause(0.01);  % Give Arduino time to echo
  expectedLength = length(packet);
  echoed = read(arduinoObj, expectedLength, "uint8");
  if isequal(echoed, packet)
    disp("✅ Echo confirmed");
  else
    disp("❌ Echo mismatch");
    fprintf("Sent:   %s\n", mat2str(packet));
    fprintf("Echoed: %s\n", mat2str(echoed));
  end
end

for i=1:9
% === Send Data Packet ===
dataPayload = uint8(randi([0, 255], 1, 640));  % 640 random bytes
dataPacket = [uint8('D'), dataPayload, uint8(13)];
sendPacketAndConfirm(arduinoObj, dataPacket);
end

% === Send Print Packet ===
printPayload = uint8([margin, palette, intensity]);
printPacket = [uint8('P'), printPayload, uint8(13)];  % CR = 13
sendPacketAndConfirm(arduinoObj, printPacket);

arduinoObj=[];
