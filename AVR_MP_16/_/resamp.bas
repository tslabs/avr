DIM s AS STRING * 1
DIM s1 AS STRING * 1
DIM s2 AS STRING * 1
OPEN "med.wav" FOR BINARY AS 1
OPEN "med1.wav" FOR BINARY AS 2

i = 16000 / 44100
c = 0
SEEK 1, 45
SEEK 2, 45
GET 1, , s1
GET 1, , s

FOR a = 45 TO LOF(2)

p = ASC(s): p1 = ASC(s1)
r = p1 * (1 - c) + p * c
s2 = CHR$(r)

'PRINT p, r: SLEEP

PUT 2, , s2

c = c + i
IF c >= 1 THEN
 c = c - 1
 s1 = s
 GET 1, , s
END IF

NEXT a

CLOSE

