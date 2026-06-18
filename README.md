# Simulador de Códigos BCH para el Análisis de Rendimiento en Canales con Ruido

## 📋 Información del Proyecto
* **Contexto:** Trabajo de Fin de Grado (TFG)
* **Titulación:** Grado en Ingeniería de Computadores
* **Centro:** Escuela Técnica Superior de Ingenierías Informática y de Telecomunicación (ETSIIT)
* **Institución:** Universidad de Granada (UGR)

---

## 📖 Descripción General
Este proyecto consiste en un simulador para códigos **Bose-Chaudhuri-Hocquenghem (BCH)** diseñado para evaluar la tasa de error en entornos de transmisión digital. El software permite realizar simulaciones estadísticas mediante el método de Montecarlo y procesar archivos reales (como imágenes) introduciendo ruido controlado para analizar la decodificación sistemática y la reconstrucción de los datos.

### Detalles de la Implementación
* **Campos de Galois Configurables:** Soporte para cuerpos dinámicos $GF(2^m)$ con parámetros de diseño en el rango de $3 \leq m \leq 15$.
* **Decodificación:** Implementación del algoritmo algebraico de **Berlekamp-Massey** (para hallar el polinomio localizador de errores) y la **Búsqueda de Chien** (para la localización de raíces y corrección de errores).
* **Paralelismo:** Uso de directivas `OpenMP` para distribuir la carga de trabajo entre los núcleos de la CPU durante las simulaciones estadísticas.
* **Estructura de Datos (`PackedBit`):** Estructura de empaquetado binario que agrupa los bits en bloques `uint64_t`, lo que permite agilizar las operaciones de división polinómica mediante desplazamientos de bits (`XOR Shifted`).

---

## 🛠️ Requisitos del Sistema y Dependencias

Para compilar y ejecutar este simulador se requiere un entorno que cumpla con los siguientes estándares:

1. **Compilador:** GCC (`g++`) compatible con el estándar **C++17**.
   * *Justificación de C++17:* El simulador utiliza la librería `<filesystem>`.
2. **Soporte OpenMP:** La biblioteca y los flags de compilación de `OpenMP` deben estar instalados en el sistema.
3. **Entorno de Ejecución:** Entornos como Linux o el Subsistema de Windows para Linux (WSL).

---

## 📂 Estructura del Repositorio

El código está organizado de la siguiente manera:

```text
.
├── Makefile                     # Automatización de la compilación y ejecución
├── include/                     # Archivos de cabecera (.h)
│   ├── BCH_Codec.h              # Interfaz del codificador/decodificador BCH
│   ├── BCH_Simulator.h          # Configuración del motor de simulación
│   ├── Channel.h                # Definición de los modelos de canal
│   ├── GaloisField.h            # Estructura del Campo de Galois GF(2^m)
│   ├── PackedBit.h              # Estructura de empaquetado binario
│   └── Polynomial.h             # Aritmética polinómica en campos finitos
├── src/                         # Código fuente (.cpp)
│   ├── main.cpp                 # Punto de entrada y parseo de argumentos CLI
│   └── core/                    # Núcleo algorítmico del simulador
│       ├── BCH_Codec.cpp        # Implementación de codificación/decodificación
│       ├── BCH_Simulator.cpp    # Control de flujos (Montecarlo y Archivos)
│       ├── Channel.cpp          # Modelado matemático de canales (AWGN, BSC)
│       ├── GaloisField.cpp      # Tablas log/antilog y operaciones en GF
│       └── Polynomial.cpp       # Evaluación (Horner) y aritmética de polinomios
├── scripts/                     # Herramientas auxiliares
│   └── generador_primitivo.cpp  # Buscador de polinomios primitivos
└── results/                     # Directorio de salidas
    ├── csv/                     # Reportes estadísticos exportados
    └── (archivos modificados)   # Archivos corregidos y archivos con ruido

```

---

## 🚀 Guía de Compilación

El proyecto cuenta con un archivo `Makefile` para la compilación y ejecución. El entorno genera automáticamente los directorios de salida necesarios al compilar.

* **Compilar el proyecto:**
```bash
make

```


*(Genera el ejecutable `bch_sim` aplicando optimizaciones estándar `-O3 -march=native -fopenmp`)*.
* **Limpiar el entorno:**
```bash
make clean

```


*(Elimina los archivos objeto de la carpeta `build/` y el binario ejecutable)*.
* **Menú de ayuda:**
```bash
make help

```



---

## 🕹️ Manual de Ejecución (`make run`)

