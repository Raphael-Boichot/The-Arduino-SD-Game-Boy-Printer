function [output]=add_checksum(input)
checksum=sum(input(3:end));
LSB=rem(checksum,256);%OK
MSB=rem(((checksum-LSB)/256),256);
output=[input,LSB,MSB,0,0];
