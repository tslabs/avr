DIM c AS STRING * 1
OPEN "1.wav" FOR BINARY AS 1
FOR a = 65 TO LOF(1) STEP 2
GET 1, a, c
c = CHR$(ASC(c) AND 192)
PUT 1, a, c
NEXT a
CLOSE

