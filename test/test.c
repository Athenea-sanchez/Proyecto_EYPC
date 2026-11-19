#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "modulo_apertura.h"
#include "lectura.h"
#include "seleccion.h"
#include "codificacion.h"

// Funciones auxiliares para imprimir en texto legible lo que clasifica el Módulo 2
const char* nombreCategoria(CategoriaLinea cat) {
    switch (cat) {
        case CAT_ETIQUETA:          return "ETIQUETA";
        case CAT_DATOS:             return "DATOS";
        case CAT_OP_LOGICA:         return "OP_LOGICA";
        case CAT_OP_ARITMETICA:     return "OP_ARITMETICA";
        case CAT_MOV_DATOS:         return "MOV_DATOS";
        case CAT_CONTROL_PILA:      return "CONTROL_PILA";
        case CAT_LLAMADA_SISTEMA:   return "LLAMADA_SISTEMA";
        case CAT_CONTROL_FUNCIONES: return "CONTROL_FUNCIONES";
        case CAT_DIRECTIVA:         return "DIRECTIVA";
        default:                    return "DESCONOCIDO";
    }
}

const char* nombreTipoOperando(TipoOperando tipo) {
    switch (tipo) {
        case OP_REGISTRO:  return "REGISTRO";
        case OP_INMEDIATO: return "INMEDIATO";
        case OP_MEMORIA:   return "MEMORIA";
        case OP_ETIQUETA:  return "ETIQUETA";
        case OP_NINGUNO:   return "NINGUNO";
        default:           return "DESCONOCIDO";
    }
}

// Auxiliar para visualizar la salida binaria del Módulo 4 convertida a bytes Hexadecimales
void imprimirBinarioComoHex(const char* cadena_binaria) {
    if (cadena_binaria == NULL) return;
    size_t len = strlen(cadena_binaria);

    for (size_t i = 0; i + 8 <= len; i += 8) {
        char byte_str[9];
        strncpy(byte_str, cadena_binaria + i, 8);
        byte_str[8] = '\0';

        unsigned int val = (unsigned int)strtol(byte_str, NULL, 2);
        printf("%02X ", val);
    }
    printf("\n");
}

int main(int argc, char* argv[]) {
    const char* ruta = (argc > 1) ? argv[1] : "docs/prueba.asm";
    bool archivo_ok = false;

    printf("=== PRUEBA MODULOS 1, 2, 3 Y 4 (APERTURA, LEXICO, SELECCION Y CODIFICACION) ===\n");
    printf("Archivo objetivo: %s\n\n", ruta);

    // ---------------------------------------------------------
    // MÓDULO 1: APERTURA Y VERIFICACIÓN DEL ARCHIVO
    // ---------------------------------------------------------
    FILE* archivo = obtenerArchivoVerificado(ruta, &archivo_ok);
    if (!archivo_ok || archivo == NULL) {
        printf("\n[ABORTADO] Fallo en Modulo 1.\n");
        return 1;
    }
    printf("[M1 OK] Archivo abierto y verificado.\n\n");

    // ---------------------------------------------------------
    // MÓDULO 1.5: CARGA EN MEMORIA DINÁMICA Y NÚMERO DE LÍNEA
    // ---------------------------------------------------------
    int total_lineas = 0;
    LineaFuente* lineas_memoria = cargarArchivoEnMemoria(archivo, &total_lineas);
    if (lineas_memoria == NULL) {
        printf("[ABORTADO] Fallo al cargar en memoria dinamica.\n");
        fclose(archivo);
        return 1;
    }

    printf("[M1 MEMORIA OK] Se cargaron %d lineas en memoria dinamica:\n", total_lineas);
    for (int i = 0; i < total_lineas; i++) {
        printf("  Linea %d: %s", lineas_memoria[i].numero_linea, lineas_memoria[i].texto);
    }
    printf("\n\n");

    rewind(archivo);

    // ---------------------------------------------------------
    // MÓDULOS 2, 3 Y 4: LECTURA, SELECCIÓN Y CODIFICACIÓN
    // ---------------------------------------------------------
    Instruccion inst;
    CategoriaLinea categoria;
    int num_sentencia = 1;

    while (procesarLineaModulo2(archivo, &inst, &categoria)) {
        printf("--------------------------------------------------\n");
        printf("Sentencia #%d:\n", num_sentencia++);
        printf("  [M2] Categoria   : %s\n", nombreCategoria(categoria));
        printf("  [M2] Mnemónico   : '%s'\n", inst.mnemonico);
        printf("  [M2] Operando 1  : '%s' (%s)\n", inst.operando1, nombreTipoOperando(inst.tipo_op1));
        printf("  [M2] Operando 2  : '%s' (%s)\n", inst.operando2, nombreTipoOperando(inst.tipo_op2));

        // Ignorar directivas (BITS 32), etiquetas (_start:) y datos (msg db ...) en el codificador
        if (categoria == CAT_DIRECTIVA || categoria == CAT_ETIQUETA || categoria == CAT_DATOS) {
            printf("  [M3/M4] Omitida en codificador (Directiva / Etiqueta / Datos).\n");
            continue;
        }

        // MÓDULO 3: Selección de regla IA-32 y validación de operandos
        ReglaInstruccionIA32 regla;
        int tipo_codificacion = 0;

        if (!procesarInstruccionModulo3(inst, &regla, &tipo_codificacion)) {
            printf("  [M3] AVISO: Instruccion no soportada en esta etapa.\n");
            continue;
        }

        printf("  [M3] Regla IA-32 : Opcode=%s | Ruta=%s (%d)\n",
               regla.opcode_hex,
               (tipo_codificacion == 1) ? "COMUN (ModR/M)" : "ESPECIAL (+rd/Imm)",
               tipo_codificacion);

        // MÓDULO 4: Codificación binaria (ModR/M, SIB, Desplazamiento, Little Endian)
        char* cadena_bin = generarCadenaIntermediaModulo4(inst, regla, tipo_codificacion);
        if (cadena_bin != NULL) {
            printf("  [M4] Binario     : %s\n", cadena_bin);
            printf("  [M4] Hexadecimal : ");
            imprimirBinarioComoHex(cadena_bin);
            free(cadena_bin);
        }
    }

    printf("--------------------------------------------------\n");
    printf("\n=== FIN DE PRUEBA DE MODULOS 1 AL 4 ===\n");

    liberarArchivoEnMemoria(lineas_memoria, total_lineas);
    fclose(archivo);
    return 0;
}
