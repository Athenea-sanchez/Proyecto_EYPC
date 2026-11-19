#ifndef SELECCION_H
#define SELECCION_H

#include <stdbool.h>
#include "estructuras.h"

int obtenerTamanoOperando(
    const char* operando,
    TipoOperando tipo);

ReglaInstruccionIA32 buscarReglaEnDiccionario(
    const char* mnemonico,
    TipoOperando tipo_dest,
    TipoOperando tipo_orig,
    int tamano_bits);

int determinarTipoCodificacion(
    ReglaInstruccionIA32 regla);

bool validarCompatibilidadOperandos(
    TipoOperando tipo_dest,
    TipoOperando tipo_orig,
    int tamano_dest,
    int tamano_orig);

bool procesarInstruccionModulo3(
    Instruccion inst_entrada,
    ReglaInstruccionIA32* regla_salida,
    int* tipo_codificacion_salida);

#endif // SELECCION_H
