#include <utility.h>
#include <unwind-cxx.h>
#include <system_configuration.h>
#include <Utils.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include <TimerOne.h>
#include <String.h>

#include <Snake.h>
#include <DropBall.h>
#include <font.h>
#include <StandardCplusplus.h>

/***************************************************************************************
* Name    : LED CUBE 8x8x8 74HC595
* By      : Liam Jackson

Based on code by Joseph Francis (74hc595 SPI)
and by chr at instructables
http://www.instructables.com/id/Led-Cube-8x8x8/step70/Run-the-cube-on-an-Arduino/
Font found somewhere
****************************************************************************************/
/* Modified to work with Voxel
* Adriaan Delport
* Hackable Designs

/*	Copied some animation from BlackX LED Cube 0.1
*	Added CubeAnimation library. Examples of usage in loop()
*   Mikhail Rogozhin 2016
***************************************************************************************/
#define AXIS_X 1
#define AXIS_Y 2
#define AXIS_Z 3

//--- Pins for shift registers
#define SHIFTREGISTER_PORT  PORTD
#define SER_DATA_OUT        0x10
#define SER_SHFT_CLK        0x08
#define SER_LOAD_CLK        0x04

//--- Pin connected to ST_CP of 74HC595
int latchPin = 2;
//--- Pin connected to SH_CP of 74HC595
int clockPin = 3;
//--- Pin connected to DS of 74HC595
int dataPin = 4;
//--- Used for faster latching
int latchPinPORTB = 0x04;
//--- Arduino led
int led = 13;

//---Plane Selection
int PlaneSel3 = A0;
int PlaneSel2 = A1;
int PlaneSel1 = A2;
int PlaneSel0 = A3;

int PlaneEn_n = 5;

//-- Buttons
int But1 = 9;
int But0 = 8;

//holds value for all the pins, [x][y][z]
byte cube[8][8];

const float Test[8][8] =
{ { 4.949747468, 4.301162634, 3.807886553, 3.535533906, 3.535533906, 3.807886553, 4.301162634, 4.949747468 },
{ 4.301162634, 3.535533906, 2.915475947, 2.549509757, 2.549509757, 2.915475947, 3.535533906, 4.301162634 },
{ 3.807886553, 2.915475947, 2.121320344, 1.58113883, 1.58113883, 2.121320344, 2.915475947, 3.807886553 },
{ 3.535533906, 2.549509757, 1.58113883, 0.707106781, 0.707106781, 1.58113883, 2.549509757, 3.535533906 },
{ 3.535533906, 2.549509757, 1.58113883, 0.707106781, 0.707106781, 1.58113883, 2.549509757, 3.535533906 },
{ 3.535533906, 4.301162634, 4.301162634, 4.301162634, 4.301162634, 4.301162634, 4.301162634, 4.301162634 },
{ 4.301162634, 3.535533906, 2.915475947, 2.549509757, 2.549509757, 2.915475947, 3.535533906, 4.301162634 },
{ 4.949747468, 4.301162634, 3.807886553, 3.535533906, 3.535533906, 3.807886553, 4.301162634, 4.949747468 }
};

//Counts through the layers
int current_layer = 0;

// LED CUBE Overall Brightness
int Brightness = 0;
// Temp Button Value
int valButton = HIGH;
//--- This process is run by the timer and does the PWM control
void iProcess(){
	//last layer store
	int oldLayerBit = current_layer + 2;

	//increment layer count
	current_layer++;
	if (current_layer >= 8){
		current_layer = 0;
	}

	//--- Run through all the shift register values and send them (last one first)
	// latching in the process
	latchOff();
	for (int i = 0; i < 8; i++){
		ShiftRegisterByteTransfer(cube[current_layer][i]);
	}

	//Hide the old layer
	//digitalWrite(oldLayerBit, LOW);
	digitalWrite(PlaneSel3, HIGH);

	//New data on the pins
	latchOn();
	//new layer high
	//digitalWrite(current_layer + 2, HIGH);
	SelectLayer(current_layer);
	digitalWrite(PlaneSel3, LOW);
	latchOn();
}

//--- Direct port access latching
void latchOn(){
	//bitSet(PORTB,latchPinPORTB);
	SHIFTREGISTER_PORT |= SER_LOAD_CLK;
}
void latchOff(){
	//bitClear(PORTB,latchPinPORTB);
	SHIFTREGISTER_PORT &= ~(SER_LOAD_CLK);
}

//--- Used to setup SPI based on current pin setup
//    this is called in the setup routine;
void setupSPI(){
	byte clr;
	SPCR |= ((1 << SPE) | (1 << MSTR)); // enable SPI as master
	SPCR &= ~((1 << SPR1) | (1 << SPR0)); // clear prescaler bits
	clr = SPSR; // clear SPI status reg
	clr = SPDR; // clear SPI data reg
	SPSR |= (1 << SPI2X); // set prescaler bits
	delay(10);
}

void SelectLayer(int _iLayer)
{
	switch (_iLayer)
	{
	case 0:
		digitalWrite(PlaneSel2, LOW);
		digitalWrite(PlaneSel1, LOW);
		digitalWrite(PlaneSel0, LOW);
		break;
	case 1:
		digitalWrite(PlaneSel2, LOW);
		digitalWrite(PlaneSel1, LOW);
		digitalWrite(PlaneSel0, HIGH);
		break;
	case 2:
		digitalWrite(PlaneSel2, LOW);
		digitalWrite(PlaneSel1, HIGH);
		digitalWrite(PlaneSel0, LOW);
		break;
	case 3:
		digitalWrite(PlaneSel2, LOW);
		digitalWrite(PlaneSel1, HIGH);
		digitalWrite(PlaneSel0, HIGH);
		break;
	case 4:
		digitalWrite(PlaneSel2, HIGH);
		digitalWrite(PlaneSel1, LOW);
		digitalWrite(PlaneSel0, LOW);
		break;
	case 5:
		digitalWrite(PlaneSel2, HIGH);
		digitalWrite(PlaneSel1, LOW);
		digitalWrite(PlaneSel0, HIGH);
		break;
	case 6:
		digitalWrite(PlaneSel2, HIGH);
		digitalWrite(PlaneSel1, HIGH);
		digitalWrite(PlaneSel0, LOW);
		break;
	case 7:
		digitalWrite(PlaneSel2, HIGH);
		digitalWrite(PlaneSel1, HIGH);
		digitalWrite(PlaneSel0, HIGH);
		break;
	}
}
void ShiftRegisterByteTransfer(byte data)
{
	for (int i = 0; i < 8; i++)
	{
		// Set data pin to match data bit
		if ((data >> i) & 0x1 == 1)
			SHIFTREGISTER_PORT |= SER_DATA_OUT;
		else
			SHIFTREGISTER_PORT &= ~(SER_DATA_OUT);

		// Toggle shift clock to load in bit
		SHIFTREGISTER_PORT ^= SER_SHFT_CLK;
		SHIFTREGISTER_PORT ^= SER_SHFT_CLK;
	}
}
//--- The really fast SPI version of shiftOut
byte spi_transfer(byte data)
{
	SPDR = data;			  // Start the transmission
	loop_until_bit_is_set(SPSR, SPIF);
	return SPDR;			  // return the received byte, we don't need that
}

void setup() {
	Serial.begin(9600);
	randomSeed(analogRead(A5));

	pinMode(led, OUTPUT);
	pinMode(latchPin, OUTPUT);
	pinMode(clockPin, OUTPUT);
	pinMode(dataPin, OUTPUT);

	digitalWrite(latchPin, LOW);
	digitalWrite(dataPin, LOW);
	digitalWrite(clockPin, LOW);

	pinMode(PlaneSel3, OUTPUT);
	pinMode(PlaneSel2, OUTPUT);
	pinMode(PlaneSel1, OUTPUT);
	pinMode(PlaneSel0, OUTPUT);
	pinMode(PlaneEn_n, OUTPUT);

	digitalWrite(PlaneSel3, HIGH);
	digitalWrite(PlaneSel2, LOW);
	digitalWrite(PlaneSel1, LOW);
	digitalWrite(PlaneSel0, LOW);
	analogWrite(PlaneEn_n, Brightness);
	digitalWrite(PlaneEn_n, LOW);

	pinMode(But1, INPUT);    // declare pushbutton as input
	pinMode(But0, INPUT);    // declare pushbutton as input

	//--- Setup to run SPI
	//setupSPI();
	//-- Wait for button press before starting demo
	//while (valButton == HIGH)
	//  valButton = digitalRead(But1);
	//--- Activate the PWM timer
	Timer1.initialize(1000); // Timer for updating pwm pins
	Timer1.attachInterrupt(iProcess);
}



