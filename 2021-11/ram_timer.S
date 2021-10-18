.data
. = 0x00080
  .byte 0x00, 0x00, 0x00, 0xf0

. = 0x5000
  .ascii "Hello World\n\0"

.code16
. = 0xe8000
  cli

  movb $0x11, %al // ICW1
  outb %al, $0x20 // M
  outb %al, $0xa0 // S

  movb $0x20, %al // ICW2(M)
  outb %al, $0x21
  movb $0x28, %al // ICW2(S)
  outb %al, $0xa1

  movb $0x04, %al // ICW3(M)
  outb %al, $0x21
  movb $0x02, %al // ICW3(S)
  outb %al, $0xa1

  movb $0x01, %al // ICW4(M)
  outb %al, $0x21 // M
  outb %al, $0xa1 // S

  // PIT以外の割り込みをマスク
  movb $0xfe, %al
  outb %al, $0x21 
  movb $0xff, %al
  outb %al, $0xa1

  // PITモード指定
  movb $0x34, %al
  outb %al, $0x43
  // カウンタ0セット 
  movb $0x9b, %al
  outb %al, $0x40
  movb $0x2e, %al
  outb %al, $0x40

  // 100回を数えるためのカウンタ
  movb $0x64, %cl

  sti

loop:
  nop
  jmp loop

. = 0xf0000
  cli
  // 100回に1回だけ出力する
  decb %cl
  test $0xff, %cl
  jnz print_end

  movb $0x64, %cl
  movw $0x5000, %bx
print:
  movb (%bx), %al
  test $0xff, %al
  jz print_end
  outb %al, $0x11
  inc %bx
  jmp print
print_end:
  movb $0x20, %al
  outb %al, $0x20
  outb %al, $0xa0
  sti
  iret
  
. = 0xffff0
  jmpl $0xe800, $0x0000
