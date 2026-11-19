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

int leerLineaArchivo(FILE* archivo,char* buffer_linea,int tamano_max, int*num_linea);


#endif



