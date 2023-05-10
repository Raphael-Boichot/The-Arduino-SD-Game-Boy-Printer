function [packet_RX]=send_packet(packet_TX,arduinoObj)
packet_RX=zeros(length(packet_TX));
for i=1:1:length(packet_TX)
    fwrite(arduinoObj,packet_TX(i))
    packet_RX(i) = fread(arduinoObj,1);
end