#include <stdio.h>
#include "errores.h"

static int contador_errores = 0;

void iniciar_errores(void) {
    contador_errores = 0;
}

void reportar_error(TipoError tipo, int linea, const char *mensaje, const char *detalle) {
    const char *etiqueta = "GENERAL";
    switch (tipo) {
        case ERR_LEXICO:     etiqueta = "LEXICO";     break;
        case ERR_SINTACTICO: etiqueta = "SINTACTICO"; break;
        case ERR_SEMANTICO:  etiqueta = "SEMANTICO";  break;
        case ERR_MEMORIA:    etiqueta = "MEMORIA";    break;
    }

    if (linea > 0) {
        fprintf(stderr, "[ERROR %s] Linea %d: %s", etiqueta, linea, mensaje);
    } else {
        fprintf(stderr, "[ERROR %s]: %s", etiqueta, mensaje);
    }

    if (detalle != NULL && detalle[0] != '\0') {
        fprintf(stderr, " -> '%s'", detalle);
    }
    fprintf(stderr, "\n");

    contador_errores++;
}

int hay_errores(void) {
    return contador_errores > 0;
}

int obtener_total_errores(void) {
    return contador_errores;
}