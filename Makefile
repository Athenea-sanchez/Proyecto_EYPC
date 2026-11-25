CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
TARGET = ensamblador.exe

SRCS = src/modulo_apertura.c \
       src/lectura.c \
       src/seleccion.c \
       src/codificacion.c \
       src/conversion.c \
       src/tablas.c \
       test/test.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

# Ejecuta el archivo por defecto (docs/prueba.asm)
run: $(TARGET)
	./$(TARGET)

# Ejecuta cada caso de prueba de la carpeta docs/ por separado
oficial: $(TARGET)
	./$(TARGET) docs/ejemplo_oficial.asm

clase: $(TARGET)
	./$(TARGET) docs/clase_adelante_atras.asm

multiples: $(TARGET)
	./$(TARGET) docs/multiples_saltos.asm

err-inexistente: $(TARGET)
	./$(TARGET) docs/sin_etiqueta_error.asm

err-doble: $(TARGET)
	./$(TARGET) docs/etiqueta_doble_error.asm

# Ejecuta toda la batería completa en una sola pasada
test-all: $(TARGET)
	@echo "=========================================="
	@echo "1. PRUEBA BASE (docs/prueba.asm)"
	@echo "=========================================="
	-./$(TARGET) docs/prueba.asm
	@echo "\n=========================================="
	@echo "2. EJEMPLO OFICIAL GUIA (docs/ejemplo_oficial.asm)"
	@echo "=========================================="
	-./$(TARGET) docs/ejemplo_oficial.asm
	@echo "\n=========================================="
	@echo "3. EJERCICIO CLASE (docs/clase_adelante_atras.asm)"
	@echo "=========================================="
	-./$(TARGET) docs/clase_adelante_atras.asm
	@echo "\n=========================================="
	@echo "4. MULTIPLES SALTOS Y SIN USAR (docs/multiples_saltos.asm)"
	@echo "=========================================="
	-./$(TARGET) docs/multiples_saltos.asm
	@echo "\n=========================================="
	@echo "5. ERROR: SIN ETIQUETA (docs/sin_etiqueta_error.asm)"
	@echo "=========================================="
	-./$(TARGET) docs/sin_etiqueta_error.asm
	@echo "\n=========================================="
	@echo "6. ERROR: ETIQUETA DOBLE (docs/etiqueta_doble_error.asm)"
	@echo "=========================================="
	-./$(TARGET) docs/etiqueta_doble_error.asm

clean:
	rm -f $(TARGET)