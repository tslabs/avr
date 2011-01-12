;PI-Regulator
;	v.001
;("Last cry")
;
;(c)2010 TS-Labs inc.

//----------------- Setup -----------------

#define		f_clk	8000					;Main CLK, kHz
#define		n_sec	4						;Period of PWM, sec
#define		p_led	4						;Display Refresh Period, CLK's
#define		k_del	1500					;Start delay for a key
#define		k_rep	250						;Start repeat period for a key
#define		d_rep	5						;Decrement for repeat period for a key
#define		m_rep	20						;Minimal repeat period for a key
#define		q_delay 8500					;Delay for menu quit timeout
#define		a_delay 10000					;Delay for save annoying timeout

#define		t0_min	-40
#define		t0_max	300
#define		ti_min	10
#define		ti_max	600
#define		kp_min	5
#define		kp_max	300

#define		k_t0	7	
#define		k_ti	6
#define		k_kp	5
#define		k_mm	4
#define		k_save	3
#define		k_up	0
#define		k_down	2

//----------------- Calculations -----------------

#define		presc	1024					;Prescaler value (for calc's, NOT for setup)
#define		f_pwm	(f_clk*1000/presc)		;PWM clock freq, Hz
;#define	p_pwm	(int(f_pwm*n_sec))		;PWM period, CLK's
#define		p_pwm	320						;PWM period / 100, CLK's
#define		sec_p	f_clk*1000*8/presc/p_led	;1 sec period for LED INT
#define		sec_d	8


//----------------- Symbols Definitions -----------------

#define		nul		r13				;predefined 0
#define		dl		r14				;16 bit accumulator's LSB
#define		dh		r15				;16 bit accumulator's MSB
#define		a		r16				;8 bit accumulator
#define		b		r17				;8 bit accumulator
#define		c		r18				;8 bit accumulator
#define		i		r19				;8 bit counter (20 years on counting scene! :)
#define		cl		r20				;16 bit accumulator's LSB
#define		ch		r21				;16 bit accumulator's MSB
#define		bl		r22				;16 bit accumulator's LSB
#define		bh		r23				;16 bit accumulator's MSB
#define		al		r24				;16 bit accumulator's LSB
#define		ah		r25				;16 bit accumulator's MSB

//----------------- Macro Definitions -----------------

#include	"m8def.inc"

.macro		sti
			ldi a,@1
			sts @0,a
.endmacro

.macro		stiw
			ldi a,low(@1)
			sts @0,a
			ldi a,high(@1)
			sts @0+1,a
.endmacro

.macro		ldaw
			lds al,@0
			lds ah,@0+1
.endmacro

.macro		staw
			sts @0,al
			sts @0+1,ah
.endmacro

.macro		ldzw
			lds zl,@0
			lds zh,@0+1
.endmacro

.macro		stzw
			sts @0,zl
			sts @0+1,zh
.endmacro

.macro		staa
			sts @0,a
			sts @0+1,a
.endmacro

.macro		cpaw
			cpi al,low(@0)
			brne L1
			cpi ah,high(@0)
			brne L1
			rjmp pc+2
L1:
.endmacro

.macro		eepr
			ldi b,@0
			rcall E_RD
.endmacro

.macro		eepw
			ldi b,@0
			rcall E_WR
.endmacro


//----------------- Code Segment -----------------

.cseg
.org 		0
			rjmp MAIN

.org		OC2addr
			rjmp INT_LED
			or R2,R0
			or R22,R3
			cpi R19,0x02
			cpi R19,0x01

.org		OVF1addr
			rjmp INT_PWM
			subi r18,0x40 
			mov r21,r3 
			ori r20,0x1c 
			andi r22,0x32
			and r2,r12 

.org		ADCCaddr
			rjmp INT_ADC
			ori r18,0x90 
			ori r22,0x3e 
			and r2,r14 


.org INT_VECTORS_SIZE

//----------------- Tables -----------------

DIGS:		;Common wires of digits (anodes/cathodes) highlight
			.db 0b110000,0b101000,0b011000,0b111001,0b111010,0b111100
FONT:		;Font for numbers
			.db 250,34,185,171,99,203,219,162,251,235	;"0123456789"
			.db 243,91,216,59,217,209					;"AbCdEF"

