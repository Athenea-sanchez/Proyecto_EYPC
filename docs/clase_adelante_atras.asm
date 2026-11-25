section .data
    msg db "Hola", 0

section .text
    global _start

_start:
    mov esi, msg
_recorrer:
    mov al, [esi]
    cmp al, 0
    je _fin_cadena
    inc esi
    jmp _recorrer
_fin_cadena:
    mov eax, 1
    xor ebx, ebx
    int 0x80