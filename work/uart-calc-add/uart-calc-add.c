/*******************************************************************
uart-calc-add.c

Template for simple task of serial minimal calculator (add operation
only for now) which receives two decimal unsigned numbers from serial
input (each terminated by new line) and prints sum of these two to
serial output terminated by single newline character. 

The mips-elf-gcc compiler is required to build the code
  https://cw.fel.cvut.cz/wiki/courses/b35apo/documentation/mips-elf-gnu/start

The included Makefile can be used to build the project and run it
in the qtmips_cli simulator variant. To run ELF binary in qtmips_gui
use make to compile binary or run on command line
  mips-elf-gcc -ggdb -nostartfiles -nostdlib -static -march=mips32 crt0local.S uart-calc-add.c -o uart-calc-add

Place file on path work/uart-calc-add/uart-calc-add.c in your
subject personal GIT repository.

Licence: Public Domainx
 *******************************************************************/

#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


/*
 * Next macros provides location of knobs and LEDs peripherals implemented on QtMips simulator.
 * More information can be found on page https://github.com/ppisa/QtMips
 */


#define SERIAL_PORT_BASE   0xffffc000

#define SERP_RX_ST_REG_o         0x00
#define SERP_RX_ST_REG_READY_m    0x1
#define SERP_RX_ST_REG_IE_m       0x2
#define SERP_RX_DATA_REG_o       0x04

#define SERP_TX_ST_REG_o         0x08
#define SERP_TX_ST_REG_READY_m    0x1
#define SERP_TX_DATA_REG_o       0x0c


static inline void serp_write_reg(uint32_t base, uint32_t reg, uint32_t val) {
  *((volatile uint32_t *)(base + reg)) = val;
}

static inline uint32_t serp_read_reg(uint32_t base, uint32_t reg) {
  return *((volatile uint32_t *)(base + reg));
}

// wait till status for reading from uart indicates ready
static inline void waitTillReadFromUARTisReady() {
	while (!(serp_read_reg(SERIAL_PORT_BASE, SERP_RX_ST_REG_o) & SERP_RX_ST_REG_READY_m));
}

// wait till it can be written to uart
static inline void waitTillWriteIsReady() {
	 while (!(serp_read_reg(SERIAL_PORT_BASE, SERP_TX_ST_REG_o) & SERP_TX_ST_REG_READY_m));
}

static inline void serp_tx_byte(int data) {
	while (!(serp_read_reg(SERIAL_PORT_BASE, SERP_TX_ST_REG_o) & SERP_TX_ST_REG_READY_m));
	serp_write_reg(SERIAL_PORT_BASE, SERP_TX_DATA_REG_o, data);
}

static inline int addNumbersFromUART(uint32_t numA, uint32_t numB) {
	numA = numA;
	numB = numB;
	return (int) (numA + numB);
}

static inline int loadCharDigitFromUART(){
	waitTillReadFromUARTisReady();
	int number = * ((volatile uint32_t*) (SERIAL_PORT_BASE + SERP_RX_DATA_REG_o));
	return number;
}

static inline int loadNumberFromUART() {
	int number = 0;
	int digit = loadCharDigitFromUART();
	while ( (char) digit != '\n') {
		number = number * 10 + (digit-'0');
		digit = loadCharDigitFromUART();
	}
	return number;
}

/*
 * The main entry into example program
 */
int main(int argc, char *argv[])
{

	volatile uint32_t numberA = loadNumberFromUART();
	volatile uint32_t numberB = loadNumberFromUART();

	volatile int result = addNumbersFromUART(numberA, numberB);

	//int result = numberA - 48;
	// convert uint_32 to string
	int resultStr[6] = {0, 0, 0, 0, 0, 0};
	for (int i = 0; i < 6; i++) {
		resultStr[i] = (result % 10) + 0x30;
		result = result / 10;
	}

	int leadingZerosCount = 0;
	for (int i = 5; i >= 0; i--) {
		if (resultStr[i] != '0') {
			break;
		}
		leadingZerosCount++;
	}
	int resultLen = 5 - leadingZerosCount;

	for (int i = resultLen; i >= 0; i--) {
		serp_tx_byte(resultStr[i]);
	}
	serp_tx_byte('\n');

  	return 0;
}
