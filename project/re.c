#define _POSIX_C_SOURCE 200112L

#include <stdlib.h>
#include <stdio.h>

#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"


#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define LCD_WIDTH 480
#define LCD_HEIGHT 320
#define CELL_SIZE 20 
#define APPLE_COLOR 0xC841 //red color  
#define SNAKE_COLOR 0xFFFF
#define BORDER_COLOR 0xFFFE
#define BORDER_SIZE 10
#define APPLES_COUNT 7

typedef struct CellDirection{
	int posX;
	int posY;
	unsigned char direction; 
} CellDirection;


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
		
	newtio.c_cc[VTIME]    = 0;   // inter-character timer unused
	newtio.c_cc[VMIN]     = 1;   // blocking read until 1 char received
	
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd,TCSANOW,&newtio);
	
	/*while (STOP==FALSE) {       // loop for input 
		res = read(fd,buf,255);   // returns after 5 chars have been input 
		buf[res]=0;               // so we can printf... 
		printf(":%s:%d\n", buf, res);
		if (buf[0]=='z') STOP=TRUE;
	}*/
	//tcsetattr(fd,TCSANOW,&oldtio);
}

void printSnakeToLcd(uint16_t * content, unsigned char * parlcd_mem_base) {
	parlcd_write_cmd(parlcd_mem_base, 0x2c);
	for (int i = 0; i < 320 * 480 ; i++) {
		parlcd_write_data(parlcd_mem_base, content[i]);
	}
} 

void blackLcd(unsigned char * parlcd_mem_base) {
	int c = 0;
	parlcd_write_cmd(parlcd_mem_base, 0x2c);
	for (int i = 0; i < 320 ; i++) {
		for (int j = 0; j < 480 ; j++) {
			parlcd_write_data(parlcd_mem_base, c);
	 	}
	}
}

bool isInRange(int currentRow, int currentCol, int pointX, int pointY, int range) {

	if ( (abs(currentRow-pointX) < range) && (abs(currentCol-pointY) < range)  ) {
		return true;
	}
	return false;
}

