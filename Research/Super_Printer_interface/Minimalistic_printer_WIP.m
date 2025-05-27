clc
clear

disp('-----------------------------------------------------------')
disp('|Beware, this code is for GNU Octave ONLY !!!             |')
disp('-----------------------------------------------------------')

pkg load instrument-control

% === Setup Serial Port ===
arduinoObj = serialport('COM9','baudrate',115200, 'Parity', 'none', 'Timeout', 2);
configureTerminator(arduinoObj, "CR");  % Sets terminator to CR (carriage return)
pause(2);  % Give Arduino time to initialize

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

% === Send Print Packet ===
printPayload = uint8([65, 66, 67]);  % 'A', 'B', 'C'
printPacket = [uint8('P'), printPayload, uint8(13)];  % CR = 13
sendPacketAndConfirm(arduinoObj, printPacket);

% === Send Data Packet ===
dataPayload = uint8(randi([0, 255], 1, 640));  % 640 random bytes
dataPacket = [uint8('D'), dataPayload, uint8(13)];
sendPacketAndConfirm(arduinoObj, dataPacket);

arduinoObj=[];
