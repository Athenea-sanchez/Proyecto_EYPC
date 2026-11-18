#ifndef APERTURA_H
#define APERTURA_H

#include <stdio.h>
#include <stdbool.h>

bool ExtensionArchivo(const char* ruta_archivo);

/* Apertura del archivo en modo lectura */
FILE* abrirFlujoLectura(const char* ruta_archivo);

long calcularTamanoArchivo(FILE* archivo);

bool validarArchivoNoVacio(long tamano_bytes);

void gestionarErrorApertura(int codigo_error, const char* ruta_archivo);

FILE* obtenerArchivoVerificado(const char* ruta_archivo,bool* estado_verificacion);

int leerLineaArchivo(FILE* archivo,char* buffer_linea,int tamano_max);


#endif


4.4 ARCHIVO MÓDULO 2.h -LECTURA
Contiene las funciones encargadas de leer y analizar las líneas del ensamblador. 

#ifndef LECTURA_H
#define LECTURA_H

#include <stdio.h>
#include <stdbool.h>
//#include "estructuras.h"

int leerSiguienteLinea(FILE* archivo, char* buffer_linea, int tamano_max);

void limpiarLinea(char* linea_cruda);

CategoriaLinea clasificarConDiccionario(const char* linea_limpia);

bool separarTokensInstruccion(const char* linea_limpia,char* mnemonico,char* operando1,char* operando2);

TipoOperando identificarTipoOperando(const char* operando);

bool procesarLineaModulo2(FILE* archivo,Instruccion* instruccion_salida,CategoriaLinea* categoria_salida);

#endif

