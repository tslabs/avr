
;AVR Terminal User Interface
;	v.001
;
;(c)2010 TS-Labs inc.


//----------------- Setup -----------------

#define		chip	128						;16 - ATmega16; 128 - ATmega128
#define		f_clk	19000					;Main CLK, kHz
#define		baud	115200					;RS232 USART Baud Rate
#define		ubrr	(f_clk*1000/16/baud-1)

//----------------- Header -----------------

#if			chip==16
#include	"m16def.inc"
#elif		chip==128
#include	"m128def.inc"
#endif

#define		a		r16				;8 bit GPR
#define		b		r17				;8 bit GPR
#define		c		r18				;8 bit GPR
#define		d		r19				;8 bit GPR
#define		e		r20				;8 bit GPR
#define		f		r21				;8 bit GPR
#define		i		r22				;8 bit GPR
#define		j		r23				;8 bit GPR
#define		wl		r24				;16 bit GPR's LSB
#define		wh		r25				;16 bit GPR's MSB

#if			chip==16

#elif		chip==128

;USART
#define		UBRRH	UBRR1H
#define		UBRRL	UBRR1L
#define		UDR		UDR1
#define		UCSRA	UCSR1A
#define		UCSRB	UCSR1B
#define		UCSRC	UCSR1C
#define		UDRE	UDRE1
#define		RXCIE	RXCIE1
#define		TXCIE	TXCIE1
#define		UDRIE	UDRIE1
#define		RXEN	RXEN1
#define		TXEN	TXEN1
#define		UCSZ0	UCSZ10

;SPI
#define		P_SPI	PORTB
#define		DDR_SPI	DDRB
#define		S_SS	0
#define		S_SCK	1
#define		S_MOSI	2
#define		S_MISO	3
#endif

;NAND
#define		N_O		PORTA
#define		N_I		PINA
#define		N_IOD	DDRA
#define		N_CR	PORTD
#define		N_CRD	DDRD
#define		N_CRI	PIND
#define		N_RB	0
#define		N_CE	1
#define		N_RE	4
#define		N_WE	5
#define		N_ALE	6
#define		N_CLE	7

;#define		n_pg_sz	2048+64					;NAND Page Size
#define		n_pg_sz	512+16					;NAND Page Size (low)
;#define		n_ppb	64						;Pages per Block
#define		n_ppb	32						;Pages per Block (low)
#define		n_sz	2						;NAND Size, Gb

#if			n_sz==1
#define		N_ADRB	N_ADR2
#else
#define		N_ADRB	N_ADR3
#endif

#if			n_sz==1
#define		N_ADRPB	N_ADR4
#else
#define		N_ADRPB	N_ADR5
#endif

;SPI Flash Commands
#define		FC_ID	$9F
#define		FC_RD	$03
#define		FC_WR	$02
#define		FC_RDS	$05
#define		FC_WRS1	$01
#define		FC_WRS2	$31
#define		FC_WE	$06

;NAND Flash Commands
#define		NC_RS	$FF
#define		NC_ID	$90
#define		NC_RDS	$70
#define		NC_RD1	$00
#define		NC_RD2	$30
#define		NC_WR1	$80
#define		NC_WR2	$10
#define		NC_ER1	$60
#define		NC_ER2	$D0

//----------------- Macro -----------------

.macro		msg
			ldi zl,low(@0*2)
			ldi zh,high(@0*2)
			rcall U_MSG
.endmacro

.macro		tb
			ldi a,@0
			rcall T_BYTE
.endmacro

.macro		ldiwz
			ldi zl,low(@0)
			ldi zh,high(@0)
.endmacro

.macro		nce_on
			cbi N_CR,N_CE
.endmacro

.macro		nce_off
			sbi N_CR,N_CE
.endmacro

.macro		ncle_on
			sbi N_CR,N_CLE
.endmacro

.macro		ncle_off
			cbi N_CR,N_CLE
.endmacro

.macro		nale_on
			sbi N_CR,N_ALE
.endmacro

.macro		nale_off
			cbi N_CR,N_ALE
.endmacro

.macro		nio_o
			ser a
			out N_IOD,a
.endmacro

.macro		nio_i
			clr a
			out N_IOD,a
			out N_O,a
.endmacro

.macro		nre
			cbi N_CR,N_RE
			nop
			nop
			in @0,N_I
			sbi N_CR,N_RE
.endmacro

.macro		nwe
			out N_O,@0
			cbi N_CR,N_WE
			sbi N_CR,N_WE