#define		D_MNS	1	;"-"
#define		D_DP	4	;"."
#define		D_SPC	0	;" "

MENU_KEY:					;Submenu's key event vectors
			rjmp MK_0		;Main
			rjmp MK_1		;T0
			rjmp MK_2		;ti
			rjmp MK_3		;Kp

//----------------- Main Program -----------------

MAIN:
			ldi a, high(RAMEND)
			out SPH, a
			ldi a, low(RAMEND)	;Main program start
			out SPL, a 			;Set Stack Pointer to top of RAM

			clr nul
			rcall P_INIT
			rcall V_INIT
			rcall MI_0

			sei
			rjmp PC

MI_0:		;Main menu init
			sti MNU,0
			stiw M_DEL,0
			stiw Q_DEL,0
			sti FLF,0

			lds a,MM
			tst a
			ldi b,5
			ldi c,1
			brne MI0_1

			ldi b,-1
			clr c
MI0_1:
			sts DP,b
			sts FLD,c

			lds al,PW
			clr ah
			rjmp NUM2

MI_1:		;T0 menu init
			sti MNU,1
			sti DP,-1
			stiw M_DEL,1
			stiw Q_DEL,q_delay

			ldi b,E_T0
			rcall TMP_RD
			rjmp NUM2

MI_2:		;ti menu init
			sti MNU,2
			sti DP,4
			stiw M_DEL,1
			stiw Q_DEL,q_delay

			ldi b,E_TI
			rcall TMP_RD
			rjmp NUM2

MI_3:		;Kp menu init
			sti MNU,3
			sti DP,4
			stiw M_DEL,1
			stiw Q_DEL,q_delay

			ldi b,E_KP
			rcall TMP_RD
			rjmp NUM2

MI_MM:		;MM Switch
			lds a,MM
			tst a
			brne MM1

			;Auto > Man
			lds a,PW
			eepw E_PW
			ser a
MM2:
			sts MM,a
			eepw E_MM
			rjmp MI_0

MM1:		;Man > Auto
			clr a
			rjmp MM2

MK_0:		;Main menu keys handling
			lds b,MM
			tst b
			breq M01			;If mode=Auto, no keys polling

			lds b,PW
			sbrc a,k_up
			rjmp M0_UP			;PW++
			sbrc a,k_down
			rjmp M0_DW			;PW--

M01:		ret

M0_UP:		cpi b,100			;INC PW in mode=Manual
			breq M01
			inc b
			rjmp M02

M0_DW:		tst b				;DEC PW in mode=Manual
			breq M01
			dec b
M02:		sts PW,b

			mov al,b
			clr ah
			rcall NUM2
			ret

MK_1:		;T0 menu keys handling
			ldaw TMP
			sbrc a,k_up
			rjmp M1_UP
			sbrc a,k_down
			rjmp M1_DW
			sbrc a,k_save
			rjmp M1_SV
			ret

MK_2:		;ti menu keys handling
			ldaw TMP
			sbrc a,k_up
			rjmp M2_UP
			sbrc a,k_down
			rjmp M2_DW
			sbrc a,k_save
			rjmp M2_SV
			ret

MK_3:		;Kp menu keys handling
			ldaw TMP
			sbrc a,k_up
			rjmp M3_UP
			sbrc a,k_down
			rjmp M3_DW
			sbrc a,k_save
			rjmp M3_SV
			ret

M1_SV:		ldi b,E_T0
			rjmp TMP_WR

M2_SV:		;Check if TT isn't greater than 'ti and null TT if yes

			lds bl,TT
			lds bh,TT+1
			push al
			push ah
			add al,al
			adc ah,ah
			mov cl,al
			mov ch,ah
			add al,al
			adc ah,ah
			add al,al
			adc ah,ah
			cp bl,al
			cpc bh,ah
			brcs M2_SV1
			clr a
			staa TT
M2_SV1:		pop ah
			pop al
			ldi b,E_TI
			rjmp TMP_WR

M3_SV:		ldi b,E_KP
			rjmp TMP_WR

M1_UP:		cpaw t0_max
			adiw al,1
			rjmp MU1

M1_DW:		cpaw t0_min
			sbiw al,1
