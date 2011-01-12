CLS

OPEN "1.txt" FOR INPUT AS 1
OPEN "2.txt" FOR INPUT AS 2
OPEN "3.txt" FOR OUTPUT AS 3

INPUT #1, a1
INPUT #2, b1
c1 = INT(b1 / 2.66 * 1024)

DO UNTIL EOF(1)
INPUT #1, a
INPUT #2, b
c = INT(b / 2.66 * 1024)
'PRINT a, b, c

e = c - c1
FOR d = c1 TO c - 1
f = (d - c1) / e: g = (c - d) / e
x = INT(a1 * g + a * f + .001)
PRINT #3, , ".dw ", x, ";"; d
NEXT d

a1 = a: b1 = b: c1 = c

LOOP

CLOSE

