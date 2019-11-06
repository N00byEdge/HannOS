.intel_syntax noprefix

.extern isrHandler

.code64

.altmacro

.section .bss
.global intnum
        intnum: .byte 0
.global exceptionStackPtr
        exceptionStackPtr: .quad 0

.macro makeHandler num
.section .text
handler\num :
  mov byte ptr[intnum], \num
  jmp isrTramp
.section .data
.quad handler\num
.endm

.section .data
.global handlers
        handlers:
.set current, 0
.rept 0x100
  makeHandler %current
  .set current, current + 1
.endr

.section .text
isrTramp:
  mov qword ptr[exceptionStackPtr], rsp
  push rdi
  push rsi

  push rdx
  push rcx
  push rbx
  push rax

  push rbp
  call isrHandler
  pop rbp

  pop rax
  pop rbx
  pop rcx
  pop rdx

  pop rsi
  pop rdi

.global ignoreIRQ
        ignoreIRQ:
  iretq