MU1:		staw TMP
			sti FLF,0
			stiw M_DEL,a_delay
			stiw Q_DEL,0
			rjmp NUM2

M2_UP:		cpaw ti_max
			adiw al,1
MU2:		staw TMP
			sti FLF,0
			stiw M_DEL,a_delay
			stiw Q_DEL,0
			rjmp NUM2

M2_DW:		cpaw ti_min
			sbiw al,1
			rjmp MU2

M3_UP:		cpaw kp_max
			adiw al,1
			rjmp MU1

M3_DW:		cpaw kp_min
			sbiw al,1
			rjmp MU1

KBD:							;KBD driver
			in a,PINB
			ori a,2
			com a
			ldi zl,low(k_del)
			ldi zh,high(k_del)
			brne KB4
KB6:
			ldi b,low(k_rep)
			sts RKEY,b
			ldi b,high(k_rep)
			sts RKEY+1,b
			rjmp KB1
KB4:			
			lds b,LKEY
			cp a,b
			brne KB6

			ldzw DKEY
			sbiw zl,1
			breq KB3
			clr a
			rjmp KB2
KB3:
			ldzw RKEY
			sbiw zl,d_rep
			cpi zl,low(m_rep)
			ldi b,high(m_rep)
			cpc zh,b
			brcc KB5
			ldi zl,low(m_rep)
			ldi zh,high(m_rep)
KB5:
			stzw RKEY
KB1:
			sts LKEY,a
KB2:
			stzw DKEY
			tst a
			brne K_EVT
			ret

K_EVT:							;KBD event handling
			sbrc a,k_t0
			rjmp MI_1

			sbrc a,k_ti
			rjmp MI_2

			sbrc a,k_kp
			rjmp MI_3

			sbrc a,k_mm
			rjmp MI_MM

			clr zh
			lds zl,MNU
			subi zl,-MENU_KEY
			ijmp


;-------------- Display --------------------

LED:		;Screen refresh
			ldi c,D_SPC
			lds b,DIG
			cpi b,3
			brcc LD3
			com c
LD3:		out PORTD,c			;Blank digit

			ldi zl,low(DIGS*2)
			clr zh
			add zl,b
			lpm c,Z
			out PORTC,c			;Switch common wire

			lds a,FLS
			sbrs a,7			;Check if Flash active
			rjmp LD2			;If no, highlight digit

			lds a,FLF
			cpi b,3
			brcc LD4
			sbrc a,0
			rjmp LD5
			rjmp LD2

LD4:		sbrc a,1
			rjmp LD5

LD2:		ldi zl,low(DBUF1)
			clr zh
			add zl,b
			ld c,Z				;Read font
			lds a,DP
			cp a,b
			brne LD6
			
			lds a,FLD			;DP Flash
			sbrs a,0
			rjmp LD7
			lds a,FLS
			sbrc a,7
			rjmp LD6

LD7:		ldi a,D_DP
			eor c,a				;Dec point print
LD6:		out PORTD,c			;Symbol out

LD5:		inc b
			cpi b,6
			brcs LD1
			clr b
LD1:		sts DIG,b
			
			lds a,FLS
			inc a
			sts FLS,a
			ret

MDEL:		;Screen Flashes (Delay for timeouts, etc.)

			ldaw M_DEL
			mov a,al
			or a,ah
			breq MD2			;If not set - go check quit timeout
			sbiw al,1
			staw M_DEL
			brne MD1

			sti FLF,2			;If fallen to 0 - set right flash
MD1:		ret

MD2:		ldaw Q_DEL
			mov a,al
			or a,ah
			breq MD1
			sbiw al,1
			staw Q_DEL
			brne MD1
			rjmp MI_0
			
NUM1:		;Number printing (ah:al - number)
			ldi xl,low(DBUF1)
			ldi xh,high(DBUF1)
			ser c
			rjmp NM1
NUM2:
			ldi xl,low(DBUF2)
			ldi xh,high(DBUF2)
			clr c
NM1:
			clr bh
			tst ah
			brpl NM5
			com al
			com ah
			adiw al,1
			ldi a,D_MNS
			eor a,c
			st X+,a
			rjmp NM6
NM5:
			ldi bl,100
			rcall NM2
