;***************************************************************************
;* U S B   S T A C K   F O R   T H E   A V R   F A M I L Y
;*
;* File Name            :"USBtiny2313.asm"
;* Title                :AVR309:USB to UART protocol converter (simple - small FIFO)
;* Date                 :09.01.2006
;* Version              :1.1
;* Target MCU           :ATtiny2313
;* AUTHOR		:Ing. Igor Cesko
;* 			 Slovakia
;* 			 cesko@internet.sk
;* 			 http://www.cesko.host.sk
;*
;* DESCRIPTION:
;*  USB protocol implementation into MCU with noUSB interface:
;*  Device:
;*  Universal USB interface (8-bit I/O port + RS232 serial line + EEPROM)
;*  + added RS232 FIFO buffer
;*
;* The timing is adapted for 12 MHz crystal (overclocked MCU !!!)
;*
;*
;* to add your own functions - see section: TEMPLATE OF YOUR FUNCTION
;*
;* to customize device to your company you must change VendorUSB ID (VID)
;* to VID assigned to your company (for more information see www.usb.org)
;*
;***************************************************************************
.include "tn2313def.inc"

.equ	E2END			=127

.equ	inputport		=PINB
.equ	outputport		=PORTB
.equ	USBdirection		=DDRB
.equ	DATAplus		=1		;signal D+ on PB1
.equ	DATAminus		=0		;signal D- on PB0 - mount 1.5kOhm pull-up on this pin
.equ	USBpinmask		=0b11111100	;mask low 2 bits (D+,D-) on PB
.equ	USBpinmaskDplus		=~(1<<DATAplus)	;mask D+ bit on PB
.equ	USBpinmaskDminus	=~(1<<DATAminus);mask D- bit on PB

.equ	TSOPPort		=PINB
.equ	TSOPpullupPort		=PORTB
.equ	TSOPPin			=2		;signal OUT from IR sensor TSOP1738 on PB2

.equ	LEDPortLSB		=PORTD		;connecting LED diode LSB
.equ	LEDPinLSB		=PIND		;connecting LED diode LSB (input)
.equ	LEDdirectionLSB		=DDRD		;input/output LED LSB
.equ	LEDPortMSB		=PORTB		;LED diodes MSB
.equ	LEDPinMSB		=PINB		;LED diodes MSB  (input)
.equ	LEDdirectionMSB		=DDRB		;input/output LED MSB
.equ	LEDlsb0			=3		;LED0 to pin PD3
.equ	LEDlsb1			=5		;LED1 to pin PD5
.equ	LEDlsb2			=6		;LED2 to pin PD6
.equ	LEDmsb3			=3		;LED3 to pin PB3
.equ	LEDmsb4			=4		;LED4 to pin PB4
.equ	LEDmsb5			=5		;LED5 to pin PB5
.equ	LEDmsb6			=6		;LED6 to pin PB6
.equ	LEDmsb7			=7		;LED7 to pin PB7

.equ	SOPbyte			=0b10000000	;Start of Packet byte
.equ	DATA0PID		=0b11000011	;PID for DATA0 part
.equ	DATA1PID		=0b01001011	;PID for DATA1 part
.equ	OUTPID			=0b11100001	;PID for OUT part
.equ	INPID			=0b01101001	;PID for IN part
.equ	SOFPID			=0b10100101	;PID for SOF part
.equ	SETUPPID		=0b00101101	;PID for SETUP part
.equ	ACKPID			=0b11010010	;PID for ACK part
.equ	NAKPID			=0b01011010	;PID for NAK part
.equ	STALLPID		=0b00011110	;PID for STALL part
.equ	PREPID			=0b00111100	;PID for PRE part

.equ	nSOPbyte		=0b00000001	;Start of Packet byte - reverse order
.equ	nDATA0PID		=0b11000011	;PID for DATA0 part - reverse order
.equ	nDATA1PID		=0b11010010	;PID for DATA1 part - reverse order
.equ	nOUTPID			=0b10000111	;PID for OUT part - reverse order
.equ	nINPID			=0b10010110	;PID for IN part - reverse order
.equ	nSOFPID			=0b10100101	;PID for SOF part - reverse order
.equ	nSETUPPID		=0b10110100	;PID for SETUP part - reverse order
.equ	nACKPID			=0b01001011	;PID for ACK part - reverse order
.equ	nNAKPID			=0b01011010	;PID for NAK part - reverse order
.equ	nSTALLPID		=0b01111000	;PID for STALL part - reverse order
.equ	nPREPID			=0b00111100	;PID for PRE part - reverse order

.equ	nNRZITokenPID		=~0b10000000	;PID mask for Token packet (IN,OUT,SOF,SETUP) - reverse order NRZI
.equ	nNRZISOPbyte		=~0b10101011	;Start of Packet byte - reverse order NRZI
.equ	nNRZIDATA0PID		=~0b11010111	;PID for DATA0 part - reverse order NRZI
.equ	nNRZIDATA1PID		=~0b11001001	;PID for DATA1 part - reverse order NRZI
.equ	nNRZIOUTPID		=~0b10101111	;PID for OUT part - reverse order NRZI
.equ	nNRZIINPID		=~0b10110001	;PID for IN part - reverse order NRZI
.equ	nNRZISOFPID		=~0b10010011	;PID for SOF part - reverse order NRZI
.equ	nNRZISETUPPID		=~0b10001101	;PID for SETUP part - reverse order NRZI
.equ	nNRZIACKPID		=~0b00100111	;PID for ACK part - reverse order NRZI
.equ	nNRZINAKPID		=~0b00111001	;PID for NAK part - reverse order NRZI
.equ	nNRZISTALLPID		=~0b00000111	;PID for STALL part - reverse order NRZI
.equ	nNRZIPREPID		=~0b01111101	;PID for PRE part - reverse order NRZI
.equ	nNRZIADDR0		=~0b01010101	;Address = 0 - reverse order NRZI

						;status bytes - State
.equ	BaseState		=0		;
.equ	SetupState	=1		;
.equ	InState			=2		;
.equ	OutState		=3		;
.equ	SOFState		=4		;
.equ	DataState		=5		;
.equ	AddressChangeState	=6		;

						;Flags of required task
.equ	DoNone					=0
.equ	DoReceiveOutData			=1
.equ	DoReceiveSetupData			=2
.equ	DoPrepareOutContinuousBuffer		=3
.equ	DoReadySendAnswer			=4


.equ	CRC5poly		=0b00101		;CRC5 polynomial
.equ	CRC5zvysok		=0b01100		;CRC5 remainder after correct CRC5
.equ	CRC16poly		=0b1000000000000101	;CRC16 polynomial
.equ	CRC16zvysok		=0b1000000000001101	;CRC16 remainder after correct CRC16

.equ	MAXUSBBYTES		=14			;maximum bytes in USB input message
.equ	MAXRS232LENGTH		=36			;maximum length of RS232 code (count of ones and zeros together) (attention: MAXRS232LENGTH must be even number !!!)
.equ	NumberOfFirstBits	=10			;how many first bits allowed be longer
.equ	NoFirstBitsTimerOffset	=256-12800*12/1024	;Timeout 12.8ms (12800us) to terminate after firsts bits(12Mhz:clock, 1024:timer predivider, 256:timer overflow value)
.equ	InitBaudRate		=12000000/16/57600-1	;UART on 57600 (for 12MHz=12000000Hz)

.equ	InputBufferBegin	=RAMEND-127				;start of receiving buffer
.equ	InputShiftBufferBegin	=InputBufferBegin+MAXUSBBYTES		;start of receiving shift buffer
.equ	RS232BufferBegin	=InputShiftBufferBegin+MAXUSBBYTES	;start of buffer for RS232 receiving

.equ	MyInAddressSRAM		=RS232BufferBegin+MAXRS232LENGTH+1
.equ	MyOutAddressSRAM	=MyInAddressSRAM+1

.equ	OutputBufferBegin	=RAMEND-MAXUSBBYTES-2	;begin of transmitting buffer
.equ	AckBufferBegin		=OutputBufferBegin-3	;begin of transmitting buffer Ack
.equ	NakBufferBegin		=AckBufferBegin-3	;begin of transmitting buffer Nak

.equ	StackBegin		=NakBufferBegin-1	;bottom of stack

.def	ConfigByte		=R1		;0=unconfigured state
.def	backupbitcount		=R2		;backup bitcount register in INT0 disconnected
.def	RAMread			=R3		;if reading from SRAM
.def	backupSREGTimer		=R4		;backup Flag register in Timer interrupt
.def	backupSREG		=R5		;backup Flag register in INT0 interrupt
.def	ACC			=R6		;accumulator
.def	lastBitstufNumber	=R7		;position in bitstuffing
.def	OutBitStuffNumber	=R8		;how many bits to send last byte - bitstuffing
.def	BitStuffInOut		=R9		;if insertion or deleting of bitstuffing
.def	TotalBytesToSend	=R10		;how many bytes to send
.def	TransmitPart		=R11		;order number of transmitting part
.def	InputBufferLength	=R12		;length prepared in input USB buffer
.def	OutputBufferLength	=R13		;length answers prepared in USB buffer
.def	MyOutAddress		=R14		;my USB address (Out Packet) for update
.def	MyInAddress		=R15		;my USB address (In/SetupPacket)


.def	ActionFlag		=R16		;what to do in main program loop
.def	temp3			=R17		;temporary register
.def	temp2			=R18		;temporary register
.def	temp1			=R19		;temporary register
.def	temp0			=R20		;temporary register
.def	bitcount		=R21		;counter of bits in byte
.def	ByteCount		=R22		;counter of maximum number of received bytes
.def	inputbuf		=R23		;receiver register
.def	shiftbuf		=R24		;shift receiving register
.def	State			=R25		;state byte of status of state machine
.def	RS232BufptrX		=R26		;XL register - pointer to buffer of received IR codes
.def	RS232BufferFull		=R27		;XH register - flag of full RS232 Buffer
.def	USBBufptrY		=R28		;YL register - pointer to USB buffer input/output
.def	ROMBufptrZ		=R30		;ZL register - pointer to buffer of ROM data

