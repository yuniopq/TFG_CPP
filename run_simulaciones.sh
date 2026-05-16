#!/bin/bash

echo "========================================"
echo "     EJECUCIÓN DE SIMULACIONES BCH      "
echo "========================================"

# Compilar primero
echo "Compilando proyecto..."
make -j all

if [ $? -ne 0 ]; then
    echo "❌ Error al compilar. Corrige los errores primero."
    exit 1
fi

echo "✅ Compilación exitosa."
echo ""

# === ANÁLISIS 1: Mismo m, distintos t ===

echo "🚀 Iniciando simulaciones..."

# m=4 (muy rápido)
echo "→ m=4 ..."
./bch_sim 4 1 0 12 1
./bch_sim 4 3 0 12 1
./bch_sim 4 5 0 12 1

# m=7
echo "→ m=7 ..."
./bch_sim 7 3  0 12 1
./bch_sim 7 7  0 12 1
./bch_sim 7 11 0 12 1
./bch_sim 7 15 0 12 1

# m=11
echo "→ m=11 ..."
./bch_sim 11 7  0 12 1
./bch_sim 11 15 0 12 1
./bch_sim 11 25 0 12 1
./bch_sim 11 40 0 12 1

# m=15 (los más pesados)
echo "→ m=15 (puede tardar bastante)..."
./bch_sim 15 10 0 12 1
./bch_sim 15 20 0 12 1
./bch_sim 15 35 0 12 1
./bch_sim 15 50 0 12 1

echo ""
echo "========================================"
echo "¡Todas las simulaciones han finalizado!"
echo "Resultados en: results/csv/"
echo "========================================"
