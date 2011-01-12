@ECHO OFF
"C:\Program Files\Atmel\AVR Tools\AvrAssembler2\avrasm2.exe" -S "c:\@\AVR_MP_16\labels.tmp" -fI -W+ie -C V2E -o "c:\@\AVR_MP_16\AVR_MP_16.hex" -d "c:\@\AVR_MP_16\AVR_MP_16.obj" -e "c:\@\AVR_MP_16\AVR_MP_16.eep" -m "c:\@\AVR_MP_16\AVR_MP_16.map" "c:\@\AVR_MP_16\AVR_MP_16.asm"
