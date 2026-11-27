# Ensamblador IA-32 en C

Implementación modular en lenguaje C de un **ensamblador de una sola pasada (*one-pass assembler*)** para la arquitectura **IA-32 (32 bits)**. El sistema toma un archivo fuente en lenguaje ensamblador (`.asm`), realiza su validación, análisis léxico, selección de reglas del repertorio IA-32, codificación binaria (`ModR/M`, `SIB` y formatos especiales), conversión a hexadecimal en formato *Little Endian*, resolución de saltos en una pasada mediante **Backpatching** y detención inmediata ante errores léxicos, sintácticos, semánticos o de memoria.

---

## Estructura del Repositorio

```text
├── docs/
│   ├── prueba.asm                      # Archivo fuente ensamblador de prueba por defecto
│   ├── ejemplo_oficial.asm             # Caso oficial de la guía (salto hacia adelante con backpatching)
│   ├── clase_adelante_atras.asm        # Ejercicio de clase (.data, salto adelante JE y salto atrás JMP)
│   ├── multiples_saltos.asm            # Múltiples saltos a una misma etiqueta y detección de etiqueta sin usar
│   ├── sin_etiqueta_error.asm          # Error semántico: salto hacia etiqueta no declarada
│   ├── etiqueta_doble_error.asm        # Error semántico: etiqueta duplicada
│   ├── error_variable_no_declarada.asm # Error semántico: variable en memoria no declarada en .data
│   ├── error_sin_coma.asm              # Error sintáctico: falta de coma separadora entre operandos
│   └── error_memoria_invalida.asm      # Error de memoria: escala/direccionamiento ilegal en IA-32
├── include/
│   ├── estructuras.h                   # Estructuras de datos, enums y constantes compartidas
│   ├── errores.h                       # Prototipos del módulo centralizado de errores
│   ├── modulo_apertura.h               # Prototipos del Módulo 1 (Apertura y verificación)
│   ├── lectura.h                       # Prototipos del Módulo 2 (Lectura y análisis léxico)
│   ├── seleccion.h                     # Prototipos del Módulo 3 (Selección de instrucción y validación)
│   ├── codificacion.h                  # Prototipos del Módulo 4 (Codificación común y especial)
│   ├── conversion.h                    # Prototipos del Módulo 5 (Conversión hexadecimal y Little Endian)
│   └── tablas.h                        # Prototipos del Módulo 6 (Tablas de memoria y backpatching)
├── src/
│   ├── errores.c                       # Implementación del sistema centralizado de errores
│   ├── modulo_apertura.c               # Implementación del Módulo 1
│   ├── lectura.c                       # Implementación del Módulo 2
│   ├── seleccion.c                     # Implementación del Módulo 3
│   ├── codificacion.c                  # Implementación del Módulo 4
│   ├── conversion.c                    # Implementación del Módulo 5
│   └── tablas.c                        # Implementación del Módulo 6
├── test/
│   └── test.c                          # Programa principal de pruebas (main)
├── comparar.ps1                        # Script PowerShell de validación automática contra NASM
├── Makefile                            # Automatización de compilación y batería completa de pruebas
└── README.md                           # Documentación del proyecto
```

---

## Arquitectura Modular

El diseño está dividido en **6 etapas funcionales** respaldadas por un **Módulo Centralizado de Errores**:

### *Módulo 1 — Apertura y Verificación (`modulo_apertura.h` / `modulo_apertura.c`)*
- Valida que la extensión del archivo de entrada sea estrictamente `.asm`.
- Abre el flujo en modo lectura (`FILE*`), calcula su tamaño en bytes y comprueba que no esté vacío.
- Carga el código fuente en memoria dinámica (`LineaFuente*`) conservando el número de línea exacto de cada sentencia.

### *Módulo 2 — Lectura y Análisis Léxico (`lectura.h` / `lectura.c`)*
- Lee el archivo línea por línea y depura espacios, tabulaciones, saltos de línea y comentarios (`;`).
- Clasifica la sentencia mediante diccionario semántico (`ETIQUETA`, `DATOS`, `MOV_DATOS`, `OP_ARITMETICA`, `OP_LOGICA`, `CONTROL_PILA`, `CONTROL_FUNCIONES`, `LLAMADA_SISTEMA`, `DIRECTIVA`).
- Separa la instrucción en tokens (`mnemónico`, `operando1`, `operando2`), determina la naturaleza de cada operando (`REGISTRO`, `INMEDIATO`, `MEMORIA`, `ETIQUETA`) y consulta el diccionario de opcodes base.

### *Módulo 3 — Selección de Instrucción y Validación de Memoria (`seleccion.h` / `seleccion.c`)*
- Calcula el tamaño en bits ($8$, $16$, $32$) de los operandos y valida compatibilidad de tipos y anchos.
- Verifica la gramática de direccionamiento indirecto en IA-32 ($\text{Base} + \text{Índice} \times \text{Escala} + \text{Desplazamiento}$), rechazando factores de escala distintos de $1, 2, 4, 8$, el uso de `ESP` como registro índice, operadores duplicados (`**`, `++`, `+-`) y exceso de registros.
- Consulta el diccionario estático IA-32 para obtener el opcode real, formato `OP/En` (`MR`, `RM`, `MI`, `OI`, `O`), extensión `/0`–`/7` y suma de registro (`+rd`).
- Determina la ruta de codificación hacia el Módulo 4 (**1 = Común**, **2 = Especial**).

