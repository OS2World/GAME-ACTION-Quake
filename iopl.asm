.386

R2SEG SEGMENT PARA PUBLIC USE16 'CODE'
      ASSUME  CS:R2SEG, DS:NOTHING

many_times360 proc near

loop30_1:
	mov al, 1
plane_loop_1:
	out dx, al
	push si
	push di

	mov bp, 16 * 24
	ALIGN 4, 90h
loop16_1:
	mov bh, byte ptr [si + 12]
	mov bl, byte ptr [si + 8]
	shl ebx, 16
	mov bh, byte ptr [si + 4]
	mov bl, byte ptr [si]

	mov dword ptr es:[di], ebx
	add di, 4
	add si, 16
        dec bp
        jnz loop16_1
	pop di
	pop si
	inc si
	shl al, 1
	cmp al, 16
	jnz plane_loop_1
        add di, 96 * 16
        add si, 384 * 16 - 4
	dec cx
	jnz loop30_1
	retn
many_times360 endp

one_time_seg360 proc near

	mov al, 1
plane_loop_2:
	out dx, al
        push ds
	push si
	push di

	mov bp, 16 * 24
	ALIGN 4, 90h
loop16_2:
	mov bh, byte ptr [si + 12]
	mov bl, byte ptr [si + 8]
	shl ebx, 16
	mov bh, byte ptr [si + 4]
	mov bl, byte ptr [si]

	mov dword ptr es:[di], ebx
	add di, 4
	add si, 16
	jnc no_new_seg1_2
        mov bx, ds
	add bx, 8
	mov ds, bx
no_new_seg1_2:
        dec bp
        jnz loop16_2
	pop di
	pop si
	pop ds
	inc si
	shl al, 1
	cmp al, 16
	jnz plane_loop_2
        add di, 96 * 16
        add si, 384 * 16 - 4
	jnc no_new_seg2_2
	mov bx, ds
	add bx, 8
	mov ds, bx
no_new_seg2_2:
	retn
one_time_seg360 endp

many_times320 proc near

loop30_3:
	mov al, 1
plane_loop_3:
	out dx, al
	push si
	push di

	mov bp, 16 * 20
	ALIGN 4, 90h
loop16_3:
	mov bh, byte ptr [si + 12]
	mov bl, byte ptr [si + 8]
	shl ebx, 16
	mov bh, byte ptr [si + 4]
	mov bl, byte ptr [si]

	mov dword ptr es:[di], ebx
	add di, 4
	add si, 16
        dec bp
        jnz loop16_3
	pop di
	pop si
	inc si
	shl al, 1
	cmp al, 16
	jnz plane_loop_3
        add di, 80 * 16
        add si, 320 * 16 - 4
	dec cx
	jnz loop30_3
	retn
many_times320 endp

one_time_seg320 proc near

	mov al, 1
plane_loop_4:
	out dx, al
        push ds
	push si
	push di

	mov bp, 16 * 20
	ALIGN 4, 90h
loop16_4:
	mov bh, byte ptr [si + 12]
	mov bl, byte ptr [si + 8]
	shl ebx, 16
	mov bh, byte ptr [si + 4]
	mov bl, byte ptr [si]

	mov dword ptr es:[di], ebx
	add di, 4
	add si, 16
	jnc no_new_seg1_4
        mov bx, ds
	add bx, 8
	mov ds, bx
no_new_seg1_4:
        dec bp
        jnz loop16_4
	pop di
	pop si
	pop ds
	inc si
	shl al, 1
	cmp al, 16
	jnz plane_loop_4
        add di, 80 * 16
        add si, 320 * 16 - 4
	jnc no_new_seg2_4
	mov bx, ds
	add bx, 8
	mov ds, bx
no_new_seg2_4:
	retn
one_time_seg320 endp

nrows_360 proc near
	mov al, 1
plane_loop_5:
	out dx, al
	push si
	push di
	mov bp, cx
	ALIGN 4, 90h
loop16_5:
	mov bh, byte ptr [si + 12]
	mov bl, byte ptr [si + 8]
	shl ebx, 16
	mov bh, byte ptr [si + 4]
	mov bl, byte ptr [si]

	mov dword ptr es:[di], ebx
	add di, 4
	add si, 16
        dec bp
        jnz loop16_5
	pop di
	pop si
	inc si
	shl al, 1
	cmp al, 16
	jnz plane_loop_5
	retn
nrows_360 endp

nrows_320 proc near
	mov al, 1
plane_loop_6:
	out dx, al
	push si
	push di
	mov bp, cx
