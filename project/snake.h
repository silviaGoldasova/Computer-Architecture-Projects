
#ifndef SNAKE_H
#define SNAKE_H

typedef struct Cell{
	int posX;
	int posY;
	unsigned char direction; 
} Cell;

void printBoardToLcd(uint16_t * content, unsigned char * parlcd_mem_base);

void blackLcd(unsigned char * parlcd_mem_base);

bool isInRange(int currentRow, int currentCol, int pointX, int pointY, int range);

void initializeSnakeAndDirection(int initX, int initY, int snakeLength, uint16_t * snakeArr, Cell ** directionArr);

void initializeBorders(uint16_t * snakeArr);

void redrawSnakeCell(uint16_t * snakeArr, int posX, int posY, uint16_t color);

int addApple(uint16_t * snakeArr);

void distributeApples(uint16_t * snakeArr, int * applesArr, int applesCount);

void shiftDirCell(Cell * cell, unsigned char prevCellDir);

bool isWithinLCD(int posX, int posY);

bool isCellOccupied(uint16_t * snakeArr, Cell * head);

bool isApple(uint16_t * snakeArr, Cell * head);

bool snakeMakeMove(uint16_t * snakeArr, Cell ** directionArr, int * length, unsigned char * mem_base, bool * isEaten);

void updateDirection(Cell * cell, unsigned char newDirection);

unsigned char getRandomDirection(unsigned char currentDir);

unsigned char mapKeyToDirection(unsigned char lastDirection, char pressedKey);

unsigned char getKeyboardInput(unsigned char lastDirection);

int isDirPossible(char lastDir, char desiredDir);

char generateComputerMoveDir(uint16_t * board, int * chosenAppleIndex, int * applesArr, Cell * head, int applesCount);

bool isAnyAppleLeft(int * applesArr, int len);

void playRandomVsSSH(unsigned char *mem_base, unsigned char *parlcd_mem_base, int * snakeLengthS1, int * snakeLengthS2, uint16_t * board, Cell ** directionArrS1, Cell ** directionArrS2, int applesCount);

void playRandomVsRandom(unsigned char *mem_base, unsigned char *parlcd_mem_base, int * snakeLengthS1, int * snakeLengthS2, uint16_t * board, Cell ** directionArrS1, Cell ** directionArrS2, int applesCount);

#endif