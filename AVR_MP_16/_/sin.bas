OPEN "sin.txt" FOR INPUT AS 1
OPEN "sin.asm" FOR OUTPUT AS 2
FOR a = 1 TO 8
PRINT #2, "			.db	";
FOR b = 1 TO 8
INPUT #1, a$
PRINT #2, a$; ",";
NEXT b
PRINT #2,
NEXT a
CLOSE

