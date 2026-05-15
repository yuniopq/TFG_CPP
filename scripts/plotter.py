import os
import glob
import pandas as pd
import matplotlib.pyplot as plt

# =============================================================================
# CONFIGURACIÓN
# =============================================================================
CARPETA_CSV = "results/csv"  # Cambia esto si tu carpeta se llama "resultado/csv"
CARPETA_GRAFICAS = "results/plot"

def cargar_y_limpiar_datos(carpeta):
    # 1. Buscar todos los archivos CSV en la carpeta
    archivos_csv = glob.glob(os.path.join(carpeta, "*.csv"))
    
    if not archivos_csv:
        print(f"❌ No se encontraron archivos .csv en la ruta: {carpeta}")
        return None
        
    print(f"📂 Se han encontrado {len(archivos_csv)} archivos CSV. Leyendo...")
    
    dfs = []
    for archivo in archivos_csv:
        try:
            df = pd.read_csv(archivo)
            
            # Unificar nombres de columnas (algunos tienen fer/frames en lugar de cwer/codewords)
            df = df.rename(columns={'fer': 'cwer', 'frames': 'codewords'})
            dfs.append(df)
        except Exception as e:
            print(f"⚠️ Error leyendo {archivo}: {e}")
            
    # 2. Unir todo en un solo DataFrame
    df_total = pd.concat(dfs, ignore_index=True)
    
    # 3. Forzar a que todo sea numérico (esto elimina silenciosamente filas de texto/encabezados basura)
    df_total = df_total.apply(pd.to_numeric, errors='coerce').dropna()
    
    return df_total

def generar_graficas_ber(df):
    ruta_ber = os.path.join(CARPETA_GRAFICAS, "BER")
    os.makedirs(ruta_ber, exist_ok=True)
    
    m_values = sorted(df['m'].unique())
    
    for m in m_values:
        df_m = df[df['m'] == m]
        t_values = sorted(df_m['t'].unique())
        n = int(df_m['n'].iloc[0])
        
        plt.figure(figsize=(10, 7))
        
        # Plotear la curva sin codificar (tomamos la primera, ya que es igual para todas)
        df_uncoded = df_m[df_m['t'] == t_values[0]].sort_values('ebno_db')
        plt.plot(df_uncoded['ebno_db'], df_uncoded['ber_uncoded'], 
                 marker='X', color='black', linestyle='--', linewidth=2, label='Sin codificar (Uncoded)')

        # Plotear una curva por cada valor de t
        for t in t_values:
            df_t = df_m[df_m['t'] == t].sort_values('ebno_db')
            k = int(df_t['k'].iloc[0])
            tasa = k / n
            plt.plot(df_t['ebno_db'], df_t['ber'], marker='o', 
                     label=f't={int(t)} (k={k}, R={tasa:.4f})')

        plt.yscale('log')
        plt.ylim(bottom=1e-8, top=1)
        plt.xlim(left=0, right=df_m['ebno_db'].max())
        
        plt.title(f"Rendimiento BER - Código BCH con m={int(m)} (n={n})", fontsize=14, fontweight='bold')
        plt.xlabel("Eb/N0 (dB)", fontsize=12)
        plt.ylabel("Tasa de Error de Bit (BER)", fontsize=12)
        plt.grid(True, which="both", ls="--", alpha=0.6)
        plt.legend(loc='lower left', fontsize=10)
        
        archivo_salida = os.path.join(ruta_ber, f"Curva_BER_m{int(m)}.png")
        plt.savefig(archivo_salida, dpi=300, bbox_inches='tight')
        plt.close()
        print(f"   ✅ Gráfica BER generada: {archivo_salida}")

def generar_graficas_tiempo(df):
    ruta_tiempos = os.path.join(CARPETA_GRAFICAS, "Tiempos")
    os.makedirs(ruta_tiempos, exist_ok=True)
    
    m_values = sorted(df['m'].unique())
    
    for m in m_values:
        df_m = df[df['m'] == m]
        t_values = sorted(df_m['t'].unique())
        n = int(df_m['n'].iloc[0])
        
        # Agrupar por t y sacar el tiempo promedio (o el máximo si quieres ver el peor caso)
        tiempos = df_m.groupby('t')['avg_dec_us'].mean()
        
        plt.figure(figsize=(9, 5))
        plt.plot(tiempos.index, tiempos.values, marker='s', color='darkred', linestyle='-', linewidth=2)
        
        plt.title(f"Complejidad de Decodificación - m={int(m)} (n={n})", fontsize=14, fontweight='bold')
        plt.xlabel("Capacidad de Corrección Teórica (t)", fontsize=12)
        plt.ylabel("Tiempo Promedio Decodificación (µs)", fontsize=12)
        plt.grid(True, ls="--", alpha=0.6)
        
        archivo_salida = os.path.join(ruta_tiempos, f"Tiempos_Dec_m{int(m)}.png")
        plt.savefig(archivo_salida, dpi=300, bbox_inches='tight')
        plt.close()
        print(f"   ✅ Gráfica Tiempos generada: {archivo_salida}")

if __name__ == "__main__":
    print("Iniciando análisis de resultados...\n")
    df_global = cargar_y_limpiar_datos(CARPETA_CSV)
    
    if df_global is not None:
        print("\n📊 Generando gráficas de Tasa de Error (BER)...")
        generar_graficas_ber(df_global)
        
        print("\n⏱️ Generando gráficas de Tiempos de Computación...")
        generar_graficas_tiempo(df_global)
        
        print("\n🎉 ¡Análisis completado! Revisa la carpeta results/plot/")