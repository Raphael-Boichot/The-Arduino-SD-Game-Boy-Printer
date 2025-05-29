function port = detectArduino()
  ports = serialportlist();
  for i = 1:length(ports)
    portCandidate = ports{i};
    disp(['Testing ', portCandidate, '...']);
    try
      s = serialport(portCandidate,'baudrate',250000, 'Parity', 'none', 'Timeout', 2);
      pause(1);  % Give time for Arduino to send welcome message
      % Try to read any startup message
      data = readline(s);
      if not(isempty(strfind(data,"[ARDUINO_READY]")))
        disp(['✅ Arduino detected on ', portCandidate]);
        port = portCandidate;
        return;
      else
        disp(['❌ No match on ', portCandidate]);
      end
      clear s
    catch
      disp(['⚠️ Error accessing ', portCandidate]);
    end
  end
  error("❌ Arduino not detected on any port.");
  end
