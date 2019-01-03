
# mp1.S - missile-command version
# Good luck, and have fun!

.data
	
    # Constants for accessing the fields of a struct missile, 
    # struct missile is defined in rtc.h and mp1.h

	NEXT 	  = 0
	X	  = 4
	Y	  = 8
	VX	  = 12
	VY	  = 16
	DEST_X	  = 20
	DEST_Y	  = 24
	EXPLODED  = 28
	C	  = 32

    # Character to draw for an explosion - '@'

	EXPLOSION = 64


# Data shared between this file and rtc.c helper functions

# This '.globl' directive makes these symbols visible externally
.globl mp1_missile_list, base_alive, mp1_score

mp1_missile_list:	.long 0x0	# Head of list pointer
base_alive:		.long 0x0	# Array of 3 bytes, plus a padding byte
mp1_score:		.long 0x0	# Player's current score


# Data private to this  file

base_pic:     	    .string "/^^^\"	# Picture of a live base
dead_base_pic: 	    .string "xxxxx"	# Picture of a dead base
crosshairs_x:		.long 0x0	# X-position of the crosshairs
crosshairs_y:		.long 0x0	# Y-position of the crosshairs

.text

# void mp1_poke(void);
# You can use this function to write to video memory.
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX

mp1_poke:
	movl    vmem_base_addr(,1),%edx
	movb    %cl,(%edx,%eax,1)
	ret

#   ----------------- Exported functions ---------------------

# void mp1_rtc_tasklet(unsigned long garbage);
# Performs three tasks:
#	(1) updates the list of missiles (implement this in update_missiles,
#           below, and call it from here).
#	(2) Redraw the bases - they may have been overwritten with missiles
#	(3) Redraw the crosshairs - it may have been overwritten with missiles
# Inputs   : none
# Outputs  : none
# Registers: Standard C calling convention

.globl mp1_rtc_tasklet
mp1_rtc_tasklet:
	pushl	%ebp																				# save old base pointer
	movl	%esp, %ebp																	# move new base pointer into ebp
	pushl	%ebx																				# save callee saved registers
	pushl %esi
	pushl %edi
	pushl %eax																				# save caller saved registers
	pushl %ecx
	pushl %edx
	call	process_missiles
	call	redraw_cities
	popl	%edx																				# restore registers
	popl 	%ecx
	popl	%eax
	popl	%edi
	popl	%esi
	popl	%ebx
	leave
	ret

# int mp1_ioctl(unsigned long arg, unsigned int cmd)
# The dispatch function for the MP1 ioctls - should use the cmd argument
# and a jumptable to execute one of the specific ioctls implemented below.
# Inputs   : unsigned long arg - parameter to the mp1_ioctl_....
#          : unsigned int cmd  - specifies which mp1_ioctl_... function 
#          :                     to execute
# Outputs  : Returns an integer - depends on which ioctl() function is called
# Registers: Standard C calling convention

.globl mp1_ioctl
mp1_ioctl:
	movl	8(%esp), %eax
	cmpl	$0, %eax
	jl		invalid
	cmpl	$4, %eax
	jg		invalid
	jmp		*functions(,%ebx,4)
invalid:
	movl	$-1, %eax
	ret
functions:
	.long mp1_ioctl_startgame, mp1_ioctl_addmissile, mp1_ioctl_movexhairs, mp1_ioctl_getstatus, mp1_ioctl_endgame

# ----------------- Functions private to this file -------------------

# void process_missiles();
# Tasks:
#	(1) Updates the list of missiles
# Inputs   : none
# Outputs  : none
# Registers: Standard C calling convention
process_missiles:
	pushl	%ebp																				# save old base pointer
	movl	%esp, %ebp																	# save new base pointer into ebp
	pushl	%ebx																				# save callee saved registers
	pushl %esi
	pushl %edi
	movl	mp1_missile_list, %ebx											# get missile list head pointer

process_missiles_loop_start:
	cmpl	$0, %ebx																		# check if pointer value is 0
	je		process_missiles_finished
	
	cmpl	$0, EXPLODED(%ebx)													# compare if exploded is 0
	je		process_missiles_update_x_y									# if 0, not exploded, skip
	call	process_missiles_explode										# else call missiles explode
	jmp		process_missiles_loop_end										# jmp to end

