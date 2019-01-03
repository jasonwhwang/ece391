.text

.globl rtc_driver_assembly
.globl keyboard_driver_assembly
.globl pit_driver_assembly

rtc_driver_assembly:
    pushal
    pushfl

    call rtc_handler

    popfl
    popal

    iret

keyboard_driver_assembly:
     pushal
     pushfl

     call keypress

     popfl
     popal
     iret

 pit_driver_assembly:
    pushal
    pushfl

    call pit_handler

    popfl
    popal
    iret



.end
