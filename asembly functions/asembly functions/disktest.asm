		add $s0, $zero, $imm, 128        # 128 words in sector
		add $s1, $zero, $imm, 1          # $s1 = flag = 1
		add $s2, $zero, $imm, 0          # $s2 = j = 0   
		add $t0, $zero, $imm, 1          # the number of irq1 enable
		out $imm, $zero, $t0, 1          # irq1enable = 1
read1:
		add $t3, $zero, $imm, 16         # disk buffer num
		out $imm, $t3, $zero, 0          #  disk buffer  address= 0x0
		add $t3, $zero, $imm, 15         # disk sector
		out $imm, $t3, $zero, 0          # sector 0
		add $t1, $zero, $imm, 1          # cmd = 1 (read)
		add $s2, $s2, $imm, 1            # j += 1
		in $t3, $zero, $imm, 17          # get disk status
		bne $imm, $t3, $zero, wait       # if (diskstatus != 0) ->  wait
		beq $imm, $zero, $zero, diskop   # if disk is free -> diskop
read2:
		add $t3, $zero, $imm, 16         # disk buffer num
		out $imm, $t3, $zero, 512        # disk buffer  address= 0x200
		add $t3, $zero, $imm, 15         # disk sector
		out $imm, $t3, $zero, 1          # sector 1
		add $t1, $zero, $imm, 1          # cmd = 1 (read)
		add $s2, $s2, $imm, 1            # j += 1
		in $t3, $zero, $imm, 17          # get disk status
		bne $imm, $t3, $zero, wait       # if (diskstatus != 0) ->  wait
		beq $imm, $zero, $zero, diskop   # if disk is free -> diskop
read3:
		add $t3, $zero, $imm, 16         # disk buffer num
		out $imm, $t3, $zero, 1024       # disk buffer  address= 0x400
		add $t3, $zero, $imm, 15         # disk sector
		out $imm, $t3, $zero, 2          # sector 2
		add $t1, $zero, $imm, 1          # cmd = 1 (read)
		in $t3, $zero, $imm, 17          # get disk status
		bne $imm, $t3, $zero, wait       # if (diskstatus != 0) ->  wait
		beq $imm, $zero, $zero, diskop   # if disk is free -> diskop
read4:
		add $t3, $zero, $imm, 16         # disk buffer num
		out $imm, $t3, $zero, 1536       # disk buffer  address= 0x600
		add $t3, $zero, $imm, 15         # disk sector
		out $imm, $t3, $zero, 3          # sector 3
		add $t1, $zero, $imm, 1          # cmd = 1 (read)
		add $s2, $s2, $imm, 1            # j += 1
		in $t3, $zero, $imm, 17          # get disk status
		bne $imm, $t3, $zero, wait       # if (diskstatus != 0) ->  wait
		beq $imm, $zero, $zero, diskop   # if disk is free -> diskop
xor:
		add $s1, $zero, $zero, 0         # flag = 0 - ignore irq1)
		add $t0, $zero, $zero, 0         # i = 0
xorloop:
		beq $imm, $t0, $s0, done         # if i == 128 -> done
		lw $t2, $t0, $imm, 0             # load the i word of the first sector
		lw $t3, $t0, $imm, 512           # lead the i word of the second sector
		xor $t2, $t2, $t3, 0             # (i word of the first sector) xor (i word of the second sector)
		lw $t3, $t0, $imm, 1024          # lead the i word of the third sector
		xor $t2, $t2, $t3, 0             # (i word of the first sector) xor (i word of the second sector) xor (i word of the third sector)
		lw $t3, $t0, $imm, 1536          # lead the i word of the fourth sector
		xor $t2, $t2, $t3, 0             # (i word of the first sector) xor (i word of the second sector) xor (i word of the third sector) xor (i word of the fourth sector)
		sw $t2, $t0, $imm, 2048          # store the xor of the i words
		add $t0, $t0, $imm, 1            # i += 1
		beq $imm, $zero, $zero, xorloop  # do one more xor
done:
		add $t0, $zero, $imm, 16         # set the number of disk buffer reg
		out $imm, $zero, $t0, 2048       # set the address of disk buffer to 2048
		add $t0, $zero, $imm, 15         # set the number of disk sector reg
		out $imm, $zero, $t0, 4          # sector 4
		add $t1, $zero, $imm, 2          # cmd = 2 (write)
		add $s2, $s2, $imm, 1            # j += 1
		add $s1, $zero, $imm, 1          # flag = 1
		in $t0, $zero, $imm, 17          # get disk status
		bne $imm, $t0, $zero, wait       # if (diskstatus != 0) ->  wait
		beq $imm, $zero, $zero, diskop   # if disk is free -> diskop
wait:
		add $t0, $zero, $imm, 6          # the number of irq handler
		out $imm, $zero, $t0, irq     # set the pc of irq hendler to diskop
loop:
		beq $imm, $zero, $zero, loop     # infinite loop
irq:
		beq $imm, $s1, $zero, finalReturn  # if flag == 0 return from regular ret
		add $t0, $zero, $imm, 14         # the number of disk cmd
		out $t1, $zero, $t0, 0           # disk cmd = op that was set on $t1 before the wait
		add $t1, $zero, $imm, 1          # temp = 1
		beq $imm, $s2, $t1, return2      # if j == 1 return from irq to read2
		add $t1, $zero, $imm, 2          # temp = 2
		beq $imm, $s2, $t1, return3      # if j == 2 return from irq to read3
		add $t1, $zero, $imm, 3          # temp = 3
		beq $imm, $s2, $t1, return4      # if j == 3 return from irq to read4
		add $t1, $zero, $imm, 4          # temp = 4
		beq $imm, $s2, $t1, return5      # if j == 4 return from irq to xor
		add $t1, $zero, $imm, 5          # temp = 5
		beq $imm, $s2, $t1, exit         # if j == 5 return from irq to exit
diskop:
		add $t0, $zero, $imm, 14         # the number of disk cmd
		out $t1, $zero, $t0, 0           # disk cmd = op that was set on $t1 before the call
		add $t1, $zero, $imm, 1          # temp = 1
		beq $imm, $s2, $t1, read2        # if j == 1 return from irq to read2
		add $t1, $zero, $imm, 2          # temp = 2
		beq $imm, $s2, $t1, read3        # if j == 2 return from irq to read3
		add $t1, $zero, $imm, 3          # temp = 3
		beq $imm, $s2, $t1, read4        # if j == 3 return from irq to read4
		add $t1, $zero, $imm, 4          # temp = 4
		beq $imm, $s2, $t1, xor      # if j == 4 return from irq to xor
		add $t1, $zero, $imm, 5          # temp = 5
		beq $imm, $s2, $t1, exit         # if j == 5 return from irq to exit
return2:
		add $t0, $zero, $imm, 7          # the number of irqreturn reg
		out $imm, $zero, $t0, read2      
		reti $zero, $zero, $zero, 0      # return from irq1
return3:
		add $t0, $zero, $imm, 7          # the number of irqreturn reg
		out $imm, $zero, $t0, read3      
		reti $zero, $zero, $zero, 0      # return from irq1
return4:
		add $t0, $zero, $imm, 7          # the number of irqreturn reg
		out $imm, $zero, $t0, read4      
		reti $zero, $zero, $zero, 0      # return from irq1
return5:
		add $t0, $zero, $imm, 7          # the number of irqreturn reg
		out $imm, $zero, $t0, xor      
		reti $zero, $zero, $zero, 0      # return from irq1
finalReturn:
		reti $zero, $zero, $zero, 0      # return from irq1 
exit:
		halt $zero, $zero, $zero, 0
		

