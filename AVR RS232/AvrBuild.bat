@ECHO OFF
"C:\Program Files\Atmel\AVR Tools\AvrAssembler2\avrasm2.exe" -S "c:\@\RS232\labels.tmp" -fI -W+ie -C V2E -o "c:\@\RS232\RS232.hex" -d "c:\@\RS232\RS232.obj" -e "c:\@\RS232\RS232.eep" -m "c:\@\RS232\RS232.map" "c:\@\RS232\RS232.asm"
