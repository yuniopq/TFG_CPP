import subprocess
import os

EXE_PATH = "./build/bch_test"
# Lista de valores de m para el análisis de escalabilidad
M_VALUES = [7, 9, 11, 13, 15]
ITERACIONES = 20  # Pocas iteraciones porque m=15 es lento
BER = 0.001       # Ruido constante para comparar tiempos

def run_scalability_test():
    if not os.path.exists("results"):
        os.makedirs("results")

    print(f"🔬 Iniciando test de escalabilidad (BER constante = {BER})")
    
    for m in M_VALUES:
        # Ajustamos t proporcionalmente para que no tarde una eternidad en m=15
        # pero que sea un reto real para el código.
        t = 10 if m < 10 else 50 if m < 13 else 100
        
        print(f"  > Probando m={m} (n={2**m-1}), t={t}...", end=" ", flush=True)
        
        cmd = [EXE_PATH, str(m), str(t), str(ITERACIONES), str(BER)]
        
        try:
            # Ejecutamos y esperamos
            subprocess.run(cmd, capture_output=True, text=True)
            print("✅")
        except Exception as e:
            print(f"❌ Error en m={m}: {e}")

if __name__ == "__main__":
    run_scalability_test()