/**********************************************************************************************
	Snake game program. The game can be played on MZ_APO board.

	font_functions.h - Consists of fuctions which take care of printing to the MZ_APO
	LCD display: be it pixels or whole words (using wTahoma_88 font).
	Functions printing texts to the display were grouped to form functions printing whole menus.

  	Developed by: Silvia Goldasova
  	Date: May 2020

 ***********************************************************************************************/


#ifndef FONT_FUNCTIONS_H
#define FONT_FUNCTIONS_H

#include "font_types.h"

int getCharWidth(font_descriptor_t* fdes, int ch);

void drawChar(font_descriptor_t* fdes, uint16_t * board, int charWidth, int ch, int posX, int posY);

void drawCharLarger(font_descriptor_t* fdes, uint16_t * board, int charWidth, int ch, int posX, int posY);

void printText(char * str, int length, int posX, int posY, uint16_t * board);

void printMenuMode(uint16_t * board);

void printMenuAppleCount(uint16_t * board);

int getKeyboardMenuInput();

void cleanBoardArr(uint16_t * board);

void printBoard(uint16_t * content, unsigned char * parlcd_mem_base);

void printSnakeLengths(uint16_t * board, int len1, int len2, double time, unsigned char *parlcd_mem_base);

void runMenu(int * mode, int * applesCount, uint16_t * board, unsigned char *parlcd_mem_base);

#endif