.endmacro

//----------------- Main Program -----------------

.cseg
.org 		0
			rjmp MAIN

#if			chip==16
.org 		URXCaddr
#elif		chip==128
.org 		URXC1addr
#endif
			rjmp U_RX

#if			chip==16
.org 		UTXCaddr
#elif		chip==128
.org 		UTXC1addr
#endif
			rjmp U_TX

MAIN:
			ldi a, high(RAMEND)
			out SPH, a
			ldi a, low(RAMEND)	;Main program start
			out SPL, a 			;Set Stack Pointer to top of RAM

			rcall U_INIT
;			rcall S_INIT
			rcall N_INIT
			sei

			msg M_INTRO
;			msg M_HELP

/*			rcall F_ID			;Read SPI Flash ID
			mov a,r0
			rcall HEXS
			mov a,r1
			rcall HEXS
			mov a,r2
			rcall HEXS
			mov a,r3
			rcall HEXS
			rcall CR

			ldi b,0
			rcall F_WS1			;Unprotect SPI Flash sectors

			rcall F_RDS			;Read SPI Flash status
			mov a,r0
			rcall HEXS
			mov a,r1
			rcall HEXS
			rcall CR

			clr zh
			clr zl
			clr yh
			ldi i,16
MN2:		rcall F_RD
			rcall HEXS
			inc yh
			dec i
			brne MN2
			rcall CR
;*/
			rcall N_RS
			rcall N_ID

			mov a,r0
			rcall HEXS
			mov a,r1
			rcall HEXS
			mov a,r2
			rcall HEXS
			mov a,r3
			rcall HEXS
			mov a,r4
			rcall HEXS
			rcall CR
			rcall CR

			ldi f,$00
			ldi e,$00
			ldi d,$00

;			rcall N_TST
			rcall N_DUMP

/*
MN1:
			rcall N_RD
			rcall HOME
			rcall DUMP_SRAM

			rjmp MN1
;*/
			rjmp PC


;-------------------- USART ------------------

U_INIT:		;USART Initialisation
			ldi a,high(ubrr)				;Set Baud Rate
			sts UBRRH,a
			ldi a,low(ubrr)
			sts UBRRL,a

			ldi a,(1<<RXCIE)|(1<<TXCIE)|(1<<RXEN)|(1<<TXEN)		;Enable Rx and Tx, RXC Interrupt
			sts UCSRB,a

			ldi a,(3<<UCSZ0)				;Set frame format: 8 data, 1 stop bit
			sts UCSRC,a
			
			clr a
			sts T_AD,a
			sts R_AD,a
			sts T_SZ,a
			sts R_SZ,a
			ret
			
U_RX:		;USART Rx INT Handler
;			ret
			push a
			in a,SREG
			push a

			lds a,R_SZ
			inc a
			breq UR1				;No room to receive
			sts R_SZ,a

			push xl
			push xh
			
			ldi xl,low(R_BUF)
			ldi xh,high(R_BUF)
			lds a,R_AD
			inc a
			sts R_AD,a
			dec a
			add xl,a
			clr a
			adc xh,a

			lds a,UDR
			st X,a
			sts UDR,a
;			rcall HEX
;			rcall SPC

			pop xh
			pop xl
UR1:		pop a
			out SREG,a
			pop a
			reti

U_TX:		;USART Tx INT Handler
;			ret
			push a
			in a,SREG
			push a

			lds a,T_SZ
			tst a
			breq UT1				;Nothing to send
			dec a
			sts T_SZ,a

			push xl
			push xh
			
			ldi xl,low(T_BUF)
			ldi xh,high(T_BUF)
			lds a,T_AD
			inc a
			sts T_AD,a
			dec a
			add xl,a
			clr a
			adc xh,a

			ld a,X
			sts UDR,a					;Put data byte into out buffer, sends the data

			pop xh
			pop xl
UT1:		pop a
			out SREG,a
			pop a
			reti

CR:			;CR+LF Transmit
;			ret
			ldi zl,low(CRLF*2)
			ldi zh,high(CRLF*2)

U_MSG:		;USART Send Message
UM2:
			lpm a,Z+
			tst a
			breq UM1
			rcall T_BYTE
			rjmp UM2
UM1:
			ret

SPC:		ldi a,$20