NM6:
			ldi bl,10
			rcall NM2
			ser bh
			ldi bl,1

NM2:		;xh:xl - addr of DBUF
			;c - need invert font

			mov r3,al
			mov r4,ah
			mov r5,bl
			clr r6
			rcall DIV

			clr a
			ldi zl,low(FONT*2)
			clr zh
			add zl,r1
			lpm a,Z
			tst r1
			breq NM3
			ser bh
			rjmp NM4
NM3:
			tst bh
			brne NM4
			ldi a, D_SPC
NM4:
			eor a,c
			st X+,a

			mul bl,r1
			sub al,r0
			sbc ah,r1
			ret

;---------------- Time -------------------

TIME:
			ldaw TS
			sbiw al,sec_d
			brcc TM1

			push al
			push ah

			ldaw TT
			adiw al,1

			push al
			push ah
			ldi b,E_TI
			rcall AX_RD
			mov bl,al
			mov bh,ah
			pop ah
			pop al

			add bl,bl
			adc bh,bh
			mov cl,bl
			mov ch,bh
			add bl,bl
			adc bh,bh
			add bl,cl
			adc bh,ch
			cp al,bl
			cpc ah,bh
			brcs TM2
			
			clr al
			clr ah

TM2:		staw TT
;			rcall NUM1 		;debug!!!

			pop ah
			pop al
			ldi bl,low(sec_p)
			ldi bh,high(sec_p)
			add al,bl
			adc ah,bh

TM1:		staw TS
			ret

;---------------- Maths -------------------

