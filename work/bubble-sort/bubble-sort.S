// bubble-sort.S file template, rename and implement the algorithm
// Test algorithm in qtmips_gui program
// Select the CPU core configuration with delay-slot
// This setups requires (for simplicity) one NOP instruction after
// each branch and jump instruction (more during lecture about pipelining)
// The code will be compiled and tested by external mips-elf-gcc
// compiler by teachers, you can try make in addition, but testing
// by internal assembler should be enough

// copy directory with the project to your repository to
// the directory work/bubble-sort
// critical is location of the file work/bubble-sort/bubble-sort.S
// which is checked by the scripts

// Directives to make interesting windows visible
#pragma qtmips show registers
#pragma qtmips show memory

.set noreorder
.set noat

.globl    array_size
.globl    array_start

.text
.globl _start
.ent _start

_start:
	
	// data inicialization

	la $10, array_start // saves address of arr start to register
	la $11, array_size  // get address of the array_size
	lw $11, 0($11)	// save the array_size N to the register

	addi $1, $0, 0      // int i=0 {for the outer loop)
	addi $2, $11, -1      // horna medza vonkajsieho cyklu = N-1	
	addi $4, $11, -1	// upper boundary of the inner loop = N-1
			// tmp regiter = $5
	
outer:
	beq $1, $2, outer_done
	nop
	addi $1, $1, 1	// i++
	
	addi $3, $0, 0	// int j = 0 for the inner loop
	add $8, $10, $0  	// temp for addresses
inner:	
	beq $3, $4, inner_done
	nop	
	addi $3, $3, 1	// j++	

  	lw $6, 0($8)	// arr[j]
	lw $7, 4($8)	// arr[j+1]
 	
	slt $9, $7, $6	// bge $7, $6, end_if
	beq $9, $0, end_if
	nop
	add $5, $0, $7	// tmp = arr[j+1]
	add $7, $0, $6	// arr[j+1] = arr[j]
	add $6, $0, $5	// arr[j] = tmp
	sw $6, 0($8)
	sw $7, 4($8)
end_if:

  	addi $8, $8, 4
	j inner
	nop	
inner_done:
	addi $4, $4, -1 	// setting the boundaries for the inner loop
	j outer
	nop
outer_done:	


//Final infinite loop
end_loop:
	cache 9, 0($0)  // flush cache memory
	break           // stop the simulator
	j end_loop
	nop

.end _start

.data
// .align    2 // not supported by QtMips yet

array_size:
.word	15
array_start:
.word	5, 3, 4, 1, 15, 8, 9, 2, 10, 6, 11, 1, 6, 9, 12

// Specify location to show in memory window
#pragma qtmips focus memory array_size
