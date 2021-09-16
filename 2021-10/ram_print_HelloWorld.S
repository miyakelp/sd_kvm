.data
. = 0x00040
  .byte 0x00, 0x00, 0x00, 0xf0

. = 0x5000
  .ascii "Hello World\n\0"

.code16
. = 0xe8000
  movw $0x5000, %bx
  int  $0x10
  hlt

. = 0xf0000
print:
  movb (%bx), %al
  test $0xff, %al
  jz print_end
  outb %al, $0x11
  inc %bx
  jmp print
print_end:
  iret
  
. = 0xffff0
  jmpl $0xe800, $0x0000
