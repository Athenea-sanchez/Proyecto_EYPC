#ifndef CODIFICACION_H
#define CODIFICACION_H

#include "estructuras.h"

char* convertirRegistroABinario(
    const char* nombre_registro);

char* calcularBitsMod(
    TipoOperando tipo_rm);

char* construirByteModRM(
    const char* destino,
    const char* origen,
    TipoOperando tipo_dest,
    TipoOperando tipo_orig,
    int extension_reg,
    TipoCodificacion op_en);

char* sumarHexadecimal(
    const char* opcode_hex,
    const char* valor_registro_hex);

char* convertirInmediatoABinario(
    const char* valor_inmediato,
    int tamano_bits);

char* procesarCodificacionEspecial(
    ReglaInstruccionIA32 regla,
    Instruccion inst);

char* generarCadenaIntermediaModulo4(
    Instruccion inst,
    ReglaInstruccionIA32 regla,
    int tipo_codificacion);

#endif // CODIFICACION_H
