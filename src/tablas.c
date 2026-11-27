#include "tablas.h"
#include "conversion.h"
#include "errores.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static void escribirDisp32LittleEndian(uint8_t* dest, int32_t valor) {
    dest[0] = (uint8_t)(valor & 0xFF);
    dest[1] = (uint8_t)((valor >> 8) & 0xFF);
    dest[2] = (uint8_t)((valor >> 16) & 0xFF);
    dest[3] = (uint8_t)((valor >> 24) & 0xFF);
}

static void formatearBytesATexto(const uint8_t* bytes, int cantidad, char* salida) {
    salida[0] = '\0';
    for (int i = 0; i < cantidad; i++) {
        char par[4];
        sprintf(par, "%02X", bytes[i]);
        strcat(salida, par);
        if (i < cantidad - 1) strcat(salida, " ");
    }
}

void inicializarGestorTablas(GestorTablas* gestor, uint32_t base_codigo) {
    if (gestor == NULL) return;
    memset(gestor, 0, sizeof(GestorTablas));
    gestor->contador_datos  = 0x00401000;
    gestor->contador_codigo = (base_codigo != 0) ? base_codigo : 0x00403000;
}

int buscarIndiceSimbolo(const GestorTablas* gestor, const char* nombre) {
    if (gestor == NULL || nombre == NULL) return -1;
    for (int i = 0; i < gestor->total_simbolos; i++) {
        if (strcmp(gestor->simbolos[i].nombre, nombre) == 0) {
            return i;
        }
    }
    return -1;
}

bool registrarSimbolo(GestorTablas* gestor, const char* nombre, const char* seccion, size_t linea) {
    if (gestor == NULL || nombre == NULL || gestor->total_simbolos >= MAX_FILAS_TABLA) {
        return false;
    }

    int idx_existente = buscarIndiceSimbolo(gestor, nombre);
    if (idx_existente != -1 && gestor->simbolos[idx_existente].definida) {
        reportar_error(ERR_SEMANTICO, (int)linea, "Etiqueta o simbolo duplicado", nombre);
        return false;
    }

    Simbolo* sim = &gestor->simbolos[gestor->total_simbolos];
    strncpy(sim->nombre, nombre, sizeof(sim->nombre) - 1);
    sim->nombre[sizeof(sim->nombre) - 1] = '\0';

    strncpy(sim->seccion, seccion, sizeof(sim->seccion) - 1);
    sim->seccion[sizeof(sim->seccion) - 1] = '\0';

    sim->direccion = (strcmp(seccion, ".data") == 0) ? gestor->contador_datos : gestor->contador_codigo;
    formatearDireccionHex32(sim->direccion, sim->direccion_hex);
    sim->definida = true;
    sim->usada = false;

    gestor->total_simbolos++;
    return true;
}

