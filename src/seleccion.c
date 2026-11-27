#include "seleccion.h"
#include "errores.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

    {"",           OP_NINGUNO,    OP_NINGUNO,    0,     "",      OP_EN_INVALIDO, -1, false,    0}
};

static void aMayusculasSel(char* dest, const char* orig) {
    int i = 0;
    while (orig[i] != '\0' && i < MAX_TOKEN - 1) {
        dest[i] = (char)toupper((unsigned char)orig[i]);
        i++;
    }
    dest[i] = '\0';
}

int obtenerTamanoOperando(const char* operando, TipoOperando tipo) {
    if (tipo == OP_NINGUNO) return 32;
    if (operando == NULL) return 0;
    if (tipo == OP_ETIQUETA) return 32;

    char op[MAX_TOKEN];
    aMayusculasSel(op, operando);

    if (tipo == OP_REGISTRO) {
        if (strcmp(op, "EAX") == 0 || strcmp(op, "EBX") == 0 ||
            strcmp(op, "ECX") == 0 || strcmp(op, "EDX") == 0 ||
            strcmp(op, "ESI") == 0 || strcmp(op, "EDI") == 0 ||
            strcmp(op, "ESP") == 0 || strcmp(op, "EBP") == 0) {
            return 32;
        }
        if (strcmp(op, "AX") == 0 || strcmp(op, "BX") == 0 ||
            strcmp(op, "CX") == 0 || strcmp(op, "DX") == 0 ||
            strcmp(op, "SI") == 0 || strcmp(op, "DI") == 0 ||
            strcmp(op, "SP") == 0 || strcmp(op, "BP") == 0) {
            return 16;
        }
        if (strcmp(op, "AL") == 0 || strcmp(op, "BL") == 0 ||
            strcmp(op, "CL") == 0 || strcmp(op, "DL") == 0 ||
            strcmp(op, "AH") == 0 || strcmp(op, "BH") == 0 ||
            strcmp(op, "CH") == 0 || strcmp(op, "DH") == 0) {
            return 8;
        }
        return -1;
    }

    if (tipo == OP_INMEDIATO) {
        char* fin = NULL;
        strtol(op, &fin, 0);
        if (fin == op || *fin != '\0') return -1;
        return 32;
    }

    if (tipo == OP_MEMORIA) {
        size_t len = strlen(op);
        if (len < 3 || op[0] != '[' || op[len - 1] != ']') return -1;

        // 1. Extraer contenido sin corchetes ni espacios
        char interior[MAX_TOKEN] = {0};
        int k = 0;
        for (size_t i = 1; i < len - 1 && k < MAX_TOKEN - 1; i++) {
            if (!isspace((unsigned char)op[i])) {
                interior[k++] = op[i];
            }
        }
        interior[k] = '\0';
        if (interior[0] == '\0') return -1;

        // 2. Validar caracteres permitidos y detectar operadores consecutivos o sueltos
        if (interior[0] == '+' || interior[0] == '*') return -1;
        char ultimo = interior[k - 1];
        if (ultimo == '+' || ultimo == '-' || ultimo == '*') return -1;

        for (int i = 0; i < k; i++) {
            char c = interior[i];
            if (!isalnum((unsigned char)c) && c != '+' && c != '-' && c != '*' && c != '_') {
                return -1; // Caracter ilegal (ej. '/', '(', '&', etc.)
            }
            if ((c == '+' || c == '-' || c == '*') && i + 1 < k) {
                char sig = interior[i + 1];
                // Atrapa '**', '++', '--', '+-', '-+', '*+', '*-', '+*', '-*'
                if (sig == '+' || sig == '-' || sig == '*') {
                    return -1;
                }
            }
        }

        // 3. Separar por '+' y '-' para validar cada término individualmente
        // En IA-32 solo caben máximo 2 registros (Base + Índice) y a lo sumo 1 término escalado
        int total_registros = 0;
        int terminos_escalados = 0;

        char copia[MAX_TOKEN];
        strcpy(copia, interior);

        // Si empieza con '-' (ej. [-4+EAX]), avanzar 1 carácter
        char* cursor = (copia[0] == '-') ? copia + 1 : copia;
        char* termino = strtok(cursor, "+-");

        while (termino != NULL) {
            if (termino[0] == '\0') return -1;

            char* ast = strchr(termino, '*');
            if (ast != NULL) {
                // Solo se permite un término con '*' en toda la dirección (Índice * Escala)
                terminos_escalados++;
                if (terminos_escalados > 1) return -1;

                *ast = '\0';
                const char* izq = termino;
                const char* der = ast + 1;

                if (izq[0] == '\0' || der[0] == '\0') return -1;

                // La izquierda debe ser registro válido de 32 bits y NO puede ser ESP
                if (strcmp(izq, "ESP") == 0) return -1;
                if (strcmp(izq, "EAX") != 0 && strcmp(izq, "EBX") != 0 &&
                    strcmp(izq, "ECX") != 0 && strcmp(izq, "EDX") != 0 &&
                    strcmp(izq, "ESI") != 0 && strcmp(izq, "EDI") != 0 &&
                    strcmp(izq, "EBP") != 0) {
                    return -1; // Ej. [AX*2] o [EBX*ECX]
                }

                // La derecha debe ser escala constante: únicamente 1, 2, 4 u 8
                char* fin_esc = NULL;
                long escala = strtol(der, &fin_esc, 10);
                if (fin_esc == der || *fin_esc != '\0') return -1;
                if (escala != 1 && escala != 2 && escala != 4 && escala != 8) return -1;

                total_registros++;
            } else {
                // Término simple: o es registro de 32 bits, o es desplazamiento numérico
                if (strcmp(termino, "EAX") == 0 || strcmp(termino, "EBX") == 0 ||
                    strcmp(termino, "ECX") == 0 || strcmp(termino, "EDX") == 0 ||
                    strcmp(termino, "ESI") == 0 || strcmp(termino, "EDI") == 0 ||
                    strcmp(termino, "ESP") == 0 || strcmp(termino, "EBP") == 0) {
                    total_registros++;
                } else {
                    // Verificar que sea número válido (decimal o 0xHEX)
                    char* fin_num = NULL;
                    strtol(termino, &fin_num, 0);
                    if (fin_num == termino || *fin_num != '\0') {
                        return -1; // Registro de 16/8 bits ([AX]) o basura ([EAX+foo])
                    }
                }
            }

            termino = strtok(NULL, "+-");
        }

        // IA-32 soporta máximo 2 registros en memoria (Base + Índice) y al menos 1 registro
        if (total_registros < 1 || total_registros > 2) {
            return -1;
        }

        return 32;
    }

    return 32;
}

