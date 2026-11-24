#include "conversion.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Convierte 4 caracteres ('0' o '1') en un valor de 0 a 15 y devuelve su carácter hexadecimal
char nibbleAHex(const char* cuatro_bits) {
    int valor = 0;
    for (int i = 0; i < 4; i++) {
        // Desplaza a la izquierda y suma el bit actual
        valor = (valor << 1) | (cuatro_bits[i] == '1' ? 1 : 0);
    }
    const char* tabla_hex = "0123456789ABCDEF";
    return tabla_hex[valor & 0x0F];
}

// Recorre la cadena binaria del Módulo 4 en grupos de 8 bits (1 byte = 2 nibbles)
char* convertirBinarioAHexadecimal(const char* cadena_binaria, int* num_bytes_salida) {
    if (cadena_binaria == NULL) return NULL;

    size_t len = strlen(cadena_binaria);
    int bytes = (int)(len / 8); // Cada 8 bits equivalen a 1 byte
    if (num_bytes_salida != NULL) {
        *num_bytes_salida = bytes;
    }

    // Reservar espacio: 2 caracteres hex + 1 espacio por cada byte
    char* hex_salida = (char*)calloc((bytes * 3) + 2, sizeof(char));
    if (!hex_salida) return NULL;

    int pos = 0;
    for (int b = 0; b < bytes; b++) {
        const char* byte_bin = cadena_binaria + (b * 8);
        hex_salida[pos++] = nibbleAHex(byte_bin);       // Primer nibble (bits 7..4)
        hex_salida[pos++] = nibbleAHex(byte_bin + 4);   // Segundo nibble (bits 3..0)

        if (b < bytes - 1) {
            hex_salida[pos++] = ' '; // Separador visual entre bytes
        }
    }
    hex_salida[pos] = '\0';
    return hex_salida;
}

// Convierte una dirección entera de 32 bits a formato texto de 8 dígitos en mayúsculas
void formatearDireccionHex32(uint32_t direccion, char* buffer_salida) {
    if (buffer_salida == NULL) return;
    sprintf(buffer_salida, "%08X", direccion);
}

// Reordena los pares de bytes de izquierda a derecha en orden inverso (Little Endian)
void invertirBytesLittleEndian(const char* hex_big_endian, char* hex_little_endian) {
    if (hex_big_endian == NULL || hex_little_endian == NULL) return;

    char copia[128];
    strncpy(copia, hex_big_endian, sizeof(copia) - 1);
    copia[sizeof(copia) - 1] = '\0';

    char* tokens[16];
    int count = 0;
    char* tok = strtok(copia, " ");
    while (tok != NULL && count < 16) {
        tokens[count++] = tok;
        tok = strtok(NULL, " ");
    }

    hex_little_endian[0] = '\0';
    for (int i = count - 1; i >= 0; i--) {
        strcat(hex_little_endian, tokens[i]);
        if (i > 0) strcat(hex_little_endian, " ");
    }
}
