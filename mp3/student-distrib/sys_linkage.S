.data
  SYS_MIN = 1
  SYS_MAX = 10
  NEG_ONE = -1

.text

.globl sys_call_assembly

sys_call_assembly:

  # ,push registers
  pushl %ebp
  pushl %edi
  pushl %esi

  # ,push args from right to left before jumping
  pushl %edx
  pushl %ecx
  pushl %ebx
  # check if sys call number less than 1
  cmpl $SYS_MIN, %eax
  jl RETURN_NEG_ONE

  # check if sys call number greater 10
  cmpl $SYS_MAX, %eax
  jg RETURN_NEG_ONE
  sti
  call *jump_table(,%eax,4)

  # teardown
  popl %ebx
  popl %ecx
  popl %edx

  # ,pop registers
  popl %esi
  popl %edi
  popl %ebp

  iret

  # return -1 if invalid sys call value
  RETURN_NEG_ONE:
  movl $NEG_ONE, %eax
  iret

  jump_table:
      .long dummy_jmp, sys_halt, sys_execute, sys_read, sys_write, sys_open, sys_close, sys_getargs, sys_vidmap, sys_set_handler, sys_sigreturn