bool agregarEntradaDatos(GestorTablas* gestor, Instruccion inst, size_t linea) {
    if (gestor == NULL || gestor->total_datos >= MAX_FILAS_TABLA) return false;

    if (!registrarSimbolo(gestor, inst.mnemonico, ".data", linea)) {
        return false;
    }

    FilaDatos* fila = &gestor->datos[gestor->total_datos];
    fila->direccion = gestor->contador_datos;
    formatearDireccionHex32(fila->direccion, fila->direccion_hex);
    strncpy(fila->etiqueta, inst.mnemonico, MAX_TOKEN - 1);

    char raw[256];
    if (inst.operando2[0] != '\0') {
        snprintf(raw, sizeof(raw), "%s, %s", inst.operando1, inst.operando2);
    } else {
        snprintf(raw, sizeof(raw), "%s", inst.operando1);
    }

    strcpy(fila->tipo_dato, "DB");
    strncpy(fila->valor_original, raw, sizeof(fila->valor_original) - 1);

    fila->bytes_hex[0] = '\0';
    int bytes_totales = 0;
    bool dentro_comillas = false;

    const char* p = raw;
    while (*p != '\0' && !isspace((unsigned char)*p)) p++;
    while (*p != '\0' && isspace((unsigned char)*p)) p++;

    while (*p != '\0') {
        if (*p == '\'' || *p == '"') {
            dentro_comillas = !dentro_comillas;
            p++;
            continue;
        }
        if (dentro_comillas) {
            char hex_char[4];
            sprintf(hex_char, "%02X ", (unsigned char)*p);
            strcat(fila->bytes_hex, hex_char);
            bytes_totales++;
            p++;
        } else if (isdigit((unsigned char)*p)) {
            char* fin = NULL;
            long val = strtol(p, &fin, 0);
            char hex_num[4];
            sprintf(hex_num, "%02X ", (unsigned int)(val & 0xFF));
            strcat(fila->bytes_hex, hex_num);
            bytes_totales++;
            p = fin;
        } else {
            p++;
        }
    }

    size_t len_hex = strlen(fila->bytes_hex);
    if (len_hex > 0 && fila->bytes_hex[len_hex - 1] == ' ') {
        fila->bytes_hex[len_hex - 1] = '\0';
    }

    fila->num_bytes = bytes_totales;
    gestor->contador_datos += (uint32_t)bytes_totales;
    gestor->total_datos++;
    return true;
}

bool agregarEntradaCodigo(GestorTablas* gestor, Instruccion inst, const char* hex_maquina, int num_bytes) {
    if (gestor == NULL || gestor->total_codigo >= MAX_FILAS_TABLA) return false;
    if (gestor->longitud_buffer + (uint32_t)num_bytes >= MAX_BUFFER_CODIGO) return false;

    FilaCodigo* fila = &gestor->codigo[gestor->total_codigo];
    fila->direccion = gestor->contador_codigo;
    formatearDireccionHex32(fila->direccion, fila->direccion_hex);

    if (inst.operando2[0] != '\0') {
        snprintf(fila->instruccion_asm, sizeof(fila->instruccion_asm), "%.30s %.45s, %.45s",
                 inst.mnemonico, inst.operando1, inst.operando2);
    } else if (inst.operando1[0] != '\0') {
        snprintf(fila->instruccion_asm, sizeof(fila->instruccion_asm), "%.30s %.90s",
                 inst.mnemonico, inst.operando1);
    } else {
        snprintf(fila->instruccion_asm, sizeof(fila->instruccion_asm), "%.120s", inst.mnemonico);
    }

    strncpy(fila->codigo_maquina, hex_maquina, sizeof(fila->codigo_maquina) - 1);
    fila->num_bytes = num_bytes;
    fila->pendiente_salto = false;

    char copia[128];
    strncpy(copia, hex_maquina, sizeof(copia) - 1);
    copia[sizeof(copia) - 1] = '\0';

    char* tok = strtok(copia, " ");
    while (tok != NULL) {
        gestor->buffer_codigo[gestor->longitud_buffer++] = (uint8_t)strtol(tok, NULL, 16);
        tok = strtok(NULL, " ");
    }

    gestor->contador_codigo += (uint32_t)num_bytes;
    gestor->total_codigo++;
    return true;
}

