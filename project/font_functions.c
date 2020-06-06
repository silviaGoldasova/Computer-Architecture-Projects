#include "font_types.h"

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
