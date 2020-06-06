#include <stdio.h>
#include "font_types.h"
#include <stdlib.h>
#include <string.h>
#include "mzapo_parlcd.h"


#ifndef LCD_WIDTH
#define LCD_WIDTH 480
#endif

#ifndef LCD_HEIGHT
#define LCD_HEIGHT 320
#endif

#define TEXT_COLOR 0xAD9C

void drawPixel(uint16_t * board, int posX, int posY) {

	if ((unsigned) posX >= LCD_WIDTH || (unsigned) posY > LCD_HEIGHT) {
		return;
	}
	
	board[posY*LCD_WIDTH + posX] = TEXT_COLOR;

}

int getCharWidth(font_descriptor_t* fdes, int ch) {
	int width = 0;
	if ((ch >= fdes->firstchar) && (ch-fdes->firstchar < fdes->size)) {
		ch -= fdes->firstchar;
		
		if (!fdes->width) {
			width = fdes->maxwidth;
		} else {
			width = fdes->width[ch];
		}
	}
	return width;
}

void drawChar(font_descriptor_t* fdes, uint16_t * board, int charWidth, int ch, int posX, int posY) {

	int charOffset = ch - fdes->firstchar;
	uint16_t * charStartPtr = (uint16_t *) fdes->bits + charOffset * fdes->height;

	for (int j = 0; j < fdes->height; j++) {
		uint16_t line = *charStartPtr;
		for (int i = 0; i < charWidth; i++) {		
			
			if (line & 0x8000) {
				drawPixel(board, posX, posY);
			}
			line <<= 1;
		}
		charStartPtr++;
	}

}

void drawCharLarger(font_descriptor_t* fdes, uint16_t * board, int charWidth, int ch, int posX, int posY) {

	int charOffset = ch - fdes->firstchar;
	uint16_t * charStartPtr = (uint16_t *) fdes->bits + fdes-> offset[charOffset];

	for (int j = 0; j < fdes->height; j++) {
		uint16_t line = *charStartPtr;
		for (int i = 0; i < charWidth; i++) {		
			
			if (!(i & 15)) {	// if (i & 0xFF) == 0
				line = *(charStartPtr++);
			}

			if (line & 0x8000) {
				drawPixel(board, posX + i, posY + j);
			}
			line <<= 1;
		}
	}

}

void printText(char * str, int length, int posX, int posY, uint16_t * board) {

	font_descriptor_t* fdes = &font_wTahoma_88;
	int charWidth = 20;

	for (int i=0; i < length; i++) {
		if (*str == ' ') {
			posX += charWidth;
			str++;
			continue;
		}

		charWidth = getCharWidth(fdes, *str);
		drawCharLarger(fdes, board, charWidth, *str, posX, posY);
		posX += charWidth;
		str++;
	}

}

void printMenuMode(uint16_t * board){

	char strMode[]="Mode:";
	char strMode1[]="1:AI vs AI";
	char strMode2[]="2:AI vs Person";
	
	printText(strMode, strlen(strMode), 5, 20, board);
	printText(strMode1, strlen(strMode1), 5, 120, board);
	printText(strMode2, strlen(strMode2), 2, 200, board);

}

void printMenuAppleCount(uint16_t * board) {

	char firstLine[]="Select number";
	char secondLine[]="of food pieces:";
	//char thirdLine[]="pieces:";
	char fourthLine[]="<5, 25>";

	printText(firstLine, strlen(firstLine), 10, 1, board);
	printText(secondLine, strlen(secondLine), 10, 80, board);
	//printText(thirdLine, strlen(thirdLine), 10, 160, board);
	printText(fourthLine, strlen(fourthLine), 50, 180, board);

}

int getKeyboardMenuInput() {
	int num;
	int ret = scanf("%d", &num);

	if (ret != 1) {
		printf("Wrong input. Please, enter the input once again.");
		return -1;
	}

	return num;
}

void cleanBoardArr(uint16_t * board){
	for (int i = 0; i < 320*480; i++) {
		board[i] = 0x0;
	}
}

void printBoard(uint16_t * content, unsigned char * parlcd_mem_base) {
	parlcd_write_cmd(parlcd_mem_base, 0x2c);
	for (int i = 0; i < 320 * 480 ; i++) {
		parlcd_write_data(parlcd_mem_base, content[i]);
	}
}

void printSnakeLengths(uint16_t * board, int len1, int len2, double time, unsigned char *parlcd_mem_base){

	cleanBoardArr(board);

	int strLength1 = snprintf( NULL, 0, "#1 length: %d", len1);
	int strLength2 = snprintf( NULL, 0, "#2 length: %d", len2);
	int strLength3 = snprintf( NULL, 0, "Time %.2f s", time);

	char str1[strLength1];
	char str2[strLength2];
	char str3[strLength3];

	snprintf(str1, strLength1 + 1, "#1 length: %d", len1);
	snprintf(str2, strLength2 + 1, "#2 length: %d", len2);
	snprintf(str3, strLength3 + 1, "Time %.2f s", time);

	printText(str1, strLength1, 5, 30, board);
	printText(str2, strLength2, 5, 130, board);
	printText(str3, strLength3, 5, 230, board);

	printBoard(board, parlcd_mem_base);

}

void runMenu(int * mode, int * applesCount, uint16_t * board, unsigned char *parlcd_mem_base) {
	
	// menu #1 - select mode
	printMenuMode(board);
	printBoard(board, parlcd_mem_base);
	
	printf("Enter the mode number (1 for Computer vs Computer, 2 for Computer  vs Person): ");
	while (1) {
		*mode = getKeyboardMenuInput();
		if (*mode == 1 || *mode == 2) {
			printf("Mode %d has been accepted.\n", *mode);
			break;
		} else {
			if (*mode != -1) {
				printf("Wrong input. Please, enter the input once again.\n");
			}
		}
	}

	cleanBoardArr(board);
	
	// menu #2 - select apples count
	printMenuAppleCount(board);
	printBoard(board, parlcd_mem_base);
	
	printf("Enter the desired number of food pieces from the interval <5, 25>: ");
	while (1) {
		*applesCount = getKeyboardMenuInput();
		if (*applesCount >= 5 && * applesCount <= 25) {
			printf("Apples count %d has been accepted.\n", *applesCount);
			break;
		} else {
			if (*applesCount != -1) {
				printf("Wrong input. Please, enter the input once again.\n");
			}
		}
	
	}
	cleanBoardArr(board);
}