T_BYTE:		;Put byte to Tx buffer
;			ret
			lds b,T_SZ
			inc b
			breq T_BYTE					;If T_BUF is full, then wait

			cli
			lds b,T_SZ
			tst b
			brne TB2					;If T_BUF is not empty, normal flow

			lds r0,UCSRA				;Check for empty transmit buffer
			sbrc r0,UDRE
			rjmp TB3					;If transmit finished, go to simpler write to UDR

TB2:		inc b
			sts T_SZ,b

			lds xl,T_AD
			add b,xl
			dec b
			ldi xl,low(T_BUF)
			ldi xh,high(T_BUF)
			add xl,b
			clr b
			adc xh,b

			st X,a
			
			lds b,T_SZ
			dec b
			brne TB1

TB3:		sts UDR,a					;Put data byte into out buffer, sends the data

TB1:		reti

HEXS:		;Print HEX Number + SPC
;			ret
			rcall HEX
			rjmp SPC

HEX:		;Print HEX Number
;			ret
			push a
			andi a,$f0
			swap a
			rcall HX1
			pop a
			andi a,$0f

HX1:		cpi a,10
			brcs HX2
			subi a,-$07
HX2:		subi a,-$30
			rjmp T_BYTE

HEX_PG:		;Print Page number
			mov a,f
			rcall HEX
			mov a,e
			rcall HEX
			mov a,d
			rjmp HEXS

DUMP_SRAM:	;Dump of SRAM
			push i
			push j

			ldi zl,low(PG_BUF)
			ldi zh,high(PG_BUF)
			ldi j,33
;			ldi j,20
DM1:
			mov c,zl
			subi c,low(PG_BUF)
			ldi b,high(PG_BUF)
			mov a,zh
			sbc a,b
			rcall HEX
			mov a,c
			rcall HEXS
			
			ldi i,16
;			ldi i,64
;			ldi i,32
DM2:
			ld a,Z+
			rcall HEX
			dec i
			brne DM2

			tb $d
			tb $a
			dec j
			brne DM1
			
			pop j
			pop i
			ret

HOME:		tb $a
			ldi i,22
HM1:		tb $1b
			tb $5b
			tb $41
			dec i
			brne HM1
			ret

;------------------- SPI ---------------------

S_INIT:		;SPI Init
			sbi P_SPI,S_SS						;Set /SS to 1
			sbi DDR_SPI,S_SS
			sbi DDR_SPI,S_MOSI
			sbi DDR_SPI,S_SCK					;Set MOSI and SCK output

			ldi a,(1<<SPE)|(1<<MSTR)|(0<<SPR1)|(0<<SPR0)	;Enable SPI Master, SCK=fck/2
			out SPCR,a
			ldi a,(1<<SPI2X)
			out SPSR,a
			ret

F_CMD:		;SPI Flash Command Start (Assert /SS, issue CMD)
			cbi P_SPI,S_SS

