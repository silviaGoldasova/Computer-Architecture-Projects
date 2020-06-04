#define _POSIX_C_SOURCE 200112L

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>

#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"




bool isInRange(int currentRow, int currentCol, int pointX, int pointY, int range) {

	if ( (abs(currentRow-pointX) < range) && (abs(currentCol-pointY) < range)  ) {
		return true;
	}
	return false;

}

/*void basicArr(uint16_t * snakeArr, unsigned char * parlcd_mem_base){

	int pos = 0, c;

	for (int k=0; pos < 480; k++) {
		parlcd_write_cmd(parlcd_mem_base, 0x2c);
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
  		printSnakeToLcd(snakeArr, parlcd_mem_base);

		pos = pos + 10;
	 	sleep(0.01);
	 	printf(".");
	 	printf("#: %d\n", k);
	 }

}*/

int main(int argc, char *argv[]) {
  unsigned char *mem_base;
  unsigned char *parlcd_mem_base;
  int i,j,k;
  //unsigned int c;
  unsigned int c;

  printf("Hello world - updated\n");

  sleep(1);

  /*
   * Setup memory mapping which provides access to the peripheral
   * registers region of RGB LEDs, knobs and line of yellow LEDs.
   */
  mem_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);

  /* If mapping fails exit with error code */
  if (mem_base == NULL)
	exit(1);
  
  parlcd_mem_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);


  if (parlcd_mem_base == NULL)
	exit(1);


  parlcd_hx8357_init(parlcd_mem_base);
  parlcd_write_cmd(parlcd_mem_base, 0x2c);
  for (i = 0; i < 320 ; i++) {
	for (j = 0; j < 480 ; j++) {
	  c = 0;
	  parlcd_write_data(parlcd_mem_base, c);
	}
  }

  	
	int pos = 0;
	for (k=0; pos < 480; k++) {
		parlcd_write_cmd(parlcd_mem_base, 0x2c);
		for (i = 0; i < 320 ; i++) {
			for (j = 0; j < 480 ; j++) {
			

				if (isInRange(i, j, 150, pos, 50)) {
					c = 0xFFFFFFFF;
				} else {
					c = 0x0;
				}
				/*if (i < pos) {
				if ((i == 50 || i == 51 || i == 52) && (j == pos || j == pos+1 || j == pos+2 ) ) {
					c = 0xFFFFFFFF;
				} else {
					c = 0;
				}*/
				parlcd_write_data(parlcd_mem_base, c);

		  }
		}
		pos = pos + 10;
	 	sleep(0.01);
	 	printf(".");
	 	printf("#: %d\n", k);
	 }


 
  for (k=0; k<1; k++) {
	
	parlcd_write_cmd(parlcd_mem_base, 0x2c);
	for (i = 0; i < 320 ; i++) {
	  for (j = 0; j < 480 ; j++) {
		c = 0;
		parlcd_write_data(parlcd_mem_base, c);
	  }
	}
  }


  printf("Goodbye world\n");

  return 0;
}