			add $sp, $zero, $imm, 4096					# set $sp = 4096 (size of data memory)
			add $t0, $zero, $imm, 3						# set $t0=3
			sub $sp, $sp, $t0		  					# adjust stack for 3 items	(every enter is 4 byte size so only need to sub 3 places for 3 items)

			sw $s0, $sp, $imm, 0						# save s0 in 0($sp)
			sw $s1, $sp, $imm, 1						# save s1 in 1($sp)
			sw $s2, $sp, $imm, 2						# save s2 in 2($sp)
			~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			add $t0, $zero, $imm, 0x100					# t0 = 0x100
			add $a0, $zero, $t0							# a0 = 0x100
			limm $t0, $zero, $zero, 0x110				# t0 = 0x110
			add $a1, $zero, $t0, 0						# a1 = 0x110
			limm $t0, $zero, $zero, 0x120				# t0 = 0x120
			add $v0, $zero, $t0, 0						# v0 = 0x120
			
			
			limm $t0, $zero, $zero, 1					# t0 = 1
			add $s0, $zero, $t0, 0						# int i = 1
			add $s1, $zero, $t0, 0						# int j = 1
			add $s2, $zero, $t0, 0						# int n = 1
			
for1:
			limm $t0, $zero, $zero, 4					# t0 = 4
			sub	$t0, $s0, $t0, 0						# t0 = i - 4
			branch 	bgt, $t0, $zero, exitloop1			# if i - 4 > 0 goto exit loop1

for2:							
			limm $t0, $zero, $zero, 4					# t0 = 4
			sub	$t0, $s2, $t0, 0						# t0 = n - 4
			branch bgt $t0,	$zero, exitloop2			# if n - 4 > 0 goto exit loop2

for3:							
			limm $t0, $zero, $zero, 4					# t0 = 4
			sub	$t0, $s1, $t0, 0						# t0 = j - 4
			branch bgt, $t0, $zero, exitloop3			# if j - 4 > 0 goto exit loop3

			limm $t0, $zero, $zero, 1					# t0 = 1
			sub	$t1, $s0, $t0, 0						# t1 = i - 1
			sub $t2, $s1, $t0, 0						# t2 = j - 1
			limm $t0, $zero, $zero, 4					# t0 = 4
			mul	$t1, $t1, $t0, 0						# t1 = 4(i - 1)

			add $t0, $t1, $t2, 0						# t0 = 4(i - 1) + (j - 1)
			add	$t0, $t0, $a0, 0						# t0 = a0 + 4(i - 1) + (j - 1)
			lw $t0, $t0, $zero, 0						# t0 = a_i,j

			limm $t1, $zero, $zero, 4					# t1 = 4
			mul $t2, $t2, $t1, 0						# t2 = 4(j - 1)
			limm $t1, $zero, $zero, 1					# t1 = 1
			sub $t1, $s2, $t1, 0						# t1 = n - 1
			add $t1, $t2, $t1, 0						# t1 = 4(j - 1) + (n - 1)
			add	$t1, $a1, $t1, 0						# t1 = a1 + 4(j - 1) + (n - 1)
			lw $t1, $t1, $zero, 0						# t1 = b_j,n

			mul $t2, $t0, $t1, 0						# t2 = a_i,j * b_j,n
			add	$t3, $t3, $t2, 0						# t3 = t3 + (a_i,j * b_j,n)

			limm $t0, $zero, $zero, 1					# t0 = 1
			add	$s1, $s1, $t0, 0						# j++

			jal $zero, $zero, $zero, for3				# go to the start of loop 3
			
			
exitloop3:
			limm $t0, $zero, $zero, 1					# t0 = 1
			sub $t1, $s0, $t0, 0						# t1 = i - 1
			sub	$t2, $s2, $t0, 0						# t2 = n - 1
			limm $t0 $zero, $zero, 4					# t0 = 4
			mul $t1, $t1, $t0, 0						# t1 = 4(i - 1)
			
			add $t0, $t1, $t2, 0						# t0 = 4(i - 1) + (n - 1)
			add	$t0, $t0, $v0, 0						# t0 = v0 + 4(i - 1) + (n - 1)
			sw	$t3, $t0, $zero, 0						# store c_i,n in v0 + 4(i - 1) + (n - 1)
			
			add $t3, $zero, $zero, 0					# t3 = 0
			limm $t0, $zero, $zero, 1					# t0 = 1
			add $s2, $s2, $t0, 0						# n++
						
			add $s1, $t0, $zero, 0						# j = 1

			jal $zero, $zero, $zero, for2				# go to the start of loop 2 after finish loop 3


exitloop2:						
			limm $t0, $zero, $zero, 1					# t0 = 1
			add $s0, $s0, $t0, 0						# i++

			add $s2, $t0, $zero, 0						# n = 1

			jal $zero, $zero, $zero, for1				# go to the start of loop 1 after finish loop 2		


exitloop1:	

			lw $s0, $sp, $zero, 0						# restore s0 from sp
			lw $s1, $sp, $zero, 1						# restore s1 from sp	
			lw $s2, $sp, $zero, 2						# restore s2 from sp	
			limm $t0, $zero, $zero, 3					# set $t0=3
			add $sp, $sp, $t0, 0						# pop 3 items from stack			
			halt $zero, $zero, $zero, 0					# finish and exit program