// ==========================================================================================
//   TEXT Functions
// ==========================================================================================
char font_data[128][8] = {
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },    // 0 :
	//		|          |
	//		|          |
	//		|          |
	//		|          |
	//		|          |
	//		|          |
	//		|          |
	//		|          |

	{ 0x00, 0x3E, 0x41, 0x55, 0x41, 0x55, 0x49, 0x3E },    // 1 : 
	//		|          |
	//		|   *****  |
	//		|  *     * |
	//		|  * * * * |
	//		|  *     * |
	//		|  * * * * |
	//		|  *  *  * |
	//		|   *****  |

	{ 0x00, 0x3E, 0x7F, 0x6B, 0x7F, 0x6B, 0x77, 0x3E },    // 2 : 
	//		|          |
	//		|   *****  |
	//		|  ******* |
	//		|  ** * ** |
	//		|  ******* |
	//		|  ** * ** |
	//		|  *** *** |
	//		|   *****  |

	{ 0x00, 0x22, 0x77, 0x7F, 0x7F, 0x3E, 0x1C, 0x08 },    // 3 : 
	//		|          |
	//		|   *   *  |
	//		|  *** *** |
	//		|  ******* |
	//		|  ******* |
	//		|   *****  |
	//		|    ***   |
	//		|     *    |

	{ 0x00, 0x08, 0x1C, 0x3E, 0x7F, 0x3E, 0x1C, 0x08 },    // 4 : 
	//		|          |
	//		|     *    |
	//		|    ***   |
	//		|   *****  |
	//		|  ******* |
	//		|   *****  |
	//		|    ***   |
	//		|     *    |

	{ 0x00, 0x08, 0x1C, 0x2A, 0x7F, 0x2A, 0x08, 0x1C },    // 5 : 
	//		|          |
	//		|     *    |
	//		|    ***   |
	//		|   * * *  |
	//		|  ******* |
	//		|   * * *  |
	//		|     *    |
	//		|    ***   |

	{ 0x00, 0x08, 0x1C, 0x3E, 0x7F, 0x3E, 0x08, 0x1C },    // 6 : 
	//		|          |
	//		|     *    |
	//		|    ***   |
	//		|   *****  |
	//		|  ******* |
	//		|   *****  |
	//		|     *    |
	//		|    ***   |

	{ 0x00, 0x00, 0x1C, 0x3E, 0x3E, 0x3E, 0x1C, 0x00 },    // 7 : 
	//		|          |
	//		|          |
	//		|    ***   |
	//		|   *****  |
	//		|   *****  |
	//		|   *****  |
	//		|    ***   |
	//		|          |

	{ 0xFF, 0xFF, 0xE3, 0xC1, 0xC1, 0xC1, 0xE3, 0xFF },    // 8 : 
	//		| ******** |
	//		| ******** |
	//		| ***   ** |
	//		| **     * |
	//		| **     * |
	//		| **     * |
	//		| ***   ** |
	//		| ******** |

	{ 0x00, 0x00, 0x1C, 0x22, 0x22, 0x22, 0x1C, 0x00 },    // 9 : 	
	//		|          |
	//		|          |
	//		|    ***   |
	//		|   *   *  |
	//		|   *   *  |
	//		|   *   *  |
	//		|    ***   |
	//		|          |

	{ 0xFF, 0xFF, 0xE3, 0xDD, 0xDD, 0xDD, 0xE3, 0xFF },    // 10 : 

	//		| ******** |
	//		| ******** |
	//		| ***   ** |
	//		| ** *** * |
	//		| ** *** * |
	//		| ** *** * |
	//		| ***   ** |
	//		| ******** |

	{ 0x00, 0x0F, 0x03, 0x05, 0x39, 0x48, 0x48, 0x30 },    // 11 : 
	//		|          |
	//		|     **** |
	//		|       ** |
	//		|      * * |
	//		|   ***  * |
	//		|  *  *    |
	//		|  *  *    |
	//		|   **     |

	{ 0x00, 0x08, 0x3E, 0x08, 0x1C, 0x22, 0x22, 0x1C },    // 12 : 
	//		|          |
	//		|     *    |
	//		|   *****  |
	//		|     *    |
	//		|    ***   |
	//		|   *   *  |
	//		|   *   *  |
	//		|    ***   |

	{ 0x00, 0x18, 0x14, 0x10, 0x10, 0x30, 0x70, 0x60 },    // 13 : 

	//		|          |
	//		|    **    |
	//		|    * *   |
	//		|    *     |
	//		|    *     |
	//		|   **     |
	//		|  ***     |
	//		|  **      |

	{ 0x00, 0x0F, 0x19, 0x11, 0x13, 0x37, 0x76, 0x60 },    // 14 : 
	//		|          |
	//		|     **** |
	//		|    **  * |
	//		|    *   * |
	//		|    *  ** |
	//		|   ** *** |
	//		|  *** **  |
	//		|  **      |

	{ 0x00, 0x08, 0x2A, 0x1C, 0x77, 0x1C, 0x2A, 0x08 },    // 15 : 
	//		|          |
	//		|     *    |
	//		|   * * *  |
	//		|    ***   |
	//		|  *** *** |
	//		|    ***   |
	//		|   * * *  |
	//		|     *    |

	{ 0x00, 0x60, 0x78, 0x7E, 0x7F, 0x7E, 0x78, 0x60 },    // 16 : 
	//		|          |
	//		|  **      |
	//		|  ****    |
	//		|  ******  |
	//		|  ******* |
	//		|  ******  |
	//		|  ****    |
	//		|  **      |

	{ 0x00, 0x03, 0x0F, 0x3F, 0x7F, 0x3F, 0x0F, 0x03 },    // 17 : 
	//		|          |
	//		|       ** |
	//		|     **** |
	//		|   ****** |
	//		|  ******* |
	//		|   ****** |
	//		|     **** |
	//		|       ** |

	{ 0x00, 0x08, 0x1C, 0x2A, 0x08, 0x2A, 0x1C, 0x08 },    // 18 : 
	//		|          |
	//		|     *    |
	//		|    ***   |
	//		|   * * *  |
	//		|     *    |
	//		|   * * *  |
	//		|    ***   |
	//		|     *    |

	{ 0x00, 0x66, 0x66, 0x66, 0x66, 0x00, 0x66, 0x66 },    // 19 : 
	//		|          |
	//		|  **  **  |
	//		|  **  **  |
	//		|  **  **  |
	//		|  **  **  |
	//		|          |
	//		|  **  **  |
	//		|  **  **  |

	{ 0x00, 0x3F, 0x65, 0x65, 0x3D, 0x05, 0x05, 0x05 },    // 20 : 
	//		|          |
	//		|   ****** |
	//		|  **  * * |
	//		|  **  * * |
	//		|   **** * |
	//		|      * * |
	//		|      * * |
	//		|      * * |

	{ 0x00, 0x0C, 0x32, 0x48, 0x24, 0x12, 0x4C, 0x30 },    // 21 : 
	//		|          |
	//		|     **   |
	//		|   **  *  |
	//		|  *  *    |
	//		|   *  *   |
	//		|    *  *  |
	//		|  *  **   |
	//		|   **     |

	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x7F, 0x7F },    // 22 : 
	//		|          |
	//		|          |
	//		|          |
	//		|          |
	//		|          |
	//		|  ******* |
	//		|  ******* |
	//		|  ******* |

	{ 0x00, 0x08, 0x1C, 0x2A, 0x08, 0x2A, 0x1C, 0x3E },    // 23 : 
	//		|          |
	//		|     *    |
	//		|    ***   |
	//		|   * * *  |
	//		|     *    |
	//		|   * * *  |
	//		|    ***   |
	//		|   *****  |

	{ 0x00, 0x08, 0x1C, 0x3E, 0x7F, 0x1C, 0x1C, 0x1C },    // 24 : 
	//		|          |
	//		|     *    |
	//		|    ***   |
	//		|   *****  |
	//		|  ******* |
	//		|    ***   |
	//		|    ***   |
	//		|    ***   |

	{ 0x00, 0x1C, 0x1C, 0x1C, 0x7F, 0x3E, 0x1C, 0x08 },    // 25 : 
	//		|          |
	//		|    ***   |
	//		|    ***   |
	//		|    ***   |
	//		|  ******* |
	//		|   *****  |
	//		|    ***   |
	//		|     *    |

	{ 0x00, 0x08, 0x0C, 0x7E, 0x7F, 0x7E, 0x0C, 0x08 },    // 26 : 
	//		|          |
	//		|     *    |
	//		|     **   |
	//		|  ******  |
	//		|  ******* |
	//		|  ******  |
	//		|     **   |
	//		|     *    |

	{ 0x00, 0x08, 0x18, 0x3F, 0x7F, 0x3F, 0x18, 0x08 },    // 27 : 
	//		|          |
	//		|     *    |
	//		|    **    |
	//		|   ****** |
	//		|  ******* |
	//		|   ****** |
	//		|    **    |
	//		|     *    |

	{ 0x00, 0x00, 0x00, 0x70, 0x70, 0x70, 0x7F, 0x7F },    // 28 : 
	//		|          |
	//		|          |
	//		|          |
	//		|  ***     |
	//		|  ***     |
	//		|  ***     |
	//		|  ******* |
	//		|  ******* |

	{ 0x00, 0x00, 0x14, 0x22, 0x7F, 0x22, 0x14, 0x00 },    // 29 : 
	//		|          |
	//		|          |
	//		|    * *   |
	//		|   *   *  |
	//		|  ******* |
	//		|   *   *  |
	//		|    * *   |
	//		|          |

	{ 0x00, 0x08, 0x1C, 0x1C, 0x3E, 0x3E, 0x7F, 0x7F },    // 30 : 
	//		|          |
	//		|     *    |
	//		|    ***   |
	//		|    ***   |
	//		|   *****  |
	//		|   *****  |
	//		|  ******* |
	//		|  ******* |

	{ 0x00, 0x7F, 0x7F, 0x3E, 0x3E, 0x1C, 0x1C, 0x08 },    // 31 : 
	//		|          |
	//		|  ******* |
	//		|  ******* |
	//		|   *****  |
	//		|   *****  |
	//		|    ***   |
	//		|    ***   |
	//		|     *    |

	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },    // 32 :  
	//		|          |
	//		|          |
	//		|          |
	//		|          |
	//		|          |
	//		|          |
	//		|          |
	//		|          |

	{ 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18 },    // 33 : !
	//		|          |
	//		|    **    |
	//		|    **    |
	//		|    **    |
	//		|    **    |
	//		|    **    |
	//		|          |
	//		|    **    |

	{ 0x00, 0x36, 0x36, 0x14, 0x00, 0x00, 0x00, 0x00 },    // 34 : "
	//		|          |
	//		|   ** **  |
	//		|   ** **  |
	//		|    * *   |
	//		|          |
	//		|          |
	//		|          |
	//		|          |

	{ 0x00, 0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36 },    // 35 : #
	//		|          |
	//		|   ** **  |
	//		|   ** **  |
	//		|  ******* |
	//		|   ** **  |
	//		|  ******* |
	//		|   ** **  |
	//		|   ** **  |

	{ 0x00, 0x08, 0x1E, 0x20, 0x1C, 0x02, 0x3C, 0x08 },    // 36 : $
	//		|          |
	//		|     *    |
	//		|    ****  |
	//		|   *      |
	//		|    ***   |
	//		|       *  |
	//		|   ****   |
	//		|     *    |

	{ 0x00, 0x60, 0x66, 0x0C, 0x18, 0x30, 0x66, 0x06 },    // 37 : %
	//		|          |
	//		|  **      |
	//		|  **  **  |
	//		|     **   |
	//		|    **    |
	//		|   **     |
	//		|  **  **  |
	//		|      **  |

	{ 0x00, 0x3C, 0x66, 0x3C, 0x28, 0x65, 0x66, 0x3F },    // 38 : &
	//		|          |
	//		|   ****   |
	//		|  **  **  |
	//		|   ****   |
	//		|   * *    |
	//		|  **  * * |
	//		|  **  **  |
	//		|   ****** |

	{ 0x00, 0x18, 0x18, 0x18, 0x30, 0x00, 0x00, 0x00 },    // 39 : '
	//		|          |
	//		|    **    |
	//		|    **    |
	//		|    **    |
	//		|   **     |
	//		|          |
	//		|          |
	//		|          |

	{ 0x00, 0x60, 0x30, 0x18, 0x18, 0x18, 0x30, 0x60 },    // 40 : (
	//		|          |
	//		|  **      |
	//		|   **     |
	//		|    **    |
	//		|    **    |
	//		|    **    |
	//		|   **     |
	//		|  **      |

	{ 0x00, 0x06, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x06 },    // 41 : )
	//		|          |
	//		|      **  |
	//		|     **   |
	//		|    **    |
	//		|    **    |
	//		|    **    |
	//		|     **   |
	//		|      **  |

	{ 0x00, 0x00, 0x36, 0x1C, 0x7F, 0x1C, 0x36, 0x00 },    // 42 : *
	//		|          |
	//		|          |
	//		|   ** **  |
	//		|    ***   |
	//		|  ******* |
	//		|    ***   |
	//		|   ** **  |
	//		|          |

	{ 0x00, 0x00, 0x08, 0x08, 0x3E, 0x08, 0x08, 0x00 },    // 43 : +
	//		|          |
	//		|          |
	//		|     *    |
	//		|     *    |
	//		|   *****  |
	//		|     *    |
	//		|     *    |
	//		|          |

	{ 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x30, 0x60 },    // 44 : ,
	//		|          |
	//		|          |
	//		|          |
	//		|          |
	//		|   **     |
	//		|   **     |
	//		|   **     |
	//		|  **      |

	{ 0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00 },    // 45 : -
	//		|          |
	//		|          |
	//		|          |
	//		|          |
	//		|   ****   |
	//		|          |
	//		|          |
	//		|          |

	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x60 },    // 46 : .
	//		|          |
	//		|          |
	//		|          |
	//		|          |
	//		|          |
	//		|          |
	//		|  **      |
	//		|  **      |

	{ 0x00, 0x00, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x00 },    // 47 : /
	//		|          |
	//		|          |
	//		|      **  |
	//		|     **   |
	//		|    **    |
	//		|   **     |
	//		|  **      |
	//		|          |

	{ 0x00, 0x3C, 0x66, 0x6E, 0x76, 0x66, 0x66, 0x3C },    // 48 : 0
	//		|          |
	//		|   ****   |
	//		|  **  **  |
	//		|  ** ***  |
	//		|  *** **  |
	//		|  **  **  |
	//		|  **  **  |
	//		|   ****   |

	{ 0x00, 0x18, 0x18, 0x38, 0x18, 0x18, 0x18, 0x7E },    // 49 : 1
	//		|          |
	//		|    **    |
	//		|    **    |
	//		|   ***    |
	//		|    **    |
	//		|    **    |
	//		|    **    |
	//		|  ******  |

	{ 0x00, 0x3C, 0x66, 0x06, 0x0C, 0x30, 0x60, 0x7E },    // 50 : 2
	//		|          |
	//		|   ****   |
	//		|  **  **  |
	//		|      **  |
	//		|     **   |
	//		|   **     |
	//		|  **      |
	//		|  ******  |

	{ 0x00, 0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C },    // 51 : 3
	//		|          |
	//		|   ****   |
	//		|  **  **  |
	//		|      **  |
	//		|    ***   |
	//		|      **  |
	//		|  **  **  |
	//		|   ****   |

	{ 0x00, 0x0C, 0x1C, 0x2C, 0x4C, 0x7E, 0x0C, 0x0C },    // 52 : 4
	//		|          |
	//		|     **   |
	//		|    ***   |
	//		|   * **   |
	//		|  *  **   |
	//		|  ******  |
	//		|     **   |
	//		|     **   |

	{ 0x00, 0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3C },    // 53 : 5
	//		|          |
	//		|  ******  |
	//		|  **      |
	//		|  *****   |
	//		|      **  |
	//		|      **  |
	//		|  **  **  |
	//		|   ****   |

	{ 0x00, 0x3C, 0x66, 0x60, 0x7C, 0x66, 0x66, 0x3C },    // 54 : 6
	//		|          |
	//		|   ****   |
	//		|  **  **  |
	//		|  **      |
	//		|  *****   |
	//		|  **  **  |
	//		|  **  **  |
	//		|   ****   |

	{ 0x00, 0x7E, 0x66, 0x0C, 0x0C, 0x18, 0x18, 0x18 },    // 55 : 7
	//		|          |
	//		|  ******  |
	//		|  **  **  |
	//		|     **   |
	//		|     **   |
	//		|    **    |
	//		|    **    |
	//		|    **    |

	{ 0x00, 0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C },    // 56 : 8
	//		|          |
	//		|   ****   |
	//		|  **  **  |
	//		|  **  **  |
	//		|   ****   |
	//		|  **  **  |
	//		|  **  **  |
	//		|   ****   |

	{ 0x00, 0x3C, 0x66, 0x66, 0x3E, 0x06, 0x66, 0x3C },    // 57 : 9
	//		|          |
	//		|   ****   |
	//		|  **  **  |
	//		|  **  **  |
	//		|   *****  |
	//		|      **  |
	//		|  **  **  |
	//		|   ****   |

	{ 0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00 },    // 58 : :
	//		|          |
	//		|          |
	//		|    **    |
	//		|    **    |
	//		|          |
	//		|    **    |
	//		|    **    |
	//		|          |

	{ 0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x30 },    // 59 : ;
	//		|          |
	//		|          |
	//		|    **    |
	//		|    **    |
	//		|          |
	//		|    **    |
	//		|    **    |
	//		|   **     |

	{ 0x00, 0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06 },    // 60 : <
	//		|          |
	//		|      **  |
	//		|     **   |
	//		|    **    |
	//		|   **     |
	//		|    **    |
	//		|     **   |
	//		|      **  |

	{ 0x00, 0x00, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x00 },    // 61 : =
	//		|          |
	//		|          |
	//		|          |
	//		|   ****   |
	//		|          |
	//		|   ****   |
	//		|          |
	//		|          |

	{ 0x00, 0x60, 0x30, 0x18, 0x0C, 0x18, 0x30, 0x60 },    // 62 : >
	//		|          |
	//		|  **      |
	//		|   **     |
	//		|    **    |
	//		|     **   |
	//		|    **    |
	//		|   **     |
	//		|  **      |

	{ 0x00, 0x3C, 0x66, 0x06, 0x1C, 0x18, 0x00, 0x18 },    // 63 : ?
	//		|          |
	//		|   ****   |
	//		|  **  **  |
	//		|      **  |
	//		|    ***   |
	//		|    **    |
	//		|          |
	//		|    **    |

	{ 0x00, 0x38, 0x44, 0x5C, 0x58, 0x42, 0x3C, 0x00 },    // 64 : @
	//		|          |
	//		|   ***    |
	//		|  *   *   |
	//		|  * ***   |
	//		|  * **    |
	//		|  *    *  |
	//		|   ****   |
	//		|          |

	{ 0x00, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66 },    // 65 : A
	//		|          |
	//		|   ****   |
	//		|  **  **  |
	//		|  **  **  |
	//		|  ******  |
	//		|  **  **  |
	//		|  **  **  |
	//		|  **  **  |

	{ 0x00, 0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C },    // 66 : B
	//		|          |
	//		|  *****   |
	//		|  **  **  |
	//		|  **  **  |
	//		|  *****   |
	//		|  **  **  |
	//		|  **  **  |
	//		|  *****   |

	{ 0x00, 0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C },    // 67 : C
	//		|          |
	//		|   ****   |
	//		|  **  **  |
	//		|  **      |
	//		|  **      |
	//		|  **      |
	//		|  **  **  |
	//		|   ****   |

	{ 0x00, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x7C },    // 68 : D
	//		|          |
	//		|  *****   |
	//		|  **  **  |
	//		|  **  **  |
	//		|  **  **  |
	//		|  **  **  |
	//		|  **  **  |
	//		|  *****   |

	{ 0x00, 0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x7E },    // 69 : E
	//		|          |
	//		|  ******  |
	//		|  **      |
	//		|  **      |
	//		|  *****   |
	//		|  **      |
	//		|  **      |
	//		|  ******  |

	{ 0x00, 0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x60 },    // 70 : F
	//		|          |
	//		|  ******  |
	//		|  **      |
	//		|  **      |
	//		|  *****   |
	//		|  **      |
	//		|  **      |
	//		|  **      |

	{ 0x00, 0x3C, 0x66, 0x60, 0x60, 0x6E, 0x66, 0x3C },    // 71 : G
	//		|          |
	//		|   ****   |
	//		|  **  **  |
	//		|  **      |
	//		|  **      |
	//		|  ** ***  |
	//		|  **  **  |
	//		|   ****   |

	{ 0x00, 0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66 },    // 72 : H
	//		|          |
	//		|  **  **  |
	//		|  **  **  |
	//		|  **  **  |
	//		|  ******  |
	//		|  **  **  |
	//		|  **  **  |
	//		|  **  **  |

	{ 0x00, 0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C },    // 73 : I
	//		|          |
	//		|   ****   |
	//		|    **    |
	//		|    **    |
	//		|    **    |
	//		|    **    |
	//		|    **    |
	//		|   ****   |

	{ 0x00, 0x1E, 0x0C, 0x0C, 0x0C, 0x6C, 0x6C, 0x38 },    // 74 : J
	//		|          |
	//		|    ****  |
	//		|     **   |
	//		|     **   |
	//		|     **   |
	//		|  ** **   |
	//		|  ** **   |
	//		|   ***    |

	{ 0x00, 0x66, 0x6C, 0x78, 0x70, 0x78, 0x6C, 0x66 },    // 75 : K
	//		|          |
	//		|  **  **  |
	//		|  ** **   |
	//		|  ****    |
	//		|  ***     |
	//		|  ****    |
	//		|  ** **   |
	//		|  **  **  |

	{ 0x00, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E },    // 76 : L
	//		|          |
	//		|  **      |
	//		|  **      |
	//		|  **      |
	//		|  **      |
	//		|  **      |
	//		|  **      |
	//		|  ******  |

	{ 0x00, 0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63 },    // 77 : M
	//		|          |
	//		|  **   ** |
	//		|  *** *** |
	//		|  ******* |
	//		|  ** * ** |
	//		|  **   ** |
	//		|  **   ** |
	//		|  **   ** |

	{ 0x00, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x63, 0x63 },    // 78 : N
	//		|          |
	//		|  **   ** |
	//		|  ***  ** |
	//		|  **** ** |
	//		|  ** **** |
	//		|  **  *** |
	//		|  **   ** |
	//		|  **   ** |

	{ 0x00, 0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C },    // 79 : O
	//		|          |
	//		|   ****   |
	//		|  **  **  |
	//		|  **  **  |
	//		|  **  **  |
	//		|  **  **  |
	//		|  **  **  |
	//		|   ****   |

	{ 0x00, 0x7C, 0x66, 0x66, 0x66, 0x7C, 0x60, 0x60 },    // 80 : P
	//		|          |
	//		|  *****   |
	//		|  **  **  |
	//		|  **  **  |
	//		|  **  **  |
	//		|  *****   |
	//		|  **      |
	//		|  **      |

	{ 0x00, 0x3C, 0x66, 0x66, 0x66, 0x6E, 0x3C, 0x06 },    // 81 : Q
	//		|          |
	//		|   ****   |
	//		|  **  **  |
	//		|  **  **  |
	//		|  **  **  |
	//		|  ** ***  |
	//		|   ****   |
	//		|      **  |

	{ 0x00, 0x7C, 0x66, 0x66, 0x7C, 0x78, 0x6C, 0x66 },    // 82 : R
	//		|          |
	//		|  *****   |
	//		|  **  **  |
	//		|  **  **  |
	//		|  *****   |
	//		|  ****    |
	//		|  ** **   |
	//		|  **  **  |

	{ 0x00, 0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C },    // 83 : S
	//		|          |
	//		|   ****   |
	//		|  **  **  |
	//		|  **      |
	//		|   ****   |
	//		|      **  |
	//		|  **  **  |
	//		|   ****   |

	{ 0x00, 0x7E, 0x5A, 0x18, 0x18, 0x18, 0x18, 0x18 },    // 84 : T
	//		|          |
	//		|  ******  |
	//		|  * ** *  |
	//		|    **    |
	//		|    **    |
	//		|    **    |
	//		|    **    |
	//		|    **    |

	{ 0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3E },    // 85 : U
	//		|          |
	//		|  **  **  |
	//		|  **  **  |
	//		|  **  **  |
	//		|  **  **  |
	//		|  **  **  |
	//		|  **  **  |
	//		|   *****  |

	{ 0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18 },    // 86 : V
	//		|          |
	//		|  **  **  |
	//		|  **  **  |
	//		|  **  **  |
	//		|  **  **  |
	//		|  **  **  |
	//		|   ****   |
	//		|    **    |

	{ 0x00, 0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63 },    // 87 : W
	//		|          |
	//		|  **   ** |
	//		|  **   ** |
	//		|  **   ** |
	//		|  ** * ** |
	//		|  ******* |
	//		|  *** *** |
	//		|  **   ** |

	{ 0x00, 0x63, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x63 },    // 88 : X
	//		|          |
	//		|  **   ** |
	//		|  **   ** |
	//		|   ** **  |
	//		|    ***   |
	//		|   ** **  |
	//		|  **   ** |
	//		|  **   ** |

	{ 0x00, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18 },    // 89 : Y
	//		|          |
	//		|  **  **  |
	//		|  **  **  |
	//		|  **  **  |
	//		|   ****   |
	//		|    **    |
	//		|    **    |
	//		|    **    |

	{ 0x00, 0x7E, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x7E },    // 90 : Z
	//		|          |
	//		|  ******  |
	//		|      **  |
	//		|     **   |
	//		|    **    |
	//		|   **     |
	//		|  **      |
	//		|  ******  |

	{ 0x00, 0x1E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1E },    // 91 : [
	//		|          |
	//		|    ****  |
	//		|    **    |
	//		|    **    |
	//		|    **    |
	//		|    **    |
	//		|    **    |
	//		|    ****  |

	{ 0x00, 0x00, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x00 },    // 92 : \
										//		|          |
										//		|          |
										//		|  **      |
										//		|   **     |
										//		|    **    |
										//		|     **   |
										//		|      **  |
										//		|          |

	{ 0x00, 0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78 },    // 93 : ]
	//		|          |
	//		|  ****    |
	//		|    **    |
	//		|    **    |
	//		|    **    |
	//		|    **    |
	//		|    **    |
	//		|  ****    |

	{ 0x00, 0x08, 0x14, 0x22, 0x41, 0x00, 0x00, 0x00 },    // 94 : ^
	//		|          |
	//		|     *    |
	//		|    * *   |
	//		|   *   *  |
	//		|  *     * |
	//		|          |
	//		|          |
	//		|          |

	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F },    // 95 : _
	//		|          |
	//		|          |
	//		|          |
	//		|          |
	//		|          |
	//		|          |
	//		|          |
	//		|  ******* |

	{ 0x00, 0x0C, 0x0C, 0x06, 0x00, 0x00, 0x00, 0x00 },    // 96 : `
	//		|          |
	//		|     **   |
	//		|     **   |
	//		|      **  |
	//		|          |
	//		|          |
	//		|          |
	//		|          |

	{ 0x00, 0x00, 0x00, 0x3C, 0x06, 0x3E, 0x66, 0x3E },    // 97 : a
	//		|          |
	//		|          |
	//		|          |
	//		|   ****   |
	//		|      **  |
	//		|   *****  |
	//		|  **  **  |
	//		|   *****  |

	{ 0x00, 0x60, 0x60, 0x60, 0x7C, 0x66, 0x66, 0x7C },    // 98 : b
	//		|          |
	//		|  **      |
	//		|  **      |
	//		|  **      |
	//		|  *****   |
	//		|  **  **  |
	//		|  **  **  |
	//		|  *****   |

	{ 0x00, 0x00, 0x00, 0x3C, 0x66, 0x60, 0x66, 0x3C },    // 99 : c
	//		|          |
	//		|          |
	//		|          |
	//		|   ****   |
	//		|  **  **  |
	//		|  **      |
	//		|  **  **  |
	//		|   ****   |

	{ 0x00, 0x06, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3E },    // 100 : d
	//		|          |
	//		|      **  |
	//		|      **  |
	//		|      **  |
	//		|   *****  |
	//		|  **  **  |
	//		|  **  **  |
	//		|   *****  |

	{ 0x00, 0x00, 0x00, 0x3C, 0x66, 0x7E, 0x60, 0x3C },    // 101 : e
	//		|          |
	//		|          |
	//		|          |
	//		|   ****   |
	//		|  **  **  |
	//		|  ******  |
	//		|  **      |
	//		|   ****   |

	{ 0x00, 0x1C, 0x36, 0x30, 0x30, 0x7C, 0x30, 0x30 },    // 102 : f
	//		|          |
	//		|    ***   |
	//		|   ** **  |
	//		|   **     |
	//		|   **     |
	//		|  *****   |
	//		|   **     |
	//		|   **     |

	{ 0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x3C },    // 103 : g
	//		|          |
	//		|          |
	//		|   *****  |
	//		|  **  **  |
	//		|  **  **  |
	//		|   *****  |
	//		|      **  |
	//		|   ****   |

	{ 0x00, 0x60, 0x60, 0x60, 0x7C, 0x66, 0x66, 0x66 },    // 104 : h
	//		|          |
	//		|  **      |
	//		|  **      |
	//		|  **      |
	//		|  *****   |
	//		|  **  **  |
	//		|  **  **  |
	//		|  **  **  |

	{ 0x00, 0x00, 0x18, 0x00, 0x18, 0x18, 0x18, 0x3C },    // 105 : i
	//		|          |
	//		|          |
	//		|    **    |
	//		|          |
	//		|    **    |
	//		|    **    |
	//		|    **    |
	//		|   ****   |

	{ 0x00, 0x0C, 0x00, 0x0C, 0x0C, 0x6C, 0x6C, 0x38 },    // 106 : j
	//		|          |
	//		|     **   |
	//		|          |
	//		|     **   |
	//		|     **   |
	//		|  ** **   |
	//		|  ** **   |
	//		|   ***    |

	{ 0x00, 0x60, 0x60, 0x66, 0x6C, 0x78, 0x6C, 0x66 },    // 107 : k
	//		|          |
	//		|  **      |
	//		|  **      |
	//		|  **  **  |
	//		|  ** **   |
	//		|  ****    |
	//		|  ** **   |
	//		|  **  **  |

	{ 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18 },    // 108 : l
	//		|          |
	//		|    **    |
	//		|    **    |
	//		|    **    |
	//		|    **    |
	//		|    **    |
	//		|    **    |
	//		|    **    |

	{ 0x00, 0x00, 0x00, 0x63, 0x77, 0x7F, 0x6B, 0x6B },    // 109 : m
	//		|          |
	//		|          |
	//		|          |
	//		|  **   ** |
	//		|  *** *** |
	//		|  ******* |
	//		|  ** * ** |
	//		|  ** * ** |

	{ 0x00, 0x00, 0x00, 0x7C, 0x7E, 0x66, 0x66, 0x66 },    // 110 : n
	//		|          |
	//		|          |
	//		|          |
	//		|  *****   |
	//		|  ******  |
	//		|  **  **  |
	//		|  **  **  |
	//		|  **  **  |

	{ 0x00, 0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x3C },    // 111 : o
	//		|          |
	//		|          |
	//		|          |
	//		|   ****   |
	//		|  **  **  |
	//		|  **  **  |
	//		|  **  **  |
	//		|   ****   |

	{ 0x00, 0x00, 0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60 },    // 112 : p
	//		|          |
	//		|          |
	//		|  *****   |
	//		|  **  **  |
	//		|  **  **  |
	//		|  *****   |
	//		|  **      |
	//		|  **      |

	{ 0x00, 0x00, 0x3C, 0x6C, 0x6C, 0x3C, 0x0D, 0x0F },    // 113 : q
	//		|          |
	//		|          |
	//		|   ****   |
	//		|  ** **   |
	//		|  ** **   |
	//		|   ****   |
	//		|     ** * |
	//		|     **** |

	{ 0x00, 0x00, 0x00, 0x7C, 0x66, 0x66, 0x60, 0x60 },    // 114 : r
	//		|          |
	//		|          |
	//		|          |
	//		|  *****   |
	//		|  **  **  |
	//		|  **  **  |
	//		|  **      |
	//		|  **      |

	{ 0x00, 0x00, 0x00, 0x3E, 0x40, 0x3C, 0x02, 0x7C },    // 115 : s
	//		|          |
	//		|          |
	//		|          |
	//		|   *****  |
	//		|  *       |
	//		|   ****   |
	//		|       *  |
	//		|  *****   |

	{ 0x00, 0x00, 0x18, 0x18, 0x7E, 0x18, 0x18, 0x18 },    // 116 : t
	//		|          |
	//		|          |
	//		|    **    |
	//		|    **    |
	//		|  ******  |
	//		|    **    |
	//		|    **    |
	//		|    **    |

	{ 0x00, 0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3E },    // 117 : u
	//		|          |
	//		|          |
	//		|          |
	//		|  **  **  |
	//		|  **  **  |
	//		|  **  **  |
	//		|  **  **  |
	//		|   *****  |

	{ 0x00, 0x00, 0x00, 0x00, 0x66, 0x66, 0x3C, 0x18 },    // 118 : v
	//		|          |
	//		|          |
	//		|          |
	//		|          |
	//		|  **  **  |
	//		|  **  **  |
	//		|   ****   |
	//		|    **    |

	{ 0x00, 0x00, 0x00, 0x63, 0x6B, 0x6B, 0x6B, 0x3E },    // 119 : w
	//		|          |
	//		|          |
	//		|          |
	//		|  **   ** |
	//		|  ** * ** |
	//		|  ** * ** |
	//		|  ** * ** |
	//		|   *****  |

	{ 0x00, 0x00, 0x00, 0x66, 0x3C, 0x18, 0x3C, 0x66 },    // 120 : x
	//		|          |
	//		|          |
	//		|          |
	//		|  **  **  |
	//		|   ****   |
	//		|    **    |
	//		|   ****   |
	//		|  **  **  |

	{ 0x00, 0x00, 0x00, 0x66, 0x66, 0x3E, 0x06, 0x3C },    // 121 : y
	//		|          |
	//		|          |
	//		|          |
	//		|  **  **  |
	//		|  **  **  |
	//		|   *****  |
	//		|      **  |
	//		|   ****   |

	{ 0x00, 0x00, 0x00, 0x3C, 0x0C, 0x18, 0x30, 0x3C },    // 122 : z
	//		|          |
	//		|          |
	//		|          |
	//		|   ****   |
	//		|     **   |
	//		|    **    |
	//		|   **     |
	//		|   ****   |

	{ 0x00, 0x0E, 0x18, 0x18, 0x30, 0x18, 0x18, 0x0E },    // 123 : {
	//		|          |
	//		|     ***  |
	//		|    **    |
	//		|    **    |
	//		|   **     |
	//		|    **    |
	//		|    **    |
	//		|     ***  |

	{ 0x00, 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18 },    // 124 : |
	//		|          |
	//		|    **    |
	//		|    **    |
	//		|    **    |
	//		|          |
	//		|    **    |
	//		|    **    |
	//		|    **    |

	{ 0x00, 0x70, 0x18, 0x18, 0x0C, 0x18, 0x18, 0x70 },    // 125 : }
	//		|          |
	//		|  ***     |
	//		|    **    |
	//		|    **    |
	//		|     **   |
	//		|    **    |
	//		|    **    |
	//		|  ***     |

	{ 0x00, 0x00, 0x00, 0x3A, 0x6C, 0x00, 0x00, 0x00 },    // 126 : ~
	//		|          |
	//		|          |
	//		|          |
	//		|   *** *  |
	//		|  ** **   |
	//		|          |
	//		|          |
	//		|          |

	{ 0x00, 0x08, 0x1C, 0x36, 0x63, 0x41, 0x41, 0x7F }    // 127 : 
	//		|          |
	//		|     *    |
	//		|    ***   |
	//		|   ** **  |
	//		|  **   ** |
	//		|  *     * |
	//		|  *     * |
	//		|  ******* |


};

