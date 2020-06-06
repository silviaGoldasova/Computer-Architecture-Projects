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