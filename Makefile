CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
TARGET = ensamblador.exe

SRCS = src/modulo_apertura.c \
       src/lectura.c \
       src/seleccion.c \
       src/codificacion.c \
       src/conversion.c \
       src/tablas.c \
       src/errores.c \
       test/test.c

PS = @powershell.exe -NoProfile -ExecutionPolicy Bypass -File comparar.ps1

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

# Casos exitosos (ensamblador propio + comparacion NASM)
run: $(TARGET)
	$(PS) docs/prueba.asm

oficial: $(TARGET)
	$(PS) docs/ejemplo_oficial.asm

clase: $(TARGET)
	$(PS) docs/clase_adelante_atras.asm

multiples: $(TARGET)
	$(PS) docs/multiples_saltos.asm

# Casos de error (validacion propia + rechazo en NASM)
err-inexistente: $(TARGET)
	$(PS) docs/sin_etiqueta_error.asm

err-doble: $(TARGET)
	$(PS) docs/etiqueta_doble_error.asm

err-variable: $(TARGET)
	$(PS) docs/error_variable_no_declarada.asm

err-coma: $(TARGET)
	$(PS) docs/error_sin_coma.asm

err-memoria: $(TARGET)
	$(PS) docs/error_memoria_invalida.asm

# Ejecuta y compara los 9 casos uno por uno
test-all: run oficial clase multiples err-inexistente err-doble err-variable err-coma err-memoria

clean:
	rm -f $(TARGET)
