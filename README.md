# Ensamblador  IA-32 en C

Implementación modular en lenguaje C de un ensamblador para la arquitectura **Intel IA-32 (x86)**. El sistema toma un archivo fuente en lenguaje ensamblador (`.asm`), realiza su validación, análisis léxico, selección de reglas del repertorio IA-32, codificación binaria (`ModR/M` y formatos especiales), conversión a hexadecimal en formato *Little Endian* y almacenamiento estructurado en tablas de memoria.

---

##  Estructura del Repositorio

```text
├── docs/
│   └── prueba.asm            # Archivo fuente ensamblador de prueba
├── include/
│   ├── estructuras.h         # Estructuras de datos, enums y constantes compartidas
│   ├── modulo_apertura.h     # Prototipos del Módulo 1 (Apertura y verificación)
│   └── lectura.h             # Prototipos del Módulo 2 (Lectura y análisis léxico)
├── src/
│   ├── modulo_apertura.c     # Implementación del Módulo 1
│   └── lectura.c             # Implementación del Módulo 2
├── test/
│   └── test.c                # Programa principal de pruebas (main)
└── README.md                 # Documentación del proyecto
```

---

##  Arquitectura Modular (6 Etapas)

El diseño está dividido en 6 módulos independientes con responsabilidades estrictas:

### *Módulo 1 — Apertura y Verificación (`modulo_apertura.h` / `modulo_apertura.c`)*
* Valida que la extensión del archivo de entrada sea `.asm`.
* Abre el flujo en modo lectura (`FILE*`), calcula su tamaño en bytes y comprueba que no esté vacío.
* Gestiona códigos de error de apertura y entrega el descriptor listo en el byte `0` al Módulo 2.

### *Módulo 2 — Lectura y Análisis Léxico (`lectura.h` / `lectura.c`)*
* Lee el archivo línea por línea y depura espacios, tabulaciones, saltos de línea y comentarios (`;`).
* Clasifica la sentencia mediante diccionario semántico (`ETIQUETA`, `DATOS`, `MOV_DATOS`, `OP_ARITMETICA`, `OP_LOGICA`, `CONTROL_PILA`, `CONTROL_FUNCIONES`, `LLAMADA_SISTEMA`, `DIRECTIVA`).
* Separa la instrucción en tokens (`mnemónico`, `operando1`, `operando2`), determina la naturaleza de cada operando (`REGISTRO`, `INMEDIATO`, `MEMORIA`, `ETIQUETA`) y consulta el diccionario de opcodes base.

### *Módulo 3 — Selección de Instrucción IA-32 (`seleccion.h` / `seleccion.c`)*
* Calcula el tamaño en bits (`8`, `16`, `32`) de los operandos.
* Valida compatibilidad de tipos y tamaños según las reglas de la arquitectura x86.
* Consulta el diccionario estático IA-32 para obtener el opcode real, formato `OP/En` (`MR`, `RM`, `MI`, `OI`, `D`, `M`, `O`, `I`, `NP`), extensión `/0`–`/7` y suma de registro (`+rb`, `+rw`, `+rd`).
* Determina la ruta de codificación hacia el Módulo 4 (**1 = Común**, **2 = Especial**).

### *Módulo 4 — Codificación Común y Especial (`codificacion.h` / `codificacion.c`)*
* **Submódulo 4.1 (Común):** Ensambla el byte estructural `ModR/M` (`mod` + `reg` + `r/m`) traduciendo registros a 3 bits y modos de direccionamiento a 2 bits.
* **Submódulo 4.2 (Especial):** Ejecuta sumas aritméticas en hexadecimal sobre el opcode base (`+r`) y convierte operandos inmediatos a binario.

### *Módulo 5 — Conversión Hexadecimal y Little Endian (`conversion.h` / `conversion.c`)*
* Traduce bloques de 4 bits (nibbles) y bytes binarios a hexadecimal tradicional separado en pares de 2 caracteres.
* Aplica el reordenamiento **Little Endian** (byte menos significativo primero) a los valores inmediatos y direcciones de memoria.
* Ensambla la cadena final en el orden estricto IA-32 (`Opcode` + `ModR/M` + `Inmediato/Desplazamiento`).

### *Módulo 6 — Gestión de Tablas y Memoria Universal (`tablas.h` / `tablas.c`)*
* Inicializa y controla la memoria universal para las secciones de datos (`0x00401000`) y código (`0x00403000`) contando pares de bytes.
* Administra las matrices bidimensionales: **Tabla de Símbolos**, **Sección de Datos**, **Sección de Código** y **Referencias Pendientes**.
* Ejecuta la segunda pasada para calcular desplazamientos relativos (`offset`) mediante complemento a 2 y parchar las instrucciones de salto (`JMP`, `JZ`, `JNZ`).


---

##  Requisitos del Entorno

* Compilador **GCC** (estándar C99 o superior).
* Entorno Linux / GitHub Codespaces / MinGW.

## Compilación y Ejecución

En esta etapa se compilan el módulo de apertura y lectura en memoria dinámica (`src/modulo_apertura.c`), el módulo de análisis léxico (`src/lectura.c`) y el programa de pruebas (`test/test.c`) utilizando las cabeceras de `include/` y el estándar C99.

### Requisitos
- Compilador **GCC** (estándar C99 o superior).
- Entorno **Windows (PowerShell con MinGW)** o **Linux / WSL / GitHub Codespaces**.

### Preparación y Compilación (Windows / PowerShell)
Si `src/modulo_apertura.c` contiene caracteres no ASCII (`ñ`) en nombres de variables, primero se normaliza el archivo en UTF-8 para evitar errores de compilación en GCC:

```powershell
$txt = [System.IO.File]::ReadAllText("src\modulo_apertura.c", [System.Text.Encoding]::UTF8)
$txt =$txt.Replace("tamaño_bytes", "tamano_bytes").Replace("tamaño", "tamano")
[System.IO.File]::WriteAllText("src\modulo_apertura.c", $txt, [System.Text.Encoding]::UTF8)

A continuación, se compila el proyecto desde la carpeta raíz:
gcc -Wall -Wextra -std=c99 -Iinclude src/modulo_apertura.c src/lectura.c test/test.c -o ensamblador.exe

Finalmente, se ejecuta el programa pasando como argumento el archivo fuente de prueba:
.\ensamblador.exe docs/prueba.asm