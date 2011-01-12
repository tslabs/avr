@ECHO OFF
"C:\Program Files\Atmel\AVR Tools\AvrAssembler2\avrasm2.exe" -S "c:\@\PI_reg\labels.tmp" -fI -W+ie -C V2E -o "c:\@\PI_reg\PI_reg.hex" -d "c:\@\PI_reg\PI_reg.obj" -e "c:\@\PI_reg\PI_reg.eep" -m "c:\@\PI_reg\PI_reg.map" "c:\@\PI_reg\PI_reg.asm"
