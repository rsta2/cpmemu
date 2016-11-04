	title	'Shutdown for CPMemu'

	maclib	z80

reboot	equ	0
bdos	equ	5
tpa	equ	100h

port	equ	0e0h		; control port

	org	tpa

start:	lxi	d,savemsg
	mvi	c,9
	call	bdos
	mvi	a,'S'
	out	port		; trigger saving RAM disk

	lxi	d,downmsg
	mvi	c,9
	call	bdos
	mvi	a,'Q'
	out	port		; trigger leaving emulator

	jmp	reboot

savemsg: db	'Saving RAM disk to persistent storage.',0dh,0ah,'$'
downmsg: db	'CP/M emulator will shutdown now.',0dh,0ah,'$'

	end	start
