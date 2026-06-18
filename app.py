import customtkinter as ctk, subprocess, threading, os
from customtkinter import filedialog
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import os

class App(ctk.CTk): 
    def __init__(self):
        super().__init__()
        self.title("Simulador BCH")
        self.geometry("950x750")

        self.protocol("WM_DELETE_WINDOW", self.cerrar_app)

        self.tabs = ctk.CTkTabview(self)
        self.tabs.pack(fill="both", expand=True, padx=10, pady=10)

        self.tab_stats = self.tabs.add("Monte Carlo (AWGN)")
        self.tab_files = self.tabs.add("Imágenes")
        
        self.crear_pestana_MonteCarlo()
        self.crear_pestana_archivo()

    def cerrar_app(self):
        self.destroy()
        os._exit(0)  

    def crear_pestana_MonteCarlo(self):
        panel = ctk.CTkFrame(self.tab_stats, width=220)
        panel.pack(side="left", fill="y", padx=10)

        self.m_s = self.crear_input(panel, "Parámetro m:", "Grado del cuerpo GF(2^m)", "7")
        self.t_s = self.crear_input(panel, "Parámetro t:", "Capacidad correctora (bits)", "10")
        self.min_s = self.crear_input(panel, "Min Eb/N0:", "Relación señal-ruido inicial (dB)", "0")
        self.max_s = self.crear_input(panel, "Max Eb/N0:", "Relación señal-ruido final (dB)", "12")
        self.step_s = self.crear_input(panel, "Paso:", "Incremento del barrido (dB)", "1")

        self.btn_s = ctk.CTkButton(panel, text="Iniciar Montecarlo", command=self.ejecutar_montecarlo)
        self.btn_s.pack(pady=15)

        self.btn_save_plot = ctk.CTkButton(panel, text="Guardar Gráfica", command=self.save_plot)
        self.btn_save_plot.pack(pady=5)

        self.fig, self.ax = plt.subplots(figsize=(5, 4))
        self.canvas = FigureCanvasTkAgg(self.fig, master=self.tab_stats)
        self.canvas.get_tk_widget().pack(side="right", fill="both", expand=True)
    
    def crear_pestana_archivo(self):
        panel2 = ctk.CTkFrame(self.tab_files, width=220)
        panel2.pack(side="left", fill="y", padx=10)

        ctk.CTkButton(panel2, text="Elegir Archivo", command=self.pick_file).pack(pady=10)
        
        self.lbl_status = ctk.CTkLabel(panel2, text="")
        self.lbl_status.pack()

        self.filepath = ""
        self.lbl_file = ctk.CTkLabel(panel2, text="Ningún archivo seleccionado", text_color="gray")
        self.lbl_file.pack(pady=5)

        ctk.CTkButton(panel2, text="Carpeta Destino", command=self.pick_save_dir).pack(pady=10)
        
        self.savepath = ""
        self.lbl_save = ctk.CTkLabel(panel2, text="Por defecto (results/)", text_color="gray")
        self.lbl_save.pack(pady=5)

        self.m_f = self.crear_input(panel2, "Parámetro m:", "Grado del cuerpo GF(2^m)", "7")
        self.t_f = self.crear_input(panel2, "Parámetro t:", "Capacidad correctora (bits)", "10")
        
        ctk.CTkLabel(panel2, text="Tipo de Canal:", font=("Arial", 12, "bold")).pack(pady=(10, 0))
        ctk.CTkLabel(panel2, text="Modelo físico/estadístico", font=("Arial", 10), text_color="gray").pack()
        self.canal = ctk.CTkOptionMenu(panel2, values=["awgn", "bsc"])
        self.canal.pack(padx=10, pady=(0, 10))        
        
        self.noise = self.crear_input(panel2, "Nivel de Ruido:", "AWGN (dB) o BSC (Prob. error)", "4")
        
        self.btn_f = ctk.CTkButton(panel2, text="Procesar Archivo", command=self.run_file)
        self.btn_f.pack(pady=15)

    def crear_input(self, contenedor, texto, descripcion, valor_inicial, alineacion="top", margen=5):
        frame = ctk.CTkFrame(contenedor, fg_color="transparent")
        frame.pack(side=alineacion, padx=margen, pady=5)

        ctk.CTkLabel(frame, text=texto, font=("Arial", 12, "bold")).pack()
        ctk.CTkLabel(frame, text=descripcion, font=("Arial", 10), text_color="gray").pack()

        input = ctk.CTkEntry(frame, width=80)
        input.insert(0, valor_inicial)
        input.pack(pady=(2, 0)) 
        
        return input

    def ejecutar_montecarlo(self):
        self.btn_s.configure(state="disabled", text="Simulando...")
        self.ax.clear()

        def run():
            cmd = ["./bch_sim", "-m", self.m_s.get(), "-t", self.t_s.get(), 
                   "-min", self.min_s.get(), "-max", self.max_s.get(), 
                   "-step", self.step_s.get(), "-q"]
            
            proceso = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            stdout, _ = proceso.communicate() 
            ebnos, bers = [], []

            for line in stdout.splitlines():
                try:
                    vals = line.split(",")
                    if len(vals) >= 2:
                        ebnos.append(float(vals[0]))
                        bers.append(float(vals[1]) if float(vals[1]) > 0 else 1e-9)
                except: pass
            
            if ebnos and bers:
                self.ax.semilogy(ebnos, bers, 'bo-')
                self.ax.set(title="Evolución BER (AWGN)", xlabel="Eb/N0 (dB)", ylabel="BER")
                self.ax.grid(True, ls="--", alpha=0.6)
                self.canvas.draw()
            
            self.btn_s.configure(state="normal", text="Iniciar Montecarlo")
        threading.Thread(target=run, daemon=True).start()

    def pick_file(self):
        self.filepath = filedialog.askopenfilename(filetypes=[("Todos los archivos", "*.*")])
        if self.filepath:
            self.lbl_file.configure(text=os.path.basename(self.filepath), text_color="black")    

    def pick_save_dir(self):
        self.savepath = filedialog.askdirectory()
        if self.savepath:
            self.lbl_save.configure(text=self.savepath, text_color="black")

    def save_plot(self):
        path = filedialog.asksaveasfilename(defaultextension=".png", filetypes=[("PNG", "*.png"), ("PDF", "*.pdf")])
        if path:
            self.fig.savefig(path)

    def run_file(self):
        if not self.filepath:
            self.lbl_status.configure(text="No se ha seleccionado ningún archivo.", text_color="red")
            return
        
        self.lbl_status.configure(text="Procesando archivo...", text_color="black")
        self.btn_f.configure(state="disabled")

        def run():
            ruido_val = self.noise.get()
            
            cmd = ["./bch_sim", "-m", self.m_f.get(), "-t", self.t_f.get(), 
                   "-chan", self.canal.get(), "-min", ruido_val, "-prob", ruido_val, 
                   "-file", self.filepath, "-q"]
            
            if self.savepath:
                cmd.extend(["-out", self.savepath])
                
            p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            p.communicate()

            self.btn_f.configure(state="normal")
            if p.returncode == 0:
                self.lbl_status.configure(text="Archivo procesado correctamente.", text_color="green")
            else:
                self.lbl_status.configure(text="Error al procesar el archivo.", text_color="red")

        threading.Thread(target=run, daemon=True).start()
        
if __name__ == "__main__":
    app = App()
    app.mainloop()