void effect_text(int delayt, char string[]){
	fill(0x00);
	int charNum = strlen(string);
	for (int ltr = 0; ltr < charNum; ltr++){// For each letter in string array
		for (int dist = 0; dist < 8; dist++) { //bring letter forward
			fill(0x00);//blank last row
			int rev = 0;
			for (int rw = 7; rw >= 0; rw--) {//copy rows
				cube[rev][dist] = bitswap(font_data[string[ltr]][rw]);
				rev++;
			}
			delay_ms(delayt);
		}
	}
}

void blink() {
	fill(0x00);
	digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
	delay(1000);              // wait for a second
	digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
	delay(1000);
}

#define MAZE_STACK_LAYERS 10

// draws maze from starting position
void maze_recurse(int x, int y, int z, int delayms, int &stack){
	stack++;
	if (stack > MAZE_STACK_LAYERS){
		Serial.println("maze hit stack limit");
		return;
	}
	if (!inrange(x, y, z)) { //|| getvoxel(x, y, z) != 0x0){//count_current > count_end){ //
		maze_recurse(random(0, 8), random(0, 8), random(0, 8), delayms, stack);
		return;
	}
	setvoxel(x, y, z);
	delay(delayms);

	int direct = random(0, 3);
	int sign = 1;
	sign = random(1, 3);
	if (sign == 2)
		sign = -1;
	if (direct == 0)
		maze_recurse(x + sign, y, z, delayms, stack);
	if (direct == 1)
		maze_recurse(x, y + sign, z, delayms, stack);
	else
		maze_recurse(x, y, z + sign, delayms, stack);
}

