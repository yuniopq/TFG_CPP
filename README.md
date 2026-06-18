# Diseño e Implementación de un Simulador de Códigos BCH para el Análisis de Rendimiento en Canales con Ruido

## 📋 Información del Proyecto
* **Contexto:** Trabajo de Fin de Grado (TFG)
* **Titulación:** Grado en Ingeniería de Computadores
* **Centro:** Escuela Técnica Superior de Ingenierías Informática y de Telecomunicación (ETSIIT)
* **Institución:** Universidad de Granada (UGR)

---

## 📖 Descripción General
Este proyecto consiste en un simulador de alto rendimiento para códigos **Bose-Chaudhuri-Hocquenghem (BCH)** diseñado para evaluar de forma cuantitativa y cualitativa la tasa de error en entornos de transmisión digital. El software permite realizar barridos estadísticos exhaustivos mediante simulaciones de Montecarlo y procesar archivos reales (como imágenes) inyectando ruido controlado para comprobar la fidelidad de la reconstrucción y decodificación sistemática.

### Características Destacadas
* **Campos de Galois Configurables:** Soporte completo para cuerpos dinámicos $GF(2^m)$ con parámetros de diseño en el rango de $3 \leq m \leq 15$.
* **Decodificación Avanzada:** Implementación eficiente del algoritmo algebraico de **Berlekamp-Massey** (para hallar el polinomio localizador de errores) y la **Búsqueda de Chien** (para la corrección mediante la localización de raíces).
* **Paralelismo Avanzado:** Optimización multihilo utilizando directivas `OpenMP` para maximizar el uso de los núcleos de la CPU en simulaciones intensivas.
* **Optimización de Memoria (`PackedBit`):** Estructura de empaquetado binario a bajo nivel que compacta los vectores de bits en bloques `uint64_t`, acelerando de forma crítica las operaciones de división polinómica mediante desplazamientos de bits por hardware (`XOR Shifted`).

---

## 🛠️ Requisitos del Sistema y Dependencias

Para compilar y ejecutar este simulador se requiere un entorno de desarrollo moderno que cumpla con los siguientes estándares:

1. **Compilador:** GCC (`g++`) compatible con el estándar **C++17**.
   * *¿Por qué C++17?* El simulador utiliza la librería nativa `<filesystem>` (específicamente `std::filesystem::file_size`) dentro del módulo de lectura de archivos (`BCH_Simulator.cpp`) para la gestión óptima y portable de tamaños en disco sin depender de llamadas de bajo nivel del sistema operativo.
2. **Soporte OpenMP:** La biblioteca y los flags de compilación multihilo de `OpenMP` deben estar instalados y configurados en el sistema.
3. **Entorno de Ejecución:** Entornos basados en POSIX como Linux o el Subsistema de Windows para Linux (WSL).

---

## 📂 Estructura del Repositorio

El código está organizado siguiendo las convenciones de desarrollo en C++:

```text
.
├── Makefile                     # Automatización de la compilación y ejecución
├── include/                     # Archivos de cabecera (.h)
│   ├── BCH_Codec.h              # Interfaz del codificador/decodificador BCH
│   ├── BCH_Simulator.h          # Configuración del motor de simulación
│   ├── Channel.h                # Definición de los modelos de canal
│   ├── GaloisField.h            # Estructura del Campo de Galois GF(2^m)
│   ├── PackedBit.h              # Estructura de empaquetado binario optimizado
│   └── Polynomial.h             # Aritmética polinómica en campos finitos
├── src/                         # Código fuente (.cpp)
│   ├── main.cpp                 # Punto de entrada y parseo de argumentos CLI
│   └── core/                    # Núcleo algorítmico del simulador
│       ├── BCH_Codec.cpp        # Implementación de codificación/decodificación
│       ├── BCH_Simulator.cpp    # Control de flujos (Montecarlo y Archivos)
│       ├── Channel.cpp          # Modelado matemático de canales (AWGN, BSC)
│       ├── GaloisField.cpp      # Tablas log/antilog y operaciones en GF
│       └── Polynomial.cpp       # Evaluación (Horner) y aritmética de polinomios
├── scripts/                     # Herramientas de investigación adicionales
│   └── generador_primitivo.cpp  # Buscador matemático de polinomios primitivos
└── results/                     # Directorio autogenerado de salidas
    ├── csv/                     # Reportes estadísticos exportados
    └── (imágenes/archivos)      # Archivos corruptos vs. corregidos

```

---

## 🚀 Guía de Compilación y Automatización

El proyecto cuenta con un sistema robusto de automatización en su `Makefile`. Antes de ejecutar cualquier simulación, el entorno crea automáticamente los directorios necesarios.

### Comandos Principales de Construcción

* **Compilar el proyecto:**
```bash
make

```


*(Genera el binario optimizado `bch_sim` aplicando flags de rendimiento `-O3 -march=native -fopenmp`)*.
* **Limpiar el entorno:**
```bash
make clean

```


*(Elimina los archivos objeto de la carpeta `build/` y el binario ejecutable)*.
* **Menú de ayuda rápida:**
```bash
make help

```



---

## 🕹️ Manual de Ejecución Completo (`make run`)

