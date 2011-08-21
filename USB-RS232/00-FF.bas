open "testbin.txt" for output as 1
for a=0 to 255
print #1,chr$(a);
next a
close