void effect_z_updown(int iterations, int delay)
{
	unsigned char positions[64];
	unsigned char destinations[64];

	int i, y, move;

	for (i = 0; i<64; i++)
	{
		positions[i] = 4;
		destinations[i] = rand() % 8;
	}

	for (i = 0; i<8; i++)
	{
		effect_z_updown_move(positions, destinations, AXIS_Z);
		delay_ms(delay);
	}

	for (i = 0; i<iterations; i++)
	{
		for (move = 0; move<8; move++)
		{
			effect_z_updown_move(positions, destinations, AXIS_Z);
			delay_ms(delay);
		}

		delay_ms(delay * 4);


		for (y = 0; y<32; y++)
		{
			destinations[rand() % 64] = rand() % 8;
		}

	}

}

void effect_z_updown_move(unsigned char positions[64], unsigned char destinations[64], char axis)
{
	int px;
	for (px = 0; px<64; px++)
	{
		if (positions[px]<destinations[px])
		{
			positions[px]++;
		}
		if (positions[px]>destinations[px])
		{
			positions[px]--;
		}
	}

	draw_positions_axis(AXIS_Z, positions, 0);
}

void effect_wormsqueeze(int size, int axis, int direction, int iterations, int delay)
{
	int x, y, i, j, k, dx, dy;
	int cube_size;
	int origin = 0;

	if (direction == -1)
		origin = 7;

	cube_size = 8 - (size - 1);

	x = rand() % cube_size;
	y = rand() % cube_size;

	for (i = 0; i<iterations; i++)
	{
		dx = ((rand() % 3) - 1);
		dy = ((rand() % 3) - 1);

		if ((x + dx) > 0 && (x + dx) < cube_size)
			x += dx;

		if ((y + dy) > 0 && (y + dy) < cube_size)
			y += dy;

		shift(axis, direction);


		for (j = 0; j<size; j++)
		{
			for (k = 0; k<size; k++)
			{
				if (axis == AXIS_Z)
					setvoxel(x + j, y + k, origin);

				if (axis == AXIS_Y)
					setvoxel(x + j, origin, y + k);

				if (axis == AXIS_X)
					setvoxel(origin, y + j, x + k);
			}
		}

		delay_ms(delay);
	}
}