El comando `make run` expone de forma directa toda la versatilidad del simulador. Dependiendo de los argumentos proporcionados, el software conmutará automáticamente entre sus dos grandes flujos de trabajo.

### 📑 Parámetros Configurables

Puedes modificar el comportamiento de la ejecución inyectando variables directamente en la línea de comandos:

| Variable | Descripción Técnica | Valor por Defecto | Rangos / Opciones |
| --- | --- | --- | --- |
| `M` | Grado del cuerpo matemático $GF(2^M)$ | `7` | $3 \leq M \leq 15$ |
| `T` | Capacidad de corrección del código (bits erróneos) | `10` | $1 \leq 2t < 2^M-1$ |
| `MIN` | Relación señal-ruido inicial ($E_b/N_0$) en dB / Inyección física | `0` | Valores reales (ej. `4.5`) |
| `MAX` | Relación señal-ruido límite ($E_b/N_0$) en dB | `10` | Valores reales |
| `STEP` | Incremento o paso del barrido de ruido | `1` | Valores reales positivos |
| `FILE` | Ruta absoluta o relativa de un archivo para procesar | *Vacío* (Modo Estadístico) | Ruta a cualquier extensión |
| `CHAN` | Modelo físico de canal a instanciar (solo con `FILE`) | `awgn` | `awgn` o `bsc` |
| `PROB` | Probabilidad intrínseca de error por bit (solo con `CHAN=bsc`) | `0.01` | Rango $[0.0, 1.0]$ |

---

### 1️⃣ Posibilidades en Modo Estadístico (Montecarlo)

Este modo se activa de manera predeterminada cuando el argumento `FILE` se deja en blanco o se omite. Por consistencia de diseño teórico, **fuerza el uso del canal AWGN** para simular la degradación física de la señal y generar curvas de tasa de error.

* **Barrido por defecto (BCH(127,57) con $t=10$ de 0 a 10 dB):**
```bash
make run

```


* **Personalización del código y del rango de simulación:**
Ejemplo para un código BCH(15,7) con capacidad de corregir 2 bits, evaluando desde 0 dB hasta 8 dB con saltos finos de 0.5 dB:
```bash
make run M=4 T=2 MIN=0 MAX=8 STEP=0.5

```


* **Simulación exhaustiva de alta precisión:**
Ajusta los parámetros para evaluar códigos de bloques grandes con un espectro de ruido específico:
```bash
make run M=8 T=15 MIN=2 MAX=6 STEP=1

```


*Nota: En todos los casos estadísticos, la ejecución genera de manera automática un archivo estructurado `.csv` dentro de `results/csv/` con métricas de tiempos (codificación/decodificación en microsegundos), bloques procesados, BER, BER sin codificar y CWER.*

---

### 2️⃣ Posibilidades en Modo Cualitativo (Procesamiento de Archivos)

Se activa inmediatamente al asignar una ruta válida a la variable `FILE`. El programa toma el archivo binario completo, lo convierte a un flujo continuo de bits, fragmenta los datos en bloques de información $k$, añade la paridad sistemática y los somete al canal seleccionado empleando el valor de `MIN` o `PROB` como punto crítico de inyección de ruido.

#### Variante A: Evaluación sobre canal AWGN (Ruido Gaussiano)

Se calcula la atenuación de la señal física simulando una modulación BPSK.

* **Procesar una imagen a 4 dB de relación señal-ruido:**
```bash
make run FILE=img.bmp MIN=4

```


* **Procesar un binario con alta tolerancia de ruido (1 dB):**
```bash
make run FILE=documento.pdf M=6 T=5 MIN=1

```



#### Variante B: Evaluación sobre canal BSC (Canal Simétrico Binario)

Modifica el comportamiento del canal simulando una probabilidad puramente estadística de inversión de bits (*bit-flipping*).

* **Inyección de un 5% de error por bit (BER constante) sobre un archivo:**
```bash
make run FILE=img.bmp CHAN=bsc PROB=0.05

```


* **Evaluación extrema con un 10% de tasa de error intrínseca:**
```bash
make run FILE=datos.dat M=8 T=20 CHAN=bsc PROB=0.10

```


*Al finalizar el flujo de archivos, el simulador vuelca dos copias en `results/`: una versión distorsionada sin codificar (`uncoded_...`) y la versión recuperada tras pasar por el decodificador (`out_...`), permitiendo comparar visualmente la eficiencia del código empleado.*

---

## 📈 Formato de Salida de los Resultados (CSV)

Los reportes generados de forma automática registran la siguiente cabecera estándar para facilitar su posterior análisis o representación gráfica mediante herramientas externas (como scripts de Python/Matplotlib):

```csv
m,t,n,k,ebno_db,ber,ber_uncoded,cwer,codewords,avg_enc_us,avg_dec_us

```

* **ber:** Bit Error Rate (Tasa de error de bit con codificación BCH).
* **ber_uncoded:** Tasa de error de bit experimentada directamente en el canal físico.
* **cwer:** Codeword Error Rate (Tasa de palabras código erróneas incorregibles).
* **avg_enc_us / avg_dec_us:** Tiempo medio de procesamiento en microsegundos por bloque.

```
