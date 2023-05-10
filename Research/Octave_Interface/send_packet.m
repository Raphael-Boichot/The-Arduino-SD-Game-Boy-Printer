function [packet_RX]=send_packet(packet_TX)
global arduinoObj
packet_RX=zeros(length(packet_TX));

for i=1:1:length(packet_TX)
    fwrite(arduinoObj,packet_TX(i),"uint8");
    packet_RX(i) = fread(arduinoObj,1,"uint8");
##    disp([dec2hex(packet_TX(i)),'<->',dec2hex(packet_RX(i))])
end