;requirements on descriptors
.equ	GET_STATUS		=0
.equ	CLEAR_FEATURE		=1
.equ	SET_FEATURE		=3
.equ	SET_ADDRESS		=5
.equ	GET_DESCRIPTOR		=6
.equ	SET_DESCRIPTOR		=7
.equ	GET_CONFIGURATION	=8
.equ	SET_CONFIGURATION	=9
.equ	GET_INTERFACE		=10
.equ	SET_INTERFACE		=11
.equ	SYNCH_FRAME		=12

;descriptor types
.equ	DEVICE			=1
.equ	CONFIGURATION		=2
.equ	STRING			=3
.equ	INTERFACE		=4
.equ	ENDPOINT		=5

.equ	USER_FNC_NUMBER		=100


;------------------------------------------------------------------------------------------
;********************************************************************
;* Interrupt table
;********************************************************************
.cseg
;------------------------------------------------------------------------------------------
.org 0						;after reset
		rjmp	reset
;------------------------------------------------------------------------------------------
.org INT0addr					;external interrupt INT0
		rjmp	INT0handler
;------------------------------------------------------------------------------------------
.org URXCaddr					;receiving from serial line
		push	temp0
		in	temp0,UDR			;put to temp0 received data from UART
		sei					;enable interrupts to service USB
		in	backupSREGTimer,SREG		;backup SREG
		cbi	UCSRB,RXCIE			;disable interrupt from UART receiving
		cpi	RS232BufferFull,MAXRS232LENGTH-4
		brcc	NoIncRS232BufferFull
		push	RS232BufptrX
		lds	RS232BufptrX,RS232BufferBegin+2	;set position to begin of buffer write RS232 code : 3-th.byte of header (code length + reading + writing + reserve)
		st	X+,temp0			;and save it to buffer
		cpi	RS232BufptrX,RS232BufferBegin+MAXRS232LENGTH+1	;if not reached maximum of RS232 buffer
		brne	NoUARTBufferOverflow		;then continue
		ldi	RS232BufptrX,RS232BufferBegin+4	;otherwise set position to buffer begin
 NoUARTBufferOverflow:
		sts	RS232BufferBegin+2,RS232BufptrX	;save new offset of buffer write RS232 code : 3-th.byte of header (code length + reading + writing + reserve)
		inc	RS232BufferFull			;increment length of RS232 buffer
		pop	RS232BufptrX
 NoIncRS232BufferFull:
		pop	temp0
		out	SREG,backupSREGTimer		;restore SREG
		cli					;disable interrupt because to prevent reentrant interrupt call
		sbi	UCSRB,RXCIE			;enable interrupt from receiving of UART
		reti
;------------------------------------------------------------------------------------------
;********************************************************************
;* Init program
;********************************************************************
;------------------------------------------------------------------------------------------
reset:			;initialization of processor and variables to right values
		ldi	temp0,StackBegin	;initialization of stack
		out	SPL,temp0

		clr	XH			;RS232 pointer
		clr	YH			;USB pointer
		clr	ZH			;ROM pointer
		sts	RS232BufferBegin+0,YH	;clear lengths of RS232 code in buffer
		ldi	temp0,RS232BufferBegin+4
		sts	RS232BufferBegin+1,temp0;znuluj ukazovatel citania
		sts	RS232BufferBegin+2,temp0;znuluj ukazovatel zapisu
		clr	RS232BufferFull

		rcall	InitACKBufffer		;initialization of ACK buffer
		rcall	InitNAKBufffer		;initialization of NAK buffer

		rcall	USBReset		;initialization of USB addresses

		sbi	TSOPpullupPort,TSOPpin	;set pull-up on TSOP input

		ldi	temp0,(1<<LEDlsb0)+(1<<LEDlsb1)+(1<<LEDlsb2)
		out	LEDPortLSB,temp0	;set pull-up on all LED LSB
		ldi	temp0,(1<<LEDmsb3)+(1<<LEDmsb4)+(1<<LEDmsb5)+(1<<LEDmsb6)+(1<<LEDmsb7)
		out	LEDPortMSB,temp0	;set pull-up on all LED MSB

		sbi	PORTD,0			;set pull-up on RxD input
		ldi	temp0,InitBaudRate	;set UART speed
		out	UBRRL,temp0
		sbi	UCSRB,TXEN		;enable transmiting of UART
		sbi	UCSRB,RXEN		;enable receiving of UART
		sbi	UCSRB,RXCIE		;enable interrupt from receiving of UART

		ldi	temp0,0x0F		;INT0 - respond to leading edge
		out	MCUCR,temp0		;
		ldi	temp0,1<<INT0		;enable external interrupt INT0
		out	GIMSK,temp0
;------------------------------------------------------------------------------------------
;********************************************************************
;* Main program
;********************************************************************
		sei					;enable interrupts globally
Main:
		sbis	inputport,DATAminus	;waiting till change D- to 0
		rjmp	CheckUSBReset		;and check, if isn't USB reset

		cpi	ActionFlag,DoReceiveSetupData
		breq	ProcReceiveSetupData
		cpi	ActionFlag,DoPrepareOutContinuousBuffer
		breq	ProcPrepareOutContinuousBuffer
		rjmp	Main

CheckUSBReset:
		ldi	temp0,255		;counter duration of reset (according to specification is that cca 10ms - here is cca 100us)
WaitForUSBReset:
		sbic	inputport,DATAminus	;waiting till change D+ to 0
		rjmp	Main
		dec	temp0
		brne	WaitForUSBReset
		rcall	USBReset
		rjmp	Main

ProcPrepareOutContinuousBuffer:
		rcall	PrepareOutContinuousBuffer	;prepare next sequence of answer to buffer
		ldi	ActionFlag,DoReadySendAnswer
		rjmp	Main
ProcReceiveSetupData:
		ldi	USBBufptrY,InputBufferBegin	;pointer to begin of receiving buffer
		mov	ByteCount,InputBufferLength	;length of input buffer
		rcall	DecodeNRZI		;transfer NRZI coding to bits
		rcall	MirrorInBufferBytes	;invert bits order in bytes
		rcall	BitStuff		;removal of bitstuffing
		;rcall	CheckCRCIn		;check CRC
		rcall	PrepareUSBOutAnswer	;prepare answers to transmitting buffer
		ldi	ActionFlag,DoReadySendAnswer
		rjmp	Main
;********************************************************************
;* Main program END
;********************************************************************
;------------------------------------------------------------------------------------------
;********************************************************************
;* Interrupt0 interrupt handler
;********************************************************************
INT0Handler:					;interrupt INT0
		in	backupSREG,SREG
		push	temp0
		push	temp1

		ldi	temp0,3			;counter of duration log0
		ldi	temp1,2			;counter of duration log1
		;waiting for begin packet
CheckchangeMinus:
		sbis	inputport,DATAminus	;waiting till change D- to 1
		rjmp	CheckchangeMinus
CheckchangePlus:
		sbis	inputport,DATAplus	;waiting till change D+ to 1
		rjmp	CheckchangePlus
DetectSOPEnd:
		sbis	inputport,DATAplus
		rjmp	Increment0		;D+ =0
Increment1:
		ldi	temp0,3			;counter of duration log0
		dec	temp1			;how many cycles takes log1
		nop
		breq	USBBeginPacket		;if this is end of SOP - receive packet
		rjmp	DetectSOPEnd
Increment0:
		ldi	temp1,2			;counter of duration log1
		dec	temp0			;how many cycles take log0
		nop
		brne	DetectSOPEnd		;if there isn't SOF - continue
		rjmp	EndInt0HandlerPOP2
EndInt0Handler:
		pop	ACC
		pop	RS232BufptrX
		pop	temp3
		pop	temp2
EndInt0HandlerPOP:
		pop	USBBufptrY
		pop	ByteCount
		mov	bitcount,backupbitcount	;restore bitcount register
EndInt0HandlerPOP2:
		pop	temp1
		pop	temp0
		out	SREG,backupSREG
		ldi	shiftbuf,1<<INTF0	;zero interruptu flag INTF0
		out	GIFR,shiftbuf
		reti				;otherwise finish (was only SOF - every millisecond)

USBBeginPacket:
		mov	backupbitcount,bitcount	;backup bitcount register
		in	shiftbuf,inputport	;if yes load it as zero bit directly to shift register
USBloopBegin:
		push	ByteCount		;additional backup of registers (save of time)
		push	USBBufptrY
		ldi	bitcount,6		;initialization of bits counter in byte
		ldi	ByteCount,MAXUSBBYTES	;initialization of max number of received bytes in packet
		ldi	USBBufptrY,InputShiftBufferBegin	;set the input buffer
USBloop1_6:
		in	inputbuf,inputport
		cbr	inputbuf,USBpinmask	;unmask low 2 bits
		breq	USBloopEnd		;if they are zeros - end of USB packet
		ror	inputbuf		;transfer Data+ to shift register
		rol	shiftbuf
		dec	bitcount		;decrement bits counter
		brne	USBloop1_6		;if it isn't zero - repeat filling of shift register
		nop				;otherwise is necessary copy shift register to buffer
USBloop7:
		in	inputbuf,inputport
		cbr	inputbuf,USBpinmask	;unmask low 2 bits
		breq	USBloopEnd		;if they are zeros - end of USB packet
		ror	inputbuf		;transfer Data+ to shift register
		rol	shiftbuf
		ldi	bitcount,7		;initialization of bits counter in byte
		st	Y+,shiftbuf		;copy shift register into buffer and increment pointer to buffer