process_missiles_update_x_y:
	pushl	$32																					# remove character from screen
	call	process_missiles_draw
	addl	$4, %esp

	movl	VX(%ebx), %ecx															# update x coordinate, move vx to ecx
	addl	%ecx, X(%ebx)																# add vx to x, store back at memory location
	movl	X(%ebx), %ecx																# get x from memory location into ecx
	sarl	$16, %ecx																		#	shift right 16 bits
	movl	VY(%ebx), %edx															# update y coordinate, move vy to edx
	addl	%edx, Y(%ebx)																# add vy to y, store back at memory location
	movl	Y(%ebx), %edx																# move y to edx
	sarl	$16, %edx																		# shift edx 16 bits right to get screen coordinates

	cmpl	$0, %ecx																		# check if coordinates fall within scope
	jl		process_missiles_remove_missile
	cmpl	$79, %ecx
	jg		process_missiles_remove_missile
	cmpl	$0, %edx
	jl		process_missiles_remove_missile
	cmpl	$24, %edx
	jg		process_missiles_remove_missile

	cmpl	DEST_X(%ebx), %ecx													# check if reached destination
	je		process_missiles_explosion									# if so, process explosion
	cmpl	DEST_Y(%ebx), %edx													# do the same with y
	je		process_missiles_explosion
	jmp		process_missiles_redraw											# else, just redraw the missile

process_missiles_remove_missile:
	call	process_missiles_remove
	jmp		process_missiles_loop_end

process_missiles_explosion:
	movl	$0, VX(%ebx)																# set velocities to 0
	movl	$0, VY(%ebx)
	call	process_missiles_explode										# process explode
	jmp		process_missiles_loop_end

process_missiles_redraw:
	movl	C(%ebx), %eax																# move character to be drawn to eax
	pushl	%eax
	call	process_missiles_draw												# draw character
	popl	%eax
	movl	NEXT(%ebx), %eax														# increment missile pointer
	movl	%eax, %ebx

process_missiles_loop_end:
	cmpl	$0, %ebx
	je		process_missiles_return											# if pointer is 0, return
	jmp		process_missiles_loop_start									# else continue loop

process_missiles_return:
	popl	%edi																				# restore registers
	popl	%esi
	popl	%ebx
	leave
	ret

# void process_missiles_remove(long* pointer);
# Tasks:
#	(1) Removes missile from list
# Inputs   : pointer to a missile in the list
# Outputs  : none
# Registers: pointer stored in register ebx
process_missiles_remove:
	pushl	%ebp																				# save ebp and additional registers
	movl	%esp, %ebp
	pushl %eax
	pushl %ecx
	pushl	%edx

	movl	mp1_missile_list, %ecx
	cmpl	%ecx, %ebx
	je 		list_head
remove_loop_start:
	cmpl  NEXT(%ecx), %ebx
	je		remove_missile_found
	movl	NEXT(%ecx), %eax
	movl	%eax, %ecx
	jmp		remove_loop_start
remove_missile_found:
	movl	NEXT(%ebx), %eax
	movl	%eax, NEXT(%ecx)
	pushl	%ebx
	call	mp1_free
	popl	%ebx
	movl	NEXT(%ecx), %ebx
	jmp		remove_loop_end
list_head:
	movl	NEXT(%ecx), mp1_missile_list
	pushl	%ebx
	call mp1_free
	popl	%ebx
	movl	mp1_missile_list, %ebx
remove_loop_end:
	popl	%edx
	popl	%ecx
	popl	%eax
	leave
	ret

# void process_missiles_explode(long* pointer);
# Tasks:
#	(1) Handles the explode condition, if new explode or update or remove explosion
# Inputs   : pointer to missile in list
# Outputs  : none
# Registers: head_pointer stored in register ebx
process_missiles_explode:
	pushl	%ebp																				# save base pointer
	movl	%esp, %ebp
	pushl	%ebx																				# save ebx register
	call	missile_explode															# call missile_explode
	popl	%ebx																				# restore ebx
	cmpl	$0, %eax																		# compare return value
	je		process_missiles_no_notify									# if 0, do not notify user
	pushl %ebx
	call	mp1_notify_user															# else, notify user
	popl	%ebx
