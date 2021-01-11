		add $sp, $zero, $imm, 4095                  # initiate $sp = 4095 
		add $a0, $zero, $imm, 0x100					# $a0 = &mat1 = 0x100
		add $a1, $zero, $imm, 0x110					# $a1 = &mat2 = 0x110
		jal $imm, $zero, $zero, mulmat				# goto mulmat function
		halt $zero, $zero, $zero, 0
mulmat:
		sub	$sp, $sp, $imm, 6						# adjust stack for 6 items
		sw $ra, $sp, $imm, 0						# save ra in 0($sp)
		sw $s0, $sp, $imm, 1						# save s0 in 1($sp)
		sw $s1, $sp, $imm, 2						# save s1 in 2($sp)
		sw $s2, $sp, $imm, 3						# save s2 in 3($sp)
		sw $a0, $sp, $imm, 4						# save a0 in 4($sp)
		sw $a1, $sp, $imm, 5						# save a1 in 5($sp)

		add $v0, $zero, $imm, 0x120                 # $v0 = &returnedmat
		add $s0, $zero, $zero, 0                    # i = 0
for1:
		add $t0, $zero, $imm, 4						# t0 = 4
	    beq $imm, $t0, $s0, exitloop1				# if (i = 4) -> exitloop1
		add $s1, $zero, $zero, 0                    # j = 0
for2:
		add $t0, $zero, $imm, 4						# t0 = 4
	    beq $imm, $t0, $s1, exitloop2				# if (j = 4) -> exitloop2
		add $t1, $zero, $zero, 0                    # sum = 0
		add $s2, $zero, $zero, 0                    # k = 0
for3:
		add $t0, $zero, $imm, 4						# t0 = 4
	    beq $imm, $t0, $s2, exitloop3				# if (k = 4)  -> exitloop3

		mul $t0, $s0, $imm, 4                       # t0 = i*4
		add $t0, $t0, $s2, 0                        # t0 = i*4 + k
		add $t0, $a0, $t0, 0                        # $t0 = &mat1 + i*4 + k
		lw $t2, $zero, $t0, 0                       # t2 = mat1[i][k]
		sll $t0, $s2, $imm, 2                       # t0 = k*4
		add $t0, $t0, $s1, 0                        # t0 = k*4 + j
		add $t0, $a1, $t0, 0						# $t0 = &mat2 + k*4 + j
		lw $t3, $zero, $t0, 0                       # t3 = mat2[k][j]
		mul $t2, $t2, $t3, 0                        # t2 = mat1[i][k] * mat2[k][j]
		add $t1, $t1, $t2, 0						# sum += mat1[i][k] * mat2[k][j]
		add $s2, $s2, $imm, 1						# k++
		beq $imm, $zero, $zero, for3				# jump -> for3

exitloop3:
		mul $t0, $s0, $imm, 4						# t0 = i*4
		add $t0, $t0, $s1, 0						# t0 = i*4 + j
		sw $t1, $v0, $t0, 0							# store returnedmat[i][j] = sum
		add $s1, $s1, $imm, 1						# j++
		beq $imm, $zero, $zero, for2				# jump -> for2
exitloop2:
		add $s0, $s0, $imm, 1						# i++
		beq $imm, $zero, $zero, for1				# jump -> for1
exitloop1:
		lw $ra, $sp, $imm, 0						# load ra in 0($sp)
		lw $s0, $sp, $imm, 1						# load s0 in 1($sp)
		lw $s1, $sp, $imm, 2						# load s1 in 2($sp)
		lw $s2, $sp, $imm, 3						# load s2 in 3($sp)
		lw $a0, $sp, $imm, 4						# load a0 in 4($sp)
		lw $a1, $sp, $imm, 5						# load a1 in 5($sp)
		beq $ra, $zero, $zero, 0                    # return from function