USBloop0:					;and start receiving next byte
		in	shiftbuf,inputport	;zero bit directly to shift register
		cbr	shiftbuf,USBpinmask	;unmask low 2 bits
		breq	USBloopEnd		;if they are zeros - end of USB packet
		dec	bitcount		;decrement bits counter
		nop				;
		dec	ByteCount		;if not reached maximum buffer
		brne	USBloop1_6		;then receive next

		rjmp	EndInt0HandlerPOP	;otherwise repeat back from begin

USBloopEnd:
		cpi	USBBufptrY,InputShiftBufferBegin+3	;if at least 3 byte not received
		brcs	EndInt0HandlerPOP	;then finish
		lds	temp0,InputShiftBufferBegin+0	;identifier of packet to temp0
		lds	temp1,InputShiftBufferBegin+1	;address to temp1
		brne	TestDataPacket		;if is length different from 3 - then this can be only DataPaket
TestIOPacket:
;		cp	temp1,MyAddress		;if this isn't assigned (address) for me
;		brne	TestDataPacket		;then this can be still DataPacket
TestSetupPacket:;test to SETUP packet
		cpi	temp0,nNRZISETUPPID
		brne	TestOutPacket		;if this isn't Setup PID - decode other packet
		cp	temp1,MyInAddress	;if this isn't assigned (address) for me
		brne	TestDataPacket		;then this can be still DataPacket
		ldi	State,SetupState
		rjmp	EndInt0HandlerPOP	;if this is Setup PID - receive consecutive Data packet
TestOutPacket:	;test for OUT packet
		cpi	temp0,nNRZIOUTPID
		brne	TestInPacket		;if this isn't Out PID - decode other packet
		cp	temp1,MyOutAddress	;if this isn't assigned (address) for me
		brne	TestDataPacket		;then this can be still DataPacket
		ldi	State,OutState
		rjmp	EndInt0HandlerPOP	;if this is Out PID - receive consecutive Data packet
TestInPacket:	;test on IN packet
		cpi	temp0,nNRZIINPID
		brne	TestDataPacket		;if this isn't In PID - decode other packet
		cp	temp1,MyInAddress	;if this isn't assigned (address) for me
		breq	AnswerToInRequest
TestDataPacket:	;test for DATA0 and DATA1 packet
		cpi	temp0,nNRZIDATA0PID
		breq	Data0Packet		;if this isn't Data0 PID - decode other packet
		cpi	temp0,nNRZIDATA1PID
		brne	NoMyPacked		;if this isn't Data1 PID - decode other packet
Data0Packet:
		cpi	State,SetupState	;if was state Setup
		breq	ReceiveSetupData	;receive it
		cpi	State,OutState		;if was state Out
		breq	ReceiveOutData		;receive it
NoMyPacked:
		ldi	State,BaseState		;zero state
		rjmp	EndInt0HandlerPOP	;and receive consecutive Data packet

AnswerToInRequest:
		push	temp2			;backup next registers and continue
		push	temp3
		push	RS232BufptrX
		push	ACC
		cpi	ActionFlag,DoReadySendAnswer	;if isn't prepared answer
		brne	NoReadySend		;then send NAK
		rcall	SendPreparedUSBAnswer	;transmitting answer back
		cpi	State,AddressChangeState ;if state is AddressChange
		breq	SetMyNewUSBAddress	;then is necessary to change USB address
		ldi	State,InState
		ldi	ActionFlag,DoPrepareOutContinuousBuffer
		rjmp	EndInt0Handler		;and repeat - wait for next response from USB
ReceiveSetupData:
		push	temp2			;backup next registers and continue
		push	temp3
		push	RS232BufptrX
		push	ACC
		rcall	SendACK			;accept Setup Data packet
		rcall	FinishReceiving		;finish receiving
		ldi	ActionFlag,DoReceiveSetupData
		rjmp	EndInt0Handler
ReceiveOutData:
		push	temp2			;backup next registers and continue
		push	temp3
		push	RS232BufptrX
		push	ACC
		cpi	ActionFlag,DoReceiveSetupData	;if is currently in process command Setup
		breq	NoReadySend		;then send NAK
		rcall	SendACK			;accept Out packet
		clr	ActionFlag
		rjmp	EndInt0Handler
NoReadySend:
		rcall	SendNAK			;still I am not ready to answer
		rjmp	EndInt0Handler		;and repeat - wait for next response from USB
;------------------------------------------------------------------------------------------
SetMyNewUSBAddress:		;set new USB address in NRZI coded
		lds	MyInAddress,MyInAddressSRAM
		lds	MyOutAddress,MyOutAddressSRAM
		rjmp	EndInt0Handler
;------------------------------------------------------------------------------------------
FinishReceiving:		;corrective actions for receive termination
		cpi	bitcount,7		;transfer to buffer also last not completed byte
		breq	NoRemainingBits		;if were all bytes transfered, then nothing transfer
		inc	bitcount
ShiftRemainingBits:
		rol	shiftbuf		;shift remaining not completed bits on right position
		dec	bitcount
		brne	ShiftRemainingBits
		st	Y+,shiftbuf		;and copy shift register bo buffer - not completed byte
NoRemainingBits:
		mov	ByteCount,USBBufptrY
		subi	ByteCount,InputShiftBufferBegin-1	;in ByteCount is number of received bytes (including not completed bytes)

		mov	InputBufferLength,ByteCount		;and save for use in main program
		ldi	USBBufptrY,InputShiftBufferBegin	;pointer to begin of receiving shift buffer
		ldi	RS232BufptrX,InputBufferBegin+1		;data buffer (leave out SOP)
MoveDataBuffer:
		ld	temp0,Y+
		st	X+,temp0
		dec	ByteCount
		brne	MoveDataBuffer

		ldi	ByteCount,nNRZISOPbyte
		sts	InputBufferBegin,ByteCount		;like received SOP - it is not copied from shift buffer
		ret
;------------------------------------------------------------------------------------------
;********************************************************************
;* Other procedures
;********************************************************************
;------------------------------------------------------------------------------------------
USBReset:		;initialization of USB state engine
		ldi	temp0,nNRZIADDR0	;initialization of USB address
		mov	MyOutAddress,temp0
		mov	MyInAddress,temp0
		clr	State			;initialization of state engine
		clr	BitStuffInOut
		clr	OutBitStuffNumber
		clr	ActionFlag
		clr	RAMread			;will be reading from ROM
		clr	ConfigByte		;unconfigured state
		ret
;------------------------------------------------------------------------------------------
SendPreparedUSBAnswer:	;transmitting by NRZI coding OUT buffer with length OutputBufferLength to USB
		mov	ByteCount,OutputBufferLength		;length of answer
SendUSBAnswer:	;transmitting by NRZI coding OUT buffer to USB
		ldi	USBBufptrY,OutputBufferBegin		;pointer to begin of transmitting buffer
SendUSBBuffer:	;transmitting by NRZI coding given buffer to USB
		ldi	temp1,0			;incrementing pointer (temporary variable)
		mov	temp3,ByteCount		;byte counter: temp3 = ByteCount
		ldi	temp2,0b00000011	;mask for xoring
		ld	inputbuf,Y+		;load first byte to inputbuf and increment pointer to buffer
						;USB as output:
		cbi	outputport,DATAplus	;down DATAPLUS : idle state of USB port
		sbi	outputport,DATAminus	;set DATAMINUS : idle state of USB port
		sbi	USBdirection,DATAplus	;DATAPLUS as output
		sbi	USBdirection,DATAminus	;DATAMINUS as output

		in	temp0,outputport	;idle state of USB port to temp0
SendUSBAnswerLoop:
		ldi	bitcount,7		;bits counter
SendUSBAnswerByteLoop:
		nop				;delay because timing
		ror	inputbuf		;to carry transmiting bit (in direction first LSB then MSB)
		brcs	NoXORSend		;if that it is one - don't change USB state
		eor	temp0,temp2		;otherwise state will be changed
NoXORSend:
		out	outputport,temp0	;send out to USB
		dec	bitcount		;decrement bits counter - according to carry flag
		brne	SendUSBAnswerByteLoop	;if bits counter isn't zero - repeat transmiting with next bit
		sbrs	inputbuf,0		;if is transmiting bit one - don't change USB state
		eor	temp0,temp2		;otherwise state will be changed
NoXORSendLSB:
		dec	temp3			;decrement bytes counter
		ld	inputbuf,Y+		;load next byte and increment pointer to buffer
		out	outputport,temp0	;transmit to USB
		brne	SendUSBAnswerLoop	;repeat for all buffer (till temp3=0)

		mov	bitcount,OutBitStuffNumber	;bits counter for bitstuff
		cpi	bitcount,0		;if not be needed bitstuff
		breq	ZeroBitStuf
SendUSBAnswerBitstuffLoop:
		ror	inputbuf		;to carry transmiting bit (in direction first LSB then MSB)
		brcs	NoXORBitstuffSend	;if is one - don't change state on USB
		eor	temp0,temp2		;otherwise state will be changed
NoXORBitstuffSend:
		out	outputport,temp0	;transmit to USB
		nop				;delay because of timing
		dec	bitcount		;decrement bits counter - according to carry flag
		brne	SendUSBAnswerBitstuffLoop	;if bits counter isn't zero - repeat transmiting with next bit
		ld	inputbuf,Y		;delay 2 cycle
ZeroBitStuf:
		nop				;delay 1 cycle
		cbr	temp0,3
		out	outputport,temp0	;transmit EOP on USB

		ldi	bitcount,5		;delay counter: EOP shouls exists 2 bits (16 cycle at 12MHz)
