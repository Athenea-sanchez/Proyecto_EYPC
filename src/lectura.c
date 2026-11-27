#include "lectura.h"
#include "errores.h"
#include "modulo_apertura.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

typedef struct {
    const char* palabra;
    CategoriaLinea categoria;
} DiccionarioCategoria;

typedef struct {
    const char* mnemonico;
    const char* opcode;
    bool requiere_opcode;
} DiccionarioOpcode;

static const DiccionarioCategoria DICCIONARIO_1[] = {
    {"JMP",     CAT_CONTROL_FUNCIONES},
    {"JZ",      CAT_CONTROL_FUNCIONES},
    {"JE",      CAT_CONTROL_FUNCIONES},
    {"JNZ",     CAT_CONTROL_FUNCIONES},
    {"JNE",     CAT_CONTROL_FUNCIONES},
    {"CALL",    CAT_CONTROL_FUNCIONES},
    {"RET",     CAT_CONTROL_FUNCIONES},
    {"MOV",     CAT_MOV_DATOS},
    {"ADD",     CAT_OP_ARITMETICA},
    {"SUB",     CAT_OP_ARITMETICA},
    {"CMP",     CAT_OP_ARITMETICA},
    {"INC",     CAT_OP_ARITMETICA},
    {"DEC",     CAT_OP_ARITMETICA},
    {"AND",     CAT_OP_LOGICA},
    {"OR",      CAT_OP_LOGICA},
    {"XOR",     CAT_OP_LOGICA},
    {"PUSH",    CAT_CONTROL_PILA},
    {"POP",     CAT_CONTROL_PILA},
    {"INT",     CAT_LLAMADA_SISTEMA},
    {"DB",      CAT_DATOS},
    {"DW",      CAT_DATOS},
    {"DD",      CAT_DATOS},
    {"BITS",    CAT_DIRECTIVA},
    {"SECTION", CAT_DIRECTIVA},
    {"GLOBAL",  CAT_DIRECTIVA},
    {"BITS",    CAT_DIRECTIVA},
    {"SECTION", CAT_DIRECTIVA},
    {".DATA",   CAT_DIRECTIVA},
    {".TEXT",   CAT_DIRECTIVA},
    {"GLOBAL",  CAT_DIRECTIVA},
    {NULL,      CAT_ERROR_DESCONOCIDO}
};

static const DiccionarioOpcode DICCIONARIO_2[] = {
    {"JMP",     "EB",  true},
    {"JZ",      "74",  true},
    {"JE",      "74",  true},
    {"JNZ",     "75",  true},
    {"JNE",     "75",  true},
    {"INT",     "CD",  true},
    {"RET",     "C3",  true},
    {"MOV",     "89",  true},
    {"ADD",     "01",  true},
    {"SUB",     "29",  true},
    {"CMP",     "39",  true},
    {"AND",     "21",  true},
    {"OR",      "09",  true},
    {"XOR",     "31",  true},
    {"PUSH",    "50",  true},
    {"POP",     "58",  true},
    {"DB",      "N/A", false},
    {"DW",      "N/A", false},
    {"DD",      "N/A", false},
    {"BITS",    "N/A", false},
    {"SECTION", "N/A", false},
    {"GLOBAL",  "N/A", false},
    {NULL,      "N/A", false}
};

static void pasarAMayusculas(char* destino, const char* origen) {
    int i = 0;
    while (origen[i] != '\0' && i < MAX_TOKEN - 1) {
        destino[i] = (char)toupper((unsigned char)origen[i]);
        i++;
    }
    destino[i] = '\0';
}

static void asignarOpcodeBase(const char* mnemonico, Instruccion* inst) {
    char temp[MAX_TOKEN];
    pasarAMayusculas(temp, mnemonico);

    for (int i = 0; DICCIONARIO_2[i].mnemonico != NULL; i++) {
        if (strcmp(temp, DICCIONARIO_2[i].mnemonico) == 0) {
            strcpy(inst->opcode_base, DICCIONARIO_2[i].opcode);
            inst->requiere_opcode = DICCIONARIO_2[i].requiere_opcode;
            return;
        }
    }

    strcpy(inst->opcode_base, "N/A");
    inst->requiere_opcode = false;
}

