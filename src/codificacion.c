#include "codificacion.h"
#include "errores.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


static void aMayusculasCod(char* dest, const char* orig) {
    int i = 0;
    while (orig[i] != '\0' && i < MAX_TOKEN - 1) {
        dest[i] = (char)toupper((unsigned char)orig[i]);
        i++;
    }
    dest[i] = '\0';
}

// Función 1: Traduce el nombre de un registro a su cadena binaria de 3 bits
char* convertirRegistroABinario(const char* nombre_registro) {
    char* res = (char*)calloc(4, sizeof(char));
    if (!res) return NULL;

    if (nombre_registro == NULL) {
        strcpy(res, "000");
        return res;
    }

    char reg[MAX_TOKEN];
    aMayusculasCod(reg, nombre_registro);

    if (strcmp(reg, "EAX") == 0 || strcmp(reg, "AX") == 0 || strcmp(reg, "AL") == 0) strcpy(res, "000");
    else if (strcmp(reg, "ECX") == 0 || strcmp(reg, "CX") == 0 || strcmp(reg, "CL") == 0) strcpy(res, "001");
    else if (strcmp(reg, "EDX") == 0 || strcmp(reg, "DX") == 0 || strcmp(reg, "DL") == 0) strcpy(res, "010");
    else if (strcmp(reg, "EBX") == 0 || strcmp(reg, "BX") == 0 || strcmp(reg, "BL") == 0) strcpy(res, "011");
    else if (strcmp(reg, "ESP") == 0 || strcmp(reg, "SP") == 0 || strcmp(reg, "AH") == 0) strcpy(res, "100");
    else if (strcmp(reg, "EBP") == 0 || strcmp(reg, "BP") == 0 || strcmp(reg, "CH") == 0) strcpy(res, "101");
    else if (strcmp(reg, "ESI") == 0 || strcmp(reg, "SI") == 0 || strcmp(reg, "DH") == 0) strcpy(res, "110");
    else if (strcmp(reg, "EDI") == 0 || strcmp(reg, "DI") == 0 || strcmp(reg, "BH") == 0) strcpy(res, "111");
    else strcpy(res, "000");

    return res;
}

// Función 2: Calcula los 2 bits del campo mod según el tipo de operando r/m
char* calcularBitsMod(TipoOperando tipo_rm) {
    char* mod = (char*)calloc(3, sizeof(char));
    if (!mod) return NULL;

    if (tipo_rm == OP_REGISTRO) {
        strcpy(mod, "11");
    } else if (tipo_rm == OP_MEMORIA) {
        strcpy(mod, "00");
    } else {
        strcpy(mod, "00");
    }
    return mod;
}

