/**********************************************************************************************
	Snake game program. The game can be played on MZ_APO board.

	snake.c - Consists of fuctions necessary for initializing game board with snakes and borders, 
    controlling movements of snakes, performing moves, generating food pieces, processing eating,
    and keeping the state of the game.

  	Developed by: Silvia Goldasova
  	Date: May 2020

 ***********************************************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <unistd.h>

#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"
#include "font_functions.h"
#include "leds_interaction.h"
#include "snake.h"

#ifndef LCD_WIDTH
#define LCD_WIDTH 480
#endif

#ifndef LCD_HEIGHT
#define LCD_HEIGHT 320
#endif

#ifndef CELL_SIZE
#define CELL_SIZE 20
#endif

#ifndef BORDER_SIZE
#define BORDER_SIZE 10
#endif

#define APPLE_COLOR 0xE061 //0xC841 //red color  
#define SNAKE_COLOR 0xFFFF
#define BORDER_COLOR 0xFFFE
#define BORDER_SIZE 10

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
			//printf("[%d, %d]; ", randomPosX, randomPosY);
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
	//printf("pos of head: [%d, %d], color: %hu\r\n", head->posX, head->posY, snakeArr[posInSnakeArr]);

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
		//printf("Apple eaten -> updated length: %d\r\n", *length);
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

unsigned char getRandomDirection(unsigned char currentDir) {

	unsigned char verticalDirs[] = {'U', 'D'};
	unsigned char horizontalDirs[] = {'L', 'R'};

	int randomIndex = rand() % 2;
	//printf("random index: %d,  ", randomIndex);

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
	//printf("pressedKey not identified");
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

	//printf("Updating cell head direction: from %c to %c\r\n", lastDirection, newDir);
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
	//printf("going to [%d, %d] - ", applePosX, applePosY);

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

bool isAnyAppleLeft(int * applesArr, int len) { 
	for (int i = 0; i < len; i++) {
		if (applesArr[i] != -1) {
			return true;
		}
	}
	return false;
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
				if (!(isAnyAppleLeft(applesArr, applesCount))){
					isGameOverS1 = true;
					isGameOverS2 = true;
				} 
			}

		}

		// COMPUTER MOVE
		if (!(isGameOverS1)) {
			lightGreenLED(mem_base, 1);
			currentDirS1 = generateComputerMoveDir(board, &chosenAppleIndex, applesArr, directionArrS1[*snakeLengthS1-1], applesCount);
			//printf("current: %c - ", currentDirS1);
			updateDirection(directionArrS1[*snakeLengthS1-1], currentDirS1);
			if (!(snakeMakeMove(board, directionArrS1, snakeLengthS1, mem_base, &isEaten))) {
				isGameOverS1 = true;
				lightRedLED(mem_base, 1);
				//printf("Gameover for S1");
			}
			if (isEaten) {
				applesArr[chosenAppleIndex] = -1;
				isEaten = false;
				lightBlueLED(mem_base, 1);
				if (!(isAnyAppleLeft(applesArr, applesCount))){
					isGameOverS1 = true;
					isGameOverS2 = true;
				}
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
			//printf("current: %c", currentDirS2);
			updateDirection(directionArrS2[*snakeLengthS2-1], currentDirS2);
			if (!(snakeMakeMove(board, directionArrS2, snakeLengthS2, mem_base, &isEaten))) {
				isGameOverS2 = true;
				lightRedLED(mem_base, 2);
				//printf("Gameover for S2");
			}
			if (isEaten) {
				applesArr[chosenAppleIndexS2] = -1;
				isEaten = false;
				lightBlueLED(mem_base, 2);
				if (!(isAnyAppleLeft(applesArr, applesCount))){
					isGameOverS1 = true;
					isGameOverS2 = true;
				} 
			}

		}

		// COMPUTER MOVE
		if (!(isGameOverS1)) {
			lightGreenLED(mem_base, 1);
			currentDirS1 = generateComputerMoveDir(board, &chosenAppleIndexS1, applesArr, directionArrS1[*snakeLengthS1-1], applesCount);
			//printf("current: %c", currentDirS1);
			updateDirection(directionArrS1[*snakeLengthS1-1], currentDirS1);
			if (!(snakeMakeMove(board, directionArrS1, snakeLengthS1, mem_base, &isEaten))) {
				isGameOverS1 = true;
				lightRedLED(mem_base, 1);
				//printf("Gameover for S1");
			}
			if (isEaten) {
				applesArr[chosenAppleIndexS1] = -1;
				isEaten = false;
				lightBlueLED(mem_base, 1);
				if (!(isAnyAppleLeft(applesArr, applesCount))){
					isGameOverS1 = true;
					isGameOverS2 = true;
				}
			}
		}
		//printf("\r\napples left: ");
		for (int i = 0; i < applesCount; i++) {
			if (applesArr[i] != -1) {
				//printf(".");
			}
		}

	}

}

