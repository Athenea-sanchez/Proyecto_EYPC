#ifndef ERRORES_H
#define ERRORES_H

typedef enum {
    ERR_LEXICO,
    ERR_SINTACTICO,
    ERR_SEMANTICO,
    ERR_MEMORIA
} TipoError;

// Reinicia el contador al iniciar el ensamblado
void iniciar_errores(void);

// Registra un error con categoria, numero de linea (0 si no aplica) y mensaje
void reportar_error(TipoError tipo, int linea, const char *mensaje, const char *detalle);

// Devuelve 1 si hubo al menos un error, 0 si esta limpio
int hay_errores(void);

// Devuelve el numero total de errores detectados
int obtener_total_errores(void);

#endif