// Auxiliar interna: Analiza memoria "[EBX + 4]" o "[eax*4+2]" y genera (mod, rm, bits_extra_sib_disp)
static void analizarMemoriaParaModRM(const char* expr_mem, char* mod_out, char* rm_out, char* extra_bits_out) {
    strcpy(mod_out, "00");
    strcpy(rm_out, "000");
    extra_bits_out[0] = '\0';

    size_t len = strlen(expr_mem);
    if (len < 3) return;

    char interior[MAX_TOKEN];
    strncpy(interior, expr_mem + 1, len - 2);
    interior[len - 2] = '\0';

    // Quitar espacios internos (ej. "EBX+4" o "EAX*4+2")
    char limpio[MAX_TOKEN];
    int k = 0;
    for (int i = 0; interior[i] != '\0'; i++) {
        if (!isspace((unsigned char)interior[i])) {
            limpio[k++] = (char)toupper((unsigned char)interior[i]);
        }
    }
    limpio[k] = '\0';

    // Caso 1: Direccionamiento escalado con '*' (ej. "[EAX*4+2]" -> requiere byte SIB)
    char* ptr_asterisco = strchr(limpio, '*');
    if (ptr_asterisco != NULL) {
        char reg_idx[MAX_TOKEN] = {0};
        int idx_len = (int)(ptr_asterisco - limpio);
        strncpy(reg_idx, limpio, idx_len);
        reg_idx[idx_len] = '\0';

        int escala = atoi(ptr_asterisco + 1);
        const char* ss = "00";
        if (escala == 2) ss = "01";
        else if (escala == 4) ss = "10";
        else if (escala == 8) ss = "11";

        char* bin_idx = convertirRegistroABinario(reg_idx);

        char* ptr_mas = strchr(ptr_asterisco, '+');
        char* ptr_menos = strchr(ptr_asterisco, '-');

        // En IA-32, [index*scale + disp32] sin registro base usa mod=00, rm=100 (SIB), base=101 (disp32)
        strcpy(mod_out, "00");
        strcpy(rm_out, "100");

        char* bin_disp32 = convertirInmediatoABinario(ptr_mas ? ptr_mas + 1 : (ptr_menos ? ptr_menos : "0"), 32);
        sprintf(extra_bits_out, "%s%s101%s", ss, bin_idx, bin_disp32);

        free(bin_idx);
        free(bin_disp32);
        return;
    }

    // Caso 2: Direccionamiento base + desplazamiento (ej. "[EBX]" o "[EBX+4]" o "[EBX-8]")
    char reg_base[MAX_TOKEN] = {0};
    int i = 0;
    while (limpio[i] != '\0' && limpio[i] != '+' && limpio[i] != '-') {
        reg_base[i] = limpio[i];
        i++;
    }
    reg_base[i] = '\0';

    char* bin_base = convertirRegistroABinario(reg_base);
    strcpy(rm_out, bin_base);
    free(bin_base);

    if (limpio[i] == '+' || limpio[i] == '-') {
        char num_str[MAX_TOKEN];
        strcpy(num_str, &limpio[i]);
        int disp = (int)strtol(num_str, NULL, 0);

        if (disp >= -128 && disp <= 127) {
            strcpy(mod_out, "01");
            char* bin_disp8 = convertirInmediatoABinario(num_str, 8);
            strcpy(extra_bits_out, bin_disp8);
            free(bin_disp8);
        } else {
            strcpy(mod_out, "10");
            char* bin_disp32 = convertirInmediatoABinario(num_str, 32);
            strcpy(extra_bits_out, bin_disp32);
            free(bin_disp32);
        }
    } else {
        strcpy(mod_out, "00");
    }
}

// Función 3: Construye el byte estructural ModR/M (+ SIB/desplazamiento si es memoria)
char* construirByteModRM(
    const char* destino,
    const char* origen,
    TipoOperando tipo_dest,
    TipoOperando tipo_orig,
    int extension_reg,
    TipoCodificacion op_en) {

    char* resultado = (char*)calloc(64, sizeof(char));
    if (!resultado) return NULL;

    char* mod = NULL;
    char* reg = NULL;
    char* rm = NULL;
    char extra_mem[48] = {0};

    if (op_en == OP_EN_MR) {
        reg = convertirRegistroABinario(origen);
        if (tipo_dest == OP_MEMORIA) {
            char mod_mem[3], rm_mem[4];
            analizarMemoriaParaModRM(destino, mod_mem, rm_mem, extra_mem);
            mod = strdup(mod_mem);
            rm = strdup(rm_mem);
        } else {
            mod = calcularBitsMod(tipo_dest);
            rm = convertirRegistroABinario(destino);
        }
    } else if (op_en == OP_EN_RM) {
        reg = convertirRegistroABinario(destino);
        if (tipo_orig == OP_MEMORIA) {
            char mod_mem[3], rm_mem[4];
            analizarMemoriaParaModRM(origen, mod_mem, rm_mem, extra_mem);
            mod = strdup(mod_mem);
            rm = strdup(rm_mem);
        } else {
            mod = calcularBitsMod(tipo_orig);
            rm = convertirRegistroABinario(origen);
        }
    } else if (op_en == OP_EN_MI) {
        reg = (char*)calloc(4, sizeof(char));
        sprintf(reg, "%d%d%d", (extension_reg >> 2) & 1, (extension_reg >> 1) & 1, extension_reg & 1);

        if (tipo_dest == OP_MEMORIA) {
            char mod_mem[3], rm_mem[4];
            analizarMemoriaParaModRM(destino, mod_mem, rm_mem, extra_mem);
            mod = strdup(mod_mem);
            rm = strdup(rm_mem);
        } else {
            mod = calcularBitsMod(tipo_dest);
            rm = convertirRegistroABinario(destino);
        }
    } else {
        strcpy(resultado, "00000000");
        return resultado;
    }

    sprintf(resultado, "%s%s%s%s", mod, reg, rm, extra_mem);

    free(mod);
    free(reg);
    free(rm);
    return resultado;
}

