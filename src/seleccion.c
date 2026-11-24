#include "seleccion.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Diccionario especifico para mov add y sub
// Diccionario especifico para mov, add, sub, cmp, inc y xor
static const ReglaInstruccionIA32 DICCIONARIO_REGLAS[] = {
    // Mnemónico | Destino      | Origen       | Bits | Opcode | OP/En       | Ext | SumaReg | Bytes
    {"MOV",        OP_REGISTRO,   OP_INMEDIATO,  32,    "B8",    OP_EN_OI,     -1,   true,     5},
    {"MOV",        OP_REGISTRO,   OP_ETIQUETA,   32,    "B8",    OP_EN_OI,     -1,   true,     5},
    {"MOV",        OP_REGISTRO,   OP_REGISTRO,   32,    "89",    OP_EN_MR,     -1,   false,    2},
    {"MOV",        OP_REGISTRO,   OP_MEMORIA,    32,    "8B",    OP_EN_RM,     -1,   false,    2},
    {"MOV",        OP_MEMORIA,    OP_REGISTRO,   32,    "89",    OP_EN_MR,     -1,   false,    2},
    {"MOV",        OP_REGISTRO,   OP_INMEDIATO,  16,    "B8",    OP_EN_OI,     -1,   true,     3},
    {"MOV",        OP_REGISTRO,   OP_INMEDIATO,  8,     "B0",    OP_EN_OI,     -1,   true,     2},
    {"MOV",        OP_REGISTRO,   OP_MEMORIA,    8,     "8A",    OP_EN_RM,     -1,   false,    2},

    {"ADD",        OP_REGISTRO,   OP_REGISTRO,   32,    "01",    OP_EN_MR,     -1,   false,    2},
    {"ADD",        OP_REGISTRO,   OP_MEMORIA,    32,    "03",    OP_EN_RM,     -1,   false,    2},
    {"ADD",        OP_MEMORIA,    OP_REGISTRO,   32,    "01",    OP_EN_MR,     -1,   false,    2},
    {"ADD",        OP_REGISTRO,   OP_INMEDIATO,  32,    "81",    OP_EN_MI,      0,   false,    6},

    {"SUB",        OP_REGISTRO,   OP_REGISTRO,   32,    "29",    OP_EN_MR,     -1,   false,    2},
    {"SUB",        OP_REGISTRO,   OP_MEMORIA,    32,    "2B",    OP_EN_RM,     -1,   false,    2},
    {"SUB",        OP_MEMORIA,    OP_REGISTRO,   32,    "29",    OP_EN_MR,     -1,   false,    2},
    {"SUB",        OP_REGISTRO,   OP_INMEDIATO,  32,    "81",    OP_EN_MI,      5,   false,    6},

    {"CMP",        OP_REGISTRO,   OP_INMEDIATO,  8,     "80",    OP_EN_MI,      7,   false,    3},
    {"CMP",        OP_REGISTRO,   OP_INMEDIATO,  32,    "81",    OP_EN_MI,      7,   false,    6},
    {"CMP",        OP_REGISTRO,   OP_REGISTRO,   32,    "39",    OP_EN_MR,     -1,   false,    2},

    {"INC",        OP_REGISTRO,   OP_NINGUNO,    32,    "40",    OP_EN_O,      -1,   true,     1},

    {"XOR",        OP_REGISTRO,   OP_REGISTRO,   32,    "33",    OP_EN_RM,     -1,   false,    2},
    {"XOR",        OP_REGISTRO,   OP_REGISTRO,   8,     "32",    OP_EN_RM,     -1,   false,    2},

    // Manejo de errores cuando no hay operando
    {"",           OP_NINGUNO,    OP_NINGUNO,    0,     "",      OP_EN_INVALIDO, -1, false,    0}
};
//Convierte a mayusculas el origen y destino
static void aMayusculasSel(char* dest, const char* orig) {
    int i = 0;
    while (orig[i] != '\0' && i < MAX_TOKEN - 1) {
        dest[i] = (char)toupper((unsigned char)orig[i]);
        i++;
    }
    dest[i] = '\0';
}

// Función 1: Determina el tamaño en bits (8, 16 o 32) del operando
int obtenerTamanoOperando(const char* operando, TipoOperando tipo) {
    if (tipo == OP_NINGUNO) return 32; // Para instrucciones de 1 operando como INC
    if (operando == NULL) return 0;
    if (tipo == OP_ETIQUETA) return 32; // Las direcciones de memoria son de 32 bits

    char op[MAX_TOKEN];
    aMayusculasSel(op, operando);

    if (tipo == OP_REGISTRO) {
        // Registros de 32 bits
        if (strcmp(op, "EAX") == 0 || strcmp(op, "EBX") == 0 ||
            strcmp(op, "ECX") == 0 || strcmp(op, "EDX") == 0 ||
            strcmp(op, "ESI") == 0 || strcmp(op, "EDI") == 0 ||
            strcmp(op, "ESP") == 0 || strcmp(op, "EBP") == 0) {
            return 32;
        }
        // Registros de 16 bits
        if (strcmp(op, "AX") == 0 || strcmp(op, "BX") == 0 ||
            strcmp(op, "CX") == 0 || strcmp(op, "DX") == 0 ||
            strcmp(op, "SI") == 0 || strcmp(op, "DI") == 0 ||
            strcmp(op, "SP") == 0 || strcmp(op, "BP") == 0) {
            return 16;
        }
        // Registros de 8 bits
        if (strcmp(op, "AL") == 0 || strcmp(op, "BL") == 0 ||
            strcmp(op, "CL") == 0 || strcmp(op, "DL") == 0 ||
            strcmp(op, "AH") == 0 || strcmp(op, "BH") == 0 ||
            strcmp(op, "CH") == 0 || strcmp(op, "DH") == 0) {
            return 8;
        }
        return -1; // Registro inválido
    }
//Verifica que es un inmediato, lo convierte a long y reresa el tamaño indicado
    if (tipo == OP_INMEDIATO) {
        char* fin = NULL;
        strtol(op, &fin, 0);
        if (fin == op || *fin != '\0') return -1; // Número mal formado
        return 32; // Por defecto en modo BITS 32 se promueve a 32 bits
    }
//verifica si hay registro y que tipo de registro es 
    if (tipo == OP_MEMORIA) {
        size_t len = strlen(op);
        if (len < 3 || op[0] != '[' || op[len - 1] != ']') return -1;
        // Verifica que adentro de [...] haya al menos un registro válido (ej. EAX, EBX en [eax*4+2] o [EBX+4])
        if (strstr(op, "EAX") || strstr(op, "EBX") || strstr(op, "ECX") || strstr(op, "EDX") ||strstr(op, "ESI") || strstr(op, "EDI") || strstr(op, "ESP") || strstr(op, "EBP"))
        {
            return 32;
        }
        return -1; // Memoria sin registro base válido
    }

    return 32;
}

