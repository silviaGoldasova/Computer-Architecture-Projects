// apo-sort.S file template, rename and implement the algorithm
// Test algorithm in qtmips_gui program
// Select the CPU core configuration with delay-slot
// This setups requires (for simplicity) one NOP instruction after
// each branch and jump instruction (more during lecture about pipelining)
// The code will be compiled and tested by external mips-elf-gcc
// compiler by teachers, you can try make in addition, but testing
// by internal assembler should be enough

// copy directory with the project to your repository to
// the directory work/apo-sort
// critical is location of the file work/apo-sort/apo-sort.S
// and cache parameters work/apo-sort/d-cache.par
// which is checked by the scripts

// The file d-cache.par specifies D cache parameters in the form
//   <policy>,<#sets>,<#words in block>,<#ways>,<write method>
// The example is
//   lru,1,1,1,wb
// The cache size is limited to 16 words maximum.

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

	la   $a0, array_start
	la   $a1, array_size
	lw   $a1, 0($a1) // number of elements in the array
	sll  $a1, $a1, 2 // number of elements * 4

//Insert your code there


addi $s0, $a0, 0  //Pozice pro kterou se aktualne hleda minimum (offset do pole, zvysuje se po 4 bajtech)
add $s1, $a0, $a1 //Maximalni hodnota indexu/offsetu. Slouzi k ukonceni cyklu = pocet prvku v poli * 4 (aktualne 15 * 4)
add  $s2, $0, $s0 //Pracovni pozice (offset), prvek 
// $s3 - offset nejmensiho nalezeneho prvku v aktualnim behu
// $s4 - hodnota nejmensiho nalezeneho prvku
// $s5 - tmp
// $at - pomocný registr pro kontrukce v assembleru

outer_loop:
	beq $s0, $s1, outer_end	// 0 - (N-2) including
	lw $s4, 0($s0)		// reg $sd = min value from address arr[0]
	add $s3, $s0, $0		// reg $s3 = index of min
	add $s2, $s0, $0		// reg $s2 = working position
	
	inner_loop:
	beq $s2, $s1, inner_loop_end	// while working position $s2 != $s1 max position
		lw $s5, 0($s2)	// $s5 = value from the working position

                    slt  $at, $s4, $s5	
                    bne  $at, $0, no_new_min	// jump if min < actual value 
		nop
			addi $s3, $s2, 0
			addi $s4, $s5, 0
no_new_min:
		j inner_loop
		addi $s2, $s2, 4
inner_loop_end:
	lw $s5, 0($s0)
	sw $s4, 0($s0)
	sw $s5, 0($s3)
	j outer_loop
	addi $s0, $s0, 4
outer_end:

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