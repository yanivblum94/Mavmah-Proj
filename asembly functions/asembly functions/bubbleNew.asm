		sub $sp, $sp, $imm, 3		  		# adjust stack for 3 items (every enter is 4 byte size so only need to sub 3 places for 3 items)
		sw $s0, $sp, $imm, 0				# save s0 in 0($sp)
		sw $s1, $sp, $imm, 1				# save s1 in 1($sp)
		sw $s2, $sp, $imm, 2				# save s2 in 2($sp)
		add $s1, $zero, $zero, 0			# s1 = i = 0	
		add $s0, $zero, $imm, 1024		    # s0 = 0x10241024

for1:	
		sub $t0, $s1, $imm, 15				# t0 = i - 15
		bgt $imm, $t0, $zero, exit1		    # if (i > 15) -> exitloop1
		add $s2, $zero, $zero, 0			# s2 = j = 0
for2:	
		sub $t1, $s2, $imm, 14				# t1 = j - 14
		bgt $imm, $t1, $zero, exit2		    # if (j > 14) -> exitloop2
		lw $t3, $s0, $s2, 0                 # load arr[j] to $t3
		add $t1, $s2, $imm, 1				# t1 = j + 1
		lw $t0, $t1, $s0, 0				    # load arr[j + 1] to $t0
		bge $imm, $t0, $t3, else		    # if (arr[j + 1] > arr[j]) -> else  
		sw $t3, $s0, $t1,  0			     # arr[j + 1] = arr[j] 
		sw $t0, $s0, $s2, 0			     	# arr[j] = arr[j + 1]
else:	
		add $s2, $s2, $imm, 1				# j += 1
		beq $imm, $zero, $zero, for2		# -> for2 
exit2: 		
		add $s1, $s1, $imm, 1				# i += 1
		jal $imm, $zero, $zero, for1		# go to for1 
exit1:
		lw $s0, $sp, $zero, 0				# restore s0 from 0($sp)
		lw $s1, $sp, $zero, 1				# restore s1 from 1($sp)
		lw $s2, $sp, $zero, 2				# restore s2 from 2($sp)
		add $sp, $sp, $imm, 3				# restore aloocated space from stack		
		halt $zero, $zero, $zero, 0			# halt program