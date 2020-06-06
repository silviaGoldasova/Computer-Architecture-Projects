#define _POSIX_C_SOURCE 200112L

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"
#include "font_functions.h"
//#include "font_types.h"


#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define CELL_SIZE 20 
#define APPLE_COLOR 0xE061 //0xC841 //red color  
#define SNAKE_COLOR 0xFFFF
#define BORDER_COLOR 0xFFFE
#define BORDER_SIZE 10

#define SPILED_REG_LED_LINE_o	0x004
#define SPILED_REG_LED_RGB2_o	0x014

#ifndef LCD_WIDTH
#define LCD_WIDTH 480
#endif

#ifndef LCD_HEIGHT
#define LCD_HEIGHT 320
#endif

typedef struct Cell{
	int posX;
	int posY;
	unsigned char direction; 
} Cell;


static void setSerialPort() {
	struct termios oldtio, newtio;
    int fd = open("/dev/stdin", O_RDWR | O_NOCTTY);

    if (fd < 0) {
        exit(2);
    }

    tcgetattr(fd,&oldtio); // save current port settings

	bzero(&newtio, sizeof(newtio));
	
	newtio.c_cflag = CS8 | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	// set input mode (non-canonical, no echo,...)
	newtio.c_lflag = 0;
		
	//newtio.c_cc[VTIME]    = 0;   // inter-character timer unused
	//newtio.c_cc[VMIN]     = 1;   // blocking read until 1 char received
	newtio.c_cc[VTIME] = 75; /* Set timeout of 10.0 seconds */
	newtio.c_cc[VMIN]     = 0;   // blocking read until 1 char received
	
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd,TCSANOW,&newtio);
	
	//tcsetattr(fd,TCSANOW,&oldtio);
}

void printBoardToLcd(uint16_t * content, unsigned char * parlcd_mem_base) {
	parlcd_write_cmd(parlcd_mem_base, 0x2c);
	for (int i = 0; i < 320 * 480 ; i++) {
		parlcd_write_data(parlcd_mem_base, content[i]);
	}
} 

void blackLcd(unsigned char * parlcd_mem_base) {
	parlcd_write_cmd(parlcd_mem_base, 0x2c);
	for (int i = 0; i < 320 * 480; i++) {
		parlcd_write_data(parlcd_mem_base, 0x0);	
	}
}

void cleanBoardArr(uint16_t * board){
	for (int i = 0; i < 320*480; i++) {
		board[i] = 0x0;
	}
}

bool isInRange(int currentRow, int currentCol, int pointX, int pointY, int range) {

	if ( (abs(currentRow-pointX) < range) && (abs(currentCol-pointY) < range)  ) {
		return true;
	}
	return false;
}

void initializeSnakeAndDirection(int initX, int initY, int snakeLength, uint16_t * snakeArr, Cell ** directionArr) {

	int distance = CELL_SIZE / 2;

	int upperLeftCornerX = initX-distance;
	int upperLeftCornerY = initY-distance;
	int bottomRightCornerX = upperLeftCornerX + snakeLength * CELL_SIZE;
	int bottomRightCornerY = upperLeftCornerY + CELL_SIZE;

	//printf("pos initial (upper left point): [%d, %d]\r\n", upperLeftCornerX, upperLeftCornerY);
	//printf("pos initial (bottom right point): [%d, %d]\r\n", bottomRightCornerX, bottomRightCornerY);

	for(int row = upperLeftCornerY; row < bottomRightCornerY; row++) {
		for (int col = upperLeftCornerX; col < bottomRightCornerX; col++) {
			snakeArr[row * 480 + col] = 0xFFFF;
			//direction[row * 480 + col] = 'R';
		}
	}

	for (int i = 0; i < snakeLength; i++) {
		directionArr[i] = (Cell *) malloc(sizeof(Cell));
		directionArr[i] -> posX = initX; 
		directionArr[i] -> posY = initY;
		directionArr[i] -> direction = 'R';

		//printf("cell: pos [%d, %d], dir: %c;  ", directionArr[i] ->posX, directionArr[i] -> posY, directionArr[i] -> direction);
		initX += CELL_SIZE;
	}

}