SendUSBWaitEOP:
		dec	bitcount
		brne	SendUSBWaitEOP

		sbi	outputport,DATAminus	;set DATAMINUS : idle state on USB port
		sbi	outputport,DATAminus	;delay 2 cycle: Idle should exists 1 bit (8 cycle at 12MHz)
		cbi	USBdirection,DATAplus	;DATAPLUS as input
		cbi	USBdirection,DATAminus	;DATAMINUS as input
		cbi	outputport,DATAminus	;reset DATAMINUS : the third state on USB port
		ret
;------------------------------------------------------------------------------------------
ToggleDATAPID:
		lds	temp0,OutputBufferBegin+1	;load last PID
		cpi	temp0,DATA1PID			;if last was DATA1PID byte
		ldi	temp0,DATA0PID
		breq	SendData0PID			;then send zero answer with DATA0PID
		ldi	temp0,DATA1PID			;otherwise send zero answer with DATA1PID
SendData0PID:
		sts	OutputBufferBegin+1,temp0	;DATA0PID byte
		ret
;------------------------------------------------------------------------------------------
ComposeZeroDATA1PIDAnswer:
		ldi	temp0,DATA0PID			;DATA0 PID - in the next will be toggled to DATA1PID in load descriptor
		sts	OutputBufferBegin+1,temp0	;load to output buffer
ComposeZeroAnswer:
		ldi	temp0,SOPbyte
		sts	OutputBufferBegin+0,temp0	;SOP byte
		rcall	ToggleDATAPID			;change DATAPID
		ldi	temp0,0x00
		sts	OutputBufferBegin+2,temp0	;CRC byte
		sts	OutputBufferBegin+3,temp0	;CRC byte
		ldi	ByteCount,2+2			;length of output buffer (SOP and PID + CRC16)
		ret
;------------------------------------------------------------------------------------------
InitACKBufffer:
		ldi	temp0,SOPbyte
		sts	ACKBufferBegin+0,temp0		;SOP byte
		ldi	temp0,ACKPID
		sts	ACKBufferBegin+1,temp0		;ACKPID byte
		ret
;------------------------------------------------------------------------------------------
SendACK:
		push	USBBufptrY
		push	bitcount
		push	OutBitStuffNumber
		ldi	USBBufptrY,ACKBufferBegin	;pointer to begin of ACK buffer
		ldi	ByteCount,2			;number of transmit bytes (only SOP and ACKPID)
		clr	OutBitStuffNumber
		rcall	SendUSBBuffer
		pop	OutBitStuffNumber
		pop	bitcount
		pop	USBBufptrY
		ret
;------------------------------------------------------------------------------------------
InitNAKBufffer:
		ldi	temp0,SOPbyte
		sts	NAKBufferBegin+0,temp0		;SOP byte
		ldi	temp0,NAKPID
		sts	NAKBufferBegin+1,temp0		;NAKPID byte
		ret
;------------------------------------------------------------------------------------------
SendNAK:
		push	OutBitStuffNumber
		ldi	USBBufptrY,NAKBufferBegin	;pointer to begin of ACK buffer
		ldi	ByteCount,2			;number of transmited bytes (only SOP and NAKPID)
		clr	OutBitStuffNumber
		rcall	SendUSBBuffer
		pop	OutBitStuffNumber
		ret
;------------------------------------------------------------------------------------------
ComposeSTALL:
		ldi	temp0,SOPbyte
		sts	OutputBufferBegin+0,temp0	;SOP byte
		ldi	temp0,STALLPID
		sts	OutputBufferBegin+1,temp0	;STALLPID byte
		ldi	ByteCount,2			;length of output buffer (SOP and PID)
		ret
;------------------------------------------------------------------------------------------
DecodeNRZI:	;encoding of buffer from NRZI code to binary
		push	USBBufptrY		;back up pointer to buffer
		push	ByteCount		;back up length of buffer
		add	ByteCount,USBBufptrY	;end of buffer to ByteCount
		ser	temp0			;to ensure unit carry (in the next rotation)
NRZIloop:
		ror	temp0			;filling carry from previous byte
		ld	temp0,Y			;load received byte from buffer
		mov	temp2,temp0		;shifted register to one bit to the right and XOR for function of NRZI decoding
		ror	temp2			;carry to most significant digit bit and shift
		eor	temp2,temp0		;NRZI decoding
		com	temp2			;negate
		st	Y+,temp2		;save back as decoded byte and increment pointer to buffer
		cp	USBBufptrY,ByteCount	;if not all bytes
		brne	NRZIloop		;then repeat
		pop	ByteCount		;restore buffer length
		pop	USBBufptrY		;restore pointer to buffer
		ret				;otherwise finish
;------------------------------------------------------------------------------------------
BitStuff:	;removal of bitstuffing in buffer
		clr	temp3			;counter of omitted bits
		clr	lastBitstufNumber	;0xFF to lastBitstufNumber
		dec	lastBitstufNumber
BitStuffRepeat:
		push	USBBufptrY		;back up pointer to buffer
		push	ByteCount		;back up buffer length
		mov	temp1,temp3		;counter of all bits
		ldi	temp0,8			;sum all bits in buffer
SumAllBits:
		add	temp1,temp0
		dec	ByteCount
		brne	SumAllBits
		ldi	temp2,6			;initialize counter of ones
		pop	ByteCount		;restore buffer length
		push	ByteCount		;back up buffer length
		add	ByteCount,USBBufptrY	;end of buffer to ByteCount
		inc	ByteCount		;and for safety increment it with 2 (because of shifting)
		inc	ByteCount
BitStuffLoop:
		ld	temp0,Y			;load received byte from buffer
		ldi	bitcount,8		;bits counter in byte
BitStuffByteLoop:
		ror	temp0			;filling carry from LSB
		brcs	IncrementBitstuff	;if that LSB=0
		ldi	temp2,7			;initialize counter of ones +1 (if was zero)
IncrementBitstuff:
		dec	temp2			;decrement counter of ones (assumption of one bit)
		brne	DontShiftBuffer		;if there was not 6 ones together - don't shift buffer
		cp	temp1,lastBitstufNumber	;
		ldi	temp2,6			;initialize counter of ones (if no bitstuffing will be made then must be started again)
		brcc	DontShiftBuffer		;if already was made bitstuffing - don't shift buffer

		dec	temp1	;
		mov	lastBitstufNumber,temp1	;remember last position of bitstuffing
		cpi	bitcount,1		;for pointing to 7-th bit (which must be deleted or where to insert zero)
		brne	NoBitcountCorrect
		ldi	bitcount,9	;
		inc	USBBufptrY		;increment pointer to buffer
NoBitcountCorrect:
		dec	bitcount
		bst	BitStuffInOut,0
		brts	CorrectOutBuffer	;if this is Out buffer - increment buffer length
		rcall	ShiftDeleteBuffer	;shift In buffer
		dec	temp3			;decrement counter of omission
		rjmp	CorrectBufferEnd
CorrectOutBuffer:
		rcall	ShiftInsertBuffer	;shift Out buffer
		inc	temp3			;increment counter of omission
CorrectBufferEnd:
		pop	ByteCount		;restore buffer length
		pop	USBBufptrY		;restore pointer to buffer
		rjmp	BitStuffRepeat		;and restart from begin
DontShiftBuffer:
		dec	temp1			;if already were all bits
		breq	EndBitStuff		;finish cycle
		dec	bitcount		;decrement bits counter in byte
		brne	BitStuffByteLoop	;if not yet been all bits in byte - go to next bit
						;otherwise load next byte
		inc	USBBufptrY		;increment pointer to buffer
		rjmp	BitStuffLoop		;and repeat
EndBitStuff:
		pop	ByteCount		;restore buffer length
		pop	USBBufptrY		;restore pointer to buffer
		bst	BitStuffInOut,0
		brts	IncrementLength		;if this is Out buffer - increment length of Out buffer
DecrementLength:				;if this is In buffer - decrement length of In buffer
		cpi	temp3,0			;was at least one decrement
		breq	NoChangeByteCount	;if no - don't change buffer length
		dec	ByteCount		;if this is In buffer - decrement buffer length
		subi	temp3,256-8		;if there wasn't above 8 bits over
		brcc	NoChangeByteCount	;then finish
		dec	ByteCount		;otherwise next decrement buffer length
		ret				;and finish
IncrementLength:
		mov	OutBitStuffNumber,temp3	;remember number of bits over
		subi	temp3,8			;if there wasn't above 8 bits over
		brcs	NoChangeByteCount	;then finish
		inc	ByteCount		;otherwise increment buffer length
		mov	OutBitStuffNumber,temp3	;and remember number of bits over (decremented by 8)
NoChangeByteCount:
		ret				;finish
;------------------------------------------------------------------------------------------
ShiftInsertBuffer:	;shift buffer by one bit to right from end till to position: byte-USBBufptrY and bit-bitcount
		mov	temp0,bitcount		;calculation: bitcount= 9-bitcount
		ldi	bitcount,9
		sub	bitcount,temp0		;to bitcount bit position, which is necessary to clear

		ld	temp1,Y			;load byte which still must be shifted from position bitcount
		rol	temp1			;and shift to the left through Carry (transmission from higher byte and LSB to Carry)
		ser	temp2			;FF to mask - temp2
HalfInsertPosuvMask:
		lsl	temp2			;zero to the next low bit of mask
		dec	bitcount		;till not reached boundary of shifting in byte
		brne	HalfInsertPosuvMask

		and	temp1,temp2		;unmask that remains only high shifted bits in temp1
		com	temp2			;invert mask
		lsr	temp2			;shift mask to the right - for insertion of zero bit
		ld	temp0,Y			;load byte which must be shifted from position bitcount to temp0
		and	temp0,temp2		;unmask to remains only low non-shifted bits in temp0
		or	temp1,temp0		;and put together shifted and nonshifted part

		ld	temp0,Y			;load byte which must be shifted from position bitcount
		rol	temp0			;and shift it to the left through Carry (to set right Carry for further carry)
		st	Y+,temp1		;and load back modified byte
