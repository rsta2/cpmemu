	title	'BIOS for CPMemu'

	maclib	z80

;	RAM addresses

user	equ	4
buffer	equ	80h
tpa	equ	100h
ccp	equ	0dc00h
bdos	equ	0e400h
bios	equ	0f200h

;	Port addresses

prtconstat	equ	0	; In,  console status
prtconin	equ	1	; In,  console input
prtconout	equ	2	; Out, console output

prtdsktrk	equ	10h	; Out, disk track
prtdsksec	equ	11h	; Out, disk sector
prtdskdmal	equ	12h	; Out, disk DMA address low
prtdskdmah	equ	13h	; Out, disk DMA address high
prtdskop	equ	14h	; Out, disk operation
prtdskstat	equ	15h	; In,  disk status

	org	bios

;	BIOS entries

	jmp	boot
wboote:	jmp	wboot
	jmp	const
	jmp	conin
	jmp	conout
	jmp	list
	jmp	punch
	jmp	reader
	jmp	reset
	jmp	seldsk
	jmp	settrk
	jmp	setsec
	jmp	setdma
	jmp	read
	jmp	write
	jmp	lstst
	jmp	trans

;	Disk parameter header

dph0:	dw	0		; no translation
	dw	0
	dw	0
	dw	0
	dw	dirbuf
	dw	dpb0
	dw	0		; fixed disk
	dw	map0

;	Disk parameter block

dpb0:	dw	80		; sectors per track
	db	4		; block shift, 2K blocks
	db	15		; block mask, 2K blocks
	db	0		; extend mask
	dw	389		; total number of blocks-1
	dw	127		; total number of directory entries-1
	dw	0c0h		; directory block mask
	dw	0		; fixed disk
	dw	2		; reserved tracks

;	Cold boot entry

boot:	lxi	sp,buffer
	lxi	h,tpa		; clear TPA
	lxi	d,tpa+1
	lxi	b,ccp-tpa-1
	mvi	m,0
	ldir
	lxi	h,ccp		; save CCP to be restored in warm boot
	lxi	d,ccp2
	lxi	b,bdos-ccp
	ldir
	lxi	h,msg		; display start message
nxtchr:	mov	a,m
	ora	a
	jrz	wboot
	mov	c,a
	call	conout
	inx	h
	jr	nxtchr

;	Warm boot entry

wboot:	lxi	sp,buffer
	lxi	h,zero		; init entries in page 0
	lxi	d,0
	lxi	b,zerolen
	ldir
	lxi	h,ccp2		; restore CCP
	lxi	d,ccp
	lxi	b,bdos-ccp
	ldir
	lxi	b,buffer	; set DMA address
	call	setdma
	lda	user
	ani	0f0h
	mov	c,a
	jmp	ccp

zero:	jmp	wboote		; will be copied to 0
	dw	0
	jmp	bdos+6
zerolen	equ	$-zero

;	Console

const:	in	prtconstat
	ora	a
	rz
	mvi	a,0ffh
	ret

conin:	in	prtconin
	ret

conout:	mov	a,c
	out	prtconout
	ret

;	Printer

lstst:	mvi	a,0ffh
	ret

list:	ret

;	Punch/Reader

reader:	mvi	a,1ah
punch:	ret

;	RAM disk

seldsk:	mov	a,c		; check for drive A
	ora	a
	lxi	h,0
	rnz
	lxi	h,dph0
	ret

reset:	mvi	c,0
settrk:	mov	a,c
	out	prtdsktrk
	ret

setsec:	mov	a,c
	out	prtdsksec
	ret

setdma:	mov	a,c
	out	prtdskdmal
	mov	a,b
	out	prtdskdmah
	ret

read:	mvi	a,1
	jmp	disk
write:	mvi	a,2
disk:	out	prtdskop
	in	prtdskstat
	ora	a
	rz
	mvi	a,1
	ret

trans:	mov	h,b
	mov	l,c
	ret

;	Data

msg:	db	'CP/M 2.2',0dh,0ah,0

map0:	ds	(389+8)/8		; one bit per disk block
dirbuf:	ds	128
ccp2	equ	$

	end