loop16_6:
	mov bh, byte ptr [si + 12]
	mov bl, byte ptr [si + 8]
	shl ebx, 16
	mov bh, byte ptr [si + 4]
	mov bl, byte ptr [si]

	mov dword ptr es:[di], ebx
	add di, 4
	add si, 16
        dec bp
        jnz loop16_6
	pop di
	pop si
	inc si
	shl al, 1
	cmp al, 16
	jnz plane_loop_6
	retn
nrows_320 endp

nrows_seg360 proc near
	mov al, 1
plane_loop_7:
	out dx, al
        push ds
	push si
	push di

	mov bp, cx
loop16_7:
	mov bh, byte ptr [si + 12]
	mov bl, byte ptr [si + 8]
	shl ebx, 16
	mov bh, byte ptr [si + 4]
	mov bl, byte ptr [si]

	mov dword ptr es:[di], ebx
	add di, 4
	add si, 16
	jnc no_new_seg1_7
        mov bx, ds
	add bx, 8
	mov ds, bx
no_new_seg1_7:
        dec bp
        jnz loop16_7
	pop di
	pop si
	pop ds
	inc si
	shl al, 1
	cmp al, 16
	jnz plane_loop_7
	retn
nrows_seg360 endp


PUBLIC _16_3648
_16_3648  PROC  FAR

	push bp
	mov  bp, sp
	push ds
	push es
	push esi
	push edi
	push ebx

	lds  si, [bp + 6]
	les  di, [si]
	lds  si, [si + 4]
	
	mov	dx, 03C4h
	mov	al, 2
	out	dx, al	
	inc	dx
	
	mov cx, 10
	call near ptr many_times360
	call near ptr one_time_seg360
	mov cx, 10
	call near ptr many_times360
	call near ptr one_time_seg360
	mov cx, 8
	call near ptr many_times360

	pop ebx
	pop edi
	pop esi
	pop es
	pop ds
	pop bp
	retf 4

_16_3648  ENDP

PUBLIC _16_3240
_16_3240  PROC  FAR

	push bp
	mov  bp, sp
	push ds
	push es
	push esi
	push edi
	push ebx

	lds  si, [bp + 6]
	les  di, [si]
	lds  si, [si + 4]
	
	mov	dx, 03C4h
	mov	al, 2
	out	dx, al	
	inc	dx
	
	mov cx, 12
	call near ptr many_times320
	call near ptr one_time_seg320
	mov cx, 12
	call near ptr many_times320

	pop ebx
	pop edi
	pop esi
	pop es
	pop ds
	pop bp
	retf 4

_16_3240  ENDP


PUBLIC _16_3224
_16_3224  PROC  FAR

	push bp
	mov  bp, sp
	push ds
	push es
	push esi
	push edi
	push ebx

	lds  si, [bp + 6]
	les  di, [si]
	lds  si, [si + 4]
	
	mov	dx, 03C4h
	mov	al, 2
	out	dx, al	
	inc	dx
	
	mov cx, 12
	call near ptr many_times320
	call near ptr one_time_seg320
	mov cx, 2
	call near ptr many_times320

	pop ebx
	pop edi
	pop esi
	pop es
	pop ds
	pop bp
	retf 4

_16_3224  ENDP

PUBLIC _16_3620
_16_3620  PROC  FAR

	push bp
	mov  bp, sp
	push ds
	push es
	push esi
	push edi
	push ebx
	
	lds  si, [bp + 6]
	les  di, [si]
	lds  si, [si + 4]
	
	mov	dx, 03C4h
	mov	al, 2
	out	dx, al	
	inc	dx
	
	mov cx, 10
	call near ptr many_times360
	call near ptr one_time_seg360
	mov cx, 1
	call near ptr many_times360
	mov cx, 8 * 24
	call near ptr nrows_360

	pop ebx
	pop edi
	pop esi
	pop es
	pop ds
	pop bp
	retf 4

_16_3620  ENDP

PUBLIC _16_3624
_16_3624  PROC  FAR

	push bp
	mov  bp, sp
	push ds
	push es
	push esi
	push edi
	push ebx

	lds  si, [bp + 6]
	les  di, [si]
	lds  si, [si + 4]
	
	mov	dx, 03C4h
	mov	al, 2
	out	dx, al	
	inc	dx
	
	mov cx, 10
	call near ptr many_times360
	call near ptr one_time_seg360
	mov cx, 4
	call near ptr many_times360

	pop ebx
	pop edi
	pop esi
	pop es
	pop ds
	pop bp
	retf 4