int effect_telcstairs_do(int x, int val, int delay)
{
	int y, z;

	for (y = 0, z = x; y <= z; y++, x--)
	{
		if (x < CUBE_SIZE && y < CUBE_SIZE)
		{
			cube[x][y] = val;
		}
	}
	delay_ms(delay);
	return z;
}

void effect_telcstairs(int invert, int delay, int val)
{
	int x;

	if (invert)
	{
		for (x = CUBE_SIZE * 2; x >= 0; x--)
		{
			x = effect_telcstairs_do(x, val, delay);
		}
	}
	else
	{
		for (x = 0; x < CUBE_SIZE * 2; x++)
		{
			x = effect_telcstairs_do(x, val, delay);
		}
	}
}

void effect_axis_updown_randsuspend(char axis, int delay, int sleep, int invert)
{
	unsigned char positions[64];
	unsigned char destinations[64];

	int i, px;

	// Set 64 random positions
	for (i = 0; i<64; i++)
	{
		positions[i] = 0; // Set all starting positions to 0
		destinations[i] = rand() % 8;
	}

	// Loop 8 times to allow destination 7 to reach all the way
	for (i = 0; i<8; i++)
	{
		// For every iteration, move all position one step closer to their destination
		for (px = 0; px<64; px++)
		{
			if (positions[px]<destinations[px])
			{
				positions[px]++;
			}
		}
		// Draw the positions and take a nap
		draw_positions_axis(axis, positions, invert);
		delay_ms(delay);
	}

	// Set all destinations to 7 (opposite from the side they started out)
	for (i = 0; i<64; i++)
	{
		destinations[i] = 7;
	}

	// Suspend the positions in mid-air for a while
	delay_ms(sleep);

	// Then do the same thing one more time
	for (i = 0; i<8; i++)
	{
		for (px = 0; px<64; px++)
		{
			if (positions[px]<destinations[px])
			{
				positions[px]++;
			}
			if (positions[px]>destinations[px])
			{
				positions[px]--;
			}
		}
		draw_positions_axis(axis, positions, invert);
		delay_ms(delay);
	}
}