void initializeSnakeAndDirection(int initX, int initY, int snakeLength, uint16_t * snakeArr, CellDirection ** directionArr) {

	int distance = CELL_SIZE / 2;

	int upperLeftCornerX = initX-distance;
	int upperLeftCornerY = initY-distance;
	int bottomRightCornerX = upperLeftCornerX + snakeLength * CELL_SIZE;
	int bottomRightCornerY = upperLeftCornerY + CELL_SIZE;

	//printf("pos initial (upper left point): [%d, %d]\n", upperLeftCornerX, upperLeftCornerY);
	//printf("pos initial (bottom right point): [%d, %d]\n", bottomRightCornerX, bottomRightCornerY);


	for(int row = upperLeftCornerY; row < bottomRightCornerY; row++) {
		for (int col = upperLeftCornerX; col < bottomRightCornerX; col++) {
			snakeArr[row * 480 + col] = 0xFFFF;
			//direction[row * 480 + col] = 'R';
		}
	}

	for (int i = 0; i < snakeLength; i++) {
		directionArr[i] = (CellDirection *) malloc(sizeof(CellDirection));
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

void addApple(uint16_t * snakeArr) {

	bool wasSuccess = false;
	int randomPosX, randomPosY;

	int horizontalRange = (LCD_WIDTH - 2 * BORDER_SIZE) / CELL_SIZE;
	int verticalRange = (LCD_HEIGHT - 2 * BORDER_SIZE) / CELL_SIZE;

	while(!(wasSuccess)) {
		
		randomPosX = (rand() % horizontalRange) * CELL_SIZE + BORDER_SIZE;
		randomPosY = (rand() % verticalRange) * CELL_SIZE + BORDER_SIZE;
		int posInArr = randomPosY * LCD_WIDTH + randomPosX;
		if (snakeArr[posInArr] == 0) {
			printf("[%d, %d]; ", randomPosX, randomPosY);
			redrawSnakeCell(snakeArr, randomPosX, randomPosY, APPLE_COLOR);
			wasSuccess = true;
		}

	}
	
}

void distributeApples(uint16_t * snakeArr){

	for (int i = 0; i < APPLES_COUNT; i++) {
		addApple(snakeArr);
	}

}

void shiftDirCell(CellDirection * cell, unsigned char prevCellDir) {
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
	if (posX < 0 || posX > LCD_WIDTH || posY < 0 || posY > LCD_HEIGHT) {
		return false;
	}
	return true;
}

bool isCellOccupied(uint16_t * snakeArr, CellDirection * head){

	int posInSnakeArr = (head->posY) * 480 + head->posX;
	printf("pos of head: [%d, %d], color: %hu\n", head->posX, head->posY, snakeArr[posInSnakeArr]);

	if (snakeArr[posInSnakeArr] == SNAKE_COLOR) {
		return true;
	}
	return false;

}

bool isApple(uint16_t * snakeArr, CellDirection * head) {

	int posInSnakeArr = (head->posY) * 480 + head->posX;

	if (snakeArr[posInSnakeArr] == APPLE_COLOR) {
		printf("found an apple: %hu\n", snakeArr[posInSnakeArr]);
		return true;
	}

	return false;

}

bool snakeMakeMove(uint16_t * snakeArr, CellDirection ** directionArr, int * length) {

	CellDirection * head = directionArr[*length-1];

	if (!(isWithinLCD(head->posX, head->posY))) {
		return false;
	}

	if (isApple(snakeArr, head)){
		shiftDirCell(head, head->direction);
		redrawSnakeCell(snakeArr, head->posX, head->posY, 0xFFFF);
		*length = *length + 1;
		return true;
	}

	// changes in snake array: one to make disappear last cell and draw new head cell to the new position
	redrawSnakeCell(snakeArr, directionArr[0]->posX, directionArr[0]->posY, 0x0);

	// for all cells except head
	for (int i = 0; i < *length-1; i++) {
		shiftDirCell(directionArr[i], directionArr[i+1] -> direction);		// shift a cell
	}

	shiftDirCell(head, head->direction);	// shift head cell, direction stays
	if (isCellOccupied(snakeArr, head)) {		// check if in snake array is blank position for the pos to which head is moved
		return false;
	}

	redrawSnakeCell(snakeArr, head->posX, head->posY, 0xFFFF);

	return true;

} 

void updateDirection(CellDirection * cell, unsigned char newDirection) {
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

int main(int argc, char *argv[]) {
	
	time_t t;
   	srand((unsigned) time(&t));
	
	unsigned char *mem_base;
  	unsigned char *parlcd_mem_base;
  	//unsigned int c;
  	uint16_t * snake = (uint16_t *) calloc(480*320, sizeof(uint16_t)); 
	//CellDirection ** directionArr = (CellDirection **) calloc(480*320, sizeof(CellDirection *));   
	CellDirection * directionArr[480*320];
	int snakeLength = 5;

	printf("Start\n");
	//Setup memory mapping which provides access to the peripheral  registers region of RGB LEDs, knobs and line of yellow LEDs.
	mem_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
	if (mem_base == NULL) exit(1);

	parlcd_mem_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
	if (parlcd_mem_base == NULL) exit(1);

	parlcd_hx8357_init(parlcd_mem_base);

	initializeSnakeAndDirection(BORDER_SIZE+CELL_SIZE/2+CELL_SIZE, BORDER_SIZE + CELL_SIZE/2 + 8*CELL_SIZE, snakeLength, snake, directionArr);
	initializeBorders(snake);
	distributeApples(snake);
	bool isGameOver = false;
	unsigned char currentDir = 'R';

	while (!(isGameOver)) {
		printSnakeToLcd(snake, parlcd_mem_base);
		sleep(0.7);

		currentDir = getRandomDirection(currentDir);
		updateDirection(directionArr[snakeLength-1], currentDir);
		printf("current dir: %c ", currentDir);
		
		if (!(snakeMakeMove(snake, directionArr, &snakeLength))) {
			isGameOver = true;
		}
	}
	
	setSerialPort();
	char c;
	if (read(0, &c, 1) == 1) {
		printf("%c", c);
		write(1, &c, 1);
	}

	sleep(2);

	// leave black screen
	blackLcd(parlcd_mem_base);

	// free pointers to cell direction structures in direction array
	for (int i = 0; i < snakeLength; i++) {
		free(directionArr[i]);
	}

	free(snake);
	//free(directionArr);
  	printf("End\n");

  	return 0;
}