// Función 4: Suma el código de registro (+rd) al opcode base en hexadecimal
char* sumarHexadecimal(const char* opcode_hex, const char* valor_registro_hex) {
    char* res = (char*)calloc(8, sizeof(char));
    if (!res) return NULL;

    unsigned int op = (unsigned int)strtol(opcode_hex, NULL, 16);
    unsigned int reg = (unsigned int)strtol(valor_registro_hex, NULL, 16);
    sprintf(res, "%02X", (op + reg) & 0xFF);
    return res;
}

// Función 5: Convierte un número inmediato a cadena binaria (Little Endian por bytes)
char* convertirInmediatoABinario(const char* valor_inmediato, int tamano_bits) {
    char* bin = (char*)calloc(tamano_bits + 1, sizeof(char));
    if (!bin) return NULL;

    long val = strtol(valor_inmediato, NULL, 0);
    uint32_t uval = (uint32_t)val;

    int num_bytes = tamano_bits / 8;
    int pos = 0;

    for (int b = 0; b < num_bytes; b++) {
        uint8_t byte_actual = (uint8_t)((uval >> (b * 8)) & 0xFF);
        for (int bit = 7; bit >= 0; bit--) {
            bin[pos++] = ((byte_actual >> bit) & 1) ? '1' : '0';
        }
    }
    bin[pos] = '\0';
    return bin;
}

// Auxiliar interna: Convierte un byte hexadecimal a 8 bits en texto
static void hexByteABinario(const char* hex_byte, char* bin_out) {
    unsigned int val = (unsigned int)strtol(hex_byte, NULL, 16);
    for (int i = 7; i >= 0; i--) {
        bin_out[7 - i] = ((val >> i) & 1) ? '1' : '0';
    }
    bin_out[8] = '\0';
}

// Función 6: Procesa codificación especial (+rd en opcode e inmediatos)
char* procesarCodificacionEspecial(ReglaInstruccionIA32 regla, Instruccion inst) {
    char* salida = (char*)calloc(128, sizeof(char));
    if (!salida) return NULL;

    char opcode_final_hex[8];
    strcpy(opcode_final_hex, regla.opcode_hex);

    if (regla.suma_registro) {
        char* reg_bin = convertirRegistroABinario(inst.operando1);
        int reg_num = (int)strtol(reg_bin, NULL, 2);
        char reg_hex[4];
        sprintf(reg_hex, "%X", reg_num);

        char* op_sumado = sumarHexadecimal(regla.opcode_hex, reg_hex);
        strcpy(opcode_final_hex, op_sumado);

        free(reg_bin);
        free(op_sumado);
    }

    char opcode_bin[9];
    hexByteABinario(opcode_final_hex, opcode_bin);
    strcpy(salida, opcode_bin);

    if (inst.tipo_op2 == OP_INMEDIATO) {
        char* inm_bin = convertirInmediatoABinario(inst.operando2, regla.tamano_bits);
        strcat(salida, inm_bin);
        free(inm_bin);
    }

    return salida;
}

// Función 7: Coordinadora del Módulo 4
char* generarCadenaIntermediaModulo4(
    Instruccion inst,
    ReglaInstruccionIA32 regla,
    int tipo_codificacion) {

    if (tipo_codificacion == 2) {
        return procesarCodificacionEspecial(regla, inst);
    }

    char* salida = (char*)calloc(128, sizeof(char));
    if (!salida) return NULL;

    char opcode_bin[9];
    hexByteABinario(regla.opcode_hex, opcode_bin);
    strcpy(salida, opcode_bin);

    char* modrm_bin = construirByteModRM(
        inst.operando1,
        inst.operando2,
        inst.tipo_op1,
        inst.tipo_op2,
        regla.extension_reg,
        regla.op_en
    );
    strcat(salida, modrm_bin);
    free(modrm_bin);
    
    if (regla.op_en == OP_EN_MI && inst.tipo_op2 == OP_INMEDIATO) {
        char* inm_bin = convertirInmediatoABinario(inst.operando2, regla.tamano_bits);
        strcat(salida, inm_bin);
        free(inm_bin);
    }


    return salida;
}
