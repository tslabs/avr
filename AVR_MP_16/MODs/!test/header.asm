.listmac
.list

#if			chip==16
#include	"m16def.inc"
#elif		chip==128
#include	"m128def.inc"
#endif

.macro		if_no_xm
#if	xmem==0
			@0,@1
#endif
.endmacro

.macro		if_xm
#if	xmem==1
			@0,@1
#endif
.endmacro

.macro		if_xm_
#if	xmem==1
			@0
#endif
.endmacro


//----------------- Symbols Definitions -----------------

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

#if			chip==16
#define		L_DAC 	PORTB			;For mega16
#define		L_DAC.b	3
#define		L_DAC_d	DDRB
#define		R_DAC 	PORTD
#define		R_DAC.b	7
#define		R_DAC_d	DDRD

#elif		chip==128
#define		L_DAC 	PORTB			;For mega128
#define		L_DAC.b	4
#define		L_DAC_d	DDRB
#define		R_DAC 	PORTB
#define		R_DAC.b	7
#define		R_DAC_d	DDRB
#endif

#define		L_OCR	OCR0
#define		R_OCR	OCR2
#define		L_TCCR	TCCR0
#define		R_TCCR	TCCR2



//----------------- Calculations -----------------

#define		p_int	(int(f_clk*1000/f_dac))	;INT period, CLK's
#define		p_qnt	(int(f_dac*125/100))	;Period of QUANT, samples, at BPM=1 div'ed by 2 to fit 16
											;(at BPM=125, once upon 1/50 sec
											;the effects are processed, QUANT procedure)
#define		def_spd	6						;Default speed
#define		def_tmp	125						;Default tempo
#define		P_LEN	64						;Len of pattern
#define		N_CHN	4						;Number of channels


//----------------- Definitions of MOD structures -----------------

#define		M_SAM_TAB	(MOD*2+20)
#define		MST_LEN		22
#define		MST_FT		24
#define		MST_VOL		25
#define		MST_REP		26
#define		MST_RLEN	28
#define		MST_SIZE	30		;Size of STable
#define		N_MST		31		;Number of samples

#define		M_LEN		(MOD*2+950)

#define		M_POS_TAB	(MOD*2+952)
#define		N_MPT		128

#define		M_MAGIC		(MOD*2+1080)
#define		M_PAT1		(MOD*2+1084)

//------------------ Index Variables Table definition --------------

#define		VOL		0			;.byte 1	;Current volume in channel (should be kept MUL'ed by 2!!!)
#define		N_VOL	(VOL+1)		;.byte 1	;New volume
#define		S_FAD	(N_VOL+1)	;.byte 2	;Sample addr (fractional part)
#define		S_AD	(S_FAD+2)	;.byte 3	;Sample addr
#if xmem==1
#define		S_REP	(S_AD+3)	;.byte 3	;Sample rep start addr
#define		S_EN	(S_REP+3)	;.byte 3	;Sample end addr
#define		N_AD	(S_EN+3)	;.byte 3	;Sample NEW addr
#define		N_REP	(N_AD+3)	;.byte 3	;Sample NEW rep addr
#define		N_EN	(N_REP+3)	;.byte 3	;Sample NEW end addr
#define		S_INC	(N_EN+3)	;.byte 3	;Sample increments
#else
#define		S_REP	(S_AD+2)	;.byte 2	;Sample rep start addr
#define		S_EN	(S_REP+2)	;.byte 2	;Sample end addr
#define		N_AD	(S_EN+2)	;.byte 2	;Sample NEW addr
#define		N_REP	(N_AD+2)	;.byte 2	;Sample NEW rep addr
#define		N_EN	(N_REP+2)	;.byte 2	;Sample NEW end addr
#define		S_INC	(N_EN+2)	;.byte 3	;Sample increments
#endif
#define		N_INC	(S_INC+3)	;.byte 3	;Sample NEW increments (if bit23=1 then skip increment update)
#define		SAMP	(N_INC+3)	;.byte 1	;Last used sample
#define		PER		(SAMP+1)	;.byte 2	;Current sample period
#define		APG		(PER+2)		;.byte 1	;Parameters for Arpeggio
#define		APG_F	(APG+1)		;.byte 1	;Flags for Arpeggio: bits1:0 - counter (0 to 2)
#define		SLIDE	(APG_F+1)	;.byte 1	;Current +/- slide value for sample period
#define		SL_SG	(SLIDE+1)	;.byte 1	;Slide sign (0-pos, 1-neg)
#define		LP_R	(SL_SG+1)	;.byte 1	;Loop row
#define		LP_C	(LP_R+1)	;.byte 1	;Loop counter
#define		IVT_SIZE (LP_C+1)	;Size of IVT


//----------------- SRAM variables ---------------------

.dseg
.org SRAM_START

FREE:		.byte 2

IVT0:		.byte IVT_SIZE
IVT1:		.byte IVT_SIZE
IVT2:		.byte IVT_SIZE
IVT3:		.byte IVT_SIZE

S_LEN:		.byte 1			;Song length
N_PAT:		.byte 1			;Number of patterns
POS:		.byte 1			;Current song position
N_POS:		.byte 1			;New song position
;PAT:		.byte 1			;Current pattern
ROW:		.byte 1			;Current row in pattern
N_ROW:		.byte 1			;New row in pattern
SPEED:		.byte 1			;Current speed, default = 6
TEMPO:		.byte 1			;Current tempo, default = 125
N_QNT:		.byte 2			;Current number of samples per quant
QNT_C:		.byte 2			;Counter of samples left per quant (BPM parameter)
TICK_C:		.byte 1			;Counter of ticks (speed parameter)
SUF:		.byte 1			;Sample Update Flags: if bit=1, update needed
							;(bit3 - CH3, ..., bit0 - CH0)
PUF:		.byte 1			;'Position Changed' flag
							;If an effect has done any row/position change action, flag is set,
							;so ROW/POS autoincrement at the end of the row is skipped
							;bit0 - Bxx or Dxx triggered; bit1 - E6x triggered
							;bit2 - N_ROW was set by Dxx; bit3 - N_POS was set by Bxx

#if	xmem==1
#define 	ST_SIZE	9		;Size of the table for 24 bit
#else
#define 	ST_SIZE	6		;Size of the table for 16 bit
#endif

S_TAB:		.byte ST_SIZE*(N_MST+1)	;Table of sample addresses
;.equ 		S_ADR =	S_TAB+0	;Start
;.equ		S_REP =	S_TAB+2/3	;Rep
;.equ		S_EN =	S_TAB+4/6	;End

S_VOL_TAB:	.byte 1*(N_MST+1)	;Table of sample default volumes
S_FT_TAB:	.byte 1*(N_MST+1)	;Table of sample Finetune values

