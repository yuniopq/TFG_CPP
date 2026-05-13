import pandas as pd
import matplotlib.pyplot as plt
import os
import glob

def generate_plots():
    # Buscamos todos los archivos CSV generados por el simulador BCH
    path = 'results/csv/'
    all_files = glob.glob(os.path.join(path, "BCH_m*_t*.csv"))
    legacy_metric_column = "".join(["f", "er"])
    
    if not all_files:
        print(f"⚠️ No se encontraron archivos en {path}. Ejecuta primero `run_test.py`.")
        return

    # Combinamos todos los archivos en un solo DataFrame
    df_list = [pd.read_csv(f) for f in all_files]
    df = pd.concat(df_list, ignore_index=True)

    if legacy_metric_column in df.columns and "cwer" not in df.columns:
        df = df.rename(columns={legacy_metric_column: "cwer"})

    required_columns = {"m", "t", "n", "k", "ebno_db", "ber", "cwer", "codewords", "avg_enc_us", "avg_dec_us"}
    missing = required_columns.difference(df.columns)
    if missing:
        print(f"❌ Faltan columnas en los CSV: {', '.join(sorted(missing))}")
        return

    if "ber_uncoded" not in df.columns:
        df["ber_uncoded"] = float("nan")

    os.makedirs("results/plot", exist_ok=True)
    
    # --- GRÁFICA DE ESCALABILIDAD: Tiempo vs m ---
    # Usamos el punto de Eb/N0 más bajo de cada ejecución; run_test.py genera una sola fila por m.
    df_esc = df.sort_values(['m', 't', 'ebno_db']).drop_duplicates(subset=['m', 't'], keep='first')

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
        plt.close()
        print("📊 Gráfica de escalabilidad guardada en results/plot/grafica_escalabilidad_m.png")

    # --- GRÁFICA DE FIABILIDAD: BER/CWER vs Eb/N0 para el m más alto ---
    m_max = df['m'].max()
    df_ber = df[df['m'] == m_max].sort_values('ebno_db')

    plt.figure(figsize=(10, 6))
    plt.plot(df_ber['ebno_db'], df_ber['ber'], marker='o', color='blue', label='BER')
    plt.plot(df_ber['ebno_db'], df_ber['ber_uncoded'], marker='x', color='green', label='BER sin codificar')
    plt.plot(df_ber['ebno_db'], df_ber['cwer'], marker='s', color='orange', label='CWER')
    plt.yscale('log')
    plt.title(f"Curva de error para m={m_max}, t={df_ber['t'].iloc[0]}")
    plt.xlabel("Eb/N0 (dB)")
    plt.ylabel("Tasa de error")
    plt.grid(True, which="both", ls="-", alpha=0.2)
    plt.legend()
    plt.savefig(f"results/plot/grafica_ber_m{m_max}.png")
    plt.close()
    
    print(f"📊 Gráfica de error guardada en results/plot/grafica_ber_m{m_max}.png")

if __name__ == "__main__":
    generate_plots()