void initializeBorders(uint16_t * snakeArr) {

	int offsetToLastLine = (LCD_HEIGHT-1) * LCD_WIDTH;

	int offset = 0;
	for (int j = 0; j < BORDER_SIZE; j++) {
		for (int i = 0; i < LCD_WIDTH; i++) {
			snakeArr[offset+i] = BORDER_COLOR;
			snakeArr[i+offsetToLastLine-offset] = BORDER_COLOR;
		}
		offset += LCD_WIDTH;
	}
	
	int offsetToLastCol = LCD_WIDTH-1;

	for (int i = 0; i < LCD_HEIGHT * LCD_WIDTH; i += LCD_WIDTH) {
		for (int j = 0; j < BORDER_SIZE; j++) {
			snakeArr[j + i] = BORDER_COLOR;
			snakeArr[i+offsetToLastCol-j] = BORDER_COLOR;
		}
	}
	
}

void lightGreenLED(unsigned char * mem_base, int ledNumber){
	uint32_t val_line = 0xFF00;

	if (ledNumber == 1)
		*(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = val_line;
	else
		*(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = val_line;
	
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

void redrawSnakeCell(uint16_t * snakeArr, int posX, int posY, uint16_t color) {

	int distance = CELL_SIZE / 2;

	int upperLeftCornerX = posX - distance;
	int upperLeftCornerY = posY - distance;
	int bottomRightCornerX = upperLeftCornerX + CELL_SIZE;
	int bottomRightCornerY = upperLeftCornerY + CELL_SIZE;

	for(int row = upperLeftCornerY; row < bottomRightCornerY; row++) {
		for (int col = upperLeftCornerX; col < bottomRightCornerX; col++) {
			snakeArr[row * 480 + col] = color;
		}
	}

}

int addApple(uint16_t * snakeArr) {

	bool wasSuccess = false;
	int randomPosX, randomPosY, posInArr;

	int horizontalRange = (LCD_WIDTH - 6 * BORDER_SIZE) / CELL_SIZE;
	int verticalRange = (LCD_HEIGHT - 6 * BORDER_SIZE) / CELL_SIZE;

	while(!(wasSuccess)) {
		
		randomPosX = (rand() % horizontalRange) * CELL_SIZE + 2 * BORDER_SIZE;
		randomPosY = (rand() % verticalRange) * CELL_SIZE + 3 * BORDER_SIZE;
		posInArr = randomPosY * LCD_WIDTH + randomPosX;
		if (snakeArr[posInArr] == 0) {
			printf("[%d, %d]; ", randomPosX, randomPosY);
			redrawSnakeCell(snakeArr, randomPosX, randomPosY, APPLE_COLOR);
			wasSuccess = true;
		}

	}
	return posInArr;
	
}

void distributeApples(uint16_t * snakeArr, int * applesArr, int applesCount){
	
	for (int i = 0; i < applesCount; i++) {
		applesArr[i] = addApple(snakeArr);
	}

}

void shiftDirCell(Cell * cell, unsigned char prevCellDir) {
	switch (cell -> direction){
		case 'U':
			cell -> posY = cell -> posY - CELL_SIZE;
			break;
		case 'D':
			cell -> posY = cell -> posY + CELL_SIZE;
			break;
		case 'L':
			cell -> posX = cell -> posX - CELL_SIZE;
			break;
		case 'R':
			cell -> posX = cell -> posX + CELL_SIZE;
			break;
	}
	cell -> direction = prevCellDir; // now direction of updated position (held in the cell closer to the head) 
}

bool isWithinLCD(int posX, int posY){
	if (posX < BORDER_SIZE || posX > LCD_WIDTH-BORDER_SIZE || posY < BORDER_SIZE || posY > LCD_HEIGHT-BORDER_SIZE) {
		return false;
	}
	return true;
}

bool isCellOccupied(uint16_t * snakeArr, Cell * head){

	int posInSnakeArr = (head->posY) * 480 + head->posX;
	printf("pos of head: [%d, %d], color: %hu\r\n", head->posX, head->posY, snakeArr[posInSnakeArr]);

	if (snakeArr[posInSnakeArr] == SNAKE_COLOR) {
		return true;
	}
	return false;

}

bool isApple(uint16_t * snakeArr, Cell * head) {

	int posInSnakeArr = (head->posY) * 480 + head->posX;

	if (snakeArr[posInSnakeArr] == (uint16_t) APPLE_COLOR) {
		return true;
	}

	return false;

}

bool snakeMakeMove(uint16_t * snakeArr, Cell ** directionArr, int * length, unsigned char * mem_base, bool * isEaten) {

	Cell * head = directionArr[*length-1];
	int headPosX = head->posX;
	int headPosY = head->posY;

	shiftDirCell(head, head->direction);	// shift head cell, direction stays

	if (!(isWithinLCD(head->posX, head->posY))) {
		return false;
	}

	if (isApple(snakeArr, head)){
		*isEaten = true;
		redrawSnakeCell(snakeArr, head->posX, head->posY, 0xFFFF);
		
		directionArr[*length] = directionArr[*length-1];
		directionArr[*length-1] = (Cell *) malloc(sizeof(Cell));
		directionArr[*length-1] -> posX = headPosX; 
		directionArr[*length-1] -> posY = headPosY;
		directionArr[*length-1] -> direction = directionArr[*length]->direction;

		*length = *length + 1;
		printf("Apple eaten -> updated length: %d\r\n", *length);
		return true;
	}

	// changes in snake array: one to make disappear last cell and draw new head cell to the new position
	redrawSnakeCell(snakeArr, directionArr[0]->posX, directionArr[0]->posY, 0x0);

	// for all cells except head
	for (int i = 0; i < *length-1; i++) {
		shiftDirCell(directionArr[i], directionArr[i+1] -> direction);		// shift a cell
	}
	
	if (isCellOccupied(snakeArr, head)) {		// check if in snake array is blank position for the pos to which head is moved
		return false;
	}

	redrawSnakeCell(snakeArr, head->posX, head->posY, 0xFFFF);
	return true;

} 

void updateDirection(Cell * cell, unsigned char newDirection) {
	cell -> direction = newDirection;
}

void basicArr(uint16_t * snakeArr, int pos, unsigned char * parlcd_mem_base){

	int c;

	for (int i = 0; i < 320 ; i++) {
		for (int j = 0; j < 480 ; j++) {
		
			if (isInRange(i, j, 150, pos, 50)) {
				c = 0xFFFF;
			} else {
				c = 0x0;
			}
			
			snakeArr[i*480 + j] = c;
		}
	}

}

unsigned char getRandomDirection(unsigned char currentDir) {

	unsigned char verticalDirs[] = {'U', 'D'};
	unsigned char horizontalDirs[] = {'L', 'R'};

	int randomIndex = rand() % 2;
	printf("random index: %d,  ", randomIndex);

	if (currentDir == 'U' || currentDir == 'D') {
		return horizontalDirs[randomIndex]; 
	} else {
		return verticalDirs[randomIndex]; 
	}

}

unsigned char mapKeyToDirection(unsigned char lastDirection, char pressedKey) {

	// press 'a' for left, 'd' for right

	switch(lastDirection) {
		case 'R':
			if (pressedKey == 'a')	// left
				return 'U';
			else	// right
				return 'D';
			break;
		case 'L':
			if (pressedKey == 'a')	// left
				return 'D';
			else	// right
				return 'U';
			break;
		case 'U':
			if (pressedKey == 'a')	// left
				return 'L';
			else	// right
				return 'R';
			break;
		case 'D':
			if (pressedKey == 'a')	// left
				return 'R';
			else	// right
				return 'L';
			break;
	}
	printf("pressedKey not identified");
	return 'X';

}

unsigned char getKeyboardInput(unsigned char lastDirection){

	char keyPressed;
	int ret = read(0, &keyPressed, 1);
	if (ret != 1)
		return lastDirection;
	
	//printf("ret: %d, %c\r\n", ret, keyPressed);

	if (!(keyPressed == 'a' || keyPressed == 'd')) {
		if (keyPressed == 'e')
			return 'e';
		return lastDirection;
	}
	
	unsigned char newDir = mapKeyToDirection(lastDirection, keyPressed);

	printf("Updating cell head direction: from %c to %c\r\n", lastDirection, newDir);
	return newDir;

}

int isDirPossible(char lastDir, char desiredDir) {

	switch (lastDir) {
		case 'U':
			if (desiredDir != 'D') {
				return 1;
			}
			return -2;
			break;
		case 'R':
			if (desiredDir != 'L') {
				return 1;
			}
			return -2;
			break;
		case 'D':
			if (desiredDir != 'U') {
				return 1;
			}
			return -2;
			break;
		case 'L':
			if (desiredDir != 'R') {
				return 1;
			}
			return -2;
			break;
		default:
			printf("Nonexistent direction in isDirPossible");
			break;
	}
	return 0;

}

char generateComputerMoveDir(uint16_t * board, int * chosenAppleIndex, int * applesArr, Cell * head, int applesCount){

	while (*chosenAppleIndex == -1 || applesArr[*chosenAppleIndex] == -1) {
		*chosenAppleIndex = rand() % applesCount;
	}
	
	int applePosX = applesArr[*chosenAppleIndex] % LCD_WIDTH;
	int applePosY = (applesArr[*chosenAppleIndex] - applePosX) / LCD_WIDTH;
	printf("going to [%d, %d] - ", applePosX, applePosY);

	int possibleDirs[4] = {'U', 'R', 'D', 'L'};
	int possibleDirsVals[4] = {0, 0, 0, 0};

	// is turn possible?
	for (int i = 0; i < 4; i++) {
		possibleDirsVals[i] = isDirPossible(head->direction, possibleDirs[i]);
	}
	
	// does it get snake head closer to the apple?
	if ((applePosX - head->posX) > 0) {
		possibleDirsVals[1] += 1;
	} else if ((applePosX - head->posX) < 0) {
		possibleDirsVals[3] += 1;
	}

	if (applePosY - head->posY > 0) {
		possibleDirsVals[2] += 1;
	} else if ((applePosY - head->posY) < 0) {
		possibleDirsVals[0] += 1;
	}

	int score = 2;
	while (1) {
		for (int i = 0; i < 4; i++) {
			if (possibleDirsVals[i] == score)
				return possibleDirs[i];
		}
		score--;
	}

}

void playRandomVsSSH(unsigned char *mem_base, unsigned char *parlcd_mem_base, int * snakeLengthS1, int * snakeLengthS2, uint16_t * board, Cell ** directionArrS1, Cell ** directionArrS2, int applesCount){

	int applesArr[applesCount];
	distributeApples(board, applesArr, applesCount);
	int chosenAppleIndex = -1;

	unsigned char currentDirS1 = 'R';
	unsigned char currentDirS2 = 'R';

	bool isGameOverS1 = false;
	bool isGameOverS2 = false;
	bool isEaten = false;

	while (!(isGameOverS1) || !(isGameOverS2)) {
		printBoardToLcd(board, parlcd_mem_base);
		sleep(0.5);

		// PLAYER MOVE
		//currentDir = getRandomDirection(currentDirS2);
		if (!(isGameOverS2)) {
			currentDirS2 = getKeyboardInput(currentDirS2);
			if (currentDirS2 == 'e') {
				isGameOverS2 = true;
				continue;
			} 
			lightGreenLED(mem_base, 2);

			updateDirection(directionArrS2[*snakeLengthS2-1], currentDirS2);

			if (!(snakeMakeMove(board, directionArrS2, snakeLengthS2, mem_base, &isEaten))) {
				isGameOverS2 = true;
				lightRedLED(mem_base, 2);
			}

			if (isEaten) {
				lightBlueLED(mem_base, 2);
				for (int i = 0; i < applesCount; i++) {
					if (directionArrS2[*snakeLengthS2-1]->posY * LCD_WIDTH + directionArrS2[*snakeLengthS2-1]->posX == applesArr[i]) {
						applesArr[i] = -1;
						break;
					}
				}
				isEaten = false;
			}

		}

		// COMPUTER MOVE
		if (!(isGameOverS1)) {
			lightGreenLED(mem_base, 1);
			currentDirS1 = generateComputerMoveDir(board, &chosenAppleIndex, applesArr, directionArrS1[*snakeLengthS1-1], applesCount);
			printf("current: %c - ", currentDirS1);
			updateDirection(directionArrS1[*snakeLengthS1-1], currentDirS1);
			if (!(snakeMakeMove(board, directionArrS1, snakeLengthS1, mem_base, &isEaten))) {
				isGameOverS1 = true;
				lightRedLED(mem_base, 1);
				printf("Gameover for S1");
			}
			if (isEaten) {
				applesArr[chosenAppleIndex] = -1;
				isEaten = false;
				lightBlueLED(mem_base, 1);
			}
		}
		

	}

}

void playRandomVsRandom(unsigned char *mem_base, unsigned char *parlcd_mem_base, int * snakeLengthS1, int * snakeLengthS2, uint16_t * board, Cell ** directionArrS1, Cell ** directionArrS2, int applesCount){

	int applesArr[applesCount];
	distributeApples(board, applesArr, applesCount);
	int chosenAppleIndexS1 = -1;
	int chosenAppleIndexS2 = -1;

	unsigned char currentDirS1 = 'R';
	unsigned char currentDirS2 = 'R';

	bool isGameOverS1 = false;
	bool isGameOverS2 = false;
	bool isEaten = false;

	while (!(isGameOverS1) || !(isGameOverS2)) {
		printBoardToLcd(board, parlcd_mem_base);
		sleep(0.8);

		// COMPUTER PLAYER MOVE
		if (!(isGameOverS2)) {
			lightGreenLED(mem_base, 2);
			currentDirS2 = generateComputerMoveDir(board, &chosenAppleIndexS2, applesArr, directionArrS2[*snakeLengthS2-1], applesCount);
			printf("current: %c", currentDirS2);
			updateDirection(directionArrS2[*snakeLengthS2-1], currentDirS2);
			if (!(snakeMakeMove(board, directionArrS2, snakeLengthS2, mem_base, &isEaten))) {
				isGameOverS2 = true;
				lightRedLED(mem_base, 2);
				printf("Gameover for S2");
			}
			if (isEaten) {
				applesArr[chosenAppleIndexS2] = -1;
				isEaten = false;
				lightBlueLED(mem_base, 2);
			}

		}

		// COMPUTER MOVE
		if (!(isGameOverS1)) {
			lightGreenLED(mem_base, 1);
			currentDirS1 = generateComputerMoveDir(board, &chosenAppleIndexS1, applesArr, directionArrS1[*snakeLengthS1-1], applesCount);
			printf("current: %c", currentDirS1);
			updateDirection(directionArrS1[*snakeLengthS1-1], currentDirS1);
			if (!(snakeMakeMove(board, directionArrS1, snakeLengthS1, mem_base, &isEaten))) {
				isGameOverS1 = true;
				lightRedLED(mem_base, 1);
				printf("Gameover for S1");
			}
			if (isEaten) {
				applesArr[chosenAppleIndexS1] = -1;
				isEaten = false;
				lightBlueLED(mem_base, 1);
			}
		}
		

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

void runMenu(int * mode, int * applesCount, uint16_t * board, unsigned char *parlcd_mem_base) {
	
	// menu #1 - select mode
	printMenuMode(board);
	printBoardToLcd(board, parlcd_mem_base);
	
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
	printBoardToLcd(board, parlcd_mem_base);
	
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

	printBoardToLcd(board, parlcd_mem_base);

}

int main(int argc, char *argv[]) {
	
	time_t t;
   	srand((unsigned) time(&t));
	
	unsigned char *mem_base;
  	unsigned char *parlcd_mem_base;
	
  	//unsigned int c;
  	uint16_t * board = (uint16_t *) calloc(480*320, sizeof(uint16_t)); 
	//Cell ** directionArr = (Cell **) calloc(480*320, sizeof(Cell *));   
	Cell * directionArrS1[480*320];
	Cell * directionArrS2[480*320];

	int snakeLengthS1 = 5;
	int snakeLengthS2 = 5;
	
	//Setup memory mapping which provides access to the peripheral  registers region of RGB LEDs, knobs and line of yellow LEDs.
	mem_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
	if (mem_base == NULL) exit(1);

	parlcd_mem_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
	if (parlcd_mem_base == NULL) exit(1);

	parlcd_hx8357_init(parlcd_mem_base);

	// menu
	int mode;
	int applesCount;
	runMenu(&mode, &applesCount, board, parlcd_mem_base);

	setSerialPort();
	printf("Start\r\n");

	initializeSnakeAndDirection(BORDER_SIZE + CELL_SIZE + 10, BORDER_SIZE + 8*CELL_SIZE - 40, snakeLengthS1, board, directionArrS1);
	initializeSnakeAndDirection(BORDER_SIZE + CELL_SIZE + 10, BORDER_SIZE + 8*CELL_SIZE + 100, snakeLengthS2, board, directionArrS2);
	initializeBorders(board);
	
	struct timespec start, stop;
   	clock_gettime(CLOCK_REALTIME, &start);

	if (mode == 1) {
		playRandomVsRandom(mem_base, parlcd_mem_base, &snakeLengthS1, &snakeLengthS2, board, directionArrS1, directionArrS2, applesCount);
	} else {
		playRandomVsSSH(mem_base, parlcd_mem_base, &snakeLengthS1, &snakeLengthS2, board, directionArrS1, directionArrS2, applesCount);
	}
	
	clock_gettime(CLOCK_REALTIME, &stop);
   	double accum = ( stop.tv_sec - start.tv_sec )*1000.0 + ( stop.tv_nsec - start.tv_nsec )/ 1000000.0;
   	printf("\r\nTime: %.6lf s\r\n", accum/1000);

	sleep(1);
	printSnakeLengths(board, snakeLengthS1, snakeLengthS2, accum/1000, parlcd_mem_base);
	printf("S1 length: %d, S2 length: %d", snakeLengthS1, snakeLengthS2);

	sleep(4);
	// leave black screen
	blackLcd(parlcd_mem_base);

	// free pointers to cell direction structures in direction array
	for (int i = 0; i < snakeLengthS1; i++) {
		free(directionArrS1[i]);
	}
	for (int i = 0; i < snakeLengthS2; i++) {
		free(directionArrS2[i]);
	}

	free(board);
  	printf("\r\nEnd\r\n");

  	return 0;
}