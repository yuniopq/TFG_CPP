# TFG: Implementación y Análisis de Códigos BCH (ECC)

Este proyecto forma parte de mi Trabajo de Fin de Grado (TFG) y se centra en la implementación desde cero de códigos de corrección de errores **BCH (Bose-Chaudhuri-Hocquenghem)** utilizando C++ para el motor de cómputo y Python para la visualización de resultados.

**Autor:** ZhiXiang Zhou Wang  

## 🎯 Objetivos
* Implementar la aritmética de campos finitos $GF(2^m)$.
* Desarrollar un codificador BCH basado en registros de desplazamiento (LFSR).
* Implementar el decodificador completo (Síndromes, Berlekamp-Massey y Chien Search).
* Analizar la eficiencia del código mediante curvas de BER (Bit Error Rate).

## 🛠️ Parámetros del Código
Actualmente el sistema está configurado para:
* **m:** 8 (Campo $GF(256)$)
* **n:** [RELLENAR, ej. 255] (Longitud de bloque)
* **k:** [RELLENAR] (Bits de información)
* **t:** [RELLENAR] (Capacidad de corrección)

## 📁 Estructura del Proyecto
* `/src`: Código fuente en C++.
* `/include`: Cabeceras (.h).
* `/scripts`: Scripts de Python para gráficas.
* `/results`: Datos generados por las simulaciones.

## 🚀 Instrucciones
### Requisitos
* Compilador C++ (GCC/Clang) con soporte para C++17.
* Python 3.x (para gráficas).

### Compilación y Ejecución
```bash
mkdir build && cd build
cmake ..
make
./BCH_Simulator
```

### Visualización
```bash
python3 scripts/plotter.py
```
