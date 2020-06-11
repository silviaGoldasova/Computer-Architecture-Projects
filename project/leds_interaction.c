/*******************************************************************
	Snake game program. The game can be played on MZ_APO board.

	leds_interaction.c	- Groupes all fuctions necessary for managing
	interaction with LEDS on MZ_APO board.

  	Developed by: Silvia Goldasova
  	Date: May 2020

 *******************************************************************/

#define SPILED_REG_LED_LINE_o	0x004
#define SPILED_REG_LED_RGB1_o	0x010
#define SPILED_REG_LED_RGB2_o	0x014

#include <stdint.h>
#include <unistd.h>

void lightGreenLED(unsigned char * mem_base, int ledNumber){
	uint32_t val_line = 0xFF00;

	if (ledNumber == 1)
		*(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = val_line;
	else
		*(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = val_line;

	val_line = 0x0;
	*(volatile uint32_t*)(mem_base + SPILED_REG_LED_LINE_o) = val_line;
	
	sleep(0.5);
}

void lightBlueLED(unsigned char * mem_base, int ledNumber){
	uint32_t val_line = 0xFF;
    
	if (ledNumber == 1)
		*(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = val_line;
	else
		*(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = val_line;
	
	val_line = 0xFFFFFFFF;
	*(volatile uint32_t*)(mem_base + SPILED_REG_LED_LINE_o) = val_line;

	sleep(0.35);
}

void lightRedLED(unsigned char * mem_base, int ledNumber){
	uint32_t val_line = 0xFF0000;

	if (ledNumber == 1)
		*(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = val_line;
	else
		*(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = val_line;

	val_line = 0x0;
	*(volatile uint32_t*)(mem_base + SPILED_REG_LED_LINE_o) = val_line;

	sleep(0.35);
}

void lightDownLED(unsigned char * mem_base, int ledNum) {
	uint32_t val_line = 0x0;

	switch (ledNum) {
		case 1:
			*(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = val_line;
			break;
		case 2:
			*(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = val_line;
			break;
		case 3:
			*(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = val_line;
			*(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = val_line;
			break;
	}

	*(volatile uint32_t*)(mem_base + SPILED_REG_LED_LINE_o) = val_line;
}