ShiftInsertBufferLoop:
		cpse	USBBufptrY,ByteCount	;if are not all entire bytes
		rjmp	NoEndShiftInsertBuffer	;then continue
		ret				;otherwise finish
NoEndShiftInsertBuffer:
		ld	temp1,Y			;load byte
		rol	temp1			;and shift to the left through Carry (carry from low byte and LSB to Carry)
		st	Y+,temp1		;and store back
		rjmp	ShiftInsertBufferLoop	;and continue
;------------------------------------------------------------------------------------------
ShiftDeleteBuffer:	;shift buffer one bit to the left from end to position: byte-USBBufptrY and bit-bitcount
		mov	temp0,bitcount		;calculation: bitcount= 9-bitcount
		ldi	bitcount,9
		sub	bitcount,temp0		;to bitcount bit position, which must be shifted
		mov	temp0,USBBufptrY	;backup pointera to buffer
		inc	temp0			;position of completed bytes to temp0
		mov	USBBufptrY,ByteCount	;maximum position to pointer
ShiftDeleteBufferLoop:
		ld	temp1,-Y		;decrement buffer and load byte
		ror	temp1			;and right shift through Carry (carry from higher byte and LSB to Carry)
		st	Y,temp1			;and store back
		cpse	USBBufptrY,temp0	;if there are not all entire bytes
		rjmp	ShiftDeleteBufferLoop	;then continue

		ld	temp1,-Y		;decrement buffer and load byte which must be shifted from position bitcount
		ror	temp1			;and right shift through Carry (carry from higher byte and LSB to Carry)
		ser	temp2			;FF to mask - temp2
HalfDeletePosuvMask:
		dec	bitcount		;till not reached boundary of shifting in byte
		breq	DoneMask
		lsl	temp2			;zero to the next low bit of mask
		rjmp	HalfDeletePosuvMask
DoneMask:
		and	temp1,temp2		;unmask to remain only high shifted bits in temp1
		com	temp2			;invert mask
		ld	temp0,Y			;load byte which must be shifted from position bitcount to temp0
		and	temp0,temp2		;unmask to remain only low nonshifted bits in temp0
		or	temp1,temp0		;and put together shifted and nonshifted part
		st	Y,temp1			;and store back
		ret				;and finish
;------------------------------------------------------------------------------------------
MirrorInBufferBytes:
		push	USBBufptrY
		push	ByteCount
		ldi	USBBufptrY,InputBufferBegin
		rcall	MirrorBufferBytes
		pop	ByteCount
		pop	USBBufptrY
		ret
;------------------------------------------------------------------------------------------
MirrorBufferBytes:
		add	ByteCount,USBBufptrY	;ByteCount shows to the end of message
MirrorBufferloop:
		ld	temp0,Y			;load received byte from buffer
		ldi	temp1,8			;bits counter
MirrorBufferByteLoop:
		ror	temp0			;to carry next least bit
		rol	temp2			;from carry next bit to reverse order
		dec	temp1			;was already entire byte
		brne	MirrorBufferByteLoop	;if no then repeat next least bit
		st	Y+,temp2		;save back as reversed byte  and increment pointer to buffer
		cp	USBBufptrY,ByteCount	;if not yet been all
		brne	MirrorBufferloop	;then repeat
		ret				;otherwise finish
;------------------------------------------------------------------------------------------
;CheckCRCIn:
;		kiss	USBBUFPTRY
;		kiss	ByteCount
;		ldi	USBBUFPTRY,InputBuffercompare
;		rcall	CheckCRC
;		pope	ByteCount
;		pope	USBBUFPTRY
;		lip
;------------------------------------------------------------------------------------------
AddCRCOut:
		push	USBBufptrY
		push	ByteCount
		ldi	USBBufptrY,OutputBufferBegin
		rcall	CheckCRC
		com	temp0			;negation of CRC
		com	temp1
		st	Y+,temp1		;save CRC to the end of buffer (at first MSB)
		st	Y,temp0			;save CRC to the end of buffer (then LSB)
		dec	USBBufptrY		;pointer to CRC position
		ldi	ByteCount,2		;reverse bits order in 2 bytes CRC
		rcall	MirrorBufferBytes	;reverse bits order in CRC (transmiting CRC - MSB first)
		pop	ByteCount
		pop	USBBufptrY
		ret
;------------------------------------------------------------------------------------------
CheckCRC:	;input: USBBufptrY = begin of message	,ByteCount = length of message
		add	ByteCount,USBBufptrY	;ByteCount points to the end of message
		inc	USBBufptrY		;set the pointer to message start - omit SOP
		ld	temp0,Y+		;load PID to temp0
						;and set the pointer to start of message - omit also PID
		cpi	temp0,DATA0PID		;if is DATA0 field
		breq	ComputeDATACRC		;compute CRC16
		cpi	temp0,DATA1PID		;if is DATA1 field
		brne	CRC16End		;if no then finish
ComputeDATACRC:
		ser	temp0			;initialization of remaider LSB to 0xff
		ser	temp1			;initialization of remaider MSB to 0xff
CRC16Loop:
		ld	temp2,Y+		;load message to temp2 and increment pointer to buffer
		ldi	temp3,8			;bits counter in byte - temp3
CRC16LoopByte:
		bst	temp1,7			;to T save MSB of remainder (remainder is only 16 bits - 8 bit of higher byte)
		bld	bitcount,0		;to bitcount LSB save T - of MSB remainder
		eor	bitcount,temp2		;XOR of bit message and bit remainder - in LSB bitcount
		rol	temp0			;shift remainder to the left - low byte (two bytes - through carry)
		rol	temp1			;shift remainder to the left - high byte (two bytes - through carry)
		cbr	temp0,1			;znuluj LSB remains
		lsr	temp2			;shift message to right
		ror	bitcount		;result of XOR bits from LSB to carry
		brcc	CRC16NoXOR		;if is XOR bitmessage and MSB of remainder = 0 , then no XOR
		ldi	bitcount,CRC16poly>>8	;to bitcount CRC polynomial - high byte
		eor	temp1,bitcount		;and make XOR from remains and CRC polynomial - high byte
		ldi	bitcount,LOW(CRC16poly)	;to bitcount CRC polynomial - low byte
		eor	temp0,bitcount		;and make XOR of remainder and CRC polynomial - low byte
CRC16NoXOR:
		dec	temp3			;were already all bits in byte
		brne	CRC16LoopByte		;unless, then go to next bit
		cp	USBBufptrY,ByteCount	;was already end-of-message
		brne	CRC16Loop		;unless then repeat
CRC16End:
		ret				;otherwise finish (in temp0 and temp1 is result)
;------------------------------------------------------------------------------------------
LoadDescriptorFromROM:
		lpm				;load from ROM position pointer to R0
		st	Y+,R0			;R0 save to buffer and increment buffer
		adiw	ZH:ZL,1			;increment index to ROM
		dec	ByteCount		;till are not all bytes
		brne	LoadDescriptorFromROM	;then load next
		rjmp	EndFromRAMROM		;otherwise finish
;------------------------------------------------------------------------------------------
LoadDescriptorFromROMZeroInsert:
		lpm				;load from ROM position pointer to R0
		st	Y+,R0			;R0 save to buffer and increment buffer

		bst	RAMread,3		;if bit 3 is one - don't insert zero
		brtc	InsertingZero		;otherwise zero will be inserted
		adiw	ZH:ZL,1			;increment index to ROM
		lpm				;load from ROM position pointer to R0
		st	Y+,R0			;R0 save to buffer and increment buffer
		clt				;and clear
		bld	RAMread,3		;the third bit in RAMread - for to the next zero insertion will be made
		rjmp	InsertingZeroEnd	;and continue
InsertingZero:
		clr	R0			;for insertion of zero
		st	Y+,R0			;zero save to buffer and increment buffer
InsertingZeroEnd:
		adiw	ZH:ZL,1			;increment index to ROM
		subi	ByteCount,2		;till are not all bytes
		brne	LoadDescriptorFromROMZeroInsert	;then load next
		rjmp	EndFromRAMROM		;otherwise finish
;------------------------------------------------------------------------------------------
LoadDescriptorFromSRAM:
		ld	R0,Z			;load from position RAM pointer to R0
		st	Y+,R0			;R0 save to buffer and increment buffer
		inc	ZL			;increment index to RAM
		dec	ByteCount		;till are not all bytes
		brne	LoadDescriptorFromSRAM  ;then load next
		rjmp	EndFromRAMROM		;otherwise finish
;------------------------------------------------------------------------------------------
LoadDescriptorFromEEPROM:
		out	EEAR,ZL			;set the address EEPROM
		sbi	EECR,EERE		;read EEPROM to register EEDR
		in	R0,EEDR			;load from EEDR to R0
		st	Y+,R0			;R0 save to buffer and increment buffer
		inc	ZL			;increment index to EEPROM
		dec	ByteCount		;till are not all bytes
		brne	LoadDescriptorFromEEPROM;then load next
		rjmp	EndFromRAMROM		;otherwise finish
;------------------------------------------------------------------------------------------
LoadXXXDescriptor:
		ldi	temp0,SOPbyte			;SOP byte
		sts	OutputBufferBegin,temp0		;to begin of tramsmiting buffer store SOP
		ldi	ByteCount,8			;8 byte store
		ldi	USBBufptrY,OutputBufferBegin+2	;to transmitting buffer

		and	RAMread,RAMread			;if will be reading from RAM or ROM or EEPROM
		brne	FromRAMorEEPROM			;0=ROM,1=RAM,2=EEPROM,4=ROM with zero insertion (string)
