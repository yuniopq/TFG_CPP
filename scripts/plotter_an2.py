import os
import matplotlib.pyplot as plt
import pandas as pd

# Rutas del proyecto
input_dir = os.path.join('results', 'csv_an2')
output_dir = 'results'

# Agrupaciones lógicas simétricas con m=5, m=7 y m=11
escenarios = {
    0: {
        'titulo': 'Alta Tasa ($R \\approx 0.82$)',
        'ficheros': [('BCH_m5_t1.csv', '$m=5, t=1$'), ('BCH_m7_t3.csv', '$m=7, t=3$'), ('BCH_m11_t35.csv', '$m=11, t=35$')]
    },
    1: {
        'titulo': 'Media Tasa ($R \\approx 0.50$)',
        'ficheros': [('BCH_m5_t3.csv', '$m=5, t=3$'), ('BCH_m7_t9.csv', '$m=7, t=9$'), ('BCH_m11_t93.csv', '$m=11, t=93$')]
    },
    2: {
        'titulo': 'Baja Tasa ($R \\approx 0.34$)',
        'ficheros': [('BCH_m5_t5.csv', '$m=5, t=5$'), ('BCH_m7_t14.csv', '$m=7, t=14$'), ('BCH_m11_t124.csv', '$m=11, t=124$')]
    }
}

fig, axs = plt.subplots(1, 3, figsize=(18, 5.5), sharey=True)
colores = ['#1f77b4', '#ff770f', '#2ca02c']
marcadores = ['o', 's', '^']

print("[+] Generando gráfica de tasas similares con la tríada simétrica (m=5, 7, 11)...")

for idx in range(3):
    ax = axs[idx]
    config = escenarios[idx]
    ax.set_title(config['titulo'], fontsize=12, fontweight='bold')
    
    for f_idx, (nombre_fichero, etiqueta) in enumerate(config['ficheros']):
        ruta_completa = os.path.join(input_dir, nombre_fichero)
        
        if not os.path.exists(ruta_completa):
            print(f"[-] Fichero ausente: {nombre_fichero}. Recuerda moverlo a results/csv_an2/")
            continue
            
        df = pd.read_csv(ruta_completa)
        
        # Pintar la curva de BER
        ax.semilogy(df['ebno_db'], df['ber'], color=colores[f_idx],
                    linestyle='-', marker=marcadores[f_idx],
                    markersize=5, label=etiqueta)
        
    ax.set_xlabel('$E_b/N_0$ (dB)', fontsize=11)
    if idx == 0:
        ax.set_ylabel('Bit Error Rate (BER)', fontsize=12)
    ax.grid(True, which='both', linestyle='--', alpha=0.5)
    ax.legend(loc='lower left', fontsize=10)
    ax.set_xlim([0, 10])
    ax.set_ylim([1e-6, 1.2])

plt.tight_layout()
os.makedirs(output_dir, exist_ok=True)

plt.savefig(os.path.join(output_dir, 'grafica_tasas_similares.png'), dpi=300)
plt.savefig(os.path.join(output_dir, 'grafica_tasas_similares.pdf'), bbox_inches='tight')
print("[+] ¡Gráfica de tasas similares generada con éxito en la carpeta 'results/'!")