// Función 2: Busca la regla exacta en el diccionario 
ReglaInstruccionIA32 buscarReglaEnDiccionario(
    const char* mnemonico,
    TipoOperando tipo_dest,
    TipoOperando tipo_orig,
    int tamano_bits) {
//Si es una linea invalida
    ReglaInstruccionIA32 invalida = {"", OP_NINGUNO, OP_NINGUNO, 0, "", OP_EN_INVALIDO, -1, false, 0};
    if (mnemonico == NULL) return invalida;

    char mnem_mayus[MAX_TOKEN];
    aMayusculasSel(mnem_mayus, mnemonico);

    for (int i = 0; DICCIONARIO_REGLAS[i].mnemonico[0] != '\0'; i++) {
        if (strcmp(mnem_mayus, DICCIONARIO_REGLAS[i].mnemonico) == 0 &&
            DICCIONARIO_REGLAS[i].tipo_destino == tipo_dest &&
            DICCIONARIO_REGLAS[i].tipo_origen == tipo_orig &&
            DICCIONARIO_REGLAS[i].tamano_bits == tamano_bits) {
            return DICCIONARIO_REGLAS[i];
        }
    }

    return invalida;
}

// Función 3: Decide la ruta hacia el Módulo 4 (1 = Común ModR/M, 2 = Especial +rd/Inmediato)
int determinarTipoCodificacion(ReglaInstruccionIA32 regla) {
    if (regla.op_en == OP_EN_INVALIDO) return 0;

    // Si requiere sumar registro al opcode (+rb/+rw/+rd) o formato OI/O/D/NP COD. ESPECIAL
    if (regla.suma_registro || regla.op_en == OP_EN_OI || regla.op_en == OP_EN_O) {
        return 2;
    }

    // Si usa byte estructural ModR/M (MR, RM, MI, M) -> Ruta Común (1)
    return 1;
}

// Función 4: Valida que los operandos y tamaños sean compatibles 
bool validarCompatibilidadOperandos(
    TipoOperando tipo_dest,
    TipoOperando tipo_orig,
    int tamano_dest,
    int tamano_orig) {

    if (tamano_dest <= 0 || tamano_orig <= 0) return false;

    // No existe instrucción directa memoria-a-memoria MANEJO DE ERROR
    if (tipo_dest == OP_MEMORIA && tipo_orig == OP_MEMORIA) {
        return false;
    }

    // No se puede mover o sumar hacia un valor inmediato 
    if (tipo_dest == OP_INMEDIATO) {
        return false;
    }

    // Si ambos son registros, deben ser del mismo tamaño en bits
    if (tipo_dest == OP_REGISTRO && tipo_orig == OP_REGISTRO && tamano_dest != tamano_orig) {
        return false;
    }

    return true;
}

// Función 5: Coordinadora del Módulo 3
bool procesarInstruccionModulo3(
    Instruccion inst_entrada,
    ReglaInstruccionIA32* regla_salida,
    int* tipo_codificacion_salida) {

    if (regla_salida == NULL || tipo_codificacion_salida == NULL) return false;

    int tam_dest = obtenerTamanoOperando(inst_entrada.operando1, inst_entrada.tipo_op1);
    int tam_orig = obtenerTamanoOperando(inst_entrada.operando2, inst_entrada.tipo_op2);

    if (!validarCompatibilidadOperandos(inst_entrada.tipo_op1, inst_entrada.tipo_op2, tam_dest, tam_orig)) {
        fprintf(stderr, "ERROR SELECCION (M3): Operandos incompatibles o invalidos en '%s %s, %s'\n",
                inst_entrada.mnemonico, inst_entrada.operando1, inst_entrada.operando2);
        return false;
    }

    *regla_salida = buscarReglaEnDiccionario(
        inst_entrada.mnemonico,
        inst_entrada.tipo_op1,
        inst_entrada.tipo_op2,
        tam_dest
    );

    if (regla_salida->op_en == OP_EN_INVALIDO) {
        fprintf(stderr, "ERROR SELECCION (M3): Combinacion no soportada en diccionario IA-32 -> '%s'\n",
                inst_entrada.mnemonico);
        return false;
    }

    *tipo_codificacion_salida = determinarTipoCodificacion(*regla_salida);
    return (*tipo_codificacion_salida > 0);
}