bool agregarInstruccionSalto(GestorTablas* gestor, Instruccion inst, size_t linea) {
    if (gestor == NULL || gestor->total_codigo >= MAX_FILAS_TABLA) return false;

    char mnem[MAX_TOKEN];
    int k = 0;
    while (inst.mnemonico[k] != '\0' && k < MAX_TOKEN - 1) {
        mnem[k] = (char)toupper((unsigned char)inst.mnemonico[k]);
        k++;
    }
    mnem[k] = '\0';

    uint32_t inicio_en_buffer = gestor->longitud_buffer;
    uint32_t pos_desplazamiento = 0;
    int tamano_total = 0;

    if (strcmp(mnem, "JMP") == 0) {
        gestor->buffer_codigo[gestor->longitud_buffer++] = 0xE9;
        pos_desplazamiento = gestor->longitud_buffer;
        tamano_total = 5;
    } else if (strcmp(mnem, "JZ") == 0 || strcmp(mnem, "JE") == 0) {
        gestor->buffer_codigo[gestor->longitud_buffer++] = 0x0F;
        gestor->buffer_codigo[gestor->longitud_buffer++] = 0x84;
        pos_desplazamiento = gestor->longitud_buffer;
        tamano_total = 6;
    } else if (strcmp(mnem, "JNZ") == 0 || strcmp(mnem, "JNE") == 0) {
        gestor->buffer_codigo[gestor->longitud_buffer++] = 0x0F;
        gestor->buffer_codigo[gestor->longitud_buffer++] = 0x85;
        pos_desplazamiento = gestor->longitud_buffer;
        tamano_total = 6;
    } else {
        reportar_error(ERR_SINTACTICO, (int)linea, "Instruccion de salto no soportada", inst.mnemonico);
        return false;
    }

    uint32_t dir_instruccion = gestor->contador_codigo;
    uint32_t dir_siguiente = dir_instruccion + (uint32_t)tamano_total;

    escribirDisp32LittleEndian(&gestor->buffer_codigo[pos_desplazamiento], 0);
    gestor->longitud_buffer += 4;

    int idx_fila = gestor->total_codigo;
    FilaCodigo* fila = &gestor->codigo[idx_fila];
    fila->direccion = dir_instruccion;
    formatearDireccionHex32(fila->direccion, fila->direccion_hex);
    snprintf(fila->instruccion_asm, sizeof(fila->instruccion_asm), "%s %s", inst.mnemonico, inst.operando1);
    fila->num_bytes = tamano_total;
    strncpy(fila->etiqueta_destino, inst.operando1, MAX_TOKEN - 1);

    int idx_sim = buscarIndiceSimbolo(gestor, inst.operando1);
    if (idx_sim != -1 && gestor->simbolos[idx_sim].definida) {
        gestor->simbolos[idx_sim].usada = true;
        int32_t disp = (int32_t)gestor->simbolos[idx_sim].direccion - (int32_t)dir_siguiente;
        escribirDisp32LittleEndian(&gestor->buffer_codigo[pos_desplazamiento], disp);
        fila->pendiente_salto = false;
    } else {
        fila->pendiente_salto = true;
        if (gestor->total_pendientes < MAX_FILAS_TABLA) {
            ReferenciaPendiente* ref = &gestor->pendientes[gestor->total_pendientes++];
            strncpy(ref->nombreEtiqueta, inst.operando1, sizeof(ref->nombreEtiqueta) - 1);
            ref->posicionDesplazamiento = pos_desplazamiento;
            ref->direccionSiguienteInstruccion = dir_siguiente;
            ref->tamanoDesplazamiento = 4;
            ref->numeroLinea = linea;
            ref->indice_fila_codigo = idx_fila;
            ref->resuelta = false;
        }
    }

    formatearBytesATexto(&gestor->buffer_codigo[inicio_en_buffer], tamano_total, fila->codigo_maquina);

    gestor->contador_codigo += (uint32_t)tamano_total;
    gestor->total_codigo++;
    return true;
}