// Big ugly function :p but it looks pretty
void boingboing(uint16_t iterations, int delay, unsigned char mode, unsigned char drawmode)
{
	fill(0x00);		// Blank the cube

	int x, y, z;		// Current coordinates for the point
	int dx, dy, dz;	// Direction of movement
	int lol, i;		// lol?
	unsigned char crash_x, crash_y, crash_z;

	y = rand() % 8;
	x = rand() % 8;
	z = rand() % 8;

	// Coordinate array for the snake.
	int snake[8][3];
	for (i = 0; i<8; i++)
	{
		snake[i][0] = x;
		snake[i][1] = y;
		snake[i][2] = z;
	}


	dx = 1;
	dy = 1;
	dz = 1;

	while (iterations)
	{
		crash_x = 0;
		crash_y = 0;
		crash_z = 0;


		// Let's mix things up a little:
		if (rand() % 3 == 0)
		{
			// Pick a random axis, and set the speed to a random number.
			lol = rand() % 3;
			if (lol == 0)
				dx = rand() % 3 - 1;

			if (lol == 1)
				dy = rand() % 3 - 1;

			if (lol == 2)
				dz = rand() % 3 - 1;
		}

		// The point has reached 0 on the x-axis and is trying to go to -1
		// aka a crash
		if (dx == -1 && x == 0)
		{
			crash_x = 0x01;
			if (rand() % 3 == 1)
			{
				dx = 1;
			}
			else
			{
				dx = 0;
			}
		}

		// y axis 0 crash
		if (dy == -1 && y == 0)
		{
			crash_y = 0x01;
			if (rand() % 3 == 1)
			{
				dy = 1;
			}
			else
			{
				dy = 0;
			}
		}

		// z axis 0 crash
		if (dz == -1 && z == 0)
		{
			crash_z = 0x01;
			if (rand() % 3 == 1)
			{
				dz = 1;
			}
			else
			{
				dz = 0;
			}
		}

		// x axis 7 crash
		if (dx == 1 && x == 7)
		{
			crash_x = 0x01;
			if (rand() % 3 == 1)
			{
				dx = -1;
			}
			else
			{
				dx = 0;
			}
		}

		// y axis 7 crash
		if (dy == 1 && y == 7)
		{
			crash_y = 0x01;
			if (rand() % 3 == 1)
			{
				dy = -1;
			}
			else
			{
				dy = 0;
			}
		}

		// z azis 7 crash
		if (dz == 1 && z == 7)
		{
			crash_z = 0x01;
			if (rand() % 3 == 1)
			{
				dz = -1;
			}
			else
			{
				dz = 0;
			}
		}

		// mode bit 0 sets crash action enable
		if (mode | 0x01)
		{
			if (crash_x)
			{
				if (dy == 0)
				{
					if (y == 7)
					{
						dy = -1;
					}
					else if (y == 0)
					{
						dy = +1;
					}
					else
					{
						if (rand() % 2 == 0)
						{
							dy = -1;
						}
						else
						{
							dy = 1;
						}
					}
				}
				if (dz == 0)
				{
					if (z == 7)
					{
						dz = -1;
					}
					else if (z == 0)
					{
						dz = 1;
					}
					else
					{
						if (rand() % 2 == 0)
						{
							dz = -1;
						}
						else
						{
							dz = 1;
						}
					}
				}
			}

			if (crash_y)
			{
				if (dx == 0)
				{
					if (x == 7)
					{
						dx = -1;
					}
					else if (x == 0)
					{
						dx = 1;
					}
					else
					{
						if (rand() % 2 == 0)
						{
							dx = -1;
						}
						else
						{
							dx = 1;
						}
					}
				}
				if (dz == 0)
				{
					if (z == 3)
					{
						dz = -1;
					}
					else if (z == 0)
					{
						dz = 1;
					}
					else
					{
						if (rand() % 2 == 0)
						{
							dz = -1;
						}
						else
						{
							dz = 1;
						}
					}
				}
			}

			if (crash_z)
			{
				if (dy == 0)
				{
					if (y == 7)
					{
						dy = -1;
					}
					else if (y == 0)
					{
						dy = 1;
					}
					else
					{
						if (rand() % 2 == 0)
						{
							dy = -1;
						}
						else
						{
							dy = 1;
						}
					}
				}
				if (dx == 0)
				{
					if (x == 7)
					{
						dx = -1;
					}
					else if (x == 0)
					{
						dx = 1;
					}
					else
					{
						if (rand() % 2 == 0)
						{
							dx = -1;
						}
						else
						{
							dx = 1;
						}
					}
				}
			}
		}

		// mode bit 1 sets corner avoid enable
		if (mode | 0x02)
		{
			if (	// We are in one of 8 corner positions
				(x == 0 && y == 0 && z == 0) ||
				(x == 0 && y == 0 && z == 7) ||
				(x == 0 && y == 7 && z == 0) ||
				(x == 0 && y == 7 && z == 7) ||
				(x == 7 && y == 0 && z == 0) ||
				(x == 7 && y == 0 && z == 7) ||
				(x == 7 && y == 7 && z == 0) ||
				(x == 7 && y == 7 && z == 7)
				)
			{
				// At this point, the voxel would bounce
				// back and forth between this corner,
				// and the exact opposite corner
				// We don't want that!

				// So we alter the trajectory a bit,
				// to avoid corner stickyness
				lol = rand() % 3;
				if (lol == 0)
					dx = 0;

				if (lol == 1)
					dy = 0;

				if (lol == 2)
					dz = 0;
			}
		}

		// one last sanity check
		if (x == 0 && dx == -1)
			dx = 1;

		if (y == 0 && dy == -1)
			dy = 1;

		if (z == 0 && dz == -1)
			dz = 1;

		if (x == 7 && dx == 1)
			dx = -1;

		if (y == 7 && dy == 1)
			dy = -1;

		if (z == 7 && dz == 1)
			dz = -1;


		// Finally, move the voxel.
		x = x + dx;
		y = y + dy;
		z = z + dz;

		if (drawmode == 0x01) // show one voxel at time
		{
			setvoxel(x, y, z);
			delay_ms(delay);
			clrvoxel(x, y, z);
		}
		else if (drawmode == 0x02) // flip the voxel in question
		{
			flpvoxel(x, y, z);
			delay_ms(delay);
		} if (drawmode == 0x03) // draw a snake
		{
			for (i = 7; i >= 0; i--)
			{
				snake[i][0] = snake[i - 1][0];
				snake[i][1] = snake[i - 1][1];
				snake[i][2] = snake[i - 1][2];
			}
			snake[0][0] = x;
			snake[0][1] = y;
			snake[0][2] = z;

			for (i = 0; i<8; i++)
			{
				setvoxel(snake[i][0], snake[i][1], snake[i][2]);
			}
			delay_ms(delay);
			for (i = 0; i<8; i++)
			{
				clrvoxel(snake[i][0], snake[i][1], snake[i][2]);
			}
		}


		iterations--;
	}
}

void effect_smileyspin (int count, int del, char bitmap)
{
	unsigned char dybde[] = {0,1,2,3,4,5,6,7,1,1,2,3,4,5,6,6,2,2,3,3,4,4,5,5,3,3,3,3,4,4,4,4};
	int d = 0;
	int flip = 0;
	int x, y, off, s;
	int i;
	for(i = 0; i<count; i++)
	{
		flip = 0;
		d = 0;
		off = 0;
		// front:
		for (s=0;s<7;s++){
			if(!flip){
				off++;
				if (off == 4){
					flip = 1;
					off = 0;
				}
			} else {
				off++;
			}
		        for (x=0; x<8; x++)
        		{
				d = 0;
                		for (y=0; y<8; y++)
	                	{
					if (font_getbitmappixel ( bitmap, 7-x, y)){
						if (!flip) {
							setvoxel(y, dybde[8 * off + d++], x);
						}
						else {
							setvoxel(y, dybde[31 - 8 * off - d++], x);
						}
					} else {
						d++;
					}
				}
			}
			delay_ms(del);
			fill(0x00);
		}

		// side:
		off = 0;
		flip = 0;
		d = 0;
		for (s=0;s<7;s++){
			if(!flip){
				off++;
				if (off == 4){
					flip = 1;
					off = 0;
				}
			} else {
				off++;
			}
		        for (x=0; x<8; x++)
        		{
				d = 0;
                		for (y=0; y<8; y++)
	                	{
					if (font_getbitmappixel ( bitmap, 7-x, y)){
						if (!flip)
							setvoxel(dybde[8 * off + d++], 7 - y,x);
						else
							setvoxel(dybde[31 - 8 * off - d++],7 - y,x);
					} else {
						d++;
					}
				}
			}
			delay_ms(del);
			fill(0x00);
		}


		flip = 0;
		d = 0;
		off = 0;
		// back:
		for (s=0;s<7;s++){
			if(!flip){
				off++;
				if (off == 4){
					flip = 1;
					off = 0;
				}
			} else {
				off++;
			}
		        for (x=0; x<8; x++)
        		{
				d = 0;
                		for (y=0; y<8; y++)
	                	{
					if (font_getbitmappixel ( bitmap, 7-x, 7-y)){
						if (!flip)
							setvoxel(y,dybde[8 * off + d++],x);
						else
							setvoxel(y,dybde[31 - 8 * off - d++],x);
					} else {
						d++;
					}
				}
			}
			delay_ms(del);
			fill(0x00);
		}

		// other side:
		off = 0;
		flip = 0;
		d = 0;
		for (s=0;s<7;s++){
			if(!flip){
				off++;
				if (off == 4){
					flip = 1;
					off = 0;
				}
			} else {
				off++;
			}
		        for (x=0; x<8; x++)
        		{
				d = 0;
                		for (y=0; y<8; y++)
	                	{
					if (font_getbitmappixel ( bitmap, 7-x, 7-y)){
						if (!flip)
							setvoxel(dybde[8 * off + d++], 7 - y,x);
						else
							setvoxel(dybde[31 - 8 * off - d++],7 - y,x);
					} else {
						d++;
					}
				}
			}
			delay_ms(del);
			fill(0x00);
		}

	}
}

void clearvoxel(int x, int y, int z) {
	cube[y][z] = 0x00;
}


// x along with arduino side
// y opposite from arduino
// z up-down
void loop(){
	int i, x, y, z, stack;

	DropBallClass dropBall((char(*)[CUBE_SIZE]) cube, 0);

	SnakeClass snake((char(*)[CUBE_SIZE]) cube, 75);
	point p = { 0, 0, 0 };
	snake.growSnake(p); p.x++;
	snake.growSnake(p); p.z++;
	snake.growSnake(p); p.y++;
	snake.growSnake(p); p.z++;
	snake.growSnake(p);
	
	boolean playAnimation = true;
	while (true)
	{
		blink();
		if (digitalRead(But1) == LOW) {
			playAnimation = !playAnimation;
		}
		if (playAnimation) {
			//stack = 0;
			//maze_recurse(0, 0, 0, 75, stack);
			dropBall.effectDropBall(2, 90);
			delay(200);

			effect_smileyspin(2, 750, 2);
			effect_smileyspin(2, 750, 11);

			effect_wormsqueeze(2, AXIS_Z, -1, 100, 1000);
			effect_wormsqueeze(1, AXIS_Z, 1, 100, 1000);
			effect_z_updown(20, 1000);
			fill(0x00);
			effect_telcstairs(0, 800, 0xff);
			effect_telcstairs(0, 800, 0x00);
			effect_telcstairs(1, 800, 0xff);
			effect_telcstairs(1, 800, 0xff);

			effect_axis_updown_randsuspend(AXIS_Z, 550, 5000, 0);
			effect_axis_updown_randsuspend(AXIS_Z, 550, 5000, 1);
			effect_axis_updown_randsuspend(AXIS_Z, 550, 5000, 0);
			effect_axis_updown_randsuspend(AXIS_Z, 550, 5000, 1);
			effect_axis_updown_randsuspend(AXIS_X, 550, 5000, 0);
			effect_axis_updown_randsuspend(AXIS_X, 550, 5000, 1);
			effect_axis_updown_randsuspend(AXIS_Y, 550, 5000, 0);
			effect_axis_updown_randsuspend(AXIS_Y, 550, 5000, 1);

			boingboing(50, 900, 0x01, 0x01);
			boingboing(250, 600, 0x01, 0x02);
			boingboing(250, 300, 0x01, 0x03);

			cube_stripes(75);			
		
			effect_box_wamp(1000);
			effect_text(1200, "MATRIX");
			effect_rain(250);

			effect_planboing(AXIS_Z, 400);
			effect_planboing(AXIS_Y, 400);
			effect_planboing(AXIS_X, 400);

			//effect_blinky2();

			effect_random_filler(75, 1);
			effect_random_filler(75, 0);

			for (int i = 0; i < 2; i++){

				effect_boxside_randsend_parallel(AXIS_Z, 0, 200, 2);
				delay_ms(1500);
				effect_boxside_randsend_parallel(AXIS_Z, 1, 200, 2);
				delay_ms(1500);

				effect_boxside_randsend_parallel(AXIS_X, 0, 150, 1);
				effect_boxside_randsend_parallel(AXIS_X, 1, 150, 1);

				effect_boxside_randsend_parallel(AXIS_Y, 0, 150, 1);
				effect_boxside_randsend_parallel(AXIS_Y, 1, 150, 1);

				effect_boxside_randsend_parallel(AXIS_Z, 0, 150, 1);
				effect_boxside_randsend_parallel(AXIS_Z, 1, 150, 1);
			}
			snake.effectSnake();
		} // end if (playAnimation)
	}
}

