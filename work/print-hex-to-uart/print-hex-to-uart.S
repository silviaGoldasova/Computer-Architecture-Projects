// print-hex-to-uart.S file template, rename and implement the algorithm
// The task is to process random value value injected into input_val
// unsigned integer variable and print corresponding hexadecimal value
// terminated by line feed (LF, '\n', 0x0a) character to the serial
// port (UART).

// Test algorithm in qtmips_gui program.
// Every CPU configuration conforming MIPS architecture can be chosen.
// Test script select pipelined configuration with hazard unit but without
// cache.

// Copy directory with the project to your repository to
// the directory work/print-hex-to-uart
// critical is location of the file work/print-hex-to-uart/print-hex-to-uart.S
// which is checked by the scripts

// The script loads number to print into input_val global variable
// and the implemented algorithm converts it to series of 8 hexadecimal
// digits finalized by LF ('\n', 0x0a) character. Lowercase digits a to f
// are expected for nibbles within range 10 to 15.

// When tested by actual qtmips_cli version, capture of serial output
// to the file is not implemented yet.

// Directives to make interesting windows visible
#pragma qtmips show registers
#pragma qtmips show memory


// Serial port/terminal registers
// There is mirror of this region at address 0xffff0000
// to match QtSpim and Mars emulators

.equ SERIAL_PORT_BASE,      0xffffc000 // base address of serial port region

.equ SERP_RX_ST_REG,        0xffffc000 // Receiver status register
.equ SERP_RX_ST_REG_o,          0x0000 // Offset of RX_ST_REG
.equ SERP_RX_ST_REG_READY_m,       0x1 // Data byte is ready to be read
.equ SERP_RX_ST_REG_IE_m,          0x2 // Enable Rx ready interrupt

.equ SERP_RX_DATA_REG,      0xffffc004 // Received data byte in 8 LSB bits
.equ SERP_RX_DATA_REG_o,        0x0004 // Offset of RX_DATA_REG

.equ SERP_TX_ST_REG,        0xffffc008 // Transmitter status register
.equ SERP_TX_ST_REG_o,          0x0008 // Offset of TX_ST_REG
.equ SERP_TX_ST_REG_READY_m,       0x1 // Transmitter can accept next byte
.equ SERP_TX_ST_REG_IE_m,          0x2 // Enable Tx ready interrupt

.equ SERP_TX_DATA_REG,      0xffffc00c // Write word to send 8 LSB bits to terminal
.equ SERP_TX_DATA_REG_o,        0x000c // Offset of TX_DATA_REG

.set noreorder
.set noat

.globl    input_val

.text
.globl _start
.ent _start

_start:

	la $1, SERIAL_PORT_BASE  // load base address of serial port
	la $4, input_val
	//lw   $a0, 0($a0) // number to print in hexadecimal to serial port, now loaded in register $4

//Insert your code there
	addi $5, $0, 0x0a // save 0x0a = new line to register $5
	sb $5, 4($4)
	addi $6, $0, 4   // 5 bytes to process - 5 saved to $6


next_char:lb	$10, 0($4)	// load one byte after another to register &10
	beq	$6, $zero, done	// is this the terminal zero byte
	addi	$6, $6, -1	// int i
	addi	$4, $4, 1		// move pointer to next text byte
tx_busy:	lw	$7, SERP_TX_ST_REG_o($1) 	// read status of transmitter
	andi	$7, $7, SERP_TX_ST_REG_READY_m // mask ready bit
	beq	$7, $zero, tx_busy		// if not ready wait for ready condition
	nop				// fill branch instruction delay slot

	srl $11, $10, 4
	andi $11, $11, 0xF
	slti $15, $11, 10		// branch if $11 < 10
	 bne $15, $zero, number_A
	nop
	addi $11, $11, 39 
number_A:
	addi $11, $11, 0x30

	andi $12, $10, 0xF
	slti $15, $12, 10		// branch if $12 < 10
	 bne $15, $zero, number_B
	nop
	addi $12, $12, 39
number_B:
	addi $12, $12, 0x30

	sw	$11,SERP_TX_DATA_REG_o($1) 	// write byte to Tx data register

tx_busy_mid:
	lw	$7, SERP_TX_ST_REG_o($1) 	// read status of transmitter
	andi	$7, $7, SERP_TX_ST_REG_READY_m // mask ready bit
	beq	$7, $zero, tx_busy_mid	// if not ready wait for ready condition
	nop				// fill branch instruction delay slot

	sw	$12,SERP_TX_DATA_REG_o($1) 	// write byte to Tx data register

	beq       $zero, $zero, next_char 	// unconditional branch to process next byte
	nop				// fill delay slot


done:
tx_busy_final:	lw	$7, SERP_TX_ST_REG_o($1) 	// read status of transmitter
	andi	$7, $7, SERP_TX_ST_REG_READY_m 	// mask ready bit
	beq	$7, $zero, tx_busy_final		// if not ready wait for ready condition
	nop
	sw	$5,SERP_TX_DATA_REG_o($1) 	// write byte to Tx data register
	break			// stop continuous execution


//Final infinite loop
end_loop:
	cache 9, 0($0)  // flush cache memory
	break           // stop the simulator
	j end_loop
	nop

.end _start

.data
// .align    2 // not supported by QtMips yet

input_val:
	.word 0x12abff78

// Specify location to show in memory window
#pragma qtmips focus memory input_val
