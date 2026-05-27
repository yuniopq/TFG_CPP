import os
import matplotlib.pyplot as plt
import pandas as pd

# Rutas del proyecto
input_dir = os.path.join('results', 'csv')
output_dir = 'results'
nombre_fichero = 'BCH_m7_t10.csv'  # Ajusta si el nombre cambia
ruta_completa = os.path.join(input_dir, nombre_fichero)

print(f"[+] Generando gráfica de Análisis de Latencia Computacional...")

if not os.path.exists(ruta_completa):
    print(f"[-] Error: No se encuentra el fichero {ruta_completa}.")
    print("    Asegúrate de haber ejecutado la simulación y que el CSV esté en su ruta.")
    exit(1)

# Cargar los datos
df = pd.read_csv(ruta_completa)

# Crear la figura y el primer eje (Latencia)
fig, ax1 = plt.subplots(figsize=(10, 6))

# Título
ax1.set_title('Análisis de Latencia vs. Rendimiento del Canal (m=7, t=10)', fontsize=14, fontweight='bold')

# Eje X
ax1.set_xlabel('Eb/N0 (dB)', fontsize=12)
ax1.set_xlim([df['ebno_db'].min(), df['ebno_db'].max()])

# Eje Y Izquierdo: Tiempos (Microsegundos)
color_dec = '#d62728'  # Rojo para el decodificador (costoso)
color_enc = '#2ca02c'  # Verde para el codificador (estable)

linea_dec = ax1.plot(df['ebno_db'], df['avg_dec_us'], color=color_dec, linestyle='-', 
                     marker='o', markersize=6, linewidth=2, label='Decodificación (avg_dec_us)')
linea_enc = ax1.plot(df['ebno_db'], df['avg_enc_us'], color=color_enc, linestyle='--', 
                     marker='s', markersize=6, linewidth=2, label='Codificación (avg_enc_us)')

ax1.set_ylabel('Latencia media por bloque (\u03bcs)', fontsize=12, color='black')
ax1.tick_params(axis='y', labelcolor='black')
ax1.grid(True, which='major', linestyle='--', alpha=0.7)

# Eje Y Derecho: BER (Logarítmico)
ax2 = ax1.twinx()  # Instanciar un segundo eje que comparte el mismo eje X
color_ber = '#1f77b4' # Azul para la BER

linea_ber = ax2.semilogy(df['ebno_db'], df['ber'], color=color_ber, linestyle='-.', 
                         marker='^', markersize=6, linewidth=2, label='BER Codificada')

ax2.set_ylabel('Bit Error Rate (BER)', fontsize=12, color=color_ber)
ax2.tick_params(axis='y', labelcolor=color_ber)
ax2.set_ylim([1e-8, 1])

# Combinar las leyendas de ambos ejes en una sola caja
lineas = linea_dec + linea_enc + linea_ber
etiquetas = [l.get_label() for l in lineas]
ax1.legend(lineas, etiquetas, loc='center right', fontsize=11, framealpha=0.9)

# Guardar la figura
plt.tight_layout()
os.makedirs(output_dir, exist_ok=True)

plt.savefig(os.path.join(output_dir, 'grafica_latencia_m7_t10.png'), dpi=300)
plt.savefig(os.path.join(output_dir, 'grafica_latencia_m7_t10.pdf'), bbox_inches='tight')

print(f"[+] ¡Gráfica generada con éxito en {output_dir}/grafica_latencia_m7_t10.pdf!")