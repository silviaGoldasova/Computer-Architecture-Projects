#include "font_types.h"

int getCharWidth(font_descriptor_t* fdes, int ch);

void drawChar(font_descriptor_t* fdes, uint16_t * board, int charWidth, int ch, int posX, int posY);

void drawCharLarger(font_descriptor_t* fdes, uint16_t * board, int charWidth, int ch, int posX, int posY);

void printText(char * str, int length, int posX, int posY, uint16_t * board);