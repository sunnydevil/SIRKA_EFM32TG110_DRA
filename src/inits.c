/*
 * inits.c
 *
 *  Created on: 21.03.2014
 *      Author: Labor
 */

#include "inits.h"

#define SPI_USART USART1
#define COM_USART USART0
#define PORT_B 1
#define PORT_C 2
#define PORT_D 3
#define PORT_E 4
#define SPI_MOSI 0
#define SPI_MISO 1
#define SPI_CLK 7
#define SPI_GYR 15
#define SPI_ACC 6
#define SPI_MAG 7
#define USART_TX 13
#define USART_RX 12
#define USART_CS 14

void SPI_Init(void)
{	USART1->CTRL = 0x401;
	USART1->FRAME = 0x1005;
	USART1->CLKDIV = 0x100;
	USART1->CMD = (1 << 11) | (1 << 10) | (1 << 4) | (1 << 2) | (1 << 0);
	USART1->IFC = 0x1FF9;
	USART1->ROUTE = 0xB;
	USART1->ROUTE |= USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_CLKPEN;

}


void USART_Init(void)
{
	/*
	 * Geschwindigkeiten f�r USART
	 * CLKDIV -> 0x100 = 1MBit/s	16x OVS
	 * CLKDIV -> 0x40 = 1,5MBit/s	16x OVS
	 * CLKDIV -> 0x0 = 2MBit/s		16x OVS
	 *
	 * CLKDIV -> 0x100 = 2MBit/s 	8x OVS
	 * CLKDIV -> 0x0 = 4MBit/s 		8xOVS
	 *
	 * CLKDIV -> 0x300 = 2MBit/s 	4xOVS
	 * CLKDIV -> 0x0 = 8MBit/s		4xOVS
	 */
	COM_USART->CLKDIV = 0x300;
	COM_USART->CMD = (1 << 11) | (1 << 10) | (1 << 2) | (1 << 0);	// CSMA | MSBF | CCEN | ASYNC
	COM_USART->CTRL |= ( 3 << 5 );	// Oversampling 0 = 16x; 1 = 8x; 2 = 6x; 3 = 4x
	COM_USART->IFC = 0x1FF9;
	COM_USART->ROUTE = 0x303;

}


void GPIO_Init(void)
{	GPIO->P[PORT_B].MODEL = ( 4 << 28 );
	GPIO->P[PORT_B].DOUTCLR = ( 1 << SPI_CLK );
	GPIO->P[PORT_C].MODEH = ( 4 << 28 ) | ( 4 << 24 ); 	//CS_GYR(o), CS_USART(o)
	GPIO->P[PORT_C].MODEL = ( 1 << 4 ) | ( 4 << 0 );	//MISO(i),MOSI(o)
	GPIO->P[PORT_C].DOUTCLR = (1 << USART_CS) | (1 << SPI_MISO ); 	// Initialize USART_CS and SPI_MISO low
	GPIO->P[PORT_C].DOUTSET = (1 << SPI_GYR ) | (1 << SPI_MOSI );	// Initialize SPI_GYR and SPI_MOSI High
	GPIO->P[PORT_D].MODEL = ( 4 << 28 ) | ( 4 << 24 );
	GPIO->P[PORT_D].DOUTSET = ( 1 << SPI_ACC ) | ( 1 << SPI_MAG );
	GPIO->P[PORT_E].MODEH = ( 4 << 20 ) | (1 << 16);  // USART_TX as Output,USART_RX as Input
	GPIO->P[PORT_E].DOUTSET = (1 << USART_TX);   // Initialize USART_TX  high
	GPIO->ROUTE |= GPIO_ROUTE_SWOPEN;						// Enable signal SWO
}

void Timer_Init(void)
{
	TIMER0->CTRL = 0x40;
	TIMER0->TOP = 0x140;						// 10�s

	TIMER1->CTRL = ( 5 << 24 ) | ( 1 << 7 );	// Set Prescaler to 32 for 1�s
	TIMER1->TOP = 0xFFFF;						// Set Top to FFFF
}


void WD_Init()
{
	//WDOG->CTRL = 0xA0C;
	WDOG->CTRL = 0x70C;
	WDOG->CMD = 0x0;
}

void Clocks_Init(void)
{	  CMU->CTRL = 0xC062C;            // Set HF clock divider to 0 to keep core frequency 32MHz
	  CMU->OSCENCMD |= 0x4;           // Enable HFXO Oscillator
	  while(! (CMU->STATUS & 0x8) );  // Wait for XTAL osc to stabilize
	  CMU->HFPERCLKDIV = 0x100;		  // Enable HF Peripheral Clock 32MHz without Divider
	  CMU->HFRCOCTRL = 0x3AE;
	  CMU->LFRCOCTRL = 0x4C;
	  CMU->AUXHFRCOCTRL = 0xB3;
	  CMU->LFCLKSEL = 0x5;			  // Enable ULFRCO for LFBE and LFAE
	  CMU->CMD = 0x2;                 // Select HF XTAL osc as system clock source. 32MHz XTAL
	  CMU->HFCORECLKEN0 = 0x4;
	  CMU->HFPERCLKEN0 = 0x7C; 		// Enable GPIO, Timer0, Timer1, USART0, and USART1 peripheral clocks
	  CMU->LFACLKEN0 = 0x2;			// Enable RTC Clock
	  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;		// Set CoreDebug n CMSIS
	  DWT->CTRL |= 1;										// Activate DWT								// Activate DWT

	  /* Write MSC unlock code to enable interface */
	  	  MSC->LOCK = MSC_UNLOCK_CODE;

}

//void reset_clocks(void)
//{
//	CMU_ClockEnable(cmuClock_DBG, false);   	// Enable DEBUG Interface peripheral clock
//	CMU_ClockEnable(cmuClock_TIMER0, false);   // Enable TIMER0 peripheral clock
//	CMU_ClockEnable(cmuClock_GPIO, false);   	// Enable GPIO peripheral clock
//	CMU_ClockEnable(cmuClock_USART0, false);   // Enable RS485 peripheral clock
//	CMU_ClockEnable(cmuClock_USART1, false);   // Enable SPI peripheral clock
//	GPIO_PinModeSet(gpioPortC, 0, gpioModeDisabled, 0);		// SPI-MOSI as Output, IDLE High
//	GPIO_PinModeSet(gpioPortC, 1, gpioModeDisabled, 0);		// SPI-MISO as Input, IDLE Low
//	GPIO_PinModeSet(gpioPortB, 7, gpioModeDisabled, 0);		// SPI-CLK as Output, IDLE Low
//	GPIO_PinModeSet(gpioPortC, 15, gpioModeDisabled, 0);	// SPI-CS-GYRO as Output, IDLE High
//	GPIO_PinModeSet(gpioPortD, 6, gpioModeDisabled, 0);		// SPI-CS-ACC as Output, IDLE High
//	GPIO_PinModeSet(gpioPortD, 7, gpioModeDisabled, 0);		// SPI-CS-MAG as Output, IDLE High
//	GPIO_PinModeSet(gpioPortE, 13, gpioModeDisabled, 0);	// USART_TX as Output, IDLE High
//	GPIO_PinModeSet(gpioPortE, 12, gpioModeInput, 0);		// USART_RX as Input, IDLE Low
//	GPIO_PinModeSet(gpioPortC, 14, gpioModeDisabled, 0);	// RS485 Read/Write select, IDLE Low for READ
//}
