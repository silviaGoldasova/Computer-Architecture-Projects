#define _POSIX_C_SOURCE 200112L

#include <stdlib.h>
#include <stdio.h>
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
#define APPLES_COUNT 25

#define SPILED_REG_LED_LINE_o	0x004
#define SPILED_REG_LED_RGB2_o	0x014

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
		
	newtio.c_cc[VTIME] = 75; /* Set timeout of 10.0 seconds */
	//newtio.c_cc[VTIME]    = 0;   // inter-character timer unused
	//newtio.c_cc[VMIN]     = 1;   // blocking read until 1 char received
	newtio.c_cc[VMIN]     = 0;   // blocking read until 1 char received
	
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd,TCSANOW,&newtio);
	
	/*while (STOP==FALSE) {       // loop for input 
		res = read(fd,buf,255);   // returns after 5 chars have been input 
		buf[res]=0;               // so we can printf... 
		printf(":%s:%d\r\n", buf, res);
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

void lightGreenLEDline(unsigned char * mem_base){
	uint32_t val_line = 0xFF00;
    *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = val_line;
	
	val_line = 0xFFFFFFFF;
	*(volatile uint32_t*)(mem_base + SPILED_REG_LED_LINE_o) = val_line;

	sleep(0.5);
}

void lightRedLEDline(unsigned char * mem_base){
	uint32_t val_line = 0xFF0000;
    *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = val_line;
	sleep(1);
}

void lightDownLEDline(unsigned char * mem_base) {
	uint32_t val_line = 0x0;
    *(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB1_o) = val_line;
	*(volatile uint32_t*)(mem_base + SPILED_REG_LED_RGB2_o) = val_line;
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

bool snakeMakeMove(uint16_t * snakeArr, Cell ** directionArr, int * length, unsigned char * mem_base) {

	Cell * head = directionArr[*length-1];
	int headPosX = head->posX;
	int headPosY = head->posY;

	if (!(isWithinLCD(head->posX, head->posY))) {
		return false;
	}
	
	shiftDirCell(head, head->direction);	// shift head cell, direction stays
	if (isApple(snakeArr, head)){
		lightGreenLEDline(mem_base);
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

void setReadTimeout() {

	/*fd_set selectset;
	struct timeval timeout = {3.5,0}; //timeout of 10 secs.
	FD_ZERO(&selectset);
	FD_SET(0,&selectset);
	int ret =  select(1, &selectset, NULL, NULL, &timeout);*/

	//////////////////////////

	/*char c;
	if (read(0, &c, 1) == 1) {
		blackLcd(parlcd_mem_base);
		printf("c:.%c.\r\n", c);
		//write(1, &c, 1);
	}*/
	
	//setReadTimeout();

	/*fd_set selectset;
	struct timeval timeout = {3.5,0}; //timeout of 10 secs.
	FD_ZERO(&selectset);
	FD_SET(0,&selectset);
	int ret =  select(1, &selectset, NULL, NULL, &timeout);*/
	/*if(ret == 0)	// no data
		printf("No data\r\n");
	else if(ret == -1)	//error
		printf("Error\r\n");
	else  {
		printf("Data avalable: ");
		char c;
		int ret = read(0, &c, 1);
		printf("ret: %d, %c\r\n", ret, c);
	}*/
	// stdin has data, read it
	// (we know stdin is readable, since we only asked for read events
	//and stdin is the only fd in our select set.

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
		printf("other key\r\n");
		return lastDirection;
	}
	
	unsigned char newDir = mapKeyToDirection(lastDirection, keyPressed);

	printf("Updating cell head direction: from %c to %c\r\n", lastDirection, newDir);
	return newDir;

}

int main(int argc, char *argv[]) {
	
	time_t t;
   	srand((unsigned) time(&t));
	
	unsigned char *mem_base;
  	unsigned char *parlcd_mem_base;
	
  	//unsigned int c;
  	uint16_t * snake = (uint16_t *) calloc(480*320, sizeof(uint16_t)); 
	//Cell ** directionArr = (Cell **) calloc(480*320, sizeof(Cell *));   
	Cell * directionArr[480*320];
	int snakeLength = 5;
	setSerialPort();

	printf("Start\r\n");
	//Setup memory mapping which provides access to the peripheral  registers region of RGB LEDs, knobs and line of yellow LEDs.
	mem_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
	if (mem_base == NULL) exit(1);

	parlcd_mem_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
	if (parlcd_mem_base == NULL) exit(1);

	parlcd_hx8357_init(parlcd_mem_base);
	lightDownLEDline(mem_base);

	initializeSnakeAndDirection(BORDER_SIZE+CELL_SIZE/2+CELL_SIZE, BORDER_SIZE + CELL_SIZE/2 + 8*CELL_SIZE, snakeLength, snake, directionArr);
	initializeBorders(snake);
	distributeApples(snake);
	bool isGameOver = false;
	unsigned char currentDir = 'R';


	while (!(isGameOver)) {
		printSnakeToLcd(snake, parlcd_mem_base);
		sleep(0.5);
		lightDownLEDline(mem_base);

		//currentDir = getRandomDirection(currentDir);
		currentDir = getKeyboardInput(currentDir);

		if (currentDir == 'e') break; 
		updateDirection(directionArr[snakeLength-1], currentDir);
		
		if (!(snakeMakeMove(snake, directionArr, &snakeLength, mem_base))) {
			isGameOver = true;
		}
	}
	lightRedLEDline(mem_base);

	sleep(2);

	// leave black screen
	blackLcd(parlcd_mem_base);

	// free pointers to cell direction structures in direction array
	for (int i = 0; i < snakeLength; i++) {
		free(directionArr[i]);
	}

	free(snake);
  	printf("\r\nEnd\r\n");

  	return 0;
}