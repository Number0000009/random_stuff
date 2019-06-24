// This all is just mockery at the moment
#define ENCLU	0x999
#define EENTER	0x5555555555555555
#define EEXIT	0xaaaaaaaaaaaaaaaa
#define THREAD_CONTROL_STRUCTURE 0x80000000

	.macro energize
//	ldr	x0, =EENTER
//	ldr	x1, =THREAD_CONTROL_STRUCTURE
//	ldr	x2, =exception_handler
//	svc	#ENCLU

// do_write(strout, msg, strlen(msg))
#define __NR_write 64
	mov	x0, #1
	ldr	x1, =.msg
	mov	x2, #(.msg_end - .msg)
	mov	x8, __NR_write
	svc	#0
	b	.exit
.msg:
	.ascii "Hello from thread!\n"
.msg_end:

.balign 4
.exit:
	.endm

	.macro voluntary_exit
//	ldr	x0, =EEXIT
//	svc	#ENCLU

// do_exit(EXIT_SUCCESS)
#define __NR_exit 93
	mov	x0, xzr
	mov	x8, __NR_exit
	svc	#0
	.endm

.balign 0x1000
.text
	.global simple_thread
simple_thread:
// Here we've got x0 = pointer to struct user_pt_regs

// Pop stuff from ctx
	ldr	x3, [x0]
	ldr	x4, [x0, #8]

// Store pointer to ctx
// 16 because of the SP alignment
	str	x0, [sp, #-16]!	// push x0

// Enter the enclave mode
	energize

// Do stuff while in enclave
	add	x0, x3, #1
	sub	x1, x4, #1

// Stash context back into ctx
// pop ctx pointer, 16 because of the SP alignment
	ldr	x10, [sp], #16	// pop x10
	stp	x0, x1, [x10]
	stp	x2, x3, [x10, #16]
	stp	x4, x5, [x10, #32]
	stp	x6, x7, [x10, #48]
	str	x9,	[x10, #64]

// Leave the enclave mode
	voluntary_exit

	wfe
	b .	// must never reach here

exception_handler:
// TODO: implement this
	wfe
	b .

.balign 0x1000