FromROM:
		rjmp	LoadDescriptorFromROM		;load descriptor from ROM
FromRAMorEEPROM:
		sbrc	RAMread,2			;if RAMREAD=4
		rjmp	LoadDescriptorFromROMZeroInsert	;read from ROM with zero insertion
		sbrc	RAMread,0			;if RAMREAD=1
		rjmp	LoadDescriptorFromSRAM		;load data from SRAM
		rjmp	LoadDescriptorFromEEPROM	;otherwise read from EEPROM
EndFromRAMROM:
		sbrc	RAMread,7			;if is most significant bit in variable RAMread=1
		clr	RAMread				;clear RAMread
		rcall	ToggleDATAPID			;change DATAPID
		ldi	USBBufptrY,OutputBufferBegin+1	;to transmitting buffer - position of DATA PID
		ret
;------------------------------------------------------------------------------------------
PrepareUSBOutAnswer:	;prepare answer to buffer
		rcall	PrepareUSBAnswer		;prepare answer to buffer
MakeOutBitStuff:
		inc	BitStuffInOut			;transmitting buffer - insertion of bitstuff bits
		ldi	USBBufptrY,OutputBufferBegin	;to transmitting buffer
		rcall	BitStuff
		mov	OutputBufferLength,ByteCount	;length of answer store for transmiting
		clr	BitStuffInOut			;receiving buffer - deletion of bitstuff bits
		ret
;------------------------------------------------------------------------------------------
PrepareUSBAnswer:	;prepare answer to buffer
		clr	RAMread				;zero to RAMread variable - reading from ROM
		lds	temp0,InputBufferBegin+2	;bmRequestType to temp0
		lds	temp1,InputBufferBegin+3	;bRequest to temp1
		cbr	temp0,0b10011111		;if is 5 and 6 bit zero
		brne	VendorRequest			;then this isn't  Vendor Request
		rjmp	StandardRequest			;but this is standard Request
;--------------------------
DoSetInfraBufferEmpty:
		rjmp	OneZeroAnswer			;confirm receiving with one zero
;--------------------------
DoSetRS232Baud:
		lds	temp0,InputBufferBegin+4	;first parameter - RS232 baudrate value
		out	UBRRL,temp0			;set UART speed
		rjmp	OneZeroAnswer			;confirm receiving with one zero
;--------------------------
DoGetRS232Baud:
		in	R0,UBRRL			;return UART speed in R0
		rjmp	DoGetIn				;and finish
;--------------------------
DoRS232Send:
		lds	temp0,InputBufferBegin+4	;first parameter - value transmitted to RS232
		out	UDR,temp0			;transmit data to UART
WaitForRS232Send:
		sbis	UCSRB,TXEN			;if disabled UART transmitter
		rjmp	OneZeroAnswer			;then finish - protection because loop lock in AT90S2323/2343
		sbis	UCSRA,TXC			;wait for transmition finish
		rjmp	WaitForRS232Send
		rjmp	OneZeroAnswer			;acknowledge reception with single zero
;--------------------------
DoRS232Read:
		rjmp	TwoZeroAnswer			;only acknowledge reception with two zero
;--------------------------
VendorRequest:
		clr	ZH				;for reading from RAM or EEPROM

		cpi	temp1,1				;
		breq	DoSetInfraBufferEmpty		;restart infra receiving (if it was stopped by reading from RAM)

		cpi	temp1,2				;
		breq	DoGetInfraCode			;transmit received infra code (if it is in buffer)

		cpi	temp1,3				;
		breq	DoSetDataPortDirection		;set flow direction of datal bits
		cpi	temp1,4				;
		breq	DoGetDataPortDirection		;detect of flow direction of data bits

		cpi	temp1,5				;
		breq	DoSetOutDataPort		;set data bits (if they are inputs, then pull-ups)
		cpi	temp1,6				;
		breq	DoGetOutDataPort		;detect settings of data out bits (if they are input, then pull-ups)

		cpi	temp1,7				;
		breq	DoGetInDataPort			;return value of input data port

		cpi	temp1,8				;
		breq	DoEEPROMRead			;return contents of EEPROM from given address
		cpi	temp1,9				;
		breq	DoEEPROMWrite			;write to EEPROM to given address given data

		cpi	temp1,10			;
		breq	DoRS232Send			;transmit byte to serial line
		cpi	temp1,11			;
		breq	DoRS232Read			;returns received byte from serial line(if some was received)

		cpi	temp1,12			;
		breq	DoSetRS232Baud			;set line speed of of serial line
		cpi	temp1,13			;
		breq	DoGetRS232Baud			;return line speed of serial line
		cpi	temp1,14			;
		breq	DoGetRS232Buffer		;return RS232 buffer

		cpi	temp1,USER_FNC_NUMBER+0		;
		breq	DoUserFunction0			;execute of user function0
		cpi	temp1,USER_FNC_NUMBER+1		;
		breq	DoUserFunction1			;execute of user function1
		cpi	temp1,USER_FNC_NUMBER+2		;
		breq	DoUserFunction2			;execute of user function2

		rjmp	ZeroDATA1Answer			;if that it was something unknown, then prepare zero answer

;----------------------------- USER FUNCTIONS --------------------------------------

;------------------------------TEMPLATE OF YOUR FUNCTION----------------------------
;------------------ BEGIN: This is template how to write own function --------------

;free of use are registers:
	;temp0,temp1,temp2,temp3,ACC,ZH,ZL
	;registers are destroyed after execution (use push/pop to save content)

;at the end of routine you must correctly set registers:
	;RAMread - 0=reading from ROM, 1=reading from RAM, 2=reading from EEPROM
	;temp0 - number of transmitted data bytes
	;ZH,ZL - pointer to buffer of transmitted data (pointer to ROM/RAM/EEPROM)

;to transmit data (preparing data to buffer) :
	;to transmit data you must jump to "ComposeEndXXXDescriptor"
	;to transmit one zero byte you can jump to "OneZeroAnswer"  (commonly used as confirmation of correct processing)
	;to transmit two zero byte you can jump to "TwoZeroAnswer"  (commonly used as confirmation of error in processing)

DoUserFunctionX:
DoUserFunction0:  ;send byte(s) of RAM starting at position given by first parameter in function
		lds	temp0,InputBufferBegin+4	;first parameter Lo into temp0
		;lds	temp1,InputBufferBegin+5	;first  parameter Hi into temp1
		;lds	temp2,InputBufferBegin+6	;second parameter Lo into temp2
		;lds	temp3,InputBufferBegin+7	;second parameter Hi into temp3
		;lds	ACC,InputBufferBegin+8		;number of requested bytes from USB host (computer) into ACC

		;Here add your own code:
		;-------------------------------------------------------------------
		nop					;example of code - nothing to do
		nop
		nop
		nop
		nop
		;-------------------------------------------------------------------

		mov	ZL,temp0			;will be sending value of RAM - from address stored in temp0 (first parameter Lo of function)
		inc	RAMread				;RAMread=1 - cita sa z RAM-ky
		ldi	temp0,RAMEND+1			;send max number of bytes - whole RAM
		rjmp	ComposeEndXXXDescriptor		;a prepare data
DoUserFunction1:
		rjmp	OneZeroAnswer			;only confirm receiving by one zero byte answer
DoUserFunction2:
		rjmp	TwoZeroAnswer			;only confirm receiving by two zero bytes answer
;------------------ END: This is template how to write own function ----------------

;----------------------------- USER FUNCTIONS --------------------------------------

DoGetInfraCode:
		rjmp	OneZeroAnswer			;acknowledge reception with single zero

DoEEPROMRead:
		lds	ZL,InputBufferBegin+4		;first parameter - offset in EEPROM
		ldi	temp0,2
		mov	RAMread,temp0			;RAMREAD=2 - reading from EEPROM
		ldi	temp0,E2END+1			;number my byte answers to temp0 - entire length of EEPROM
		rjmp	ComposeEndXXXDescriptor		;otherwise prepare data
DoEEPROMWrite:
		lds	ZL,InputBufferBegin+4		;first parameter - offset in EEPROM (address)
		lds	R0,InputBufferBegin+6		;second parameter - data to store to EEPROM (data)
		rjmp	EEPROMWrite			;write to EEPROM and finish this command
DoSetDataPortDirection:
		lds	ACC,InputBufferBegin+4		;first parameter - direction of data bits
		rcall	SetDataPortDirection
		rjmp	OneZeroAnswer			;acknowledge reception with single zero
DoGetDataPortDirection:
		rcall	GetDataPortDirection
		rjmp	DoGetIn

DoSetOutDataPort:
		lds	ACC,InputBufferBegin+4		;first parameter - value of data bits
		rcall	SetOutDataPort
		rjmp	OneZeroAnswer			;acknowledge reception with single zero
DoGetOutDataPort:
		rcall	GetOutDataPort
		rjmp	DoGetIn

DoGetInDataPort:
		rcall	GetInDataPort
 DoGetIn:
		ldi	ZL,0				;sending value in R0
		ldi	temp0,0x81			;RAMread=1 - reading from RAM
		mov	RAMread,temp0			;(highest bit set to 1 - to zero RAMread immediatelly)
		ldi	temp0,1				;send only single byte
		rjmp	ComposeEndXXXDescriptor		;and prepare data

DoGetRS232Buffer:
		mov	temp0,RS232BufferFull		;obtain buffer length of RS232 code
		cpi	temp0,0				;if is RS232 Buffer empty
		breq	OneZeroAnswer			;then nothing send and acknowledge reception with single zero

		lds	ACC,InputBufferBegin+8		;number of requiring bytes to ACC
		inc	temp0				;number of possible bytes (plus word of buffer length)
		cp	ACC,temp0			;if no requested more that I can send
		brcc	NoShortGetRS232Buffer		;transmit as many as requested
		mov	temp0,ACC
