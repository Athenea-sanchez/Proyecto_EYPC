# Ensamblador IA-32 en C

Implementación modular en lenguaje C de un ensamblador para la arquitectura **IA-32**. El sistema toma un archivo fuente en lenguaje ensamblador (`.asm`), realiza su validación, análisis léxico, selección de reglas del repertorio IA-32, codificación binaria (`ModR/M` y formatos especiales), conversión a hexadecimal en formato *Little Endian* y almacenamiento estructurado en tablas de memoria con resolución de saltos en dos pasadas (*backpatching*).

---

## Estructura del Repositorio

```text
├── docs/
│   ├── prueba.asm                  # Archivo fuente ensamblador de prueba por defecto
│   ├── ejemplo_oficial.asm         # Caso oficial de la guía (salto hacia adelante con backpatching)
│   ├── clase_adelante_atras.asm    # Ejercicio de clase (.data, salto adelante JE y salto atrás JMP)
│   ├── multiples_saltos.asm        # Múltiples saltos a una misma etiqueta y detección de etiqueta sin usar
│   ├── sin_etiqueta_error.asm      # Caso de prueba de error semántico (etiqueta inexistente)
│   └── etiqueta_doble_error.asm    # Caso de prueba de error semántico (etiqueta duplicada)
├── include/
│   ├── estructuras.h               # Estructuras de datos, enums y constantes compartidas
│   ├── modulo_apertura.h           # Prototipos del Módulo 1 (Apertura y verificación)
│   ├── lectura.h                   # Prototipos del Módulo 2 (Lectura y análisis léxico)
│   ├── seleccion.h                 # Prototipos del Módulo 3 (Selección de instrucción IA-32)
│   ├── codificacion.h              # Prototipos del Módulo 4 (Codificación común y especial)
│   ├── conversion.h                # Prototipos del Módulo 5 (Conversión hexadecimal y Little Endian)
│   └── tablas.h                    # Prototipos del Módulo 6 (Tablas de memoria y backpatching)
├── src/
│   ├── modulo_apertura.c           # Implementación del Módulo 1
│   ├── lectura.c                   # Implementación del Módulo 2
│   ├── seleccion.c                 # Implementación del Módulo 3
│   ├── codificacion.c              # Implementación del Módulo 4
│   ├── conversion.c                # Implementación del Módulo 5
│   └── tablas.c                    # Implementación del Módulo 6
├── test/
│   └── test.c                      # Programa principal de pruebas (main)
├── Makefile                        # Automatización de compilación y batería de pruebas
└── README.md                       # Documentación del proyecto

## Arquitectura Modular (6 Etapas)

El diseño está dividido en 6 módulos independientes con responsabilidades estrictas:

### *Módulo 1 — Apertura y Verificación (`modulo_apertura.h` / `modulo_apertura.c`)*
- Valida que la extensión del archivo de entrada sea `.asm`.
- Abre el flujo en modo lectura (`FILE*`), calcula su tamaño en bytes y comprueba que no esté vacío.
- Carga el código fuente en memoria dinámica con control de número de línea y entrega el descriptor listo en el byte `0` al Módulo 2.

### *Módulo 2 — Lectura y Análisis Léxico (`lectura.h` / `lectura.c`)*
- Lee el archivo línea por línea y depura espacios, tabulaciones, saltos de línea y comentarios (`;`).
- Clasifica la sentencia mediante diccionario semántico (`ETIQUETA`, `DATOS`, `MOV_DATOS`, `OP_ARITMETICA`, `OP_LOGICA`, `CONTROL_PILA`, `CONTROL_FUNCIONES`, `LLAMADA_SISTEMA`, `DIRECTIVA`).
- Separa la instrucción en tokens (`mnemónico`, `operando1`, `operando2`), determina la naturaleza de cada operando (`REGISTRO`, `INMEDIATO`, `MEMORIA`, `ETIQUETA`) y consulta el diccionario de opcodes base.

### *Módulo 3 — Selección de Instrucción IA-32 (`seleccion.h` / `seleccion.c`)*
- Calcula el tamaño en bits (`8`, `16`, `32`) de los operandos.
- Valida compatibilidad de tipos y tamaños según las reglas de la arquitectura.
- Consulta el diccionario estático IA-32 para obtener el opcode real, formato `OP/En` (`MR`, `RM`, `MI`, `OI`, `D`, `M`, `O`, `I`, `NP`), extensión `/0`–`/7` y suma de registro (`+rb`, `+rw`, `+rd`).
- Determina la ruta de codificación hacia el Módulo 4 (**1 = Común**, **2 = Especial**).

### *Módulo 4 — Codificación Común y Especial (`codificacion.h` / `codificacion.c`)*
- **Submódulo 4.1 (Común):** Ensambla el byte estructural `ModR/M` (`mod` + `reg` + `r/m`) traduciendo registros a 3 bits y modos de direccionamiento a 2 bits (incluyendo desplazamientos y byte `SIB`).
- **Submódulo 4.2 (Especial):** Ejecuta sumas aritméticas en hexadecimal sobre el opcode base (`+r`) y convierte operandos inmediatos a binario.

### *Módulo 5 — Conversión Hexadecimal y Little Endian (`conversion.h` / `conversion.c`)*
- Traduce bloques de 4 bits (nibbles) y bytes binarios a hexadecimal tradicional separado en pares de 2 caracteres.
- Aplica el reordenamiento **Little Endian** (byte menos significativo primero) a los valores inmediatos y direcciones de memoria.
- Ensambla la cadena final en el orden estricto IA-32 (`Opcode` + `ModR/M` + `Inmediato/Desplazamiento`).

### *Módulo 6 — Gestión de Tablas y Memoria Universal (`tablas.h` / `tablas.c`)*
- Inicializa y controla las direcciones base para las secciones de datos (`0x00401000`) y código (`0x00403000`).
- Administra e imprime las tablas estructuradas: **Tabla de Símbolos**, **Referencias Pendientes (Pasada 1)**, **Referencias Resueltas (Pasada 2)**, **Sección de Código (`.text`)**, **Sección de Datos (`.data`)** y **Volcado Lineal de Código Máquina**.
- Ejecuta la segunda pasada (*backpatching*) para calcular desplazamientos relativos (`offset`) mediante complemento a 2 y parchar las instrucciones de salto (`JMP`, `JZ`, `JE`, `JNZ`, `JNE`).

---

## Requisitos del Entorno

- Compilador **GCC** (MinGW64 / MSYS2 en Windows, o GCC nativo en Linux / WSL / GitHub Codespaces).
- Utilidad **Make** (`make` o `mingw32-make`) opcional para ejecución automatizada.

---

## Compilación y Ejecución

El proyecto utiliza un único ejecutable principal (`test/test.c`) que recibe por parámetro la ruta del archivo `.asm` a ensamblar. Puedes compilar y ejecutar las pruebas mediante `Makefile` o manualmente desde la terminal.

### Opción 1: Ejecución automatizada con Makefile 

Desde la carpeta raíz del proyecto, ejecuta cualquiera de los siguientes comandos (en Windows puedes usar `mingw32-make` si `make` no está en el PATH):

```bash
# Compilar todos los módulos
make

