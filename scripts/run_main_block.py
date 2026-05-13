#!/usr/bin/env python3
"""
BLOQUE PRINCIPAL: Fijar m y Barrer t

Objetivo: Ver cómo cambia BER, CWER y tiempo cuando aumentas la redundancia dentro del mismo campo.

Configuración:
- m=4 (pequeño):   t = 1, 2, 3, 4        → n=15, redundancia 6.7%-26.7%
- m=7 (medio):     t = 2, 5, 10, 15      → n=127, redundancia 11%-88%
- m=15 (grande):   t = 50, 100, 200, 300 → n=32767, redundancia 0.23%-0.92%

Rango Eb/N0: 0-12 dB (paso 1 dB)
"""

import subprocess
import os
import sys
import time
from pathlib import Path

# =============================================================================
# CONFIGURACIÓN: ESCENARIOS PRINCIPALES
# =============================================================================

ESCENARIOS = [
    # m = 4 (pequeño): n = 15
    {"m": 4, "t": 1,   "label": "m=4, t=1   (6.7% redundancia)"},
    {"m": 4, "t": 2,   "label": "m=4, t=2   (13.3% redundancia)"},
    {"m": 4, "t": 3,   "label": "m=4, t=3   (20% redundancia)"},
    {"m": 4, "t": 4,   "label": "m=4, t=4   (26.7% redundancia)"},
    
    # m = 7 (medio): n = 127
    {"m": 7, "t": 2,   "label": "m=7, t=2   (11% redundancia)"},
    {"m": 7, "t": 5,   "label": "m=7, t=5   (27.6% redundancia)"},
    {"m": 7, "t": 10,  "label": "m=7, t=10  (55% redundancia)"},
    {"m": 7, "t": 15,  "label": "m=7, t=15  (88% redundancia)"},
    
    # m = 15 (grande): n = 32767
    {"m": 15, "t": 50,   "label": "m=15, t=50   (0.23% redundancia)"},
    {"m": 15, "t": 100,  "label": "m=15, t=100  (0.46% redundancia)"},
    {"m": 15, "t": 200,  "label": "m=15, t=200  (0.92% redundancia)"},
    {"m": 15, "t": 300,  "label": "m=15, t=300  (1.37% redundancia)"},
]

# Configuración de la barrida de Eb/N0
SNR_MIN  = 0.0
SNR_MAX  = 12.0
SNR_STEP = 1.0

# =============================================================================
# MAIN BLOCK EXECUTION
# =============================================================================

def run_main_block():
    # Asegurar que el binario existe
    print("=" * 80)
    print("BLOQUE PRINCIPAL: FIJAR m Y BARRER t")
    print("=" * 80)
    
    print("\n🔨 Compilando proyecto...")
    os.chdir("..")  # Ir a la raíz del proyecto
    result = subprocess.run(["make", "all"], capture_output=True, text=True)
    
    if result.returncode != 0:
        print("❌ Error en compilación:")
        print(result.stderr)
        sys.exit(1)
    
    print("✅ Compilación exitosa")
    
    if not os.path.exists("results/csv"):
        os.makedirs("results/csv")
    
    # Resumen de configuración
    print(f"\n📊 Configuración:")
    print(f"   - Rango Eb/N0: {SNR_MIN} a {SNR_MAX} dB (paso {SNR_STEP} dB)")
    print(f"   - Total de escenarios: {len(ESCENARIOS)}")
    print(f"   - Objetivo: Analizar BER, CWER y tiempo con redundancia creciente")
    
    print("\n" + "=" * 80)
    print("EJECUTANDO ESCENARIOS")
    print("=" * 80)
    
    # Ejecutar cada escenario
    results_summary = []
    start_time = time.time()
    
    for idx, esc in enumerate(ESCENARIOS, 1):
        m, t = esc["m"], esc["t"]
        label = esc["label"]
        
        print(f"\n[{idx}/{len(ESCENARIOS)}] Ejecutando: {label}")
        
        # Construir comando
        comando = [
            "./bch_sim", 
            str(m), str(t), 
            str(SNR_MIN), str(SNR_MAX), str(SNR_STEP)
        ]
        
        sim_start = time.time()
        try:
            result = subprocess.run(comando, check=True, capture_output=True, text=True)
            sim_time = time.time() - sim_start
            
            # Buscar archivo generado
            csv_file = f"results/csv/BCH_m{m}_t{t}.csv"
            if os.path.exists(csv_file):
                file_size = os.path.getsize(csv_file)
                print(f"   ✅ Completado en {sim_time:.2f}s → {csv_file} ({file_size} bytes)")
                results_summary.append((label, "✅ OK", f"{sim_time:.2f}s"))
            else:
                print(f"   ⚠️  No se generó archivo CSV después de {sim_time:.2f}s")
                results_summary.append((label, "⚠️  Sin CSV", f"{sim_time:.2f}s"))
                
        except subprocess.CalledProcessError as e:
            print(f"   ❌ Error durante ejecución")
            print(f"      stderr: {e.stderr}")
            results_summary.append((label, "❌ ERROR", "---"))
    
    total_time = time.time() - start_time
    
    # Resumen final
    print("\n" + "=" * 80)
    print("RESUMEN DE RESULTADOS")
    print("=" * 80)
    print(f"\nTiempo total: {total_time:.2f}s ({total_time/60:.1f} minutos)\n")
    
    for label, status, elapsed in results_summary:
        print(f"{status} | {label:45s} | {elapsed:>8s}")
    
    print("\n" + "=" * 80)
    print("📁 Archivos CSV generados en: results/csv/")
    print("📈 Para graficar resultados, ejecuta:")
    print("   python3 scripts/plotter.py")
    print("=" * 80)

if __name__ == "__main__":
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    run_main_block()