_16_3624  ENDP

PUBLIC _16_3235
_16_3235  PROC  FAR

	push bp
	mov  bp, sp
	push ds
	push es
	push esi
	push edi
	push ebx

	lds  si, [bp + 6]
	les  di, [si]
	lds  si, [si + 4]
	
	mov	dx, 03C4h
	mov	al, 2
	out	dx, al	
	inc	dx
	
	mov cx, 12
	call near ptr many_times320
	call near ptr one_time_seg320
	mov cx, 8
	call near ptr many_times320
	mov cx, 14 * 20
	call near ptr nrows_320

	pop ebx
	pop edi
	pop esi
	pop es
	pop ds
	pop bp
	retf 4

_16_3235  ENDP

PUBLIC _16_3635
_16_3635  PROC  FAR

	push bp
	mov  bp, sp
	push ds
	push es
	push esi
	push edi
	push ebx

	lds  si, [bp + 6]
	les  di, [si]
	lds  si, [si + 4]
	
	mov	dx, 03C4h
	mov	al, 2
	out	dx, al	
	inc	dx
	
	mov cx, 10
	call near ptr many_times360
	call near ptr one_time_seg360
	mov cx, 10
	call near ptr many_times360
	mov cx, 14 * 24
	call near ptr nrows_seg360

	pop ebx
	pop edi
	pop esi
	pop es
	pop ds
	pop bp
	retf 4

_16_3635  ENDP


PUBLIC _16_3640
_16_3640  PROC  FAR

	push bp
	mov  bp, sp
	push ds
	push es
	push esi
	push edi
	push ebx

	lds  si, [bp + 6]
	les  di, [si]
	lds  si, [si + 4]
	
	mov	dx, 03C4h
	mov	al, 2
	out	dx, al	
	inc	dx
	
	mov cx, 10
	call near ptr many_times360
	call near ptr one_time_seg360
	mov cx, 10
	call near ptr many_times360
	call near ptr one_time_seg360
	mov cx, 3
	call near ptr many_times360

	pop ebx
	pop edi
	pop esi
	pop es
	pop ds
	pop bp
	retf 4

_16_3640  ENDP

PUBLIC _16_3248
_16_3248  PROC  FAR

	push bp
	mov  bp, sp
	push ds
	push es
	push esi
	push edi
	push ebx

	lds  si, [bp + 6]
	les  di, [si]
	lds  si, [si + 4]
	
	mov	dx, 03C4h
	mov	al, 2
	out	dx, al	
	inc	dx
	
	mov cx, 12
	call near ptr many_times320
	call near ptr one_time_seg320
	mov cx, 12
	call near ptr many_times320
	call near ptr one_time_seg320
	mov cx, 4
	call near ptr many_times320

	pop ebx
	pop edi
	pop esi
	pop es
	pop ds
	pop bp
	retf 4

_16_3248  ENDP

PUBLIC _16_O
_16_O PROC FAR
          push bp
          mov  bp,sp
          push ax
          push dx

          mov  ax,[bp+6]
          mov  dx,[bp+8]
          out  dx,al

          pop  dx
          pop  ax
          pop  bp

          ret  4
	
_16_O ENDP

PUBLIC _16_I
_16_I PROC FAR
          push bp
          mov  bp,sp
          push dx

          mov  dx,[bp+6]
          in   al,dx
          xor  ah,ah

          pop  dx
          pop  bp

          ret  2
_16_I ENDP

PUBLIC _16_W
_16_W PROC FAR
	push ax
	push dx
	mov dx, 3dah

	ALIGN 4, 90h
wait_one:
	in al, dx
	test al, 08h
	jnz wait_one
	ALIGN 4, 90h
wait_two:
	in al, dx
	test al, 08h
	jz wait_two

	pop dx
	pop ax
        ret
_16_W ENDP

PUBLIC _16_P
_16_P PROC FAR
	push bp
	mov  bp, sp
	push ds
	push si
	push dx
	push ax
	lds  si, [bp + 6]

	mov dx, 03c8h
	mov al, 0
	out dx, al
	nop
	nop
	nop
	nop
	nop
	nop
	mov cx, 768
	inc dx
	
	ALIGN 4, 90h
dacloop:
	mov al, [si]
	shr al, 2
	inc si
	out dx, al
	dec cx
	jnz dacloop

	pop ax
	pop dx
	pop si
	pop ds
	pop bp
	ret 4
_16_P ENDP
R2SEG   ENDS
   END