El comando `make run` permite iniciar las simulaciones. Dependiendo de los argumentos indicados, el programa selecciona el flujo correspondiente.

### 📑 Parámetros Configurables

Es posible modificar el comportamiento de la ejecución introduciendo variables en la línea de comandos:

| Variable | Descripción | Valor por Defecto | Rangos / Opciones |
| --- | --- | --- | --- |
| `M` | Grado del cuerpo matemático $GF(2^M)$ | `7` | $3 \leq M \leq 15$ |
| `T` | Capacidad de corrección del código (bits) | `10` | $1 \leq 2t < 2^M-1$ |
| `MIN` | Relación señal-ruido inicial ($E_b/N_0$) en dB | `0` | Valores reales (ej. `4.5`) |
| `MAX` | Relación señal-ruido límite ($E_b/N_0$) en dB | `10` | Valores reales |
| `STEP` | Incremento del barrido de ruido | `1` | Valores reales positivos |
| `FILE` | Ruta del archivo para procesar | *Vacío* (Modo Estadístico) | Ruta a cualquier extensión |
| `CHAN` | Modelo de canal a instanciar (solo con `FILE`) | `awgn` | `awgn` o `bsc` |
| `PROB` | Probabilidad de error por bit (solo con `CHAN=bsc`) | `0.01` | Rango $[0.0, 1.0]$ |

---

### 1️⃣ Modo Estadístico (Montecarlo)

Este modo se activa cuando el argumento `FILE` se deja en blanco o se omite. Utiliza el canal AWGN para simular la degradación de la señal y calcular las tasas de error correspondientes.

* **Ejecución con valores por defecto (BCH(127,57) con $t=10$ de 0 a 10 dB):**
```bash
make run

```


* **Personalización del código y del rango de simulación:**
Ejemplo para un código BCH(15,7) con capacidad de corregir 2 bits, evaluando desde 0 dB hasta 8 dB con incrementos de 0.5 dB:
```bash
make run M=4 T=2 MIN=0 MAX=8 STEP=0.5

```


* **Simulación con otros bloques:**
```bash
make run M=8 T=15 MIN=2 MAX=6 STEP=1

```


*Nota: Al finalizar, la ejecución genera un archivo `.csv` dentro de `results/csv/` con las métricas de tiempos (codificación/decodificación en microsegundos), bloques procesados, BER, BER sin codificar y CWER.*

---

### 2️⃣ Modo de Procesamiento de Archivos

Se activa al asignar una ruta a la variable `FILE`. El programa lee el archivo como un flujo de bits, fragmenta los datos en bloques de información $k$, añade la paridad sistemática e introduce ruido empleando los valores definidos en `MIN` o `PROB`.

#### Opción A: Canal AWGN (Ruido Gaussiano)

Simula la atenuación de la señal empleando una modulación BPSK teórica.

* **Procesar un archivo a 4 dB de relación señal-ruido:**
```bash
make run FILE=img.bmp MIN=4

```


* **Procesar un archivo aplicando otros parámetros de bloque:**
```bash
make run FILE=documento.pdf M=6 T=5 MIN=2

```



#### Opción B: Canal BSC (Canal Simétrico Binario)

Aplica una probabilidad puramente estadística de inversión de bits (*bit-flipping*).

* **Inyección de un 5% de error por bit sobre un archivo:**
```bash
make run FILE=img.bmp CHAN=bsc PROB=0.05

```


* **Evaluación con un 10% de tasa de error:**
```bash
make run FILE=datos.dat M=8 T=20 CHAN=bsc PROB=0.10

```


*El simulador genera dos copias en la carpeta `results/`: una versión con el ruido del canal sin codificar (`uncoded_...`) y la versión recuperada tras pasar por el decodificador (`out_...`), permitiendo analizar la eficacia del código.*

---

## 📈 Formato de los Resultados (CSV)

Los reportes estadísticos guardan los datos con la siguiente estructura de cabecera para facilitar su lectura desde herramientas externas (como scripts de filtrado o graficado):

```csv
m,t,n,k,ebno_db,ber,ber_uncoded,cwer,codewords,avg_enc_us,avg_dec_us

```

* **ber:** Tasa de error de bit con codificación BCH.
* **ber_uncoded:** Tasa de error de bit medida directamente en el canal.
* **cwer:** Tasa de palabras código erróneas (bloques que no se han podido corregir).
* **avg_enc_us / avg_dec_us:** Tiempo medio de procesamiento en microsegundos por bloque.

