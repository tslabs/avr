DIM c AS STRING * 1

OPEN "mod.mod" FOR BINARY AS 1
OPEN "mod.asm" FOR OUTPUT AS 2

DO WHILE NOT EOF(1)
PRINT #2, CHR$(9); CHR$(9); CHR$(9); CHR$(9); CHR$(9); ".db ";

FOR a = 1 TO 8
GET 1, , c: b$ = HEX$(ASC(c))
PRINT #2, "0x"; b$;
IF a < 8 THEN PRINT #2, ",";
NEXT a
PRINT #2,

LOOP
CLOSE

END