unsigned char bitswap(unsigned char x)//Reverses a byte (so letters aren't backwards);
{
	byte result;

	asm("mov __tmp_reg__, %[in] \n\t"
		"lsl __tmp_reg__  \n\t"   /* shift out high bit to carry */
		"ror %[out] \n\t"  /* rotate carry __tmp_reg__to low bit (eventually) */
		"lsl __tmp_reg__  \n\t"   /* 2 */
		"ror %[out] \n\t"
		"lsl __tmp_reg__  \n\t"   /* 3 */
		"ror %[out] \n\t"
		"lsl __tmp_reg__  \n\t"   /* 4 */
		"ror %[out] \n\t"

		"lsl __tmp_reg__  \n\t"   /* 5 */
		"ror %[out] \n\t"
		"lsl __tmp_reg__  \n\t"   /* 6 */
		"ror %[out] \n\t"
		"lsl __tmp_reg__  \n\t"   /* 7 */
		"ror %[out] \n\t"
		"lsl __tmp_reg__  \n\t"   /* 8 */
		"ror %[out] \n\t"
		:[out] "=r" (result) : [in] "r" (x));
	return(result);
}

// ==========================================================================================
//   Effect functions
// ==========================================================================================

void effect_box_wamp(int delayt)
{
	for (int k = 0; k < 3; k++){
		for (int i = 0; i < 4; i++){
			fill(0x00);
			box_filled(3 - i, 3 - i, 3 - i, 4 + i, 4 + i, 4 + i);
			delay_ms(delayt);
		}
		for (int i = 3; i >= 0; i--){
			fill(0x00);
			box_filled(3 - i, 3 - i, 3 - i, 4 + i, 4 + i, 4 + i);
			delay_ms(delayt);
		}
	}

	for (int k = 0; k < 3; k++){
		for (int i = 0; i < 4; i++){
			fill(0x00);
			box_walls(3 - i, 3 - i, 3 - i, 4 + i, 4 + i, 4 + i);
			delay_ms(delayt);
		}
		for (int i = 3; i >= 0; i--){
			fill(0x00);
			box_walls(3 - i, 3 - i, 3 - i, 4 + i, 4 + i, 4 + i);
			delay_ms(delayt);
		}
	}

	for (int k = 0; k < 3; k++){
		for (int i = 0; i < 4; i++){
			fill(0x00);
			box_wireframe(3 - i, 3 - i, 3 - i, 4 + i, 4 + i, 4 + i);
			delay_ms(delayt);
		}
		for (int i = 3; i >= 0; i--){
			fill(0x00);
			box_wireframe(3 - i, 3 - i, 3 - i, 4 + i, 4 + i, 4 + i);
			delay_ms(delayt);
		}
	}
}

void draw_positions_axis(char axis, unsigned char positions[64], int invert)
{
	int x, y, p;

	fill(0x00);

	for (x = 0; x < 8; x++)
	{
		for (y = 0; y < 8; y++)
		{
			if (invert)
			{
				p = (7 - positions[(x * 8) + y]);
			}
			else
			{
				p = positions[(x * 8) + y];
			}

			if (axis == AXIS_Z)
				setvoxel(x, y, p);

			if (axis == AXIS_Y)
				setvoxel(x, p, y);

			if (axis == AXIS_X)
				setvoxel(p, y, x);
		}
	}

}


void effect_boxside_randsend_parallel(char axis, int origin, int delayt, int mode)
{
	int i;
	int done;
	unsigned char cubepos[64];
	unsigned char pos[64];
	int notdone = 1;
	int notdone2 = 1;
	int sent = 0;

	for (i = 0; i < 64; i++)
	{
		pos[i] = 0;
	}

	while (notdone)
	{
		if (mode == 1)
		{
			notdone2 = 1;
			while (notdone2 && sent < 64)
			{
				i = rand() % 64;
				if (pos[i] == 0)
				{
					sent++;
					pos[i] += 1;
					notdone2 = 0;
				}
			}
		}
		else if (mode == 2)
		{
			if (sent < 64)
			{
				pos[sent] += 1;
				sent++;
			}
		}

		done = 0;
		for (i = 0; i<64; i++)
		{
			if (pos[i] > 0 && pos[i] < 7)
			{
				pos[i] += 1;
			}

			if (pos[i] == 7)
				done++;
		}

		if (done == 64)
			notdone = 0;

		for (i = 0; i < 64; i++)
		{
			if (origin == 0)
			{
				cubepos[i] = pos[i];
			}
			else
			{
				cubepos[i] = (7 - pos[i]);
			}
		}


		delay_ms(delayt);
		draw_positions_axis(axis, cubepos, 0);

	}

}


void effect_rain(int iterations)
{
	int i, ii;
	int rnd_x;
	int rnd_y;
	int rnd_num;

	for (ii = 0; ii < iterations; ii++)
	{
		rnd_num = rand() % 4;

		for (i = 0; i < rnd_num; i++)
		{
			rnd_x = rand() % 8;
			rnd_y = rand() % 8;
			setvoxel(rnd_x, rnd_y, 7);
		}

		delay_ms(1000);
		shift(AXIS_Z, -1);
	}
}

// Set or clear exactly 512 voxels in a random order.
void effect_random_filler(int delayt, int state)
{
	int x, y, z;
	int loop = 0;


	if (state == 1)
	{
		fill(0x00);
	}
	else
	{
		fill(0xff);
	}

	while (loop < 511)
	{
		x = rand() % 8;
		y = rand() % 8;
		z = rand() % 8;

		if ((state == 0 && getvoxel(x, y, z) == 0x01) || (state == 1 && getvoxel(x, y, z) == 0x00))
		{
			altervoxel(x, y, z, state);
			delay_ms(delayt);
			loop++;
		}
	}
}


void effect_blinky2()
{
	int i, r;
	fill(0x00);

	for (r = 0; r<2; r++)
	{
		i = 750;
		while (i>0)
		{
			fill(0x00);
			delay_ms(i);

			fill(0xff);
			delay_ms(200);

			i = i - (15 + (1000 / (i / 10)));
		}

		delay_ms(1000);

		i = 750;
		while (i > 0)
		{
			fill(0x00);
			delay_ms(751 - i);

			fill(0xff);
			delay_ms(200);

			i = i - (15 + (1000 / (i / 10)));
		}
	}

}

// Draw a plane on one axis and send it back and forth once.
void effect_planboing(int plane, int speedd)
{
	int i;
	for (i = 0; i < 8; i++)
	{
		fill(0x00);
		setplane(plane, i);
		delay_ms(speedd);
	}

	for (i = 7; i >= 0; i--)
	{
		fill(0x00);
		setplane(plane, i);
		delay_ms(speedd);
	}
}


// ==========================================================================================
//   Draw functions
// ==========================================================================================


// Set a single voxel to ON
void setvoxel(int x, int y, int z)
{
	if (inrange(x, y, z))
		cube[z][y] |= (1 << x);
}


// Set a single voxel to OFF
void clrvoxel(int x, int y, int z)
{
	if (inrange(x, y, z))
		cube[z][y] &= ~(1 << x);
}

// This function validates that we are drawing inside the cube.
unsigned char inrange(int x, int y, int z)
{
	if (x >= 0 && x < 8 && y >= 0 && y < 8 && z >= 0 && z < 8)
	{
		return 0x01;
	}
	else
	{
		// One of the coordinates was outside the cube.
		return 0x00;
	}
}

// Get the current status of a voxel
unsigned char getvoxel(int x, int y, int z)
{
	if (inrange(x, y, z))
	{
		if (cube[z][y] & (1 << x))
		{
			return 0x01;
		}
		else
		{
			return 0x00;
		}
	}
	else
	{
		return 0x00;
	}
}

// In some effect we want to just take bool and write it to a voxel
// this function calls the apropriate voxel manipulation function.
void altervoxel(int x, int y, int z, int state)
{
	if (state == 1)
	{
		setvoxel(x, y, z);
	}
	else
	{
		clrvoxel(x, y, z);
	}
}

// Flip the state of a voxel.
// If the voxel is 1, its turned into a 0, and vice versa.
void flpvoxel(int x, int y, int z)
{
	if (inrange(x, y, z))
		cube[z][y] ^= (1 << x);
}

// Makes sure x1 is alwas smaller than x2
// This is usefull for functions that uses for loops,
// to avoid infinite loops
void argorder(int ix1, int ix2, int *ox1, int *ox2)
{
	if (ix1 > ix2)
	{
		int tmp;
		tmp = ix1;
		ix1 = ix2;
		ix2 = tmp;
	}
	*ox1 = ix1;
	*ox2 = ix2;
}

// Sets all voxels along a X/Y plane at a given point
// on axis Z
void setplane_z(int z)
{
	int i;
	if (z >= 0 && z < 8)
	{
		for (i = 0; i < 8; i++)
			cube[z][i] = 0xff;
	}
}

