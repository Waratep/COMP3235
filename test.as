	push	sp
	push	9
	add
	pop	sp
	push	1111
	push	0
	push	0
	push	3
	mul
	add
	push	0
	add
	pop	in
	pop	sb[in]
	push	2222
	push	1
	push	0
	push	3
	mul
	add
	push	0
	add
	pop	in
	pop	sb[in]
	push	3333
	push	0
	push	1
	push	3
	mul
	add
	push	0
	add
	pop	in
	pop	sb[in]
	push	4444
	push	1
	push	1
	push	3
	mul
	add
	push	0
	add
	pop	in
	pop	sb[in]
	jmp	L501
L500:
	push	sp
	push	0
	add
	pop	sp
	push	0
	push	1
	push	3
	mul
	add
	push	fp[-4]
	add
	pop	in
	push	sb[in]
	puti
	push	fp[-4]
	call	L502, 1
	push	0
	ret
L501:
	jmp	L503
L502:
	push	sp
	push	0
	add
	pop	sp
	push	5555
	push	1
	push	1
	push	3
	mul
	add
	push	fp[-4]
	add
	pop	in
	pop	sb[in]
	push	1
	push	1
	push	3
	mul
	add
	push	fp[-4]
	add
	pop	in
	push	sb[in]
	puti
	push	0
	ret
L503:
	push	0
	call	L500, 1
	jmp	L999
L998:
	push	999999
	puti
L999:
