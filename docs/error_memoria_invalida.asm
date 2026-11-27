BITS 32
.data
    dato1 DD 10
    dato2 DD 20

.text
INICIO:
    MOV EAX, [EAX*7]
    RET