NoShortGetRS232Buffer:
		dec	temp0				;substract word length
		lds	temp1,RS232BufferBegin+1	;obtain index of reading of buffer of RS232 code
		add	temp1,temp0			;obtain where is end
		cpi	temp1,RS232BufferBegin+MAXRS232LENGTH+1	;if it would overflow
		brcs	ReadNoOverflow
		subi	temp1,RS232BufferBegin+MAXRS232LENGTH+1	;caculate how many not transfered
		sub	temp0,temp1			;and with this short length of reading
		ldi	temp1,RS232BufferBegin+4	;and start from zero
ReadNoOverflow:
		lds	ZL,RS232BufferBegin+1		;obtain index of reading of buffer of RS232 code: 2.byte of header (code length + reading + writing + reserve)
		sts	RS232BufferBegin+1,temp1	;write new index of reading of buffer of RS232 code : 2.byte of header (code length + reading + writing + reserve)
		dec	ZL				;space for length data - transmitted as first word

		sub	RS232BufferFull,temp0		;decrement buffer length
		st	Z,RS232BufferFull		;save real length to packet
		inc	temp0				;and about this word increment number of transmited bytes (buffer length)
		inc	RAMread				;RAMread=1 reading from RAM
		rjmp	ComposeEndXXXDescriptor		;and prepare data
;----------------------------- END USER FUNCTIONS ----------------------------------

OneZeroAnswer:		;send single zero
		ldi	temp0,1				;number of my bytes answers to temp0
		rjmp	ComposeGET_STATUS2

StandardRequest:
		cpi	temp1,GET_STATUS		;
		breq	ComposeGET_STATUS		;

		cpi	temp1,CLEAR_FEATURE		;
		breq	ComposeCLEAR_FEATURE		;

		cpi	temp1,SET_FEATURE		;
		breq	ComposeSET_FEATURE		;

		cpi	temp1,SET_ADDRESS		;if to set address
		breq	ComposeSET_ADDRESS		;set the address

		cpi	temp1,GET_DESCRIPTOR		;if requested descriptor
		breq	ComposeGET_DESCRIPTOR		;generate it

		cpi	temp1,SET_DESCRIPTOR		;
		breq	ComposeSET_DESCRIPTOR		;

		cpi	temp1,GET_CONFIGURATION		;
		breq	ComposeGET_CONFIGURATION	;

		cpi	temp1,SET_CONFIGURATION		;
		breq	ComposeSET_CONFIGURATION	;

		cpi	temp1,GET_INTERFACE		;
		breq	ComposeGET_INTERFACE		;

		cpi	temp1,SET_INTERFACE		;
		breq	ComposeSET_INTERFACE		;

		cpi	temp1,SYNCH_FRAME		;
		breq	ComposeSYNCH_FRAME		;
							;if not found known request
		rjmp	ZeroDATA1Answer			;if that was something unknown, then prepare zero answer

ComposeSET_ADDRESS:
		lds	temp1,InputBufferBegin+4	;new address to temp1
		rcall	SetMyNewUSBAddresses		;and compute NRZI and bitstuffing coded adresses
		ldi	State,AddressChangeState	;set state for Address changing
		rjmp	ZeroDATA1Answer			;send zero answer

ComposeSET_CONFIGURATION:
		lds	ConfigByte,InputBufferBegin+4	;number of configuration to variable ConfigByte
ComposeCLEAR_FEATURE:
ComposeSET_FEATURE:
ComposeSET_INTERFACE:
ZeroStringAnswer:
		rjmp	ZeroDATA1Answer			;send zero answer
ComposeGET_STATUS:
TwoZeroAnswer:
		ldi	temp0,2				;number of my bytes answers to temp0
ComposeGET_STATUS2:
		ldi	ZH, high(StatusAnswer<<1)	;ROMpointer to  answer
		ldi	ZL,  low(StatusAnswer<<1)
		rjmp	ComposeEndXXXDescriptor		;and complete
ComposeGET_CONFIGURATION:
		and	ConfigByte,ConfigByte		;if I am unconfigured
		breq	OneZeroAnswer			;then send single zero - otherwise send my configuration
		ldi	temp0,1				;number of my bytes answers to temp0
		ldi	ZH, high(ConfigAnswerMinus1<<1)	;ROMpointer to  answer
		ldi	ZL,  low(ConfigAnswerMinus1<<1)+1
		rjmp	ComposeEndXXXDescriptor		;and complete
ComposeGET_INTERFACE:
		ldi	ZH, high(InterfaceAnswer<<1)	;ROMpointer to answer
		ldi	ZL,  low(InterfaceAnswer<<1)
		ldi	temp0,1				;number of my bytes answers to temp0
		rjmp	ComposeEndXXXDescriptor		;and complete
ComposeSYNCH_FRAME:
ComposeSET_DESCRIPTOR:
		rcall	ComposeSTALL
		ret
ComposeGET_DESCRIPTOR:
		lds	temp1,InputBufferBegin+5	;DescriptorType to temp1
		cpi	temp1,DEVICE			;DeviceDescriptor
		breq	ComposeDeviceDescriptor		;
		cpi	temp1,CONFIGURATION		;ConfigurationDescriptor
		breq	ComposeConfigDescriptor		;
		cpi	temp1,STRING			;StringDeviceDescriptor
		breq	ComposeStringDescriptor		;
		ret
ComposeDeviceDescriptor:
		ldi	ZH, high(DeviceDescriptor<<1)	;ROMpointer to descriptor
		ldi	ZL,  low(DeviceDescriptor<<1)
		ldi	temp0,0x12			;number of my bytes answers to temp0
		rjmp	ComposeEndXXXDescriptor		;and complete
ComposeConfigDescriptor:
		ldi	ZH, high(ConfigDescriptor<<1)	;ROMpointer to descriptor
		ldi	ZL,  low(ConfigDescriptor<<1)
		ldi	temp0,9+9+7			;number of my bytes answers to temp0
ComposeEndXXXDescriptor:
		lds	TotalBytesToSend,InputBufferBegin+8	;number of requested bytes to TotalBytesToSend
		cp	TotalBytesToSend,temp0			;if not requested more than I can send
		brcs	HostConfigLength		;transmit the requested number
		mov	TotalBytesToSend,temp0		;otherwise send number of my answers
HostConfigLength:
		mov	temp0,TotalBytesToSend		;
		clr	TransmitPart			;zero the number of 8 bytes answers
		andi	temp0,0b00000111		;if is length divisible by 8
		breq	Length8Multiply			;then not count one answer (under 8 byte)
		inc	TransmitPart			;otherwise count it
Length8Multiply:
		mov	temp0,TotalBytesToSend		;
		lsr	temp0				;length of 8 bytes answers will reach
		lsr	temp0				;integer division by 8
		lsr	temp0
		add	TransmitPart,temp0		;and by addition to last non entire 8-bytes to variable TransmitPart
		ldi	temp0,DATA0PID			;DATA0 PID - in the next will be toggled to DATA1PID in load descriptor
		sts	OutputBufferBegin+1,temp0	;store to output buffer
		rjmp	ComposeNextAnswerPart
ComposeStringDescriptor:
		ldi	temp1,4+8			;if RAMread=4(insert zeros from ROM reading) + 8(behind first byte no load zero)
		mov	RAMread,temp1
		lds	temp1,InputBufferBegin+4	;DescriptorIndex to temp1
		cpi	temp1,0				;LANGID String
		breq	ComposeLangIDString		;
		cpi	temp1,2				;DevNameString
		breq	ComposeDevNameString		;
		brcc	ZeroStringAnswer		;if is DescriptorIndex higher than 2 - send zero answer
							;otherwise is VendorString
ComposeVendorString:
		ldi	ZH, high(VendorStringDescriptor<<1)	;ROMpointer to descriptor
		ldi	ZL,  low(VendorStringDescriptor<<1)
		ldi	temp0,(VendorStringDescriptorEnd-VendorStringDescriptor)*4-2	;number of my bytes answers to temp0
		rjmp	ComposeEndXXXDescriptor		;and complete
ComposeDevNameString:
		ldi	ZH, high(DevNameStringDescriptor<<1)	;ROMpointer to descriptor
		ldi	ZL,  low(DevNameStringDescriptor<<1)
		ldi	temp0,(DevNameStringDescriptorEnd-DevNameStringDescriptor)*4-2	;number of my bytes answers to temp0
		rjmp	ComposeEndXXXDescriptor		;and complete
ComposeLangIDString:
		clr	RAMread
		ldi	ZH, high(LangIDStringDescriptor<<1)	;ROMpointer to descriptor
		ldi	ZL,  low(LangIDStringDescriptor<<1)
		ldi	temp0,(LangIDStringDescriptorEnd-LangIDStringDescriptor)*2;pocet mojich bytovych odpovedi do temp0
		rjmp	ComposeEndXXXDescriptor		;and complete
;------------------------------------------------------------------------------------------
ZeroDATA1Answer:
		rcall	ComposeZeroDATA1PIDAnswer
		ret
;------------------------------------------------------------------------------------------
SetMyNewUSBAddresses:		;set new USB addresses in NRZI coded
		mov	temp2,temp1		;address to temp2 and temp1 and temp3
 		mov	temp3,temp1		;
		cpi	temp1,0b01111111	;if address contains less than 6 ones
		brne	NewAddressNo6ones	;then don't add bitstuffing
		ldi	temp1,0b10111111	;else insert one zero - bitstuffing
 NewAddressNo6ones:
		andi	temp3,0b00000111	;mask 3 low bits of Address
		cpi	temp3,0b00000111	;and if 3 low bits of Address is no all ones
		brne	NewAddressNo3ones	;then no change address
						;else insert zero after 3-rd bit (bitstuffing)
		sec				;set carry
		rol	temp2			;rotate left
		andi	temp2,0b11110111	;and inserted zero after 3-rd bit
 NewAddressNo3ones:
		sts	MyOutAddressSRAM,temp2	;store new non-coded address Out (temp2)
						;and now perform NRZI coding
		rcall	NRZIforAddress		;NRZI for AddressIn (in temp1)
		sts	MyInAddressSRAM,ACC	;store NRZI coded AddressIn

		lds	temp1,MyOutAddressSRAM	;load non-coded address Out (in temp1)
		rcall	NRZIforAddress		;NRZI for AddressOut
		sts	MyOutAddressSRAM,ACC	;store NRZI coded AddressOut

		ret				;and return
