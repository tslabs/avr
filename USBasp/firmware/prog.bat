set AD=..\..\utils\avrdude

REM "%AD%\avrdude.exe" -V -B 1 -C "%AD%\avrdude.conf" -c usbtiny -p m16 -U flash:w:"main.hex":a -U hfuse:w:0xD9:m -U lfuse:w:0xCE:m -U lock:w:0x3F:m -u -q -y
"%AD%\avrdude.exe" -V -B 1 -C "%AD%\avrdude.conf" -c usbtiny -p m16 -U flash:w:"main.hex":a -U hfuse:w:0xC9:m -U lfuse:w:0xEF:m -U lock:w:0x3F:m -u -q -y
if ERRORLEVEL 1 pause