process_missiles_no_notify:
	decl	EXPLODED(%ebx)															# decrement value of exploded
	cmpl	$0, EXPLODED(%ebx)													# check if exploded is 0
	jg		process_missiles_explode_increment					# if greater than 0, finish explode
	pushl	%ebx
	pushl $32
	call	process_missiles_draw												# else, remove explosion from screen
	addl	$4, %esp
	popl	%ebx
	call	process_missiles_remove											# remove missile from list
	jmp		process_missiles_explode_done
process_missiles_explode_increment:
	pushl	$64
	call	process_missiles_draw												# redraw explosion
	addl	$4, %esp
	movl	NEXT(%ebx), %eax														# increment missile pointer
	movl	%eax, %ebx
process_missiles_explode_done:
	leave
	ret

# void process_missiles_draw();
# Tasks:
#	(1) Removes missile from list
# Inputs   : none
# Outputs  : none
# Registers: ebx stores pointer to the missile
process_missiles_draw:
	pushl	%ebp																				# save ebp and additional registers
	movl	%esp, %ebp
	pushl %eax
	pushl %ecx
	pushl	%edx

	movl	X(%ebx), %eax																# print character to screen
	sarl	$16, %eax
	imull	$2, %eax
	movl	Y(%ebx), %ecx
	sarl	$16, %ecx
	imull	$160, %ecx
	addl	%ecx, %eax
	movb	4(%ebp), %cl
	call mp1_poke

	popl	%edx
	popl	%ecx
	popl	%eax
	leave
	ret


# void redraw_cities();
# Tasks:
#	(1) Redraw the cities
# Inputs   : none
# Outputs  : none
# Registers: none
redraw_cities:
	pushl	%ebp																				# save old base pointer
	movl	%esp, %ebp																	# save new base pointer into ebp
	pushl	%ebx																				# save callee saved registers
	pushl %esi
	pushl %edi

	movl	$3876, %eax																	# save offset in eax
	movl	$base_alive, %ebx														# save base alive array
	movl	$0, %edx																		# base pic
	movl	$0, %esi																		# character counter (max 5)
	movl	$0, %edi																		# base counter (max 3)
redraw_cities_start:
	cmpb	$0, (%ebx)
	jg 		city_alive
	movl	$dead_base_pic, %edx
	jmp		draw_city
city_alive:
	movl	$base_pic, %edx
draw_city:
	movb	(%edx), %cl
	pushl %edx
	call	mp1_poke
	popl	%edx

	addl	$2, %eax																		# increment video offset
	addl	$1, %edx																		# increment character pointer
	addl	$1, %esi																		# increment number of characters counter
	cmpl	$5, %esi																		# once counter reaches 5, all characters printed
	jl		draw_city																		# if not 5, continue to draw city

	addl	$32, %eax
	addl	$1, %ebx
	movl 	$0, %esi
	addl	$1, %edi
	cmpl	$3, %edi
	jl		redraw_cities_start
	popl	%edi																				# restore registers
	popl	%esi
	popl	%ebx
	leave
	ret



# void mp1_ioctl_startgame();
# Tasks:
#	(1) Initializes the game values
# Inputs   : none
# Outputs  : none
# Registers: none
mp1_ioctl_startgame:
	pushl %ebp
	movl	%esp, %ebp
	pushl %ebx
	pushl %esi
	pushl %edi

	movl	$0x0, mp1_missile_list
	movl	$0x00010101, base_alive											# number to initialize for base
	movl	$0x0, mp1_score
	movl	$40, crosshairs_x														# start x coordinate
	movl	$12, crosshairs_y														# start y coordinate

	popl	%edi																				# restore registers
	popl	%esi
	popl	%ebx
	leave
	ret

# void mp1_ioctl_addmissile(struct missile* user missile);
# Tasks:
#	(1) Adds missile to the list
# Inputs   : pointer to a missile struct
# Outputs  : 0 if no errors, greater than 0 if has errors
# Registers: none
mp1_ioctl_addmissile:
	pushl %ebp
	movl	%esp, %ebp
	pushl %ebx
	pushl %esi
	pushl %edi

	movl	4(%ebp), %ebx																# list head pointer
	pushl	$33																					# 33 bytes for data structure
	call	mp1_malloc																	# return new address location in eax
	addl	$4, %esp
	pushl	$33																					# push memory size
	pushl	%ebx																				# push list head pointer
	pushl	%eax																				# push new memory address
	call mp1_copy_from_user
	popl	%ecx																				# new memory location stored at ecx
	popl	%ebx																				# list pointer
	addl	$4, %esp

	cmpl	$0, %eax
	jne		failed_copy
	movl	mp1_missile_list, %esi
	movl	%esi, NEXT(%ecx)
	movl	%ecx, mp1_missile_list
	jmp 	add_finished
