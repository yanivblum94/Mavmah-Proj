		add $sp, $zero, $imm, 4095
		lw $a0, $zero, $imm, 0x100                  # load  n from  memory
		lw $a1, $zero, $imm, 0x101                  # load  k from  memory
		blt $imm, $a0, $a1, halt                    # if (n < k) -> halt
		beq $imm, $a1, $a0, equal					# if (k = n) -> equal
		jal $imm, $zero, $zero, bin                 # go to binom function
		sw $v0, $zero, $imm, 0x102                  # store the result value
		beq $imm, $zero, $zero, halt                # finished calculating -> halt
equal:
		add $t3, $zero, $imm, 1              
		sw $t3, $zero, $imm, 0x102                  # save bin(n, k) = 1 
halt:
		halt $zero, $zero, $zero, 0

bin:	
		sub	$sp, $sp, $imm, 4						# adjust stack for 4 items	(every enter is 4 byte size so only need to sub 4 places for 4 items)
		sw $s0, $sp, $imm, 0						# save s0 in 0($sp)
		sw $a0, $sp, $imm, 1						# save a0 in 1($sp)
		sw $a1, $sp, $imm, 2						# save a1 in 2($sp)
		sw $ra, $sp, $imm, 3						# save ra in 3($sp)
 
		add $v0, $zero, $imm, 1						# set v0 = 1
		beq $imm, $a1, $a0, return					# if (k == n) -> return 
		beq $imm, $a1, $zero, return				# if (k == 0) -> return

		sub $a1, $a1, $imm, 1						# k -= 1 
		sub $a0, $a0, $imm, 1						# n -= 1
		jal $imm, $zero, $zero, bin					# recursive call: $v0 = bin(n - 1, k - 1)
		add $s0, $zero, $v0, 0						# save bin(n-1,k-1) in s0
		add $a1, $a1, $imm, 1						# k = k + 1 restoring for next call
		jal $imm, $zero, $zero, bin					# recursive call: $v0 = bin(n - 1, k)
		add $v0, $v0, $s0, 0						# v0 = bin(n - 1, k - 1) + bin(n - 1, k)

return:		
		lw $s0, $sp, $imm, 0						# restore s0 from 0($sp)
		lw $a0, $sp, $imm, 1						# restore a0 from 1($sp)
		lw $a1, $sp, $imm, 2						# restore a1 from 2($sp)
		lw $ra, $sp, $imm, 3						# restore ra from 3($sp)
		add $sp, $sp, $imm, 4						# restore aloocated space from stack
		beq $ra, $zero, $zero, 0					# return to pc = $ra 