# Ejecutar toda la batería de pruebas en una sola pasada
make test-all

# Ejecutar un caso de prueba específico
make run              # Ejecuta docs/prueba.asm (por defecto)
make oficial          # Ejecuta docs/ejemplo_oficial.asm
make clase            # Ejecuta docs/clase_adelante_atras.asm
make multiples        # Ejecuta docs/multiples_saltos.asm
make err-inexistente  # Ejecuta docs/sin_etiqueta_error.asm
make err-doble        # Ejecuta docs/etiqueta_doble_error.asm

# Eliminar el ejecutable generado
make clean
```

### Opción 2: Compilación y ejecución manual (caso por caso)

Si prefieres compilar directamente con `gcc` sin utilizar el `Makefile`, ejecuta desde la carpeta raíz:

```bash
gcc -Wall -Wextra -Iinclude src/*.c test/test.c -o ensamblador.exe
```

Posteriormente, ejecuta `./ensamblador.exe` pasando como argumento el archivo de prueba que desees evaluar:

```bash
./ensamblador.exe docs/prueba.asm
./ensamblador.exe docs/ejemplo_oficial.asm
./ensamblador.exe docs/clase_adelante_atras.asm
./ensamblador.exe docs/multiples_saltos.asm
./ensamblador.exe docs/sin_etiqueta_error.asm
./ensamblador.exe docs/etiqueta_doble_error.asm
```
