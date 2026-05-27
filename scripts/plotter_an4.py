import os
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

# Rutas del proyecto (ajusta si tus CSVs de m=7 están en otra subcarpeta)
input_dir = os.path.join('results', 'csv') 
output_dir = 'results'

# Nivel de ruido donde evaluaremos la latencia (3 dB es un buen punto de esfuerzo)
ebno_target = 3.0 

# Configuraciones a comparar (m=7 fijo, variando t)
configuraciones = [
    {'file': 'BCH_m7_t1.csv', 'label': 't=1\n(R=0.94)'},
    {'file': 'BCH_m7_t3.csv', 'label': 't=3\n(R=0.83)'},
    {'file': 'BCH_m7_t7.csv', 'label': 't=7\n(R=0.61)'},
    {'file': 'BCH_m7_t15.csv', 'label': 't=15\n(R=0.17)'}
]

etiquetas = []
tiempos_enc = []
tiempos_dec = []

print(f"[+] Generando diagrama de barras de complejidad algorítmica a {ebno_target} dB...")

for conf in configuraciones:
    ruta_completa = os.path.join(input_dir, conf['file'])
    
    if not os.path.exists(ruta_completa):
        print(f"[-] Advertencia: No se encontró {conf['file']}. Omitiendo...")
        continue
        
    df = pd.read_csv(ruta_completa)
    
    # Buscar la fila más cercana al ebno_target
    fila = df.iloc[(df['ebno_db'] - ebno_target).abs().argsort()[:1]]
    
    if not fila.empty:
        etiquetas.append(conf['label'])
        tiempos_enc.append(fila['avg_enc_us'].values[0])
        tiempos_dec.append(fila['avg_dec_us'].values[0])

if not etiquetas:
    print("[-] Error: No se pudo cargar ningún dato.")
    exit(1)

# Configurar el gráfico de barras agrupadas
x = np.arange(len(etiquetas))
width = 0.35  # Ancho de las barras

fig, ax = plt.subplots(figsize=(9, 6))

barra_enc = ax.bar(x - width/2, tiempos_enc, width, label='Codificación', color='#2ca02c', edgecolor='black')
barra_dec = ax.bar(x + width/2, tiempos_dec, width, label='Decodificación', color='#d62728', edgecolor='black')

# Añadir textos, título y etiquetas personalizadas
ax.set_ylabel('Latencia media por bloque (\u03bcs)', fontsize=12)
ax.set_title(f'Impacto de la Capacidad Correctora ($t$) en la Latencia a {ebno_target} dB', fontsize=14, fontweight='bold')
ax.set_xticks(x)
ax.set_xticklabels(etiquetas, fontsize=11)
ax.legend(fontsize=11)
ax.grid(axis='y', linestyle='--', alpha=0.7)

# Función para añadir el valor numérico encima de cada barra
def autolabel(rects):
    for rect in rects:
        height = rect.get_height()
        ax.annotate(f'{height:.1f}',
                    xy=(rect.get_x() + rect.get_width() / 2, height),
                    xytext=(0, 3),  # Desplazamiento vertical de 3 puntos
                    textcoords="offset points",
                    ha='center', va='bottom', fontsize=10)

autolabel(barra_enc)
autolabel(barra_dec)

plt.tight_layout()
os.makedirs(output_dir, exist_ok=True)
plt.savefig(os.path.join(output_dir, 'grafica_complejidad_m7.pdf'), bbox_inches='tight')

print(f"[+] ¡Gráfica guardada en {output_dir}/grafica_complejidad_m7.pdf!")