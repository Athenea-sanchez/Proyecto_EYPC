
msg db 'Hola', 0

_start:
    mov eax, 10         ; Codificacion Especial
    mov ebx, eax        ; Codificacion Comun 
    add eax, [ebx]      ; Codificacion Comun 
    push eax            ; Codificacion Especial 
    cmp eax, 10         ; Codificacion Comun con extension 7
    jz _start           ; Salto condicional
    int 0x80            ; Llamada al sistema 
    ret                 ; Sin operandos 
