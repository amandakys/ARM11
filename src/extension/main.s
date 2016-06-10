/* main.s adapted from Cambridge Raspberry Pi Online tutorials
*  and code by Alex Chadwick */

.section .init
.globl _start
_start:

b main ; branch to main code

.section .text

main:

mov sp,#0x8000 ; set stack point to 0x8000

; use SetGpioFunction to turn pins into output pins
; set to 1 for output pin, 0 for input pin

pinNum .req r0
pinFunc .req r1
mov pinNum,#2
mov pinFunc,#1
bl SetGpioFunction
.unreq pinNum
.unreq pinFunc

pinNum .req r0
pinFunc .req r1
mov pinNum,#3
mov pinFunc,#1
bl SetGpioFunction
.unreq pinNum
.unreq pinFunc

pinNum .req r0
pinFunc .req r1
mov pinNum,#4
mov pinFunc,#1
bl SetGpioFunction
.unreq pinNum
.unreq pinFunc

pinNum .req r0
pinFunc .req r1
mov pinNum,#9
mov pinFunc,#1
bl SetGpioFunction
.unreq pinNum
.unreq pinFunc

pinNum .req r0
pinFunc .req r1
mov pinNum,#10
mov pinFunc,#1
bl SetGpioFunction
.unreq pinNum
.unreq pinFunc

pinNum .req r0
pinFunc .req r1
mov pinNum,#11
mov pinFunc,#1
bl SetGpioFunction
.unreq pinNum
.unreq pinFunc

pinNum .req r0
pinFunc .req r1
mov pinNum,#14
mov pinFunc,#1
bl SetGpioFunction
.unreq pinNum
.unreq pinFunc

pinNum .req r0
pinFunc .req r1
mov pinNum,#15
mov pinFunc,#1
bl SetGpioFunction
.unreq pinNum
.unreq pinFunc

pinNum .req r0
pinFunc .req r1
mov pinNum,#17
mov pinFunc,#1
bl SetGpioFunction
.unreq pinNum
.unreq pinFunc

pinNum .req r0
pinFunc .req r1
mov pinNum,#18
mov pinFunc,#1
bl SetGpioFunction
.unreq pinNum
.unreq pinFunc

pinNum .req r0
pinFunc .req r1
mov pinNum,#22
mov pinFunc,#1
bl SetGpioFunction
.unreq pinNum
.unreq pinFunc

pinNum .req r0
pinFunc .req r1
mov pinNum,#23
mov pinFunc,#1
bl SetGpioFunction
.unreq pinNum
.unreq pinFunc

pinNum .req r0
pinFunc .req r1
mov pinNum,#24
mov pinFunc,#1
bl SetGpioFunction
.unreq pinNum
.unreq pinFunc

; set pin values to 0, to turn them all off

pinNum .req r0
pinVal .req r1
mov pinNum,#2
mov pinVal,#0
bl SetGpio
.unreq pinNum
.unreq pinVal

pinNum .req r0
pinVal .req r1
mov pinNum,#3
mov pinVal,#0
bl SetGpio
.unreq pinNum
.unreq pinVal

pinNum .req r0
pinVal .req r1
mov pinNum,#4
mov pinVal,#0
bl SetGpio
.unreq pinNum
.unreq pinVal

pinNum .req r0
pinVal .req r1
mov pinNum,#9
mov pinVal,#0
bl SetGpio
.unreq pinNum
.unreq pinVal

pinNum .req r0
pinVal .req r1
mov pinNum,#10
mov pinVal,#0
bl SetGpio
.unreq pinNum
.unreq pinVal

pinNum .req r0
pinVal .req r1
mov pinNum,#11
mov pinVal,#0
bl SetGpio
.unreq pinNum
.unreq pinVal

pinNum .req r0
pinVal .req r1
mov pinNum,#14
mov pinVal,#0
bl SetGpio
.unreq pinNum
.unreq pinVal

pinNum .req r0
pinVal .req r1
mov pinNum,#15
mov pinVal,#0
bl SetGpio
.unreq pinNum
.unreq pinVal

pinNum .req r0
pinVal .req r1
mov pinNum,#17
mov pinVal,#0
bl SetGpio
.unreq pinNum
.unreq pinVal