// Clears voxels in the same manner as above
void clrplane_z(int z)
{
	int i;
	if (z >= 0 && z < 8)
	{
		for (i = 0; i < 8; i++)
			cube[z][i] = 0x00;
	}
}

void setplane_x(int x)
{
	int z;
	int y;
	if (x >= 0 && x < 8)
	{
		for (z = 0; z < 8; z++)
		{
			for (y = 0; y < 8; y++)
			{
				cube[z][y] |= (1 << x);
			}
		}
	}
}

void clrplane_x(int x)
{
	int z;
	int y;
	if (x >= 0 && x < 8)
	{
		for (z = 0; z < 8; z++)
		{
			for (y = 0; y < 8; y++)
			{
				cube[z][y] &= ~(1 << x);
			}
		}
	}
}

void setplane_y(int y)
{
	int z;
	if (y >= 0 && y < 8)
	{
		for (z = 0; z < 8; z++)
			cube[z][y] = 0xff;
	}
}

void clrplane_y(int y)
{
	int z;
	if (y >= 0 && y < 8)
	{
		for (z = 0; z < 8; z++)
			cube[z][y] = 0x00;
	}
}

void setplane(char axis, unsigned char i)
{
	switch (axis)
	{
	case AXIS_X:
		setplane_x(i);
		break;

	case AXIS_Y:
		setplane_y(i);
		break;

	case AXIS_Z:
		setplane_z(i);
		break;
	}
}

void clrplane(char axis, unsigned char i)
{
	switch (axis)
	{
	case AXIS_X:
		clrplane_x(i);
		break;

	case AXIS_Y:
		clrplane_y(i);
		break;

	case AXIS_Z:
		clrplane_z(i);
		break;
	}
}

// Fill a value into all 64 byts of the cube buffer
// Mostly used for clearing. fill(0x00)
// or setting all on. fill(0xff)
void fill(unsigned char pattern)
{
	int z;
	int y;
	for (z = 0; z < 8; z++)
	{
		for (y = 0; y < 8; y++)
		{
			cube[z][y] = pattern;
		}
	}
}


// Draw a box with all walls drawn and all voxels inside set
void box_filled(int x1, int y1, int z1, int x2, int y2, int z2)
{
	int iy;
	int iz;

	argorder(x1, x2, &x1, &x2);
	argorder(y1, y2, &y1, &y2);
	argorder(z1, z2, &z1, &z2);

	for (iz = z1; iz <= z2; iz++)
	{
		for (iy = y1; iy <= y2; iy++)
		{
			cube[iz][iy] |= byteline(x1, x2);
		}
	}

}

// Darw a hollow box with side walls.
void box_walls(int x1, int y1, int z1, int x2, int y2, int z2)
{
	int iy;
	int iz;

	argorder(x1, x2, &x1, &x2);
	argorder(y1, y2, &y1, &y2);
	argorder(z1, z2, &z1, &z2);

	for (iz = z1; iz <= z2; iz++)
	{
		for (iy = y1; iy <= y2; iy++)
		{
			if (iy == y1 || iy == y2 || iz == z1 || iz == z2)
			{
				cube[iz][iy] = byteline(x1, x2);
			}
			else
			{
				cube[iz][iy] |= ((0x01 << x1) | (0x01 << x2));
			}
		}
	}

}

// Draw a wireframe box. This only draws the corners and edges,
// no walls.
void box_wireframe(int x1, int y1, int z1, int x2, int y2, int z2)
{
	int iy;
	int iz;

	argorder(x1, x2, &x1, &x2);
	argorder(y1, y2, &y1, &y2);
	argorder(z1, z2, &z1, &z2);

	// Lines along X axis
	cube[z1][y1] = byteline(x1, x2);
	cube[z1][y2] = byteline(x1, x2);
	cube[z2][y1] = byteline(x1, x2);
	cube[z2][y2] = byteline(x1, x2);

	// Lines along Y axis
	for (iy = y1; iy <= y2; iy++)
	{
		setvoxel(x1, iy, z1);
		setvoxel(x1, iy, z2);
		setvoxel(x2, iy, z1);
		setvoxel(x2, iy, z2);
	}

	// Lines along Z axis
	for (iz = z1; iz <= z2; iz++)
	{
		setvoxel(x1, y1, iz);
		setvoxel(x1, y2, iz);
		setvoxel(x2, y1, iz);
		setvoxel(x2, y2, iz);
	}

}

// Returns a byte with a row of 1's drawn in it.
// byteline(2,5) gives 0b00111100
char byteline(int start, int end)
{
	return ((0xff << start) & ~(0xff << (end + 1)));
}

// Flips a byte 180 degrees.
// MSB becomes LSB, LSB becomes MSB.
char flipbyte(char byte)
{
	char flop = 0x00;

	flop = (flop & 0b11111110) | (0b00000001 & (byte >> 7));
	flop = (flop & 0b11111101) | (0b00000010 & (byte >> 5));
	flop = (flop & 0b11111011) | (0b00000100 & (byte >> 3));
	flop = (flop & 0b11110111) | (0b00001000 & (byte >> 1));
	flop = (flop & 0b11101111) | (0b00010000 & (byte << 1));
	flop = (flop & 0b11011111) | (0b00100000 & (byte << 3));
	flop = (flop & 0b10111111) | (0b01000000 & (byte << 5));
	flop = (flop & 0b01111111) | (0b10000000 & (byte << 7));
	return flop;
}

// Draw a line between any coordinates in 3d space.
// Uses integer values for input, so dont expect smooth animations.
void line(int x1, int y1, int z1, int x2, int y2, int z2)
{
	float xy;	// how many voxels do we move on the y axis for each step on the x axis
	float xz;	// how many voxels do we move on the y axis for each step on the x axis 
	unsigned char x, y, z;
	unsigned char lasty, lastz;

	// We always want to draw the line from x=0 to x=7.
	// If x1 is bigget than x2, we need to flip all the values.
	if (x1 > x2)
	{
		int tmp;
		tmp = x2; x2 = x1; x1 = tmp;
		tmp = y2; y2 = y1; y1 = tmp;
		tmp = z2; z2 = z1; z1 = tmp;
	}


	if (y1 > y2)
	{
		xy = (float)(y1 - y2) / (float)(x2 - x1);
		lasty = y2;
	}
	else
	{
		xy = (float)(y2 - y1) / (float)(x2 - x1);
		lasty = y1;
	}

	if (z1 > z2)
	{
		xz = (float)(z1 - z2) / (float)(x2 - x1);
		lastz = z2;
	}
	else
	{
		xz = (float)(z2 - z1) / (float)(x2 - x1);
		lastz = z1;
	}



	// For each step of x, y increments by:
	for (x = x1; x <= x2; x++)
	{
		y = (xy*(x - x1)) + y1;
		z = (xz*(x - x1)) + z1;
		delay(15);
		setvoxel(x, y, z);
	}

}

// Delay loop.
// This is not calibrated to milliseconds,
// but we had allready made to many effects using this
// calibration when we figured it might be a good idea
// to calibrate it.
void delay_ms(uint16_t x)
{
	uint8_t y, z;
	for (; x > 0; x--){
		for (y = 0; y < 90; y++){
			for (z = 0; z < 6; z++){
				asm volatile ("nop");
			}
		}
	}
}



// Shift the entire contents of the cube along an axis
// This is great for effects where you want to draw something
// on one side of the cube and have it flow towards the other
// side. Like rain flowing down the Z axiz.
void shift(char axis, int direction)
{
	int i, x, y;
	int ii, iii;
	int state;

	for (i = 0; i < 8; i++)
	{
		if (direction == -1)
		{
			ii = i;
		}
		else
		{
			ii = (7 - i);
		}


		for (x = 0; x < 8; x++)
		{
			for (y = 0; y < 8; y++)
			{
				if (direction == -1)
				{
					iii = ii + 1;
				}
				else
				{
					iii = ii - 1;
				}

				if (axis == AXIS_Z)
				{
					state = getvoxel(x, y, iii);
					altervoxel(x, y, ii, state);
				}

				if (axis == AXIS_Y)
				{
					state = getvoxel(x, iii, y);
					altervoxel(x, ii, y, state);
				}

				if (axis == AXIS_X)
				{
					state = getvoxel(iii, y, x);
					altervoxel(ii, y, x, state);
				}
			}
		}
	}

	if (direction == -1)
	{
		i = 7;
	}
	else
	{
		i = 0;
	}

	for (x = 0; x < 8; x++)
	{
		for (y = 0; y < 8; y++)
		{
			if (axis == AXIS_Z)
				clrvoxel(x, y, i);

			if (axis == AXIS_Y)
				clrvoxel(x, i, y);

			if (axis == AXIS_X)
				clrvoxel(i, y, x);
		}
	}
}

float distance2d(float x1, float y1, float x2, float y2) {
	float dist;
	dist = sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));

	return dist;
}

// Display a sine wave running out from the center of the cube.
void ripples(int iterations, int delay) {
	fill(0x00);  // Clear cube

	float origin_x, origin_y, distance, height, ripple_interval;
	int x, y, i;

	for (i = 0; i < iterations; i++) {
		for (x = 0; x < 8; x++) {
			for (y = 0; y < 8; y++) {
				//distance = Test[x][y];
				distance = distance2d(3.5, 3.5, x, y);
				ripple_interval = 1.3;
				height = 4 + sin(distance / ripple_interval + (float)i / 50) * 4;

				setvoxel(x, y, (int)height);
			}
		}
	}
	delay_ms(delay);
	fill(0x00);  // Clear cube
}

void cube_stripes(int ms)
{
	fill(0x00);
	for (uint8_t i = 0; i < 8; i++) {
		cube[0][0] |= (1 << i);
		cube[1][7] |= (1 << (7 - i));
		cube[2][0] |= (1 << i);
		cube[3][7] |= (1 << (7 - i));
		cube[4][0] |= (1 << i);
		cube[5][7] |= (1 << (7 - i));
		cube[6][0] |= (1 << i);
		cube[7][7] |= (1 << (7 - i));
		delay(ms);
	}
	for (uint8_t j = 1; j < 8; j++) {
		fill(0x00);
		for (uint8_t i = 0; i < 8; i++) {
			cube[0][j] |= (1 << i);
			cube[1][7 - j] |= (1 << i);
			cube[2][j] |= (1 << i);
			cube[3][7 - j] |= (1 << i);
			cube[4][j] |= (1 << i);
			cube[5][7 - j] |= (1 << i);
			cube[6][j] |= (1 << i);
			cube[7][7 - j] |= (1 << i);
		}
		delay(ms);
	}
}