% === Function to send and confirm echo ===
function sendPacketAndConfirm(arduinoObj, packet)
  tic
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

  while arduinoObj.NumBytesAvailable > 0 %to avoid loosing time with garbage
  discard = readline(arduinoObj);  % Clear all startup messages
  if not(isempty(strfind(discard,"Printer ready")))
    disp("✅ Printer Ready")
    read(arduinoObj, 1, "uint8");%get rid of a last lost character
  else
    disp("❌ Printer not yet ready");
  end
end

