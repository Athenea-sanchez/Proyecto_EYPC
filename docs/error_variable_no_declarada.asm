BITS 32
.data
    contador DB 5

.text
INICIO:
    MOV EAX, [variable_inexistente]
    ADD EAX, 1
    MOV EBX, EAX