int leerSiguienteLinea(FILE* archivo, char* buffer_linea, int tamano_max) {
    int dummy = 0;
    int bytes_leidos = leerLineaArchivo(archivo, buffer_linea, tamano_max, &dummy);
    if (bytes_leidos > 0) {
        return 1;
    }
    return 0;
}

void limpiarLinea(char* linea_cruda) {
    if (linea_cruda == NULL) return;

    char* punto_y_coma = strchr(linea_cruda, ';');
    if (punto_y_coma != NULL) {
        *punto_y_coma = '\0';
    }

    int fin = (int)strlen(linea_cruda) - 1;
    while (fin >= 0 && isspace((unsigned char)linea_cruda[fin])) {
        linea_cruda[fin] = '\0';
        fin--;
    }

    int inicio = 0;
    while (linea_cruda[inicio] != '\0' && isspace((unsigned char)linea_cruda[inicio])) {
        inicio++;
    }

    if (inicio > 0) {
        memmove(linea_cruda, linea_cruda + inicio, strlen(linea_cruda + inicio) + 1);
    }
}

CategoriaLinea clasificarConDiccionario(const char* linea_limpia) {
    if (linea_limpia == NULL || linea_limpia[0] == '\0') {
        return CAT_VACIA_O_COMENTARIO;
    }

    size_t len = strlen(linea_limpia);
    if (linea_limpia[len - 1] == ':') {
        return CAT_ETIQUETA;
    }

    char primera_palabra[MAX_TOKEN] = {0};
    int i = 0;
    while (linea_limpia[i] != '\0' && !isspace((unsigned char)linea_limpia[i]) && i < MAX_TOKEN - 1) {
        primera_palabra[i] = (char)toupper((unsigned char)linea_limpia[i]);
        i++;
    }
    primera_palabra[i] = '\0';

    for (int j = 0; DICCIONARIO_1[j].palabra != NULL; j++) {
        if (strcmp(primera_palabra, DICCIONARIO_1[j].palabra) == 0) {
            return DICCIONARIO_1[j].categoria;
        }
    }

    char copia[MAX_LINEA];
    strncpy(copia, linea_limpia, MAX_LINEA - 1);
    copia[MAX_LINEA - 1] = '\0';

    char* p1 = strtok(copia, " \t");
    char* p2 = strtok(NULL, " \t");
    if (p1 != NULL && p2 != NULL) {
        char p2_mayus[MAX_TOKEN];
        pasarAMayusculas(p2_mayus, p2);
        if (strcmp(p2_mayus, "DB") == 0 || strcmp(p2_mayus, "DW") == 0 || strcmp(p2_mayus, "DD") == 0) {
            return CAT_DATOS;
        }
    }

    return CAT_ERROR_DESCONOCIDO;
}

bool separarTokensInstruccion(const char* linea_limpia, char* mnemonico, char* operando1, char* operando2) {
    if (linea_limpia == NULL || mnemonico == NULL || operando1 == NULL || operando2 == NULL) {
        return false;
    }

    mnemonico[0] = '\0';
    operando1[0] = '\0';
    operando2[0] = '\0';

    char copia[MAX_LINEA];
    strncpy(copia, linea_limpia, MAX_LINEA - 1);
    copia[MAX_LINEA - 1] = '\0';

    size_t len = strlen(copia);
    if (len > 0 && copia[len - 1] == ':') {
        copia[len - 1] = '\0';
        strcpy(mnemonico, copia);
        return true;
    }

    int i = 0, j = 0;
    while (copia[i] != '\0' && !isspace((unsigned char)copia[i]) && j < MAX_TOKEN - 1) {
        mnemonico[j++] = copia[i++];
    }
    mnemonico[j] = '\0';

    while (copia[i] != '\0' && isspace((unsigned char)copia[i])) {
        i++;
    }

    if (copia[i] == '\0') {
        return true;
    }

    j = 0;
    while (copia[i] != '\0' && copia[i] != ',' && j < MAX_TOKEN - 1) {
        operando1[j++] = copia[i++];
    }
    operando1[j] = '\0';
    limpiarLinea(operando1);

    if (copia[i] == ',') {
        i++;
        while (copia[i] != '\0' && isspace((unsigned char)copia[i])) {
            i++;
        }
        if (copia[i] == '\0') {
            return false;
        }

        j = 0;
        while (copia[i] != '\0' && j < MAX_TOKEN - 1) {
            operando2[j++] = copia[i++];
        }
        operando2[j] = '\0';
        limpiarLinea(operando2);
    }

    return true;
}

