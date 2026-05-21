import os
import matplotlib.pyplot as plt
import pandas as pd

# Rutas del proyecto
input_dir = os.path.join('results', 'csv_an2')
output_dir = 'results'

# Configuraciones correlacionadas por ratio (Alta, Media, Baja)
config_m = {
    5: [(1, 'Alta ($R\\approx 0.84$)'), (3, 'Media ($R\\approx 0.52$)'), (5, 'Baja ($R\\approx 0.35$)')],
    7: [(3, 'Alta ($R\\approx 0.83$)'), (9, 'Media ($R\\approx 0.50$)'), (14, 'Baja ($R\\approx 0.34$)')],
    11: [(35, 'Alta ($R\\approx 0.81$)'), (93, 'Media ($R\\approx 0.50$)'), (124, 'Baja ($R\\approx 0.33$)')]
}

fig, axs = plt.subplots(1, 3, figsize=(18, 5.5), sharey=True)
colores = ['#1f77b4', '#ff770f', '#2ca02c']
marcadores = ['o', 's', '^']

print("[+] Generando gráfica de sensibilidad de tasa correlacionada...")

for idx, m in enumerate([5, 7, 11]):
    ax = axs[idx]
    ax.set_title(f'Progreso de Tasa en Grado $m={m}$', fontsize=12, fontweight='bold')
    
    # Pintar las tres tasas correlacionadas dentro del mismo m
    for t_idx, (t, etiqueta) in enumerate(config_m[m]):
        nombre_fichero = f'BCH_m{m}_t{t}.csv'
        ruta_completa = os.path.join(input_dir, nombre_fichero)
        
        if not os.path.exists(ruta_completa):
            print(f"[-] Fichero ausente: {nombre_fichero}")
            continue
            
        df = pd.read_csv(ruta_completa)
        
        ax.semilogy(df['ebno_db'], df['ber'], color=colores[t_idx],
                    linestyle='-', marker=marcadores[t_idx],
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

plt.savefig(os.path.join(output_dir, 'grafica_correlacion_tasas.png'), dpi=300)
plt.savefig(os.path.join(output_dir, 'grafica_correlacion_tasas.pdf'), bbox_inches='tight')
print("[+] ¡Gráfica de correlación de tasas generada con éxito!")