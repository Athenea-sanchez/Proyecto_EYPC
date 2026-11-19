msg db 'Hola', 0

_start:
    mov eax, 10         ; Codificacion Especial (+rd e inmediato Little Endian) -> B8 0A 00 00 00
    mov ebx, 20         ; Codificacion Especial -> BB 14 00 00 00
    mov ebx, eax        ; Codificacion Comun (ModR/M) -> 89 C3
    add eax, ebx        ; Codificacion Comun (ModR/M) -> 01 D8
    sub eax, ebx        ; Codificacion Comun (ModR/M) -> 29 D8
    add eax, [ebx]      ; Memoria directa sin desplazamiento -> 03 03
    mov eax, [EBX + 4]  ; Memoria con desplazamiento 8 bits -> 8B 43 04
    mov ecx, [eax*4+2]  ; Memoria con escala (SIB) y desplazamiento 32 bits -> 8B 0C 85 02 00 00 00
