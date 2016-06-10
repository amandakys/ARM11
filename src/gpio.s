ldr r0,=0x20200000
mov r1,#1
lsl r1,#24
str r1,[r0,#4]

mov r2,#1
lsl r2,#18

loop:
str r2,[r0,#40]
mov r4,0XFFFF

delay:
sub r4,r4,#1
cmp r4,#0
bne delay

str r2,[r0,#28]

mov r4,=0XFFFF

delay1:
sub r4,r4,#1
cmp r4,#0
bne delay1

b loop