failed_copy:
	pushl	%ecx
	call	mp1_free
	popl	%ecx
	movl	$-1, %eax
add_finished:
	popl	%edi																				# restore registers
	popl	%esi
	popl	%ebx
	leave
	ret


# void mp1_ioctl_movexhairs(unsigned long xhair delta packed);
# Tasks:
#	(1) Moves the cross hairs
# Inputs   : unsigned long with crosshair coordinates
# Outputs  : 0
# Registers: none
mp1_ioctl_movexhairs:
	pushl %ebp
	movl	%esp, %ebp
	pushl %ebx
	pushl %esi
	pushl %edi
																	
	movl	crosshairs_x, %eax
	imull	$2, %eax
	movl	crosshairs_y, %ebx
	imull	$160, %ebx
	addl	%ebx, %eax
	movb	$32, %cl
	call mp1_poke

	movl	crosshairs_x, %eax
	movl	crosshairs_y, %ebx
	movl	4(%ebp), %ecx
	addw	%cx, %eax
	sarl	$16, %ecx
	addw	%cx, %ebx

	cmpl	$0, %eax
	jl		x_no_add
	cmpl	$79, %eax
	jg		x_no_add
	movl	%eax, crosshairs_x
x_no_add:
	cmpl	$0, %ebx
	jl		y_no_add
	cmpl	$24, %ebx
	jg		y_no_add
	movl	%ebx, crosshairs_y
y_no_add:
	movl	crosshairs_x, %eax
	imull	$2, %eax
	movl	crosshairs_y, %ebx
	imull	$160, %ebx
	addl	%ebx, %eax
	movb	$43, %cl
	call	mp1_poke

	movl	$0, %eax
	popl	%edi																				# restore registers
	popl	%esi
	popl	%ebx
	leave
	ret


# void mp1_ioctl_getstatus(unsigned long* user status);
# Tasks:
#	(1) Gets that status of the game
# Inputs   : unsigned long* user status
# Outputs  : returns 0 if no errors, greater than 0 if errors
# Registers: none
mp1_ioctl_getstatus:
	pushl	%ebp																				# save old base pointer
	movl	%esp, %ebp																	# save new base pointer into ebp
	pushl	%ebx																				# save callee saved registers
	pushl %esi
	pushl %edi

	movl	$base_alive, %eax
	movb	(%eax), %bl
	shl		$1, %bl
	addl	$1, %eax
	addb	(%eax), %bl
	shl		$1, %bl
	addl	$1, %eax
	addb	(%eax), %bl
	shl		$17, %ebx

	movl	mp1_score, %eax
	addl	%eax, %ebx

	pushl $4
	call	mp1_malloc																	# create new space in memory
	addl	$4, %esp

	movl	%ebx, (%eax)																# move compiled score to location specified in eax
	movl	%eax, %edx																	# move address from eax to edx
	pushl	$4
	pushl	%eax
	pushl	4(%ebp)
	call mp1_copy_to_user
	addl	$12, %esp

	cmpl	$0, %eax
	je		copy_success
	movl	$-1, %eax

copy_success:
	pushl	%edx
	call 	mp1_free
	popl	%edx

	popl	%edi																				# restore registers
	popl	%esi
	popl	%ebx
	leave
	ret


# void mp1_ioctl_endgame();
# Tasks:
#	(1) Ends the game, cleans up memory
# Inputs   : none
# Outputs  : none
# Registers: none
mp1_ioctl_endgame:
	pushl	%ebp																				# save old base pointer
	movl	%esp, %ebp																	# save new base pointer into ebp
	pushl	%ebx																				# save callee saved registers
	pushl %esi
	pushl %edi

	movl	mp1_missile_list, %ebx
endgame_loop_start:
	cmpl	$0, %ebx
	je		endgame_loop_end
	call 	process_missiles_remove
	jmp		endgame_loop_start

endgame_loop_end:
	popl	%edi																				# restore registers
	popl	%esi
	popl	%ebx
	leave
	ret
	