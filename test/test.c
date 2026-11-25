#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "modulo_apertura.h"
#include "lectura.h"
#include "seleccion.h"
#include "codificacion.h"
#include "conversion.h"
#include "tablas.h"

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
    const char* ruta = (argc > 1) ? argv[1] : "docs/prueba.asm";
    bool archivo_ok = false;

    printf("                         PRUEBA                           \n");
    printf("Archivo objetivo: %s\n\n", ruta);

  
    // MÓDULO 1: APERTURA Y VERIFICACIÓN DEL ARCHIVO
  
    FILE* archivo = obtenerArchivoVerificado(ruta, &archivo_ok);
    if (!archivo_ok || archivo == NULL) {
        printf("\n[FATAL ERROR] Fallo en Modulo 1.\n");
        return 1;
    }
    printf("[M1] Archivo abierto y verificado.\n\n");

  
    // MÓDULO 1.5: CARGA EN MEMORIA DINÁMICA Y NÚMERO DE LÍNEA

    int total_lineas = 0;
    LineaFuente* lineas_memoria = cargarArchivoEnMemoria(archivo, &total_lineas);
    if (lineas_memoria == NULL) {
        printf("[FATAL ERROR] Fallo al cargar en memoria dinamica.\n");
        fclose(archivo);
        return 1;
    }

    printf("[M1 MEMORIA BIEN] Se cargaron %d lineas en memoria dinamica:\n", total_lineas);
    for (int i = 0; i < total_lineas; i++) {
        printf("  Linea %d: %s", lineas_memoria[i].numero_linea, lineas_memoria[i].texto);
    }
    printf("\n\n");

    rewind(archivo);

    
    // INICIALIZACIÓN MÓDULO 6 (TABLAS Y BÚFER DE CÓDIGO)

    GestorTablas gestor;
    inicializarGestorTablas(&gestor, 0x00403000);

  
    // FASE 1: MÓDULOS 2, 3, 4, 5 Y REGISTRO EN TABLAS (MÓDULO 6)

    Instruccion inst;
    CategoriaLinea categoria;
    int num_sentencia = 1;
    size_t numero_linea = 0;

    while (procesarLineaModulo2(archivo, &inst, &categoria)) {
        numero_linea++;

        printf("--------------------------------------------------\n");
        printf("Sentencia #%d:\n", num_sentencia++);
        printf("  [M2] Categoria   : %s\n", nombreCategoria(categoria));
        printf("  [M2] Mnemonico   : '%s'\n", inst.mnemonico);
        printf("  [M2] Operando 1  : '%s' (%s)\n", inst.operando1, nombreTipoOperando(inst.tipo_op1));
        printf("  [M2] Operando 2  : '%s' (%s)\n", inst.operando2, nombreTipoOperando(inst.tipo_op2));

        // 1. Directivas (BITS 32, SECTION .text, etc.)
        if (categoria == CAT_DIRECTIVA) {
            printf("  [M3/M4] Omitida en codificador (Directiva).\n");
            continue;
        }

        // 2. Etiquetas (ej. INICIO:, FIN:) -> Tabla de Símbolos
        if (categoria == CAT_ETIQUETA) {
            if (registrarSimbolo(&gestor, inst.mnemonico, ".text", numero_linea)) {
                printf("  [M6] Tabla Simbolos : Etiqueta '%s' registrada en 0x%08X\n",
                       inst.mnemonico, gestor.contador_codigo);
            }
            continue;
        }

        // 3. Declaración de datos (ej. msg db 'Hola', 0) -> Tabla de Datos
        if (categoria == CAT_DATOS) {
            if (agregarEntradaDatos(&gestor, inst, numero_linea)) {
                printf("  [M6] Tabla Datos    : Variable '%s' agregada a .data\n", inst.mnemonico);
            }
            continue;
        }

        // 4. Saltos (JMP, JZ, JE, JNZ, JNE) -> Referencias Pendientes / Backpatching
        if (categoria == CAT_CONTROL_FUNCIONES &&
            (strcasecmp(inst.mnemonico, "JMP") == 0 ||
             strcasecmp(inst.mnemonico, "JZ")  == 0 ||
             strcasecmp(inst.mnemonico, "JE")  == 0 ||
             strcasecmp(inst.mnemonico, "JNZ") == 0 ||
             strcasecmp(inst.mnemonico, "JNE") == 0)) {

            if (agregarInstruccionSalto(&gestor, inst, numero_linea)) {
                FilaCodigo* ultima = &gestor.codigo[gestor.total_codigo - 1];
                printf("  [M6] Salto rel32    : %s | Estado: %s\n",
                       ultima->codigo_maquina,
                       ultima->pendiente_salto ? "PENDIENTE (Backpatching)" : "RESUELTO (Salto atras)");
            }
            continue;
        }

        // 5. Instrucciones simples de control (RET / INT)
        if (strcasecmp(inst.mnemonico, "RET") == 0) {
            printf("  [M4] Hexadecimal    : C3\n");
            agregarEntradaCodigo(&gestor, inst, "C3", 1);
            continue;
        }
        if (strcasecmp(inst.mnemonico, "INT") == 0) {
            long interrupcion = strtol(inst.operando1, NULL, 0);
            char hex_int[16];
            sprintf(hex_int, "CD %02X", (unsigned int)(interrupcion & 0xFF));
            printf("  [M4] Hexadecimal    : %s\n", hex_int);
            agregarEntradaCodigo(&gestor, inst, hex_int, 2);
            continue;
        }

       // 5.5 Resolver etiqueta de datos como operando inmediato (ej. mov esi, msg)
     if (inst.tipo_op2 == OP_ETIQUETA) {
         int idx_sim = buscarIndiceSimbolo(&gestor, inst.operando2);
         if (idx_sim != -1) {
             gestor.simbolos[idx_sim].usada = true;
             snprintf(inst.operando2, sizeof(inst.operando2), "0x%X", gestor.simbolos[idx_sim].direccion);
             inst.tipo_op2 = OP_INMEDIATO;
         }
     }

        // 6. MÓDULO 3: Selección de regla IA-32 y validación de operandos
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

        // 7. MÓDULO 4: Codificación binaria
        char* cadena_bin = generarCadenaIntermediaModulo4(inst, regla, tipo_codificacion);
        if (cadena_bin != NULL) {
            printf("  [M4] Binario     : %s\n", cadena_bin);

            // 8. MÓDULO 5: Conversión de binario a pares hexadecimales
            int num_bytes = 0;
            char* hex_final = convertirBinarioAHexadecimal(cadena_bin, &num_bytes);

            if (hex_final != NULL) {
                printf("  [M5] Hexadecimal : %s (%d bytes)\n", hex_final, num_bytes);

                // 9. MÓDULO 6: Agregar instrucción a la Tabla de Código y al búfer máquina
                agregarEntradaCodigo(&gestor, inst, hex_final, num_bytes);
                free(hex_final);
            }
            free(cadena_bin);
        }
    }

    printf("--------------------------------------------------\n");


    // FASE 2: BACKPATCHING EN MEMORIA Y AUDITORÍA DE SÍMBOLOS

    printf("\n[M6 BACKPATCHING] Resolviendo referencias pendientes en memoria...\n");
    bool exito_backpatching = resolver_referencias(&gestor);
    verificarEtiquetasSinUsar(&gestor);

    // SALIDA FINAL: IMPRESIÓN DE LAS TABLAS ORDENADAS

    imprimirTodasLasTablas(&gestor);

    printf("\n.... FIN .... :D\n");

    liberarArchivoEnMemoria(lineas_memoria, total_lineas);
    fclose(archivo);
    return exito_backpatching ? 0 : 1;
}
