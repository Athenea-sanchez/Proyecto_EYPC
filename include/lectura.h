#ifndef LECTURA_H
#define LECTURA_H

#include <stdio.h>
#include <stdbool.h>
#include "estructuras.h"

int leerSiguienteLinea(FILE* archivo, char* buffer_linea, int tamano_max);
void limpiarLinea(char* linea_cruda);
CategoriaLinea clasificarConDiccionario(const char* linea_limpia);
bool separarTokensInstruccion(const char* linea_limpia, char* mnemonico, char* operando1, char* operando2);
TipoOperando identificarTipoOperando(const char* operando);
bool procesarLineaModulo2(FILE* archivo, Instruccion* instruccion_salida, CategoriaLinea* categoria_salida);

#endif
