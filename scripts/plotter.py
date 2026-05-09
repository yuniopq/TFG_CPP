import pandas as pd
import matplotlib.pyplot as plt
import os
import glob

def generate_plots():
    # Buscamos todos los archivos CSV que haya en la carpeta results
    path = 'results/csv/'
    all_files = glob.glob(os.path.join(path, "data_m*.csv"))
    
    if not all_files:
        print(f"⚠️ No se encontraron archivos en {path}. Ejecuta primero run_test.py")
        return

    # Combinamos todos los archivos en un solo DataFrame
    df_list = [pd.read_csv(f) for f in all_files]
    df = pd.concat(df_list, ignore_index=True)
    
    # --- GRÁFICA DE ESCALABILIDAD: Tiempo vs m ---
    # Filtramos para un BER constante (ej: 0.001) para comparar manzanas con manzanas
    df_esc = df[df['prob_error'] == 0.001].sort_values('m')

    if not df_esc.empty:
        plt.figure(figsize=(10, 6))
        plt.plot(df_esc['m'], df_esc['avg_enc_us'] / 1000, marker='s', ls='--', label='Codificación (ms)', color='green')
        plt.plot(df_esc['m'], df_esc['avg_dec_us'] / 1000, marker='^', ls='-', label='Decodificación (ms)', color='red')
        
        plt.title("Análisis de Escalabilidad: Impacto del Tamaño del Campo (m)")
        plt.xlabel("Valor de m (Longitud n = 2^m - 1)")
        plt.ylabel("Tiempo promedio (ms)")
        plt.xticks(df_esc['m']) # Forzamos que salgan los valores de m en el eje X
        plt.grid(True, alpha=0.3)
        plt.legend()
        
        # Añadir etiquetas con el valor de n para dar contexto
        for i, txt in enumerate(df_esc['m']):
            plt.annotate(f"n={2**txt-1}", (df_esc['m'].iloc[i], df_esc['avg_dec_us'].iloc[i]/1000), 
                         textcoords="offset points", xytext=(0,10), ha='center', fontsize=8)

        plt.savefig("results/plot/grafica_escalabilidad_m.png")
        print("📊 Gráfica de escalabilidad guardada en results/plot/grafica_escalabilidad_m.png")

    # --- GRÁFICA DE FIABILIDAD (BER vs FER) para el m más alto ---
    m_max = df['m'].max()
    df_ber = df[df['m'] == m_max].sort_values('prob_error')

    plt.figure(figsize=(10, 6))
    plt.plot(df_ber['prob_error'], df_ber['fer'], marker='o', color='blue')
    plt.xscale('log')
    plt.title(f"Curva de Error para m={m_max}, t={df_ber['t'].iloc[0]}")
    plt.xlabel("BER del Canal (Probabilidad de error de bit)")
    plt.ylabel("FER (Tasa de error de bloque)")
    plt.grid(True, which="both", ls="-", alpha=0.2)
    plt.savefig(f"results/plot/grafica_ber_m{m_max}.png")
    
    plt.show()

if __name__ == "__main__":
    generate_plots()