bool resolver_referencias(GestorTablas* gestor) {
    if (gestor == NULL) return false;

    bool sin_errores = true;

    for (int i = 0; i < gestor->total_pendientes; i++) {
        ReferenciaPendiente* ref = &gestor->pendientes[i];
        int idx_sim = buscarIndiceSimbolo(gestor, ref->nombreEtiqueta);

        if (idx_sim == -1 || !gestor->simbolos[idx_sim].definida) {
            reportar_error(ERR_SEMANTICO, (int)ref->numeroLinea,
                           "Etiqueta destino no declarada en el archivo", ref->nombreEtiqueta);
            sin_errores = false;
            continue;
        }

        gestor->simbolos[idx_sim].usada = true;

        int64_t desplazamiento_64 = (int64_t)gestor->simbolos[idx_sim].direccion -
                                    (int64_t)ref->direccionSiguienteInstruccion;

        if (ref->tamanoDesplazamiento == 1 && (desplazamiento_64 < -128 || desplazamiento_64 > 127)) {
            reportar_error(ERR_SEMANTICO, (int)ref->numeroLinea,
                           "Desplazamiento fuera de rango para rel8 [-128, 127]", ref->nombreEtiqueta);
            sin_errores = false;
            continue;
        }
        if (ref->tamanoDesplazamiento == 4 && (desplazamiento_64 < -2147483648LL || desplazamiento_64 > 2147483647LL)) {
            reportar_error(ERR_SEMANTICO, (int)ref->numeroLinea,
                           "Desplazamiento fuera de rango para rel32", ref->nombreEtiqueta);
            sin_errores = false;
            continue;
        }

        int32_t desplazamiento = (int32_t)desplazamiento_64;
        escribirDisp32LittleEndian(&gestor->buffer_codigo[ref->posicionDesplazamiento], desplazamiento);

        FilaCodigo* fila_cod = &gestor->codigo[ref->indice_fila_codigo];
        uint32_t inicio_inst = ref->posicionDesplazamiento - (uint32_t)(fila_cod->num_bytes - 4);
        formatearBytesATexto(&gestor->buffer_codigo[inicio_inst], fila_cod->num_bytes, fila_cod->codigo_maquina);
        fila_cod->pendiente_salto = false;

        ref->resuelta = true;
    }

    return sin_errores;
}

void verificarEtiquetasSinUsar(const GestorTablas* gestor) {
    if (gestor == NULL) return;
    for (int i = 0; i < gestor->total_simbolos; i++) {
        if (gestor->simbolos[i].definida && !gestor->simbolos[i].usada) {
            printf("[WARNING] La etiqueta '%s' (0x%s) fue declarada pero nunca utilizada\n",
                   gestor->simbolos[i].nombre, gestor->simbolos[i].direccion_hex);
        }
    }
}

