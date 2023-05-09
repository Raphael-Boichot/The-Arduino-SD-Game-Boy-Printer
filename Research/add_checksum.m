function [output]=add_checksum(input)
checksum=sum(input(3:end));
MSB=uint8(checksum/256);
LSB=rem(checksum,256);
output=[input,LSB,MSB,0,0];