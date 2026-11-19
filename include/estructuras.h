#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define MAX_TOKEN 64
#define MAX_LINEA 512

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

typedef enum {
    OP_NINGUNO,
    OP_REGISTRO,
    OP_INMEDIATO,
    OP_MEMORIA,
    OP_ETIQUETA
} TipoOperando;

typedef enum {
    OP_EN_MR,       // ModR/M: r/m = dest, reg = orig
    OP_EN_RM,       // ModR/M: reg = dest, r/m = orig
    OP_EN_MI,       // ModR/M: r/m = dest, reg = /0../7 + inmediato
    OP_EN_OI,       // Opcode + rd + inmediato
    OP_EN_O,        // Opcode + rd
    OP_EN_INVALIDO
} TipoCodificacion;

typedef struct {
    char mnemonico[MAX_TOKEN];
    char operando1[MAX_TOKEN];
    char operando2[MAX_TOKEN];
    TipoOperando tipo_op1;
    TipoOperando tipo_op2;
    char opcode_base[8];
    bool requiere_opcode;
} Instruccion;

typedef struct {
    char mnemonico[16];
    TipoOperando tipo_destino;
    TipoOperando tipo_origen;
    int tamano_bits;
    char opcode_hex[8];
    TipoCodificacion op_en;
    int extension_reg;
    bool suma_registro;
    int bytes_estimados;
} ReglaInstruccionIA32;

#endif // ESTRUCTURAS_H
