import os
import matplotlib.pyplot as plt
import pandas as pd

# Definir rutas base para que funcione desde la raíz del proyecto
input_dir = 'results/csv'
output_dir = 'results'

selected_t = [1, 3, 7, 15]

# Crear figura panorámica (BER a la izquierda, CWER a la derecha)
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5.5))

colors = {1: '#1f77b4', 3: '#ff770f', 7: '#d62728', 15: '#8c564b'}
markers = {1: 'o', 3: 's', 7: '^', 15: 'd'}

# 1. Cargar datos de referencia sin codificar usando la ruta correcta
uncoded_path = os.path.join(input_dir, 'BCH_m7_t1.csv')
if os.path.exists(uncoded_path):
    df_uncoded = pd.read_csv(uncoded_path)
    ax1.semilogy(df_uncoded['ebno_db'], df_uncoded['ber_uncoded'], 'k--', label='Sin codificar (Uncoded)', linewidth=2)

# 2. Iterar y graficar los códecs seleccionados
for t in selected_t:
    file_path = os.path.join(input_dir, f'BCH_m7_t{t}.csv')
    if not os.path.exists(file_path):
        print(f"[!] Archivo no encontrado: {file_path}")
        continue
        
    df = pd.read_csv(file_path)
    
    # Panel Izquierdo: BER
    ax1.semilogy(df['ebno_db'], df['ber'], color=colors[t], linestyle='-', marker=markers[t], markersize=6, label=f'BCH $t={t}$')
    
    # Panel Derecho: CWER
    ax2.semilogy(df['ebno_db'], df['cwer'], color=colors[t], linestyle='-', marker=markers[t], markersize=6, label=f'BCH $t={t}$')

# Configuración Panel BER (Izquierda)
ax1.set_xlabel('$E_b/N_0$ (dB)', fontsize=12)
ax1.set_ylabel('Bit Error Rate (BER)', fontsize=12)
ax1.set_title('Tasa de Error de Bit (BER) para $m=7$', fontsize=13, fontweight='bold')
ax1.grid(True, which='both', linestyle='--', alpha=0.5)
ax1.legend(loc='lower left', fontsize=11)
ax1.set_ylim([1e-6, 1.2])
ax1.set_xlim([0, 10])

# Configuración Panel CWER (Derecha)
ax2.set_xlabel('$E_b/N_0$ (dB)', fontsize=12)
ax2.set_ylabel('Codeword Error Rate (CWER)', fontsize=12)
ax2.set_title('Tasa de Error de Palabra (CWER) para $m=7$', fontsize=13, fontweight='bold')
ax2.grid(True, which='both', linestyle='--', alpha=0.5)
ax2.legend(loc='lower left', fontsize=11)
ax2.set_ylim([1e-6, 1.2])
ax2.set_xlim([0, 10])

plt.tight_layout()

# Guardar los archivos finales en la carpeta results/
plt.savefig(os.path.join(output_dir, 'grafica_bch_m7_limpia.png'), dpi=300)
plt.savefig(os.path.join(output_dir, 'grafica_bch_m7_limpia.pdf'), bbox_inches='tight')

print("¡Gráfica panorámica guardada con éxito en la carpeta 'results/'!")