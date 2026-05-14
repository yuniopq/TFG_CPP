#!/usr/bin/env python3
"""
MAIN BLOCK: Fix m and sweep t

Goal: See how BER, CWER, and time change as redundancy increases within the same field.

Eb/N0 range: 0-12 dB (1 dB step)
"""

import subprocess
import os
import sys
import time
from pathlib import Path

# =============================================================================
# CONFIGURATION: MAIN SCENARIOS
# =============================================================================

# =============================================================================
# CONFIGURATION: MAIN SCENARIOS (ESCALA POR DÍGITOS)
# =============================================================================

ESCENARIOS = [
    # 1 DÍGITO: m=3 (n=7)
    {"m": 3, "t": 1,   "label": "m=3, t=1   (n=7)"},
    {"m": 3, "t": 2,   "label": "m=3, t=2   (n=7)"},
    
    # 2 DÍGITOS: m=4 (n=15)
    {"m": 4, "t": 1,   "label": "m=4, t=1   (n=15)"},
    {"m": 4, "t": 2,   "label": "m=4, t=2   (n=15)"},
    {"m": 4, "t": 3,   "label": "m=4, t=3   (n=15)"},
    {"m": 4, "t": 4,   "label": "m=4, t=4   (n=15)"},

    
    # 3 DÍGITOS: m=7 (n=127)
    {"m": 7, "t": 2,   "label": "m=7, t=2   (n=127)"},
    {"m": 7, "t": 5,   "label": "m=7, t=5   (n=127)"},
    {"m": 7, "t": 10,  "label": "m=7, t=10  (n=127)"},
    {"m": 7, "t": 20,  "label": "m=7, t=20  (n=127)"},

    
    # 4 DÍGITOS: m=11 (n=2047)
    {"m": 11, "t": 15, "label": "m=11, t=15 (n=2047)"},
    {"m": 11, "t": 30, "label": "m=11, t=30 (n=2047)"},
    {"m": 11, "t": 60, "label": "m=11, t=60 (n=2047)"},
    {"m": 11, "t": 120, "label": "m=11, t=120 (n=2047)"},

    
    # 5 DÍGITOS: m=15 (n=32767)
    {"m": 15, "t": 50,  "label": "m=15, t=50  (n=32767)"},
    {"m": 15, "t": 100, "label": "m=15, t=100 (n=32767)"},
    {"m": 15, "t": 200, "label": "m=15, t=200 (n=32767)"},
    {"m": 15, "t": 300, "label": "m=15, t=300 (n=32767)"},
]
# Eb/N0 sweep configuration
SNR_MIN  = 0.0
SNR_MAX  = 12.0
SNR_STEP = 1.0

# =============================================================================
# MAIN BLOCK EXECUTION
# =============================================================================

def run_main_block():
    # Asegurar que el binario existe
    print("=" * 80)
    print("MAIN BLOCK: FIX m AND SWEEP t")
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
    print(f"   - Goal: Analyze BER, CWER, and time with increasing redundancy")
    
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
            
            # Look for generated file
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
        print(f"{status} | {label:45s} | {elapsed:>8s}")
    
    print("\n" + "=" * 80)
    print("📁 CSV files generated in: results/csv/")
    print("📈 To plot the results, run:")
    print("   python3 scripts/plotter.py")
    print("=" * 80)

if __name__ == "__main__":
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    run_main_block()
