#include <stdio.h>
#include <stdbool.h>
#include "modulo_apertura.h"
#include "lectura.h"

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

int main(int argc, char* argv[]) {
    const char* ruta = (argc > 1) ? argv[1] : "prueba.asm";
    bool archivo_ok = false;

    printf("=== PRUEBA MODULO 1 (APERTURA) Y MODULO 2 (LEXICO) ===\n");
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

    // Regresa el archivo al inicio para que tu Módulo 2 actual lo pueda leer igual
    rewind(archivo);

    // ---------------------------------------------------------
    // MÓDULO 2: LECTURA LÍNEA POR LÍNEA Y CLASIFICACIÓN
    // ---------------------------------------------------------
    Instruccion inst;
    CategoriaLinea categoria;
    int num_sentencia = 1;

    while (procesarLineaModulo2(archivo, &inst, &categoria)) {
        printf("--------------------------------------------------\n");
        printf("Sentencia #%d:\n", num_sentencia++);
        printf("  Categoria   : %s\n", nombreCategoria(categoria));
        printf("  Mnemonico   : '%s'\n", inst.mnemonico);
        printf("  Operando 1  : '%s' (%s)\n", inst.operando1, nombreTipoOperando(inst.tipo_op1));
        printf("  Operando 2  : '%s' (%s)\n", inst.operando2, nombreTipoOperando(inst.tipo_op2));
        printf("  Opcode Base : %s (Requiere opcode: %s)\n", 
               inst.opcode_base, inst.requiere_opcode ? "SI" : "NO");
    }

    printf("--------------------------------------------------\n");
    printf("\n=== FIN DE LECTURA Y CLASIFICACION ===\n");

    liberarArchivoEnMemoria(lineas_memoria, total_lineas);
    fclose(archivo);
    return 0;
}
