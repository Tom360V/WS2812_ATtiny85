
/******************************************************************************
 * ws2812.S
 *
 * Created: 2017-01-25 11:19:00
 *  Author: Tom Koene
 *
 *****************************************************************************/ 
#include <avr/io.h>

#define nof_bits	R17
#define nof_bytes	R19
#define cnt_bytes	R24
#define rgb			R21
#define valueLOW	R22
#define valueHIGH	R23
#define PIN			0 //4
#define byte		r24

//External variable
.extern sizeof_my_array
.extern my_array

/* UpdateLeds()
 * This function converts every bit in my_array to a pulse by toggling PIN
 * The periode of the pulse is the same always the same length 1.25us
 *                ________
 * bit is 0 :: ..| 400 ns |____850_ns____|...
 *                ____________
 * bit is 1 :: ..|  800 ns    |__450_ns__|...
 *
 * when all bits are send, a reset pulse shoud be send: hold PIN down for atleast 50us
 *
 */
.global UpdateLeds
UpdateLeds:
	ldi ZL, lo8(my_array)
	ldi ZH, hi8(my_array)
	LD  rgb, Z+						//copy content of Z to rgb

	LDS nof_bytes, my_array_sizeof
	LDI nof_bits, 8					//Number of Bits to send
	LDI cnt_bytes, 0

	RJMP SEND_BIT					// Skip piece of code

ACCESS_NEXT_BYTE:
	INC cnt_bytes

	LD  rgb, Z+						//copy content of Z to rgb
	LDI nof_bits, 8					//Number of Bits to send

SEND_BIT:
	SBI _SFR_IO_ADDR(PORTB), PIN	//Set PIN high!

// set next value to send

	LSL rgb							//Shift byte to left (bit7 goes to C register in SREG)
//	BRNE BIT_ONE
	BRBC 0, BIT_ONE					//Check value of C is SREG
BIT_ZERO:
	LDI valueHIGH,	5				//Set number of instructions to execute to keep pin high
	LDI valueLOW,	3				//Set number of instructions to execute to keep pin low
	rjmp HIGH
BIT_ONE:
	LDI valueHIGH,	2				//Set number of instructions to execute to keep pin high
	LDI valueLOW,	6				//Set number of instructions to execute to keep pin low
	
HIGH:								//Loop till number of instructions are burned
	DEC valueHIGH
	BRNE HIGH

	CBI _SFR_IO_ADDR(PORTB), PIN    //set PIN low

LOW:								//Loop till number of instructions are burned
	DEC valueLOW
	BRNE LOW
	
	DEC nof_bits					//Decrement number of bits to send
	BRNE SEND_BIT					//Send next bit if nof_bits > 0

	DEC nof_bytes					//else, Decrement nof_bytes to send
	BRNE ACCESS_NEXT_BYTE			//if nof_bytes > 0, jump
	
SEND_RESET_PULSE:
	LDI valueHIGH,	255 //note: timing is not measured, so maybe those values can
	LDI valueLOW,	255 //      be decremented to get a higher refreshrate
    DELAY_HIGH:
        DELAY_LOW:
	        DEC valueLOW
	        BRNE DELAY_LOW
	DEC valueHIGH
	BRNE DELAY_HIGH

//	RETI
