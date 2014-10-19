#include "intr.h"
	.h8300h
	.section .text

	.global	_intr_softerr
#	.type	_intr_softerr,@function
_intr_softerr:
	mov.l	er6,@-er7
	mov.l	er5,@-er7
	mov.l	er4,@-er7
	mov.l	er3,@-er7
	mov.l	er2,@-er7
	mov.l	er1,@-er7
	mov.l	er0,@-er7
	mov.l	er7,er1
	mov.l	#_intrstack,sp
	mov.l	er1,@-er7
	mov.w	#SOFTVEC_TYPE_SOFTERR,r0
	jsr	@_interrupt
	mov.l	@er7+,er1
	mov.l	er1,er7
	mov.l	@er7+,er0
	mov.l	@er7+,er1
	mov.l	@er7+,er2
	mov.l	@er7+,er3
	mov.l	@er7+,er4
	mov.l	@er7+,er5
	mov.l	@er7+,er6
	rte

	.global	_intr_syscall
#	.type	_intr_syscall,@function
_intr_syscall:
	mov.l	er6,@-er7
	mov.l	er5,@-er7
	mov.l	er4,@-er7
	mov.l	er3,@-er7
	mov.l	er2,@-er7
	mov.l	er1,@-er7
	mov.l	er0,@-er7
	mov.l	er7,er1
	mov.l	#_intrstack,sp
	mov.l	er1,@-er7
	mov.w	#SOFTVEC_TYPE_SYSCALL,r0
	jsr	@_interrupt
	mov.l	@er7+,er1
	mov.l	er1,er7
	mov.l	@er7+,er0
	mov.l	@er7+,er1
	mov.l	@er7+,er2
	mov.l	@er7+,er3
	mov.l	@er7+,er4
	mov.l	@er7+,er5
	mov.l	@er7+,er6
	rte

	.global	_intr_serintr
#	.type	_intr_serintr,@function
_intr_serintr:
	mov.l	er6,@-er7
	mov.l	er5,@-er7
	mov.l	er4,@-er7
	mov.l	er3,@-er7
	mov.l	er2,@-er7
	mov.l	er1,@-er7
	mov.l	er0,@-er7
	mov.l	er7,er1
	mov.l	#_intrstack,sp
	mov.l	er1,@-er7
	mov.w	#SOFTVEC_TYPE_SERINTR,r0
	jsr	@_interrupt
	mov.l	@er7+,er1
	mov.l	er1,er7
	mov.l	@er7+,er0
	mov.l	@er7+,er1
	mov.l	@er7+,er2
	mov.l	@er7+,er3
	mov.l	@er7+,er4
	mov.l	@er7+,er5
	mov.l	@er7+,er6
	rte

	.global	_intr_timintr
#	.type	_intr_timintr,@function
_intr_timintr:
	mov.l	er6,@-er7
	mov.l	er5,@-er7
	mov.l	er4,@-er7
	mov.l	er3,@-er7
	mov.l	er2,@-er7
	mov.l	er1,@-er7
	mov.l	er0,@-er7
	mov.l	er7,er1
	mov.l	#_intrstack,sp
	mov.l	er1,@-er7
	mov.w	#SOFTVEC_TYPE_TIMINTR,r0
	jsr	@_interrupt
	mov.l	@er7+,er1
	mov.l	er1,er7
	mov.l	@er7+,er0
	mov.l	@er7+,er1
	mov.l	@er7+,er2
	mov.l	@er7+,er3
	mov.l	@er7+,er4
	mov.l	@er7+,er5
	mov.l	@er7+,er6
	rte

	.global	_intr_ethintr
#	.type	_intr_ethintr,@function
_intr_ethintr:
	mov.l	er6,@-er7
	mov.l	er5,@-er7
	mov.l	er4,@-er7
	mov.l	er3,@-er7
	mov.l	er2,@-er7
	mov.l	er1,@-er7
	mov.l	er0,@-er7
	mov.l	er7,er1
	mov.l	#_intrstack,sp
	mov.l	er1,@-er7
	mov.w	#SOFTVEC_TYPE_ETHINTR,r0
	jsr	@_interrupt
	mov.l	@er7+,er1
	mov.l	er1,er7
	mov.l	@er7+,er0
	mov.l	@er7+,er1
	mov.l	@er7+,er2
	mov.l	@er7+,er3
	mov.l	@er7+,er4
	mov.l	@er7+,er5
	mov.l	@er7+,er6
	rte

	.global	_intr_bkpoint
#	.type	_intr_bkpoint,@function
_intr_bkpoint:
	mov.l	er6,@-er7
	mov.l	er5,@-er7
	mov.l	er4,@-er7
	mov.l	er3,@-er7
	mov.l	er2,@-er7
	mov.l	er1,@-er7
	mov.l	er0,@-er7
  mov.l er7,er1         ; er1 <- sp

;割り込み判定
  mov @(28, er7), er0  ; er0 = mem[er7-28]
  shal er0              ; c = er0[31]
  bcs   1f
  jmp   2f
1:                      ; if c :
  mov.l #0, er0         ;   er0 <- 0
  mov.l er0, @-er7      ;   push(er0)
  jmp   3f
2:                      ; else :
  mov.l #_intrstack,sp  ;   sp <- interstack
  mov.l er1,@-er7       ;   push(er1)
                        ; endif
3:
	mov.w	#SOFTVEC_TYPE_BKPOINT,r0
	jsr	@_interrupt

;割り込み判定
  mov.l @er7+,er1       ; er1 = pop()
  beq   1f              ; if not er1 == 1
  mov.l er1,er7         ;   sp <- er1
                        ; endif
1:
	mov.l	@er7+,er0
	mov.l	@er7+,er1
	mov.l	@er7+,er2
	mov.l	@er7+,er3
	mov.l	@er7+,er4
	mov.l	@er7+,er5
	mov.l	@er7+,er6
	rte
