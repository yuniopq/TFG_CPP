import os
import matplotlib.pyplot as plt
import pandas as pd

# Definición de rutas relativas desde tu carpeta raíz ~/TFG_CPP
input_dir = os.path.join('results', 'csv')
output_dir = 'results'

# Mapeo exacto basado en la salida de tu comando ls
configuraciones = {
    4: [1, 3, 5],
    11: [5, 11, 21, 31, 45],
    15: [7, 15, 25, 41, 61]
}

# Creamos una estructura panorámica de 3 paneles horizontales compartiendo el eje Y
fig, axs = plt.subplots(1, 3, figsize=(18, 5.5), sharey=True)

# Paletas extendidas a 5 combinaciones para cubrir todas tus curvas por panel
colores = ['#1f77b4', '#ff770f', '#2ca02c', '#d62728', '#9467bd']
marcadores = ['o', 's', '^', 'd', 'v']

print("[+] Iniciando el escaneo de datos para la gráfica de generalización...")

for idx, m in enumerate([4, 11, 15]):
    ax = axs[idx]
    lista_t = configuraciones[m]
    
    # 1. Intentar pintar la referencia 'Sin codificar' (Uncoded) usando el primer CSV disponible del grupo
    archivo_base = os.path.join(input_dir, f'BCH_m{m}_t{lista_t[0]}.csv')
    if os.path.exists(archivo_base):
        df_base = pd.read_csv(archivo_base)
        if 'ber_uncoded' in df_base.columns:
            ax.semilogy(df_base['ebno_db'], df_base['ber_uncoded'], 'k--', label='Sin codificar', linewidth=1.5)
    
    # 2. Iterar sobre tus archivos reales de 't'
    for t_idx, t in enumerate(lista_t):
        nombre_archivo = f'BCH_m{m}_t{t}.csv'
        ruta_completa = os.path.join(input_dir, nombre_archivo)
        
        if not os.path.exists(ruta_completa):
            print(f"[-] Fichero omitido (no encontrado): {ruta_completa}")
            continue
            
        df = pd.read_csv(ruta_completa)
        
        # Añadimos la curva al panel actual
        ax.semilogy(df['ebno_db'], df['ber'], color=colores[t_idx], 
                    linestyle='-', marker=marcadores[t_idx], 
                    markersize=5, label=f'$t={t}$')
        
    # Formateo estético de cada subpanel
    ax.set_xlabel('$E_b/N_0$ (dB)', fontsize=11)
    if idx == 0:
        ax.set_ylabel('Bit Error Rate (BER)', fontsize=12)
        
    ax.set_title(f'Grado $m={m}$ ($n={2**m - 1}$)', fontsize=12, fontweight='bold')
    ax.grid(True, which='both', linestyle='--', alpha=0.5)
    ax.legend(loc='lower left', fontsize=10)
    ax.set_xlim([0, 9 if m == 11 else 10])  # Escala estándar de decibelios para tus simulaciones
    ax.set_ylim([1e-6, 1.2])

# Ajustar márgenes internos automáticamente
plt.tight_layout()

# Garantizar que la carpeta de salida existe
os.makedirs(output_dir, exist_ok=True)

# Guardar en la carpeta ~/TFG_CPP/results/
ruta_png = os.path.join(output_dir, 'grafica_generalizacion_bch.png')
ruta_pdf = os.path.join(output_dir, 'grafica_generalizacion_bch.pdf')

plt.savefig(ruta_png, dpi=300)
plt.savefig(ruta_pdf, bbox_inches='tight')

print("\n[+] ¡Gráfica panorámica procesada de forma exitosa!")
print(f"[+] Imagen PNG (para revisión rápida): {ruta_png}")
print(f"[+] Vectorial PDF (para incluir en LaTeX): {ruta_pdf}")