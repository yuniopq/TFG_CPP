import pandas as pd
import matplotlib.pyplot as plt
import os
import glob

def generar_graficas_tfg():
    path = 'results/csv/'
    all_files = glob.glob(os.path.join(path, "BCH_m*_t*.csv"))
    
    if not all_files:
        print("⚠️ No hay archivos CSV en results/csv/")
        return

    df = pd.concat([pd.read_csv(f) for f in all_files], ignore_index=True)
    os.makedirs("results/plot/BER", exist_ok=True)
    os.makedirs("results/plot/Tiempos", exist_ok=True)

    m_values = sorted(df['m'].unique())

    print("📊 Generando gráficas para el TFG...")

    for m in m_values:
        df_m = df[df['m'] == m]
        t_values = sorted(df_m['t'].unique())
        n = df_m['n'].iloc[0]

        # ---------------------------------------------------------
        # GRÁFICA 1: Curvas BER (Rendimiento del Canal)
        # ---------------------------------------------------------
        plt.figure(figsize=(10, 6))
        
        # Curva Uncoded (referencia)
        df_uncoded = df_m[df_m['t'] == t_values[0]].sort_values('ebno_db')
        plt.plot(df_uncoded['ebno_db'], df_uncoded['ber_uncoded'], 
                 marker='X', color='black', linestyle='--', linewidth=2, label='BER Sin Codificar')

        # Curvas para cada t
        for t in t_values:
            df_t = df_m[df_m['t'] == t].sort_values('ebno_db')
            k = df_t['k'].iloc[0]
            redundancia = ((n - k) / n) * 100
            
            plt.plot(df_t['ebno_db'], df_t['ber'], marker='o', 
                     label=f't={t} (k={k}, Red: {redundancia:.1f}%)')

        plt.yscale('log')
        plt.ylim(bottom=1e-8, top=1) # Ajustar según hasta dónde llegue la simulación
        plt.title(f"Rendimiento BER para BCH en GF(2^{m}) - Longitud n={n}", fontsize=14)
        plt.xlabel("Eb/N0 (dB)", fontsize=12)
        plt.ylabel("Tasa de Error de Bit (BER)", fontsize=12)
        plt.grid(True, which="both", ls="--", alpha=0.5)
        plt.legend()
        
        plt.savefig(f"results/plot/BER/Curva_BER_m{m}.png", dpi=300, bbox_inches='tight')
        plt.close()

        # ---------------------------------------------------------
        # GRÁFICA 2: Tiempos de Decodificación (Rendimiento Software)
        # ---------------------------------------------------------
        # Tomamos el tiempo promedio del punto con mejor Eb/N0 (donde no hay ruido)
        # para aislar el tiempo base de decodificación.
        tiempos_dec = []
        for t in t_values:
            df_t = df_m[df_m['t'] == t]
            tiempo_base = df_t[df_t['ebno_db'] == df_t['ebno_db'].max()]['avg_dec_us'].iloc[0]
            tiempos_dec.append(tiempo_base)

        plt.figure(figsize=(8, 5))
        plt.plot(t_values, tiempos_dec, marker='s', color='red', linestyle='-', linewidth=2)
        
        plt.title(f"Complejidad de Decodificación para m={m} (n={n})", fontsize=14)
        plt.xlabel("Capacidad de Corrección (t)", fontsize=12)
        plt.ylabel("Tiempo Promedio de Decodificación (µs)", fontsize=12)
        plt.grid(True, ls="--", alpha=0.5)
        
        plt.savefig(f"results/plot/Tiempos/Tiempos_Dec_m{m}.png", dpi=300, bbox_inches='tight')
        plt.close()

    print("✅ Todas las gráficas han sido guardadas en results/plot/")

if __name__ == "__main__":
    generar_graficas_tfg()