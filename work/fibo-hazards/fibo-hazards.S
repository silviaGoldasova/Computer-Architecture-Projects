// fibo-hazards.S file template, rename and implement the algorithm
// Test algorithm in qtmips_gui program
// Select the CPU core configuration to
//    Pipelined without hazard unit and cache
// This option select version where RAW dependencies which leads
// to hazards are not resolved in hardware, you need to schedule
// instruction execution according to the pipeline structure
// (classical 5-stage MIPS) such way, that no dependency results
// in a hazard


// copy directory with the project to your repository to
// the directory work/fibo-hazards
// critical is location of the file work/fibo-hazards/fibo-hazards.S
// which is checked by the scripts

// The script loads number of the last Fibonacci series element to compute
// into fibo_limit variable and expects computed series in memory starting
// at address fibo_series, the series has to be followed by at least
// one zero element

// When tested by actual qtmips_cli version, the variant without hazard
// unit cannot be selected (this is WIP for the test script), use qtmips_gui
// which is fully configurable

// Directives to make interesting windows visible
#pragma qtmips show registers
#pragma qtmips show memory

.set noreorder
.set noat

.globl    fibo_limit
.globl    fibo_series

.text
.globl _start
.ent _start

_start:

    	//lui   $10, fibo_series / 0x10000   	// load the upper part of address
	//lui   $11, fibo_limit / 0x10000  	// load the upper part of address
	lui   $11, %hi(fibo_limit)
	addi $1, $0, 0	// int a = 0
	addi $2, $0, 1	// int b = 1

	//ori   $10, $11, fibo_series & 0xFFFF 	// append the lower part of address
	ori   $10, $11, %lo(fibo_series) 	
	//ori   $11, $11, fibo_limit & 0xFFFF 	// append the lower part of address
	ori   $11, $11, %lo(fibo_limit) 
	addi $9, $0, 2	// fibo_limit decreases, until until equal to 2

	sw $1, 0($10)	// save F(0) = 0 to mem
	sw $2, 4($10)	// save F(1) = 1 to mem
	lw   $11, 0($11) 	// number of elements in the array
	addi $10, $10, 8	// increase the offset to the address

	sw $2, 0x40($10)

loop:	
	add $3, $1, $2	// c = b + a
	addi $1, $2, 0	// a = b

	addi $10, $10, 4	// increase the offset to the address
	
	addi $2, $3, 0	// b = c
	sw $3, 0($10)	// save c to mem
	
	bne $11, $9, loop
	addi $11, $11, -1



//Final infinite loop
end_loop:
	cache 9, 0($0)  // flush cache memory
	break           // stop the simulator
	j end_loop
	nop

.end _start

.data
// .align    2 // not supported by QtMips yet

fibo_limit:
	.word 15
fibo_series:
	.skip 1000*4

// Specify location to show in memory window
#pragma qtmips focus memory fibo_limit