ReglaInstruccionIA32 buscarReglaEnDiccionario(
    const char* mnemonico,
    TipoOperando tipo_dest,
    TipoOperando tipo_orig,
    int tamano_bits) {
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

int determinarTipoCodificacion(ReglaInstruccionIA32 regla) {
    if (regla.op_en == OP_EN_INVALIDO) return 0;
    if (regla.suma_registro || regla.op_en == OP_EN_OI || regla.op_en == OP_EN_O) {
        return 2;
    }
    return 1;
}

bool validarCompatibilidadOperandos(
    TipoOperando tipo_dest,
    TipoOperando tipo_orig,
    int tamano_dest,
    int tamano_orig) {

    if (tamano_dest <= 0 || tamano_orig <= 0) return false;
    if (tipo_dest == OP_MEMORIA && tipo_orig == OP_MEMORIA) return false;
    if (tipo_dest == OP_INMEDIATO) return false;
    if (tipo_dest == OP_REGISTRO && tipo_orig == OP_REGISTRO && tamano_dest != tamano_orig) {
        return false;
    }
    return true;
}

bool procesarInstruccionModulo3(
    Instruccion inst_entrada,
    ReglaInstruccionIA32* regla_salida,
    int* tipo_codificacion_salida) {

    if (regla_salida == NULL || tipo_codificacion_salida == NULL) return false;

    char detalle[256];
    snprintf(detalle, sizeof(detalle), "%s %s, %s",
             inst_entrada.mnemonico, inst_entrada.operando1, inst_entrada.operando2);

    // Detectar corchete de memoria sin cerrar (ej. "[dato1")
    if (strchr(inst_entrada.operando1, '[') != NULL && strchr(inst_entrada.operando1, ']') == NULL) {
        reportar_error(ERR_MEMORIA, 0, "Corchete de memoria sin cerrar en destino", detalle);
        return false;
    }
    if (strchr(inst_entrada.operando2, '[') != NULL && strchr(inst_entrada.operando2, ']') == NULL) {
        reportar_error(ERR_MEMORIA, 0, "Corchete de memoria sin cerrar en origen", detalle);
        return false;
    }

    int tam_dest = obtenerTamanoOperando(inst_entrada.operando1, inst_entrada.tipo_op1);
    int tam_orig = obtenerTamanoOperando(inst_entrada.operando2, inst_entrada.tipo_op2);

    if (inst_entrada.tipo_op1 == OP_MEMORIA && tam_dest <= 0) {
        reportar_error(ERR_MEMORIA, 0, "Construccion de memoria invalida en destino", detalle);
        return false;
    }
    if (inst_entrada.tipo_op2 == OP_MEMORIA && tam_orig <= 0) {
        reportar_error(ERR_MEMORIA, 0, "Construccion de memoria invalida en origen", detalle);
        return false;
    }
    if (inst_entrada.tipo_op1 == OP_MEMORIA && inst_entrada.tipo_op2 == OP_MEMORIA) {
        reportar_error(ERR_MEMORIA, 0, "Operacion directa memoria-a-memoria no permitida en IA-32", detalle);
        return false;
    }

    if (!validarCompatibilidadOperandos(inst_entrada.tipo_op1, inst_entrada.tipo_op2, tam_dest, tam_orig)) {
        reportar_error(ERR_SINTACTICO, 0, "Operandos incompatibles o invalidos", detalle);
        return false;
    }

    *regla_salida = buscarReglaEnDiccionario(
        inst_entrada.mnemonico,
        inst_entrada.tipo_op1,
        inst_entrada.tipo_op2,
        tam_dest
    );

    if (regla_salida->op_en == OP_EN_INVALIDO) {
        reportar_error(ERR_SINTACTICO, 0, "Combinacion de instruccion no soportada en IA-32", detalle);
        return false;
    }

    *tipo_codificacion_salida = determinarTipoCodificacion(*regla_salida);
    return (*tipo_codificacion_salida > 0);
}