S_TX:		;Transmit byte by SPI (a - byte to Tx, on exit: a - Rx'ed byte)
			out SPDR,a							;Start transmission of data
ST1:		sbis SPSR,SPIF						;Wait for transmission complete
			rjmp ST1
			in a,SPDR
			ret

F_CMD2:		sbi P_SPI,S_SS
			ret

F_ADR:		;Set SPI Flash Addr (zh:zl:yh - A23..A0)
			mov a,zh
			rcall S_TX
			mov a,zl
			rcall S_TX
			mov a,yh
			rjmp S_TX

F_ID:		;SPI Flash Device ID (r0-r3)
			ldi a,FC_ID
			rcall F_CMD
			rcall S_TX
			mov r0,a
			rcall S_TX
			mov r1,a
			rcall S_TX
			mov r2,a
			rcall S_TX
			mov r3,a
			rjmp F_CMD2

F_WE:		;Write Enable
			ldi a,FC_WE
			rcall F_CMD
			rjmp F_CMD2

F_RDS:		;Read SPI Flash Status Reg (r0-r1)
			ldi a,FC_RDS
			rcall F_CMD
			rcall S_TX
			mov r0,a
			rcall S_TX
			mov r1,a
			rjmp F_CMD2

F_WS1:		;Write SPI Flash Status Reg1 (b)
			rcall F_WE
			ldi a,FC_WRS1
			rcall F_CMD
			mov a,b
			rcall S_TX
			rjmp F_CMD2

F_WS2:		;Write SPI Flash Status Reg2 (b)
			rcall F_WE
			ldi a,FC_WRS2
			rcall F_CMD
			mov a,b
			rcall S_TX
			rjmp F_CMD2

F_RD:		;SPI Flash Read (a)
			ldi a,FC_RD
			rcall F_CMD
			rcall F_ADR
			rcall S_TX
			rjmp F_CMD2

F_WR:		;SPI Flash Write Array (zh:zl:yh - A23..A0)
			rcall F_WE
			ldi a,FC_WR
			rcall F_CMD
			rcall F_ADR
			ldi a,$55
			rcall S_TX
			ldi a,$AA
			rcall S_TX
			ldi a,$78
			rcall S_TX
			ldi a,$4E
			rcall S_TX
			rcall F_CMD2

FW1:		rcall F_RDS		;Wait till Flash is ready
			sbrc r0,0
			rjmp FW1
			ret


;------------------- NAND ---------------------

N_INIT:		;NAND Port Init
			ldi a,(1<<N_CE)|(1<<N_RE)|(1<<N_WE)|(0<<N_CLE)|(0<<N_ALE)|(1<<N_RB)
			out N_CR,a
			ldi a,(1<<N_CE)|(1<<N_RE)|(1<<N_WE)|(1<<N_CLE)|(1<<N_ALE)|(0<<N_RB)
			out N_CRD,a
			ret

N_RBW:		;NAND Ready Wait
;			ret
			sbis N_CRI,N_RB
			rjmp N_RBW
			ret

N_CMD:		;NAND Command Latch (b)
			nio_o
			ncle_on
			nwe b
			nio_i
			ncle_off
			ret

N_ADR2:		;NAND Address Latch 2 bytes (Row - e:d)
			nio_o
			nale_on
			nwe d
			nwe e
			nale_off
			nio_i
			ret

N_ADR3:		;NAND Address Latch 3 bytes (Row - f:e:d)
			nio_o
			nale_on
			nwe d
			nwe e
			nwe f
			nale_off
			nio_i
			ret

N_ADR4:		;NAND Address Latch 4 bytes (Row - e:d, Col - wh:wl)
			nio_o
			nale_on
			clr a
			nwe a
			nwe a
			nwe d
			nwe e
			nale_off
			nio_i
			ret

N_ADR5:		;NAND Address Latch 5 bytes (Row - f:e:d, Col - wh:wl)
			nio_o
			nale_on
			clr a
			nwe a
			nwe a
			nwe d
			nwe e
			nwe f
			nale_off
			nio_i
			ret

N_ADRL:		;NAND Address Latch Low Size (0, d=A9..A16, e=A17..A24, f=A25)
			nio_o
			nale_on
			clr a
			nwe a
			nwe d
			nwe e
			nwe f
			nale_off
			nio_i
			ret

N_RS:		;NAND Reset
			nce_on
			ldi b,NC_RS
			rcall N_CMD
			nce_off
			rjmp N_RBW

N_ID:		;NAND ID Read (r0,r1,r2,r3,r4)
			nce_on
			ldi b,NC_ID
			rcall N_CMD
			nio_o
			nale_on
			clr b
			nwe b
			nale_off
			nio_i
			nre r0
			nre r1
			nre r2
			nre r3
			nre r4
			nce_off
			ret

N_RDS:		;NAND Status Read (a)
			nce_on
			ldi b,NC_RDS
			rcall N_CMD
			nre a
			nce_off
			ret

N_RD:		;NAND Read (Row - f:e:d)
			nce_on

			ldi b,NC_RD1
			rcall N_CMD
			rcall N_ADRPB
			ldi b,NC_RD2
			rcall N_CMD
			rcall N_RBW

			ldi xl,low(PG_BUF)
			ldi xh,high(PG_BUF)
			ldi wl,low(n_pg_sz)
			ldi wh,high(n_pg_sz)

NR1:		nre a
			st X+,a
			sbiw wl,1
			brne NR1

			nce_off
			ret

N_RDL:		;NAND Read (Row - f:e:d)	- Low sized chips ~64MB
			nce_on

			ldi b,NC_RD1
			rcall N_CMD
			rcall N_ADRL
			rcall N_RBW

			ldi xl,low(PG_BUF)
			ldi xh,high(PG_BUF)
			ldi wl,low(n_pg_sz)
			ldi wh,high(n_pg_sz)

NR2:		nre a
			st X+,a
			sbiw wl,1
			brne NR2

			nce_off
			ret

N_WR:		;NAND Write (Row - f:e:d)
;			ret
			nce_on
			ldi b,NC_WR1
			rcall N_CMD
			rcall N_ADRPB
			ldi xl,low(PG_BUF)
			ldi xh,high(PG_BUF)
			ldi wl,low(n_pg_sz)
			ldi wh,high(n_pg_sz)
			nio_o

NW1:		ld a,X+
			nwe a
			sbiw wl,1
			brne NW1

			ldi b,NC_WR2
			rcall N_CMD
			rjmp NE1

N_ER:		;NAND Block Erase (Row - f:e:d)
;			ret
			nce_on
			ldi b,NC_ER1
			rcall N_CMD
			rcall N_ADRB
			ldi b,NC_ER2
			rcall N_CMD
NE1:
			rcall N_RBW
			rjmp N_RDS

N_TST:		;NAND Test

			;Block Erase Test
TE1:
			msg M_BLK
			rcall HEX_PG

			msg M_ER
			rcall N_ER			;Erase Block
			rcall HEX
			rcall CR

			ldi j,n_ppb
TF1:
			rcall T_FF
			rcall T_WR
			rcall T_RD

			ldi a,1
			add d,a
			clr a		
			adc e,a
			adc f,a
			dec j
			brne TF1

			cpi f,n_sz
			brcs TE1
			ret

T_FF: 		;Page FF Test

			rcall N_RD			;Read Page

			ldi xl,low(PG_BUF)
			ldi xh,high(PG_BUF)
			ldi wl,low(n_pg_sz)
			ldi wh,high(n_pg_sz)
TF3:
			ld a,X+
			inc a
			brne TF2

			sbiw wl,1
			brne TF3

;			msg M_OK
			rjmp TF4
TF2:
			msg M_PG
			rcall HEX_PG
			msg M_FF
			msg M_ERR
;			rcall DUMP_SRAM
TF4:		ret

T_WR:		;Page Write Test

			;Fill Buffer with Test Pattern
			ldi xl,low(PG_BUF)
			ldi xh,high(PG_BUF)
			ldi wl,low(n_pg_sz)
			ldi wh,high(n_pg_sz)
			clr i
TP1:
			st X+,i
			inc i
			sbiw wl,1
			brne TP1

			rcall N_WR			;Write Page
			sbrs a,0
			ret

			push a
			msg M_PG
			rcall HEX_PG
			msg M_WR
			pop a
			rcall HEX
			rcall CR
			rcall N_RD
;			rcall DUMP_SRAM
			ret

T_RD:		;Page Read Test

			rcall N_RD			;Read Page

			ldi xl,low(PG_BUF)
			ldi xh,high(PG_BUF)
			ldi wl,low(n_pg_sz)
			ldi wh,high(n_pg_sz)
			clr i
TR3:
			ld a,X+
			cp a,i
			brne TR2
			inc i
			sbiw wl,2
			brne TR3

;			msg M_OK
			rjmp TR4
TR2:
			msg M_PG
			rcall HEX_PG
			msg M_RD
			msg M_ERR
;			rcall DUMP_SRAM
TR4:		ret

N_DUMP:
ND1:
			rcall N_RDL

			msg M_PG
			mov a,f
			rcall HEX
			mov a,e
			rcall HEX
			mov a,d
			rcall HEXS
			rcall CR
			rcall DUMP_SRAM

;			ldi a,0
			ldi a,1
			add d,a
			clr a		
			adc e,a
			adc f,a
;			subi f,-1
			cpi f,n_sz
			brcs ND1
			ret

;------------------- Text Messages --------------------

CRLF:		.db	$d,$a,0,0

M_INTRO:	.db $c,"AVR Terminal, v.001 ",$d,$a,"(c)2010 TS-Labs inc.",$d,$a,$d,$a,0

M_HELP:		.db "Commands: ",$d,$a,0,0

M_BLK:		.db "Block: ",0

M_PG:		.db "Page: ",0,0

M_ER:		.db "Erase: ",0

M_FF:		.db "FF: ",0,0

M_WR:		.db "Write: ",0

M_RD:		.db "Read: ",0,0

M_OK:		.db "Passed ",$d,$a,0

M_ERR:		.db "Error ",$d,$a,0,0

//----------------- SRAM variables ---------------------

.dseg
.org 		SRAM_START

T_BUF:		.byte	256			;Tx
T_AD:		.byte 	1
T_SZ:		.byte 	1

R_BUF:		.byte	256			;Rx
R_AD:		.byte	1
R_SZ:		.byte 	1

L_BUF:		.byte	64			;Buffer for command line

PG_BUF:		.byte	2048+64



