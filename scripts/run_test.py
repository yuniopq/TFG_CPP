import subprocess
import os
import sys

EXE_PATH = "./build/bch_test"
# Lista de valores de m para el análisis de escalabilidad
M_VALUES = [7, 9, 11, 13, 15]
TARGET_EBNO_DB = 5.0

def run_scalability_test():
    os.makedirs("results/csv", exist_ok=True)

    if not os.path.exists(EXE_PATH):
        print(f"❌ No se encontró el ejecutable: {EXE_PATH}. Compila primero con `make -j`.")
        sys.exit(1)

    print(f"🔬 Iniciando test de escalabilidad a Eb/N0 = {TARGET_EBNO_DB} dB")
    
    for m in M_VALUES:
        # Ajustamos t proporcionalmente para que no tarde una eternidad en m=15
        # pero que sea un reto real para el código.
        t = 10 if m < 10 else 50 if m < 13 else 100
        
        print(f"  > Probando m={m} (n={2**m-1}), t={t}...", end=" ", flush=True)
        
        # Ejecutamos un único punto para recoger tiempos medios de codificación/decodificación.
        cmd = [EXE_PATH, str(m), str(t), str(TARGET_EBNO_DB), str(TARGET_EBNO_DB), "1"]
        
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, check=True)
            if result.stdout:
                last_line = result.stdout.strip().splitlines()[-1]
                print(f"✅ {last_line}")
            else:
                print("✅")
        except Exception as e:
            print(f"❌ Error en m={m}: {e}")
            if isinstance(e, subprocess.CalledProcessError) and e.stderr:
                print(e.stderr)

if __name__ == "__main__":
    run_scalability_test()