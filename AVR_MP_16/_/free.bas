'for calculating of free tacts
'needs some debug info from RS-232

CLS
DIM c AS STRING * 1

OPEN "rs232.bin" FOR BINARY AS 1
OPEN "log.txt" FOR OUTPUT AS 2

mn = 1000000!
mx = 0
sm = 0
n = 0

DO UNTIL EOF(1)
GET 1, , c
GET 1, , c
GET 1, , c: l = ASC(c)
GET 1, , c: h = ASC(c)
f = (h * 256 + l)

IF f > 500 AND f < 30000 THEN

n = n + 1
sm = sm + f
av = sm / n

IF f < mn THEN mn = f
IF f > mx THEN mx = f

PRINT #2, f, "min:"; mn, "max:"; mx, "avg:"; av
'SLEEP

END IF

LOOP
CLOSE

