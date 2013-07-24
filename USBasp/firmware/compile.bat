make clean
make main.hex
avr-objdump -h -S main.bin > main.lst
if ERRORLEVEL 1 pause
