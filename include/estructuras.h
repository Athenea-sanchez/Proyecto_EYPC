#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define MAX_TOKEN 64
#define MAX_LINEA 512
#define MAX_FILAS_TABLA 256
#define MAX_BUFFER_CODIGO 4096
#define DIR_BASE_DATA 0x00401000
#define DIR_BASE_TEXT 0x00403000

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
    int numero_linea;
    char* texto;
} LineaFuente;

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

// tabla de simbolos 
typedef struct {
    char nombre[MAX_TOKEN];    // Identificador de la etiqueta o variable 
    char seccion[16];          // Sección a la que pertenece 
    uint32_t direccion;        // Dirección real calculada dinámicamente
    char direccion_hex[12];    // Representación hexadecimal de 8 dígitos para visualización
    bool definida;             // ETIQUETAS DUPLICADAS
    bool usada;                // BANDERA SI SE USAN LAS ETIQUETAS
} Simbolo;

// tabla de referencias pendientes 
typedef struct {
    char nombreEtiqueta[MAX_TOKEN];        // Nombre de la etiqueta destino pendiente
    uint32_t posicionDesplazamiento;       // Índice exacto dentro de buffer_codigo donde van los ceros
    uint32_t direccionSiguienteInstruccion;// Dirección de la instrucción posterior al salto
    uint8_t tamanoDesplazamiento;          // Tamaño en bytes del campo reservado 
    size_t numeroLinea;                    // Línea del archivo fuente .asm para reporte de errores
    int indice_fila_codigo;                // Índice de la fila en la tabla visual de código
    bool resuelta;                         // Cambia a true tras completar el backpatching
} ReferenciaPendiente;

// Fila visual para la Sección de Datos (.data)
typedef struct {
    uint32_t direccion;
    char direccion_hex[12];
    char etiqueta[MAX_TOKEN];
    char tipo_dato[8];
    char valor_original[128];
    char bytes_hex[256];
    int num_bytes;
} FilaDatos;

// Fila visual para la Sección de Código (.text)
typedef struct {
    uint32_t direccion;
    char direccion_hex[12];
    char instruccion_asm[128];
    char codigo_maquina[64];
    int num_bytes;
    bool pendiente_salto;
    char etiqueta_destino[MAX_TOKEN];
} FilaCodigo;

// Contenedor universal de memoria, búfer continuo de código y tablas
typedef struct {
    uint32_t contador_datos;                  // Contador de localidad para .data 0x00401000
    uint32_t contador_codigo;                 // Contador de localidad para .text

    uint8_t buffer_codigo[MAX_BUFFER_CODIGO]; // Búfer binario continuo con el código máquina real
    uint32_t longitud_buffer;                 // Total de bytes escritos en buffer_codigo

    Simbolo simbolos[MAX_FILAS_TABLA];
    int total_simbolos;

    FilaDatos datos[MAX_FILAS_TABLA];
    int total_datos;

    FilaCodigo codigo[MAX_FILAS_TABLA];
    int total_codigo;

    ReferenciaPendiente pendientes[MAX_FILAS_TABLA];
    int total_pendientes;
} GestorTablas;

#endif // ESTRUCTURAS_H