FORM_CNT:
			;Formulae counting
			; The formulae is counted as:
			;
			; P = Kp * (t/ti * (x0-x) + x0-x)
			;
			; 				where:
			; P [PW] - PWM value, %
			; x0 [E_T0] - task temperature, 'C
			; x [TC] - current temperature, 'C
			; t [TT] - current time, secs
			; ti [T_TI] - isodrome time (MUST be MUL'ed by 6!), secs
			; Kp [E_KP] - proportionality coefficient (MUST be DIV'ed by 10!)

			;Get 'x0
			ldi b,E_T0
			rcall AX_RD		;ah:al

			;Get -x
			lds bl,TC
			lds bh,TC+1
			clr dl
			clr dh
			sub dl,bl
			sbc dh,bh		;dh:dl

			;Calc x0-x
			add al,dl
			adc ah,dh
			brpl FC1
			clr al
			clr ah			;if x0-x <0 then x0-x <= 0
FC1:
			push ah
			push al

			;Get 'ti
			ldi b,E_TI
			rcall AX_RD
			add al,al
			adc ah,ah		;mul 2
			mov r5,al
			mov r6,ah
			add al,al
			adc ah,ah		;mul 4
			add r5,al
			adc r6,ah		;mul 6
							;Got 'ti in ch:cl, secs
			lds r3,TT
			lds r4,TT+1
			rcall DIV		;Got t/ti => r0:r7

			mov r6,r0
			mov r5,r7
			pop r7
			pop r8
			push r8
			push r7
			rcall MULX		;r4:r3:r2 <= t/ti * (x0-x)

			pop al
			pop ah
			add r3,al
			adc r4,ah		;r4:r3:r2 (16.8) <= (t/ti * (x0-x) + x0-x)

			push r4
			push r3
			ldi b,E_KP
			rcall AX_RD
			mov r4,ah
			mov r3,al
			ldi a,10
			mov r5,a
			clr r6
			rcall DIV		;r1:r0 (8.8) <= E_KP / 10 = Kp
			mov r6,r1
			mov r5,r0
			pop r7
			pop r8
			rcall MULX		;r3:r2 (16.0) <= P

			ldi a,100
			cp a,r2
			clr a
			cpc a,r3
			mov a,r2
			brcc FC2
			ldi a,100		;If P>100 then P=100
FC2:
			ret

MULX:		;Procedure for multiplication (16)*(16)->(24)
			;r4:r3:r2 <- r6:r5 * r8:r7
			;r0,r1 - used as temp registers

			mul r6,r8
			mov r4,r1
			mov r3,r0

			mul r5,r7
			mov r2,r1
			
			mul r6,r7
			add r2,r0
			adc r3,r1
			clr r0
			adc r4,r0

			mul r5,r8
			add r2,r0
			adc r3,r1
			clr r0
			adc r4,r0
			ret

DIV:		;Procedure for division (16)/(16)->(32 = 16.16)
			;r2:r1:r0:r7 <- r4:r3 / r6:r5
			;a,b - used as temp registers

			clr r2
			clr r1
			clr r0
			clr r7
			inc r7
			clr b
			clr a
			clc
DIV1:
			rol r3
			rol r4
			rol b
			rol a
			brcs DIV2
			cp b,r5
			cpc a,r6
			brcs DIV3
DIV2:
			sub b,r5
			sbc a,r6
			sec
			rjmp DIV4
DIV3:
			clc
DIV4:
			rol r7
			rol r0
			rol r1
			rol r2
			brcc DIV1
			ret


//-------------------- Inits -----------------------

P_INIT:		;Ports init
			ldi a,p_led-1							;Set period for LED refresh
			out OCR2,a								;8 000 000 / 1024 / 4 = 1953.125 Hz
			ldi a,((1<<CS22)|(1<<CS21)|(1<<CS20))	;Set prescaler=1024 for TC2
			ori a,((1<<WGM21)|(0<<WGM20))			;Set CTC mode for TC2
			out TCCR2,a

			ldi a,high(p_pwm*100)	;Set PWM period on TC1
			out (ICR1H),a
			ldi a,low(p_pwm*100)
			out (ICR1L),a
			ldi a,((1<<WGM11)|(0<<WGM10))			;Run Fast PWM (14) mode on TC1
			ori a,((1<<COM1A1)|(0<<COM1A0))			;Set mode for OC1A pin (~~~~\____)
			out TCCR1A,a
			ldi a,((1<<WGM13)|(1<<WGM12))
			ori a,((1<<CS12)|(0<<CS11)|(1<<CS10))	;Set prescaler=1024 for TC1
			out TCCR1B,a
			clr a									;Set PW = 0
			out (OCR1AH),a
			out (OCR1AL),a
			
			ldi a,((1<<REFS1)|(1<<REFS0))
			ori a,((0<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0))
			out ADMUX,a
			ldi a,((1<<ADEN)|(1<<ADSC)|(1<<ADFR)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0))
			out ADCSRA,a

			ldi a,((1<<OCIE2)|(1<<TOIE1))	;INT's
			out TIMSK,a					

			ldi a,2				;Set PORTB.1 to output
			out DDRB,a
			ser a
			out DDRD,a			;Set PORTD (SEG's) to output
			out DDRC,a			;Set PORTC (DIG's) to output
			ret

V_INIT:		;Variables init
			clr a
			sts DIG,a
			staa AD_C
			staa AD_S
			staa AD_S+2
			staa TT
			stiw TS,sec_p

			ser a
			sts LKEY,a
			staa AD_R

			ldi a,(255-D_MNS)
			staa DBUF1
			sts DBUF1+2,a
			ldi a,D_SPC
			staa DBUF2
			sts DBUF2+2,a
			
			eepr E_MM
			sts MM,a
			eepr E_PW
			sts PW,a
			ret
			

;------------------- EEPROM --------------------

E_RD:		;EEPROM Read
			;b - Addr; a - Data

			rcall EAD
			sbi EECR,EERE
			in a,EEDR
			ret

E_WR:		;EEPROM Write
			;b - Addr; a - Data

			rcall EAD
			out EEDR,a
			sbi EECR,EEMWE
			sbi EECR,EEWE
			ret

EAD:		;EEPROM readiness wait and Address set
			sbic EECR,EEWE
			rjmp PC-1
			out EEARL,b
			clr r8
			out EEARH,r8
			ret

TMP_RD:		;Read to TMP
			rcall AX_RD
			staw TMP
			ret

AX_RD:		rcall E_RD
			mov al,a
			inc b
			rcall E_RD
			mov ah,a
			ret

TMP_WR:		;Read to TMP
			ldaw TMP
			mov a,al
			rcall E_WR
			inc b
			mov a,ah
			rcall E_WR
			rjmp MI_0


;-------------- PWM ------------------

SET_PWM:	;Calc PW > PWM (a-PW)
			mov r8,a
			clr r7
			ldi a,high(p_pwm)
			mov r6,a
			ldi a,low(p_pwm)
			mov r5,a
			rcall MULX
			out (OCR1AH),r3
			out (OCR1AL),r2
			ret


;------------ INT's --------------------

INT_PWM:	;PWM Interrupt handling

			lds r4,AD_S+3		;Count for avg ADC value
			lsr r4
			lds r4,AD_S+2
			ror r4
			lds r3,AD_S+1
			ror r3
			lds r6,AD_C+1
			lsr r6
			lds r5,AD_C
			ror r5
			rcall DIV
			sts AD_R,r0			;Store ADC avg
			sts AD_R+1,r1

			ldi zl,low(TAB*2)	;Recalc ADC to Temp.
			ldi zh,high(TAB*2)
			add zl,r0
			adc zh,r1
			add zl,r0
			adc zh,r1
			lpm al,Z+
			lpm ah,Z
			staw TC				;Store T value
			rcall NUM1			;Print T on LED1
			
			clr a				;Null ADC avg vars
			staa AD_C
			staa AD_S
			staa AD_S+2

			lds a,MM			;Read MM status
			tst a
			brne IP1			;Go to Manual PW value

			;Auto Mode

			rcall FORM_CNT
			sts PW,a

IP1:		;Manual Mode
			lds a,MNU
			tst a
			brne IP2
								;If Menu=0 then PW->LED2
			lds al,PW
			clr ah
			rcall NUM2
IP2:
			lds a,PW
			rcall SET_PWM
			reti

INT_ADC:	;ADC Handler
			lds r0,AD_S			;Add ADC value to avg summator
			lds r1,AD_S+1
			lds r2,AD_S+2
			lds r3,AD_S+3
			clr a
			in al,ADCL
			in ah,ADCH
			add r0,al
			adc r1,ah
			adc r2,a
			adc r3,a
			sts AD_S,r0
			sts AD_S+1,r1
			sts AD_S+2,r2
			sts AD_S+3,r3

			ldaw AD_C			;INC avg counter
			adiw al,1
			staw AD_C
			reti

INT_LED:	;User Interface
			rcall MDEL			;"Timeout" flashes
			rcall KBD			;KBD handle
			rcall LED			;Screen refresh
			rcall TIME			;Time counter
			reti


//----------------- External Tables -----------------

TAB:		.include "tab.asm"


//----------------- SRAM Variables -----------------

.dseg
.org SRAM_START

DIG:	.byte 1			;Current LED digit number (0-5)
DBUF1:	.byte 3			;Display Buffer 1
DBUF2:	.byte 3			;Display Buffer 2
FLS:	.byte 1			;Flash counter
FLF:	.byte 1			;Flash Enable Flags: bit0 - Disp1, bit1 - Disp2
FLD:	.byte 1			;Flash for DP (0-off/1-on)
DP:		.byte 1			;Decimal point position (0-5, 255-off)

LKEY:	.byte 1			;Last key
DKEY:	.byte 2			;Key delay countdown
RKEY:	.byte 2			;Current key repeat delay value

MNU:	.byte 1			;Current menu number
M_DEL:	.byte 2			;Menu delay countdown
Q_DEL:	.byte 2			;Menu quit delay countdown

AD_C:	.byte 2			;ADC avg counter
AD_S:	.byte 4			;ADC avg summator
AD_R:	.byte 2			;Avg ADC value
TC:		.byte 2			;Current Temperature
PW:		.byte 1			;PW value (calculated from TC (auto) or input manually)
MM:		.byte 1			;Manual Mode (0-off, 1-on)
TT:		.byte 2			;Current time within isodrome, sec
TS:		.byte 2			;Time prescaler

TMP:	.byte 2
TMP2:	.byte 2


//----------------- EEPROM Variables -----------------

.eseg

E_T0:	.dw 100			;T0 value
E_TI:	.dw 1000		;ti value (mm.m) - .m is kept as 1/10 of minute, i.e. 6 sec!
E_KP:	.dw 1			;Kp value (xx.x) - values should be DIV'ed by 10, e.g. '200' is 20.0
E_MM:	.db 1			;Manual Mode (0-off, 1-on)
E_PW:	.db 1			;Current PW

