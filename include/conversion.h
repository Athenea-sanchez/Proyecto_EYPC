#ifndef CONVERSION_H
#define CONVERSION_H

#include <stdbool.h>
#include <stdint.h> // Permite usar tipos enteros de tamaño exacto int8_t/uint32_t
#include "estructuras.h"

// Traduce un bloque de 4 bits ("1011") a su dígito hexadecimal correspondiente ('B')
char nibbleAHex(const char* cuatro_bits);

// Convierte una cadena binaria completa a pares hexadecimales separados por espacio
// "1011100000001010000000000000000000000000" -> "B8 0A 00 00 00"
char* convertirBinarioAHexadecimal(const char* cadena_binaria, int* num_bytes_salida);

// Formatea un entero sin signo de 32 bits a una cadena hexadecimal de 8 caracteres ("00000005")
void formatearDireccionHex32(uint32_t direccion, char* buffer_salida);

// Invierte el orden de una secuencia de pares hexadecimales de Big Endian a Little Endian
void invertirBytesLittleEndian(const char* hex_big_endian, char* hex_little_endian);

#endif // CONVERSION_H
