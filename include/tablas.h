#ifndef TABLAS_H
#define TABLAS_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "estructuras.h"

// Inicializa el gestor, pone contadores en cero y establece la dirección base de .text
void inicializarGestorTablas(GestorTablas* gestor, uint32_t base_codigo);

// Registra una etiqueta en la Tabla de Símbolos; falla si la etiqueta ya estaba definida (duplicada)
bool registrarSimbolo(GestorTablas* gestor, const char* nombre, const char* seccion, size_t linea);

// Busca una etiqueta por nombre en la Tabla de Símbolos; devuelve su índice o -1 si no existe
int buscarIndiceSimbolo(const GestorTablas* gestor, const char* nombre);

// Procesa directivas de datos (DB) y actualiza el contador de .data
bool agregarEntradaDatos(GestorTablas* gestor, Instruccion inst, size_t linea);

// Escribe una instrucción codificada normal en el búfer binario y avanza el contador de localidad
bool agregarEntradaCodigo(GestorTablas* gestor, Instruccion inst, const char* hex_maquina, int num_bytes);

// Codifica saltos relativos de 32 bits (JMP, JZ/JE, JNZ/JNE):
// - Si la etiqueta ya existe arriba (salto atrás), calcula el offset al instante.
// - Si aún no existe (salto adelante), reserva 00 00 00 00 y registra referencia pendiente.
bool agregarInstruccionSalto(GestorTablas* gestor, Instruccion inst, size_t linea);

// Fase 2 (Backpatching en memoria): recorre las referencias pendientes y parcha los desplazamientos
bool resolver_referencias(GestorTablas* gestor);

// Revisa la Tabla de Símbolos y emite advertencias si alguna etiqueta definida nunca fue utilizada
void verificarEtiquetasSinUsar(const GestorTablas* gestor);

// Imprime en consola las tablas estructuradas y el volcado lineal del búfer de código máquina
void imprimirTodasLasTablas(const GestorTablas* gestor);

#endif // TABLAS_H