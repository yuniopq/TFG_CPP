#!/usr/bin/env python3
"""
MAIN BLOCK: Fix m and sweep t

Goal: See how BER, CWER, and time change as redundancy increases within 
the same field, using a percentage-based approach for t.

Eb/N0 range: 0-12 dB (1 dB step)
"""

import subprocess
import os
import sys
import time

# =============================================================================
# CONFIGURATION: DYNAMIC SCENARIOS (20-40-60-80-100%)
# =============================================================================
M_VALUES = [3, 4, 7, 11, 15]
PORCENTAJES = [20, 40, 60, 80, 100]

# Tope térmico/temporal: Evita que m=15 bloquee el portátil durante días.
# Ajusta este valor si ves que los tiempos son demasiado altos o bajos.
MAX_T_CAP = 351 

def generar_escenarios_porcentuales(m_valores, porcentajes, max_t):
    escenarios = []
    
    for m in m_valores:
        n = (2**m) - 1
        t_max = (2**(m-1)) - 1
        t_vistos = set() 
        
        for p in porcentajes:
            # 1. Calcular el valor según el porcentaje
            t_calc = max(1, round(t_max * (p / 100.0)))
            
            # 2. Aplicar el límite duro (cap) para proteger la CPU
            if t_calc > max_t:
                t_calc = max_t
                
            # 3. Forzar a número impar para evitar duplicados ciclotómicos
            if t_calc % 2 == 0:
                # Si estamos en el cap máximo y es par, bajamos 1 en lugar de subir
                # para no saltarnos el límite de seguridad.
                if t_calc == max_t:
                    t_calc -= 1
                else:
                    t_calc += 1
                
            # 4. Asegurar no superar la teoría matemática
            t_calc = min(t_calc, t_max)
            
            # 5. Evitar colisiones en bloques pequeños
            if t_calc not in t_vistos:
                t_vistos.add(t_calc)
                label = f"m={m:2d}, t={t_calc:<5d} ({p:>3}% t_max, n={n})"
                escenarios.append({
                    "m": m,
                    "t": t_calc,
                    "label": label
                })
                
    return escenarios

# Generamos la lista de escenarios antes de iniciar
# ESCENARIOS = generar_escenarios_porcentuales(M_VALUES, PORCENTAJES, MAX_T_CAP)
ESCENARIOS = [
    # --- Tasa Alta (R ~ 80%) ---
    {"m": 4,  "t": 1,   "label": "m=4,  t=1   (R=73%)"},
    {"m": 7,  "t": 3,   "label": "m=7,  t=3   (R=83%)"},
    {"m": 11, "t": 35,  "label": "m=11, t=35  (R=81%)"},

    # --- Tasa Media (R ~ 50%) ---
    {"m": 4,  "t": 2,   "label": "m=4,  t=2   (R=46%)"},
    {"m": 7,  "t": 9,   "label": "m=7,  t=9   (R=50%)"},
    {"m": 11, "t": 93,  "label": "m=11, t=93  (R=50%)"},

    # --- Tasa Baja (R ~ 33%) ---
    {"m": 4,  "t": 3,   "label": "m=4,  t=3   (R=33%)"},
    {"m": 7,  "t": 14,  "label": "m=7,  t=14  (R=33%)"},
    {"m": 11, "t": 124, "label": "m=11, t=124 (R=33%)"},
]
# Eb/N0 sweep configuration
SNR_MIN  = 0.0
SNR_MAX  = 12.0
SNR_STEP = 1.0

# =============================================================================
# MAIN BLOCK EXECUTION
# =============================================================================

def run_main_block():
    print("=" * 80)
    print("MAIN BLOCK: FIX m AND SWEEP t (PERCENTAGE SCALING)")
    print("=" * 80)
    
    print("\n🔨 Building project...")
    os.chdir("..")  # Go to the project root
    result = subprocess.run(["make", "all"], capture_output=True, text=True)
    
    if result.returncode != 0:
        print("❌ Build error:")
        print(result.stderr)
        sys.exit(1)
    
    print("✅ Build successful")
    
    if not os.path.exists("results/csv"):
        os.makedirs("results/csv")
    
    # Configuration summary
    print(f"\n📊 Configuration:")
    print(f"   - Eb/N0 range: {SNR_MIN} to {SNR_MAX} dB ({SNR_STEP} dB step)")
    print(f"   - Total scenarios: {len(ESCENARIOS)}")
    print(f"   - T-Max Cap: {MAX_T_CAP} (Safety limit for simulation times)")
    
    print("\n" + "=" * 80)
    print("RUNNING SCENARIOS")
    print("=" * 80)
    
    # Ejecutar cada escenario
    results_summary = []
    start_time = time.time()
    
    for idx, esc in enumerate(ESCENARIOS, 1):
        m, t = esc["m"], esc["t"]
        label = esc["label"]
        
        print(f"\n[{idx}/{len(ESCENARIOS)}] Running: {label}")
        
        # Construir comando (El binario espera: m t ebno_min ebno_max step)
        comando = [
            "./bch_sim", 
            str(m), str(t), 
            str(SNR_MIN), str(SNR_MAX), str(SNR_STEP)
        ]
        
        sim_start = time.time()
        try:
            # check=True lanzará CalledProcessError si el binario de C++ falla (return != 0)
            result = subprocess.run(comando, check=True, capture_output=True, text=True)
            sim_time = time.time() - sim_start
            
            # Comprobar si el archivo fue generado
            csv_file = f"results/csv/BCH_m{m}_t{t}.csv"
            if os.path.exists(csv_file):
                file_size = os.path.getsize(csv_file)
                print(f"   ✅ Completed in {sim_time:.2f}s → {csv_file} ({file_size} bytes)")
                results_summary.append((label, "✅ OK", f"{sim_time:.2f}s"))
            else:
                print(f"   ⚠️  No CSV generated after {sim_time:.2f}s")
                results_summary.append((label, "⚠️  Sin CSV", f"{sim_time:.2f}s"))
                
        except subprocess.CalledProcessError as e:
            print(f"   ❌ Error during execution")
            print(f"      stderr: {e.stderr}")
            results_summary.append((label, "❌ ERROR", "---"))
    
    total_time = time.time() - start_time
    
    # Final summary
    print("\n" + "=" * 80)
    print("RESULT SUMMARY")
    print("=" * 80)
    print(f"\nTotal time: {total_time:.2f}s ({total_time/60:.1f} minutes)\n")
    
    for label, status, elapsed in results_summary:
        print(f"{status} | {label:55s} | {elapsed:>8s}")
    
    print("\n" + "=" * 80)
    print("📁 CSV files generated in: results/csv/")
    print("📈 To plot the results, run:")
    print("   python3 scripts/plotter.py")
    print("=" * 80)

if __name__ == "__main__":
    # Aseguramos que el script se ejecuta en su propio directorio
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    run_main_block()