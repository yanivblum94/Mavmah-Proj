		add $s0, $imm, $zero, 1023          # cycels = 1024
		add $s1, $imm, $zero, 31            # num_leds = 31
		add $t0, $zero, $zero, 0		  	# $to = i = 0
		add $t1, $zero, $imm, 1             # $t1 = k = 1
		
for:	
		add $t2, $zero, $zero, 0            # $t2 = j = 0
		mul $t0, $t0, $imm, 2				# i = i * 2
		or $t0, $t0, $imm, 1                # i += 1
		sra $t3, $t0, $imm, 31              # $s3 = n = 31 >> i
		and $t3, $t3, $imm, 0x1             # bitwise-and(n, i) 
		beq $imm, $t3, $t1, exit            # if (n == 1) -> exit
		out $t0, $imm, $zero, 9		        # turn on one more led		
		beq $imm, $zero, $zero, wait        # go to wait for 1024 cycles

wait:	
		add $t2, $t2, $imm, 1				# j += 1
		beq $imm, $t2, $s0, for		        # if (f == 1024) return to for loop
		beq $imm, $zero, $zero, wait

exit:	
		halt $zero, $zero, $zero, 0         # exit 