void imprimirTodasLasTablas(const GestorTablas* gestor) {
    if (gestor == NULL) return;

    printf("\n================================================================================\n");
    printf("                          1. TABLA DE SIMBOLOS\n");
    printf("================================================================================\n");
    printf("%-18s | %-10s | %-12s | %-10s | %-6s\n", "ETIQUETA", "SECCION", "DIRECCION", "ESTADO", "USADA");
    printf("--------------------------------------------------------------------------------\n");
    for (int i = 0; i < gestor->total_simbolos; i++) {
        printf("%-18s | %-10s | 0x%-10s | %-10s | %-6s\n",
               gestor->simbolos[i].nombre,
               gestor->simbolos[i].seccion,
               gestor->simbolos[i].direccion_hex,
               gestor->simbolos[i].definida ? "Definida" : "Pendiente",
               gestor->simbolos[i].usada ? "Si" : "No");
    }
    printf("\n================================================================================\n");
    printf("          2A. TABLA DE REFERENCIAS PENDIENTES \n");
    printf("================================================================================\n");
    printf("%-14s | %-12s | %-12s | %-7s | %-6s | %-10s\n",
           "ETIQUETA", "POS. BUFFER", "DIR. SIG.", "BYTES", "LINEA", "ESTADO");
    printf("--------------------------------------------------------------------------------\n");
    if (gestor->total_pendientes == 0) {
        printf("(Sin referencias pendientes detectadas)\n");
    } else {
        for (int i = 0; i < gestor->total_pendientes; i++) {
            printf("%-14s | %-12u | 0x%-10X | %-7u | %-6zu | %-10s\n",
                   gestor->pendientes[i].nombreEtiqueta,
                   gestor->pendientes[i].posicionDesplazamiento,
                   gestor->pendientes[i].direccionSiguienteInstruccion,
                   gestor->pendientes[i].tamanoDesplazamiento,
                   gestor->pendientes[i].numeroLinea,
                   "Pendiente");
        }
    }

    printf("\n================================================================================\n");
    printf("          2B. TABLA DE REFERENCIAS RESUELTAS \n");
    printf("================================================================================\n");
    printf("%-14s | %-12s | %-12s | %-12s | %-10s\n",
           "ETIQUETA", "DIR. SIG.", "DIR. DEST.", "OFFSET HEX", "ESTADO");
    printf("--------------------------------------------------------------------------------\n");
    if (gestor->total_pendientes == 0) {
        printf("(Sin referencias para resolver)\n");
    } else {
        for (int i = 0; i < gestor->total_pendientes; i++) {
            int idx_sim = buscarIndiceSimbolo(gestor, gestor->pendientes[i].nombreEtiqueta);
            uint32_t dir_dest = (idx_sim != -1) ? gestor->simbolos[idx_sim].direccion : 0;
            int32_t offset = (int32_t)dir_dest - (int32_t)gestor->pendientes[i].direccionSiguienteInstruccion;
            printf("%-14s | 0x%-10X | 0x%-10X | 0x%08X   | %-10s\n",
                   gestor->pendientes[i].nombreEtiqueta,
                   gestor->pendientes[i].direccionSiguienteInstruccion,
                   dir_dest,
                   (uint32_t)offset,
                   gestor->pendientes[i].resuelta ? "Resuelta" : "Error");
        }
    }

    printf("\n================================================================================\n");
    printf("                     3. TABLA DE SECCION DE CODIGO (.text)\n");
    printf("================================================================================\n");
    printf("%-12s | %-24s | %-26s | %-6s\n", "DIRECCION", "INSTRUCCION ASM", "CODIGO MAQUINA (HEX)", "BYTES");
    printf("--------------------------------------------------------------------------------\n");
    for (int i = 0; i < gestor->total_codigo; i++) {
        printf("0x%-10s | %-24s | %-26s | %d\n",
               gestor->codigo[i].direccion_hex,
               gestor->codigo[i].instruccion_asm,
               gestor->codigo[i].codigo_maquina,
               gestor->codigo[i].num_bytes);
    }

    printf("\n================================================================================\n");
    printf("                     4. TABLA DE SECCION DE DATOS (.data)\n");
    printf("================================================================================\n");
    printf("%-12s | %-12s | %-20s | %-22s | %-6s\n",
           "DIRECCION", "ETIQUETA", "VALOR / DATO", "HEXADECIMAL (ASCII)", "BYTES");
    printf("--------------------------------------------------------------------------------\n");
    if (gestor->total_datos == 0) {
        printf("(Sin declaraciones en la seccion .data)\n");
    } else {
        for (int i = 0; i < gestor->total_datos; i++) {
            printf("0x%-10s | %-12s | %-20s | %-22s | %d\n",
                   gestor->datos[i].direccion_hex,
                   gestor->datos[i].etiqueta,
                   gestor->datos[i].valor_original,
                   gestor->datos[i].bytes_hex,
                   gestor->datos[i].num_bytes);
        }
        printf("--------------------------------------------------------------------------------\n");
        printf("Siguiente direccion libre en .data: 0x%08X\n", gestor->contador_datos);
    }

    printf("\n================================================================================\n");
    printf("                 CODIGO MAQUINA (%u bytes)\n", gestor->longitud_buffer);
    printf("================================================================================\n");
    for (uint32_t i = 0; i < gestor->longitud_buffer; i++) {
        printf("%02X ", gestor->buffer_codigo[i]);
    }
    printf("\n================================================================================\n");
}