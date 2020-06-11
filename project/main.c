/*******************************************************************
	Snake game program.
	The game can be played on MZ_APO board.

  	Developed by: Silvia Goldasova
  	Date: May 2020

 *******************************************************************/

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
#include "leds_interaction.h"
#include "snake.h"

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */


#define CELL_SIZE 20 
#define BORDER_SIZE 10

#ifndef LCD_WIDTH
#define LCD_WIDTH 480
#endif

#ifndef LCD_HEIGHT
#define LCD_HEIGHT 320
#endif


static void setSerialPort() {
	struct termios oldtio, newtio;
    int fd = open("/dev/stdin", O_RDWR | O_NOCTTY);

    if (fd < 0) {
        exit(2);
    }

    tcgetattr(fd,&oldtio); // save current port settings

	bzero(&newtio, sizeof(newtio));
	
	// set input mode (non-canonical, no echo,...)

	newtio.c_cflag = CS8 | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
		
	newtio.c_cc[VTIME] 	= 75; /* Set timeout of 7.5 seconds */
	newtio.c_cc[VMIN]	= 0;   // blocking read until 1 char received
	
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd,TCSANOW,&newtio);
	
	//tcsetattr(fd,TCSANOW,&oldtio);
}


int main(int argc, char *argv[]) {
	
	time_t t;
   	srand((unsigned) time(&t));
	
	unsigned char *mem_base;
  	unsigned char *parlcd_mem_base;
	
  	//unsigned int c;
  	uint16_t * board = (uint16_t *) calloc(LCD_WIDTH*LCD_HEIGHT, sizeof(uint16_t)); 
	//Cell ** directionArr = (Cell **) calloc(480*320, sizeof(Cell *));   
	Cell * directionArrS1[LCD_WIDTH*LCD_HEIGHT];
	Cell * directionArrS2[LCD_WIDTH*LCD_HEIGHT];

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

	sleep(1);
	printSnakeLengths(board, snakeLengthS1, snakeLengthS2, accum/1000, parlcd_mem_base);

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