TipoOperando identificarTipoOperando(const char* operando) {
    if (operando == NULL || operando[0] == '\0') {
        return OP_NINGUNO;
    }

    size_t len = strlen(operando);

    if (operando[0] == '[' && operando[len - 1] == ']') {
        return OP_MEMORIA;
    }

    if (isdigit((unsigned char)operando[0]) || (operando[0] == '-' && len > 1 && isdigit((unsigned char)operando[1]))) {
        return OP_INMEDIATO;
    }

    char op_mayus[MAX_TOKEN];
    pasarAMayusculas(op_mayus, operando);

    const char* registros[] = {
        "EAX", "EBX", "ECX", "EDX", "ESI", "EDI", "ESP", "EBP",
        "AX",  "BX",  "CX",  "DX",  "SI",  "DI",  "SP",  "BP",
        "AL",  "BL",  "CL",  "DL",  "AH",  "BH",  "CH",  "DH",
        NULL
    };

    for (int i = 0; registros[i] != NULL; i++) {
        if (strcmp(op_mayus, registros[i]) == 0) {
            return OP_REGISTRO;
        }
    }

    return OP_ETIQUETA;
}

bool procesarLineaModulo2(FILE* archivo, Instruccion* instruccion_salida, CategoriaLinea* categoria_salida) {
    if (archivo == NULL || instruccion_salida == NULL || categoria_salida == NULL) {
        return false;
    }

    char buffer[MAX_LINEA];

    while (leerSiguienteLinea(archivo, buffer, MAX_LINEA) == 1) {
        limpiarLinea(buffer);

        if (buffer[0] == '\0') {
            continue;
        }

        // Inicializar salida limpia por si hay error en esta línea
        memset(instruccion_salida, 0, sizeof(Instruccion));

        // 1. Clasificar con Diccionario 1 (si es desconocida, reportar error y continuar leyendo el archivo)
        *categoria_salida = clasificarConDiccionario(buffer);
        if (*categoria_salida == CAT_ERROR_DESCONOCIDO) {
            strncpy(instruccion_salida->mnemonico, buffer, MAX_TOKEN - 1);
            reportar_error(ERR_LEXICO, 0, "Instruccion o sentencia desconocida", buffer);
            return true;
        }

        // 2. Separar tokens
        if (!separarTokensInstruccion(buffer, instruccion_salida->mnemonico, instruccion_salida->operando1, instruccion_salida->operando2)) {
            strncpy(instruccion_salida->mnemonico, buffer, MAX_TOKEN - 1);
            *categoria_salida = CAT_ERROR_DESCONOCIDO;
            reportar_error(ERR_SINTACTICO, 0, "Sintaxis invalida en los operandos", buffer);
            return true;
        }

        // 3. Identificar tipo de cada operando y consultar Diccionario 2 (opcodes)
        instruccion_salida->tipo_op1 = identificarTipoOperando(instruccion_salida->operando1);
        instruccion_salida->tipo_op2 = identificarTipoOperando(instruccion_salida->operando2);
        asignarOpcodeBase(instruccion_salida->mnemonico, instruccion_salida);

        return true;
    }

    return false;
}