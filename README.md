# TFG_CPP: Implementación y análisis de códigos BCH en canales ruidosos

Este proyecto corresponde a un Trabajo de Fin de Grado centrado en la **implementación desde cero de códigos BCH** y en su **análisis de rendimiento sobre canales ruidosos**. El objetivo es estudiar cómo cambia la tasa de error cuando la información se transmite por un canal AWGN con decisión dura, comparando el caso codificado frente al no codificado.

## Qué incluye el proyecto

- Implementación de aritmética en campos finitos `GF(2^m)`.
- Codificación BCH.
- Decodificación BCH.
- Simulación de transmisión sobre canal ruidoso.
- Medición de `BER`, `FER` y tiempos medios de codificación/decodificación.
- Exportación de resultados a CSV para su posterior análisis.

## Estructura

- `src/main.cpp`: programa principal de simulación.
- `src/core/`: implementación de `BCH_Codec`, `GaloisField`, `Polynomial` y `Channel`.
- `include/`: cabeceras públicas.
- `results/csv/`: resultados generados por las simulaciones.
- `scripts/`: utilidades externas para procesar o graficar resultados.

## Requisitos

- Compilador C++ con soporte para C++17.
- `make`.
- Python 3.x, solo si vas a usar los scripts de análisis o gráficas.

## Compilación

Desde la raíz del repositorio:

```bash
make -j
```

Esto genera el ejecutable en `build/bch_test`.

## Ejecución

Formato general:

```bash
./build/bch_test <m> <t> [ebno_min] [ebno_max] [step]
```

Ejemplo:

```bash
./build/bch_test 4 2
```

También puedes usar los objetivos del `Makefile`:

```bash
make run-m4
```

## Salida

Durante la ejecución se muestra una tabla con `Eb/N0`, `BER`, `FER` y número de tramas simuladas. Además, se genera un CSV en `results/csv/` con columnas como:

- `m`
- `t`
- `n`
- `k`
- `ebno_db`
- `ber`
- `fer`
- `frames`
- `avg_enc_us`
- `avg_dec_us`

## Notas importantes

- El análisis principal del TFG es la transmisión en **canales ruidosos**, no solo la implementación del código BCH.
- La simulación compara el rendimiento con y sin codificación.
- El proyecto está pensado para facilitar el estudio experimental de curvas de error, no solo para validar el funcionamiento matemático del código.

## Autor

ZhiXiang Zhou Wang
