ldr r0,=0x20200000
mov r1,#73
lsl r1,#9
str r1,[r0,#8]

mov r2,#7
lsl r2,#23

loop:
mov r4,#0
str r4,[r0,#40]
str r2,[r0,#40] 
mov r4,#100

delay:
sub r4,r4,#1
cmp r4,#0
bne delay

str r2,[r0,#28] 
mov r4,#100

delay1:
sub r4,r4,#1
cmp r4,#0
bne delay1

b loop

