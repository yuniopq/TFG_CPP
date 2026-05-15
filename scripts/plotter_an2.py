import os
import glob
import pandas as pd
import matplotlib.pyplot as plt

# =============================================================================
# CONFIGURACIÓN
# =============================================================================
CARPETA_CSV = "results/csv_an2"
CARPETA_GRAFICAS = "results/plot_an2"  # Carpeta nueva para no pisar las anteriores

def cargar_y_limpiar_datos(carpeta):
    archivos_csv = glob.glob(os.path.join(carpeta, "*.csv"))
    if not archivos_csv:
        print(f"❌ No se encontraron archivos .csv en la ruta: {carpeta}")
        return None
        
    print(f"📂 Leyendo {len(archivos_csv)} archivos CSV para Análisis de Ganancia...")
    
    dfs = []
    for archivo in archivos_csv:
        try:
            df = pd.read_csv(archivo)
            # Unificar nombres de columnas
            df = df.rename(columns={'fer': 'cwer', 'frames': 'codewords'})
            dfs.append(df)
        except Exception as e:
            print(f"⚠️ Error leyendo {archivo}: {e}")
            
    df_total = pd.concat(dfs, ignore_index=True)
    df_total = df_total.apply(pd.to_numeric, errors='coerce').dropna()
    
    # === NUEVA LÓGICA: CLASIFICAR POR TASA OBJETIVO (R) ===
    # Calculamos la tasa real (k/n) y la clasificamos en nuestros 3 escenarios
    df_total['tasa_real'] = df_total['k'] / df_total['n']
    
    def clasificar_tasa(r):
        # Ampliamos el rango de la Tasa Alta para que entre el 0.733 de m=4
        if 0.70 <= r <= 0.85: return "Alta (~80%)"
        
        # El 0.46 de m=4 entra bien aquí
        elif 0.45 <= r <= 0.56: return "Media (~50%)"
        
        # El 0.33 de m=4 entra bien aquí
        elif 0.30 <= r <= 0.35: return "Baja (~33%)"
        
        else: return "Ignorar"        
    df_total['grupo_tasa'] = df_total['tasa_real'].apply(clasificar_tasa)
    
    # Nos quedamos solo con las filas que encajan en nuestros 3 grupos de estudio
    df_filtrado = df_total[df_total['grupo_tasa'] != "Ignorar"]
    return df_filtrado

def generar_graficas_shannon(df):
    ruta_ber = os.path.join(CARPETA_GRAFICAS, "Ganancia_Codificacion")
    os.makedirs(ruta_ber, exist_ok=True)
    
    grupos = sorted(df['grupo_tasa'].unique())
    
    for grupo in grupos:
        df_grupo = df[df['grupo_tasa'] == grupo]
        
        plt.figure(figsize=(10, 7))
        
        # 1. Plotear la curva Sin Codificar (Referencia AWGN) - ¡CORREGIDO!
        # Agrupamos TODOS los archivos por Eb/N0 y promediamos el BER Uncoded.
        # Al agrupar, si m=11 paró a los 6dB pero m=4 siguió hasta los 12dB, 
        # la recta usará los datos de m=4 para continuar el trazo hasta el final.
        df_ref = df_grupo.groupby('ebno_db')['ber_uncoded'].mean().reset_index()
        
        plt.plot(df_ref['ebno_db'], df_ref['ber_uncoded'], 
                 marker='X', color='black', linestyle='--', linewidth=2, label='Referencia Sin Codificar')

        # 2. Plotear las curvas BCH para cada m
        m_values = sorted(df_grupo['m'].unique())
        colores = {4: 'blue', 7: 'green', 11: 'red', 15: 'purple'} # Colores fijos por consistencia
        
        for m in m_values:
            df_m = df_grupo[df_grupo['m'] == m].sort_values('ebno_db')
            
            # Si hay varios 't' para el mismo 'm' que caen en la misma tasa, cogemos el primero
            t_val = df_m['t'].iloc[0] 
            df_m_t = df_m[df_m['t'] == t_val]
            
            n = int(df_m_t['n'].iloc[0])
            k = int(df_m_t['k'].iloc[0])
            r_real = k / n
            
            plt.plot(df_m_t['ebno_db'], df_m_t['ber'], marker='o', color=colores.get(m, 'orange'),
                     label=f'BCH m={int(m)} (n={n}, t={int(t_val)}) [R={r_real:.2f}]')

        plt.yscale('log')
        plt.ylim(bottom=1e-8, top=1)
        # Extendemos el límite derecho hasta el máximo Eb/N0 que haya sobrevivido (el de df_ref)
        plt.xlim(left=0, right=df_ref['ebno_db'].max())
        
        plt.title(f"Demostración de Ganancia de Codificación - Tasa {grupo}", fontsize=14, fontweight='bold')
        plt.xlabel("Eb/N0 (dB)", fontsize=12)
        plt.ylabel("Tasa de Error de Bit (BER)", fontsize=12)
        plt.grid(True, which="both", ls="--", alpha=0.6)
        plt.legend(loc='lower left', fontsize=10)
        
        nombre_archivo = grupo.replace(" ", "_").replace("(", "").replace(")", "").replace("~", "")
        archivo_salida = os.path.join(ruta_ber, f"Ganancia_Tasa_{nombre_archivo}.png")
        plt.savefig(archivo_salida, dpi=300, bbox_inches='tight')
        plt.close()
        print(f"   ✅ Gráfica de Ganancia generada: {archivo_salida}")
        