### *Módulo 4 — Codificación Común y Especial (`codificacion.h` / `codificacion.c`)*
- **Submódulo 4.1 (Común):** Ensambla el byte estructural `ModR/M` (`mod` + `reg` + `r/m`) traduciendo registros a $3$ bits y modos de direccionamiento a $2$ bits (incluyendo desplazamientos y byte `SIB`).
- **Submódulo 4.2 (Especial):** Ejecuta sumas aritméticas sobre el opcode base (`+r`) y convierte operandos inmediatos a binario.

### *Módulo 5 — Conversión Hexadecimal y Little Endian (`conversion.h` / `conversion.c`)*
- Traduce bloques de $4$ bits (nibbles) y bytes binarios a hexadecimal separado en pares de $2$ caracteres.
- Aplica el reordenamiento **Little Endian** (byte menos significativo primero) a los valores inmediatos, direcciones de memoria y desplazamientos relativos.

### *Módulo 6 — Gestión de Tablas y Backpatching (`tablas.h` / `tablas.c`)*
- Controla las direcciones base para las secciones de datos ($\text{0x00401000}$) y código ($\text{0x00403000}$).
- Administra e imprime las tablas estructuradas: **Tabla de Símbolos**, **Referencias Pendientes**, **Sección de Código (`.text`)**, **Sección de Datos (`.data`)** y **Volcado Lineal de Código Máquina**.
- Resuelve saltos hacia adelante (`JMP`, `JZ`, `JE`, `JNZ`, `JNE`) en **una sola pasada mediante *Backpatching***, parchando los $4$ bytes reservados en el búfer continuo de código con el desplazamiento relativo $rel32$ en complemento a $2$.

### *Módulo Centralizado de Errores (`errores.h` / `errores.c`)*
- Clasifica cualquier anomalía en cuatro categorías: `ERR_LEXICO`, `ERR_SINTACTICO`, `ERR_SEMANTICO` y `ERR_MEMORIA`.
- Reporta la categoría, línea exacta y detalle del fallo, deteniendo el proceso de ensamblado inmediatamente para impedir la generación de tablas corruptas o código máquina parcial.

---

## Requisitos del Entorno

- Compilador **GCC** (MinGW64 / MSYS2 en Windows, o GCC nativo en Linux / WSL).
- **NASM** (*Netwide Assembler*) instalado en el sistema para ejecutar la validación automática contra referencia.
- Utilidad **Make** (`make` o `mingw32-make`).

---

## Compilación y Ejecución

El proyecto compila un único ejecutable (`ensamblador.exe`) a partir de `test/test.c` y los módulos en `src/`.

### Opción 1: Ejecución automatizada con Makefile

```bash
# Compilar todos los módulos con -Wall -Wextra
make

# Ejecutar los 9 casos de prueba (válidos + errores)
make test-all

# Casos exitosos (generan código máquina y comparan 1:1 contra NASM)
make run              # docs/prueba.asm (por defecto)
make oficial          # docs/ejemplo_oficial.asm
make clase            # docs/clase_adelante_atras.asm
make multiples        # docs/multiples_saltos.asm

# Casos de error (reportan fallo exacto y abortan sin generar código)
make err-inexistente  # docs/sin_etiqueta_error.asm
make err-doble        # docs/etiqueta_doble_error.asm
make err-variable     # docs/error_variable_no_declarada.asm
make err-coma         # docs/error_sin_coma.asm
make err-memoria      # docs/error_memoria_invalida.asm

# Eliminar el ejecutable generado
make clean
```

### Opción 2: Compilación y ejecución manual

```bash
gcc -Wall -Wextra -Iinclude src/*.c test/test.c -o ensamblador.exe
./ensamblador.exe docs/prueba.asm
```

---

## Validación contra Ensamblador de Referencia (NASM)

Cada objetivo del `Makefile` (y el script `comparar.ps1`) invoca automáticamente a **NASM** en modo binario plano de $32$ bits sin optimizaciones (`nasm -f bin -O0`) para verificar la correspondencia exacta de los bytes generados:

1. **Formas cortas del acumulador (`EAX`):** En operaciones aritméticas con inmediatos de $32$ bits (`ADD`, `SUB`, `CMP`), nuestro ensamblador aplica la codificación general `MI` (`81 /0`, `81 /5`, `81 /7`, de $6$ bytes), válida para cualquier registro de propósito general, evitando los opcodes abreviados exclusivos de `EAX` (`05`, `2D`, `3D`).
2. **Saltos condicionales cercanos ($rel32$ vs $rel8$):** Todos los saltos condicionales (`JE`, `JNE`, `JZ`, `JNZ`) se codifican en formato cercano de $32$ bits (`0F 84`, `0F 85`, de $6$ bytes), equivalente al modificador `NEAR` en NASM.
3. **Verificación de rechazo en casos de error:** En los 5 escenarios de error (`err-*`), tanto nuestro ensamblador como NASM rechazan el archivo fuente sin emitir código máquina.
