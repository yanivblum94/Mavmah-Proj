		lw $s0, $imm, $zero, 0x100     # $s0 = r = &(0x100)
		beq $imm, $zero, $s0, exit     # if (r == 0) -> exit
		out $imm, $zero, $t1, 225      # set color to white 
		add $t1, $zero, $imm, 21		
		
		add $t0, $zero, $zero, 0       # $t0 = layer = 0       
		add $s1, $zero, $imm, 175      # $s1 = x = 175
		add $s2, $zero, $imm, 143      # $s2 = y = 143
		sub $s1, $s1, $s0, 0           # x = x - r
		add $t3, $zero, $zero, 0       # i = 0
		sub $t2, $s0, $t0, 0           # n = r - layer
		mul $t2, $t2, $imm, 2          # n = n*2
		add $t2, $t2, $imm, 1          # n += 1
		beq $imm, $zero, $zero, for1   # update pixels values in layer
head:     
		add $t0, $t0, $imm, 1          # layer += 1
		bgt $imm, $t0, $s0, second     # if (layer > r) -> second
		add $s1, $zero, $imm, 175      # x = 175
		sub $s1, $s1, $s0, 0           # x = x - r
		add $s1, $s1, $t0, 0           # x = x + layer
		sub $t2, $s0, $t0, 0           # n = r - layer
		mul $t2, $t2, $imm, 2          # n = n*2
		add $t2, $t2, $imm, 1          # n += 1
		add $s2, $zero, $imm, 143      # y = 143
		add $s2, $s2, $t0, 0           # y = y + layer
		add $t3, $zero, $zero, 0       # i = 0
		beq $imm, $zero, $zero, for1   # update pixels values in layer
		
for1:
		add $t1, $zero, $imm, 18	   # temp to set monitorcmd
		out $imm, $zero, $t1, 1        # monitorcmd = 1
		add $t1, $zero, $imm, 19	   # temp to set x = 19
		out $s1, $zero, $t1, 0         # set x index
		add $t1, $zero, $imm, 20	   #temp to set y = 20
		out $s2, $zero, $t1, 0         # set y index
		add $t3, $t3, $imm, 1          # i += 1
		add $s1, $s1, $imm, 1          # x += 1
		beq $imm, $t3, $t2, head       # if (i == n) ->  next layer
		beq $imm, $zero, $zero, for1	# else -> update  next one
second:
		add $t0, $zero, $imm, 1        # layer = 1
		add $s1, $zero, $imm, 175      # x = 175
		sub $s1, $s1, $s0, 0           # x = x - r
		add $s1, $s1, $t0, 0           # x = x + layer
		sub $t2, $s0, $t0, 0           # n = r - layer
		mul $t2, $t2, $imm, 2          # n = n*2
		add $t2, $t2, $imm, 1          # n += 1
		add $s2, $zero, $imm, 143      # y = 143  
		sub $s2, $s2, $t0, 0           # y = y - layer
		add $t3, $zero, $zero, 0       # i = 0
		beq $imm, $zero, $zero, for2  # update pixels values in layer
base:     
		add $t0, $t0, $imm, 1          # layer += 1
		bgt $imm, $t0, $s0, exit       # if (layer > r) go to exit
		add $s1, $zero, $imm, 175      # x = 175
		sub $s1, $s1, $s0, 0           # x = x - r
		add $s1, $s1, $t0, 0           # x = x + layer
		sub $t2, $s0, $t0, 0           # n = r - layer
		mul $t2, $t2, $imm, 2          # n = n*2
		add $t2, $t2, $imm, 1          # n += 1
		add $s2, $zero, $imm, 143      # y = 143 
		sub $s2, $s2, $t0, 0           # y = y - layer
		add $t3, $zero, $zero, 0       # i = 0
		beq $imm, $zero, $zero, for2  # update pixels values in layer

for2:
		add $t1, $zero, $imm, 18
		out $imm, $zero, $t1, 1        # monitorcmd = 1
		add $t1, $zero, $imm, 19	   # temp to set x = 19
		out $s1, $zero, $t1, 0         # set x index
		add $t1, $zero, $imm, 20  
		out $s2, $zero, $t1, 0         # set y index
		add $s1, $s1, $imm, 1          # x += 1
		add $t3, $t3, $imm, 1          # i += 1
		beq $imm, $t3, $t2, base       # if (i == n) -> next layer
		beq $imm, $zero, $zero, for2   # else -> update  next one
exit:  
		.word 0x100 2
		halt $zero, $zero, $zero, 0