pinNum .req r0
pinVal .req r1
mov pinNum,#18
mov pinVal,#0
bl SetGpio
.unreq pinNum
.unreq pinVal

pinNum .req r0
pinVal .req r1
mov pinNum,#22
mov pinVal,#0
bl SetGpio
.unreq pinNum
.unreq pinVal

pinNum .req r0
pinVal .req r1
mov pinNum,#23
mov pinVal,#0
bl SetGpio
.unreq pinNum
.unreq pinVal

pinNum .req r0
pinVal .req r1
mov pinNum,#24
mov pinVal,#0
bl SetGpio
.unreq pinNum
.unreq pinVal

; create a delay
decr .req r0
mov decr,#0xFFFFFF
wait1$:
    sub decr,#1
    teq decr,#0
    bne wait1$
.unreq decr

/* for number display changes to pin states are made based on current states
*  from 0 to 1, we switch pins on and off depending on what is already on
*  this makes displaying numbers heavily dependent on the order they are shown
*  we are in the progress of making this process less tedious
*  either by implementing a font file
*  or initialising pins states for each number from scratch each time
*/

; begin number display

loop$:
    ; display zero
    pinNum .req r0
    pinVal .req r1
    mov pinNum,#3
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#9
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#10
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#11
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#15
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#17
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#18
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#23
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    ; wait
    decr .req r0
    mov decr,#0xFFFFFF
    wait2$:
        sub decr,#1
        teq decr,#0
        bne wait2$
    .unreq decr

    ; display one

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#2
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#3
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#4
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#9
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#11
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#17
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#22
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#23
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#24
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    ; wait
    decr .req r0
    mov decr,#0xFFFFFF
    wait3$:
        sub decr,#1
        teq decr,#0
        bne wait3$
    .unreq decr

    ; display two
    pinNum .req r0
    pinVal .req r1
    mov pinNum,#18
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#4
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#3
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#14
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#15
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#17
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#22
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#23
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    ; wait
    decr .req r0
    mov decr,#0xFFFFFF
    wait4$:
        sub decr,#1
        teq decr,#0
        bne wait4$
    .unreq decr

    ; display three

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#24
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#22
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#17
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#18
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#2
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#11
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#22
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    ; wait
    decr .req r0
    mov decr,#0xFFFFFF
    wait5$:
        sub decr,#1
        teq decr,#0
        bne wait5$
    .unreq decr

    ; display four

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#24
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#3
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#4
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#15
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#22
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#9
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#23
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    ; wait
    decr .req r0
    mov decr,#0xFFFFFF
    wait6$:
        sub decr,#1
        teq decr,#0
        bne wait6$
    .unreq decr

    ; display five

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#15
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#24
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#3
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#10
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#22
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#23
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    ; wait
    decr .req r0
    mov decr,#0xFFFFFF
    wait7$:
        sub decr,#1
        teq decr,#0
        bne wait7$
    .unreq decr

    ; display six

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#2
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#4
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#22
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#17
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    ; wait
    decr .req r0
    mov decr,#0xFFFFFF
    wait8$:
        sub decr,#1
        teq decr,#0
        bne wait8$
    .unreq decr

    ; display seven

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#4
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#15
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#24
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#9
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#10
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#11
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#14
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#17
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#22
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#23
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#2
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    ; wait
    decr .req r0
    mov decr,#0xFFFFFF
    wait9$:
        sub decr,#1
        teq decr,#0
        bne wait9$
    .unreq decr

    ; display eight

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#4
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#15
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#24
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#2
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#9
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#14
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#17
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#23
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    ; wait
    decr .req r0
    mov decr,#0xFFFFFF
    wait10$:
        sub decr,#1
        teq decr,#0
        bne wait10$
    .unreq decr

    ; display nine

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#17
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#23
    mov pinVal,#0
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#15
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    pinNum .req r0
    pinVal .req r1
    mov pinNum,#24
    mov pinVal,#1
    bl SetGpio
    .unreq pinNum
    .unreq pinVal

    ; wait
    decr .req r0
    mov decr,#0xFFFFFF
    wait11$:
        sub decr,#1
        teq decr,#0
        bne wait11$
    .unreq decr

b loop$