;------------------------------------------------------------------------------------------
NRZIforAddress:
		clr	ACC			;original answer state - of my nNRZI USB address
		ldi	temp2,0b00000001	;mask for xoring
		ldi	temp3,8			;bits counter
SetMyNewUSBAddressesLoop:
		mov	temp0,ACC		;remember final answer
		ror	temp1			;to carry transmitting bit LSB (in direction firstly LSB then MSB)
		brcs	NoXORBits		;if one - don't change state
		eor	temp0,temp2		;otherwise state will be changed according to last bit of answer
NoXORBits:
		ror	temp0			;last bit of changed answer to carry
		rol	ACC			;and from carry to final answer to the LSB place (and reverse LSB and MSB order)
		dec	temp3			;decrement bits counter
		brne	SetMyNewUSBAddressesLoop	;if bits counter isn't zero repeat transmitting with next bit
		ret
;------------------------------------------------------------------------------------------
;------------------------------------------------------------------------------------------
PrepareOutContinuousBuffer:
		rcall	PrepareContinuousBuffer
		rcall	MakeOutBitStuff
		ret
;------------------------------------------------------------------------------------------
PrepareContinuousBuffer:
		mov	temp0,TransmitPart
		cpi	temp0,1
		brne	NextAnswerInBuffer		;if buffer empty
		rcall	ComposeZeroAnswer		;prepare zero answer
		ret
NextAnswerInBuffer:
		dec	TransmitPart			;decrement general length of answer
ComposeNextAnswerPart:
		mov	temp1,TotalBytesToSend	;decrement number of bytes to transmit
		subi	temp1,8			;is is necessary to send more as 8 byte
		ldi	temp3,8			;if yes - send only 8 byte
		brcc	Nad8Bytov
		mov	temp3,TotalBytesToSend	;otherwise send only given number of bytes
		clr	TransmitPart
		inc	TransmitPart		;and this will be last answer
Nad8Bytov:
		mov	TotalBytesToSend,temp1	;decremented number of bytes to TotalBytesToSend
		rcall	LoadXXXDescriptor
		ldi	ByteCount,2		;length of output buffer (only SOP and PID)
		add	ByteCount,temp3		;+ number of bytes
		rcall	AddCRCOut		;addition of CRC to buffer
		inc	ByteCount		;length of output buffer + CRC16
		inc	ByteCount
		ret				;finish
;------------------------------------------------------------------------------------------
.equ	USBversion		=0x0101		;for what version USB is that (1.01)
.equ	VendorUSBID		=0x03EB		; vendor identifier (Atmel=0x03EB)
.equ	DeviceUSBID		=0x21FE		;product identifier (USB to RS232 converter ATmega8=0x21FF)
.equ	DeviceVersion		=0x0002		;version number of product (version=0.02)
.equ	MaxUSBCurrent		=46		;current consumption from USB (46mA)
;------------------------------------------------------------------------------------------
DeviceDescriptor:
		.db	0x12,0x01		;0 byte - size of descriptor in byte
						;1 byte - descriptor type: Device descriptor
		.dw	USBversion		;2,3 byte - version USB LSB (1.00)
		.db	0x00,0x00		;4 byte - device class
						;5 byte - subclass
		.db	0x00,0x08		;6 byte - protocol code
						;7 byte - FIFO size in bytes
		.dw	VendorUSBID		;8,9 byte - vendor identifier (Cypress=0x04B4)
		.dw	DeviceUSBID		;10,11 byte - product identifier (teplomer=0x0002)
		.dw	DeviceVersion		;12,13 byte - product version number (verzia=0.01)
		.db	0x01,0x02		;14 byte - index of string "vendor"
						;15 byte - index of string "product"
		.db	0x00,0x01		;16 byte - index of string "serial number"
						;17 byte - number of possible configurations
DeviceDescriptorEnd:
;------------------------------------------------------------------------------------------
ConfigDescriptor:
		.db	0x9,0x02		;length, descriptor type
ConfigDescriptorLength:
		.dw	9+9+7			;entire length of all descriptors
	ConfigAnswerMinus1:			;for sending the number - congiguration number (attention - addition of 1 required)
		.db	1,1			;numInterfaces, congiguration number
		.db	0,0x80			;string index, attributes; bus powered
		.db	MaxUSBCurrent/2,0x09	;current consumption, interface descriptor length
		.db	0x04,0			;interface descriptor; number of interface
	InterfaceAnswer:			;for sending number of alternatively interface
		.db	0,1			;alternatively interface; number of endpoints except EP0
	StatusAnswer:				;2 zero answers (saving ROM place)
		.db	0,0			;interface class; interface subclass
		.db	0,0			;protocol code; string index
		.db	0x07,0x5		;length, descriptor type - endpoint
		.db	0x81,0			;endpoint address; transfer type
		.dw	0x08			;max packet size
		.db	10,0			;polling interval [ms]; dummy byte (for filling)
ConfigDescriptorEnd:
;------------------------------------------------------------------------------------------
LangIDStringDescriptor:
		.db	(LangIDStringDescriptorEnd-LangIDStringDescriptor)*2,3	;length, type: string descriptor
		.dw	0x0409			;English
LangIDStringDescriptorEnd:
;------------------------------------------------------------------------------------------
VendorStringDescriptor:
		.db	(VendorStringDescriptorEnd-VendorStringDescriptor)*4-2,3	;length, type: string descriptor
VendorStringDescriptorEnd:
;------------------------------------------------------------------------------------------
DevNameStringDescriptor:
		.db	(DevNameStringDescriptorEnd-DevNameStringDescriptor)*4-2,3;dlzka, typ: string deskriptor
		.db	"AVR309:USB to UART protocol converter (simple)"
DevNameStringDescriptorEnd:
;------------------------------------------------------------------------------------------
MaskPortData:
		bst	ACC,0
		bld	temp0,LEDlsb0
		bst	ACC,1
		bld	temp0,LEDlsb1
		bst	ACC,2
		bld	temp0,LEDlsb2
		bst	ACC,3
		bld	temp1,LEDmsb3
		bst	ACC,4
		bld	temp1,LEDmsb4
		bst	ACC,5
		bld	temp1,LEDmsb5
		bst	ACC,6
		bld	temp1,LEDmsb6
		bst	ACC,7
		bld	temp1,LEDmsb7
		ret
;------------------------------------------------------------------------------------------
SetDataPortDirection:
		in	temp0,LEDdirectionLSB		;read current LSB state to temp0 (next bit directions will be not changed)
		in	temp1,LEDdirectionMSB		;read current MSB state to temp1 (next bit directions will be not changed)
		rcall	MaskPortData
		out	LEDdirectionLSB,temp0		;and update LSB port direction
		out	LEDdirectionMSB,temp1		;and update MSB port direction
		ret
;------------------------------------------------------------------------------------------
SetOutDataPort:
		in	temp0,LEDPortLSB		;read current LSB state to temp0 (next data bits will be not changed)
		in	temp1,LEDPortMSB		;read current MSB state to temp1 (next data bits will be not changed)
		rcall	MaskPortData
		out	LEDPortLSB,temp0		;and update LSB data port
		out	LEDPortMSB,temp1		;and update MSB data port
		ret
;------------------------------------------------------------------------------------------
GetInDataPort:
		in	temp0,LEDPinMSB			;read current MSB state to temp0
		in	temp1,LEDPinLSB			;read current LSB state to temp1
MoveLEDin:
		bst	temp1,LEDlsb0			;and move LSB bits to correct positions (from temp1 to temp0)
		bld	temp0,0				;(MSB bits are in correct place)
		bst	temp1,LEDlsb1
		bld	temp0,1
		bst	temp1,LEDlsb2
		bld	temp0,2
		mov	R0,temp0			;and store result to R0
		ret
;------------------------------------------------------------------------------------------
GetOutDataPort:
		in	temp0,LEDPortMSB		;read current MSB state to temp0
		in	temp1,LEDPortLSB		;read current LSB state to temp1
		rjmp	MoveLEDin
;------------------------------------------------------------------------------------------
GetDataPortDirection:
		in	temp0,LEDdirectionMSB		;read current MSB state to temp0
		in	temp1,LEDdirectionLSB		;read current LSB state to temp1
		rjmp	MoveLEDin
;------------------------------------------------------------------------------------------
EEPROMWrite:
		out	EEAR,ZL				;set the address of EEPROM
		out	EEDR,R0				;set the data to EEPROM
		cli					;disable interrupt
		sbi	EECR,EEMPE			;set the master write enable
		sei					;enable interrupt (next instruction is performed)
		sbi	EECR,EEPE			;write
WaitForEEPROMReady:
		sbic	EECR,EEPE			;wait to the end of write
		rjmp	WaitForEEPROMReady		;in loop (max cca 4ms) (because of possible next reading/writing)
		rjmp	OneZeroAnswer			;acknowledge reception with single zero
;------------------------------------------------------------------------------------------
;********************************************************************
;* End of Program
;********************************************************************
;------------------------------------------------------------------------------------------
;------------------------------------------------------------------------------------------
;********************************************************************
;* End of file
;********************************************************************
