@ Functions available to be used with VBA
@ Compile with GAS

.file "vba.s"
.code 16
.text
.align 2
.thumb_func
.global mcount
.type mcount,function
@ records call-graph
@ r12 contains LR of previous caller
mcount:
        swi 0xfb
        mov r12, r0
        mov r0, lr
        add r0, $4
        mov lr, r0
        mov r0, r12
        bx lr

.align 2
.thumb_func
.global __gnu_mcount_nc
.type __gnu_mcount_nc,function
__gnu_mcount_nc:
        pop {r4}     @ pop lr of previous caller
        push {lr}    @ remember our return address
        mov r12, r4  @ pass r12 to the call-graph function
        swi 0xfb
        pop {r4}
        bx r4

.align 2
.global monstartup
.thumb_func
.type   monstartup,function
@ starts profiling function
@ r0=low PC, r1=high PC
monstartup:
        swi 0xfe
        bx lr

.align 2
.global moncontrol
.thumb_func
.type   moncontrol,function
@ controls profiling
@ r0=0 stops profiling, any other value starts it
moncontrol:
        swi 0xfd
        bx lr

.align 2
.global moncleanup
.thumb_func
.type   moncleanup,function
@ clean up and stop profiling
moncleanup:
        swi 0xfc
        bx lr

.align 2
.global vbalog
.thumb_func
.type   vbalog,function
@ log a message to VBA's output console or GDB console
@ r0=message to log
vbalog:
        swi 0xff
        bx lr

.data
