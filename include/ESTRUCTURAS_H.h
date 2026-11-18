#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <stdbool.h>

#define MAX_TOKEN 64
#define MAX_LINEA 512

// Categorías del Diccionario 1 (Módulo 2)
typedef enum {
    CAT_VACIA_O_COMENTARIO,
    CAT_ETIQUETA,
    CAT_DATOS,
    CAT_OP_LOGICA,
    CAT_OP_ARITMETICA,
    CAT_MOV_DATOS,
    CAT_CONTROL_PILA,
    CAT_LLAMADA_SISTEMA,
    CAT_CONTROL_FUNCIONES,
    CAT_DIRECTIVA,
    CAT_ERROR_DESCONOCIDO
} CategoriaLinea;

// Naturaleza de cada operando (Módulo 2 y 3)
typedef enum {
    OP_NINGUNO,
    OP_REGISTRO,
    OP_INMEDIATO,
    OP_MEMORIA,
    OP_ETIQUETA
} TipoOperando;

// Modos OP/En del manual IA-32 (Módulo 3 y 4)
typedef enum {
    OPEN_INVALIDO,
    OPEN_MR, // ModRM:reg = r, ModRM:r/m = r/m
    OPEN_RM, // ModRM:reg = r/m, ModRM:r/m = r
    OPEN_MI, // ModRM:r/m = r/m, Inmediato
    OPEN_OI, // Opcode + rd/rw/rb, Inmediato
    OPEN_D,  // Desplazamiento relativo (saltos)
    OPEN_M,  // ModRM:r/m = r/m (un operando)
    OPEN_O,  // Opcode + rd/rw/rb (ej. PUSH/POP)
    OPEN_I,  // Solo inmediato (ej. INT 0x80)
    OPEN_NP  // Sin operandos (ej. RET, NOP)
} FormatoOpEn;

// Estructura empaquetada por el Módulo 2
typedef struct {
    char mnemonico[MAX_TOKEN];
    char operando1[MAX_TOKEN];
    char operando2[MAX_TOKEN];
    TipoOperando tipo_op1;
    TipoOperando tipo_op2;
    char opcode_base[8];
    bool requiere_opcode;
} Instruccion;

// Regla exacta encontrada en el diccionario IA-32 (Módulo 3)
typedef struct {
    char mnemonico[MAX_TOKEN];
    TipoOperando tipo_dest;
    TipoOperando tipo_orig;
    int tamano_bits;          // 8, 16, 32 o 0
    char opcode_hex[8];       // Ej. "89", "B8", "01", "83"
    FormatoOpEn formato_open; // MR, RM, MI, OI, D, M, O, I, NP
    int extension_reg;        // 0 a 7 para /0 a /7, o -1 si no aplica
    bool suma_registro;       // true si lleva +rb, +rw, +rd
    bool encontrada;          // true si la regla existe en el diccionario
} ReglaInstruccionIA32;

#endif // ESTRUCTURAS_H
