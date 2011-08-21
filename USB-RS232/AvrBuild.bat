@ECHO OFF
"C:\Program Files\Atmel\AVR Tools\AvrAssembler2\avrasm2.exe" -S "C:\@\USB-RS232\labels.tmp" -fI -W+ie -C V2 -o "C:\@\USB-RS232\USB-RS232.hex" -d "C:\@\USB-RS232\USB-RS232.obj" -e "C:\@\USB-RS232\USB-RS232.eep" -m "C:\@\USB-RS232\USB-RS232.map" "C:\@\USB-RS232\USBtiny2313.asm"
