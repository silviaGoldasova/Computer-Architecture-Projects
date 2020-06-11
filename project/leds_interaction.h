/*******************************************************************
	Snake game program. The game can be played on MZ_APO board.

	leds_interaction.h	- Groupes all fuctions necessary for managing
	interaction with LEDS on MZ_APO board.

  	Developed by: Silvia Goldasova
  	Date: May 2020

 *******************************************************************/

#ifndef LEDS_INTERACTION_H
#define LEDS_INTERACTION_H

void lightGreenLED(unsigned char * mem_base, int ledNumber);

void lightBlueLED(unsigned char * mem_base, int ledNumber);

void lightRedLED(unsigned char * mem_base, int ledNumber);

void lightDownLED(unsigned char * mem_base, int ledNum);

#endif