def generar_grafica_complejidad_cruzada(df):
    ruta_tiempos = os.path.join(CARPETA_GRAFICAS, "Complejidad")
    os.makedirs(ruta_tiempos, exist_ok=True)
    
    plt.figure(figsize=(9, 6))
    grupos = sorted(df['grupo_tasa'].unique())
    marcadores = ['o', 's', '^']
    
    for idx, grupo in enumerate(grupos):
        df_grupo = df[df['grupo_tasa'] == grupo]
        
        # Agrupar por 'm' y sacar el tiempo medio de decodificación
        tiempos = df_grupo.groupby('m')['avg_dec_us'].mean().sort_index()
        
        plt.plot(tiempos.index, tiempos.values, marker=marcadores[idx % len(marcadores)], 
                 linestyle='-', linewidth=2, label=f'Tasa {grupo}')

    plt.title("Complejidad Computacional vs Longitud del Bloque (m)", fontsize=14, fontweight='bold')
    plt.xlabel("Parámetro del Bloque (m)", fontsize=12)
    plt.ylabel("Tiempo Promedio Decodificación (µs) - [Escala Log]", fontsize=12)
    
    # Usamos escala logarítmica también para el tiempo, porque m=11 va a dispararse frente a m=4
    plt.yscale('log') 
    plt.grid(True, which="both", ls="--", alpha=0.6)
    
    # Forzar que el eje X muestre solo números enteros para m
    m_ticks = sorted(df['m'].unique())
    plt.xticks(m_ticks, [f"m={int(m)}" for m in m_ticks])
    
    plt.legend(loc='upper left', fontsize=10)
    
    archivo_salida = os.path.join(ruta_tiempos, "Complejidad_Cruzada.png")
    plt.savefig(archivo_salida, dpi=300, bbox_inches='tight')
    plt.close()
    print(f"   ✅ Gráfica de Complejidad generada: {archivo_salida}")

if __name__ == "__main__":
    print("Iniciando análisis de Ganancia de Codificación (Análisis 2)...\n")
    df_global = cargar_y_limpiar_datos(CARPETA_CSV)
    
    if df_global is not None and not df_global.empty:
        print("\n📊 Generando gráficas del Teorema de Shannon...")
        generar_graficas_shannon(df_global)
        
        print("\n⏱️ Generando gráfica cruzada de Complejidad Computacional...")
        generar_grafica_complejidad_cruzada(df_global)
        
        print("\n🎉 ¡Análisis completado! Revisa la carpeta results/plot_an2/")
    else:
        print("⚠️ No se encontraron datos válidos que coincidan con las tasas objetivo (80%, 50%, 33%).")