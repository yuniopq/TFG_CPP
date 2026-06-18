# --- Colores para la terminal ---
CYAN  = \033[0;36m
GREEN = \033[0;32m
GOLD  = \033[0;33m
RESET = \033[0m

# --- Variables de compilación ---
CXX      = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude -MMD -MP -O3 -march=native -DNDEBUG -fopenmp
TARGET   = bch_sim

# --- Directorios ---
SRC_DIR   = src
BUILD_DIR = build
RES_DIR   = results

# --- Descubrimiento automático de fuentes ---
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

# --- Valores por defecto para ejecución ---
M    ?= 7
T    ?= 10
MIN  ?= 0
MAX  ?= 10
STEP ?= 1
FILE ?= 

# Argumentos base comunes
RUN_ARGS = -m $(M) -t $(T)

ifeq ($(FILE),)
    # 1. MODO MONTECARLO: Forzado a AWGN por diseño.
    RUN_ARGS += -min $(MIN) -max $(MAX) -step $(STEP)
else
    # 2. MODO ARCHIVO: Permite elegir canal (awgn o bsc) y probabilidad.
    CHAN ?= awgn
    PROB ?= 0.01
    RUN_ARGS += -file $(FILE) -min $(MIN) -chan $(CHAN) -prob $(PROB)
endif

# --- Reglas principales ---
.PHONY: all clean help setup run

all: setup $(TARGET)

setup:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(RES_DIR)/csv

$(TARGET): $(OBJS)
	@echo "$(CYAN) Enlazando ejecutable optimizado:$(RESET) $@"
	@$(CXX) $(CXXFLAGS) -o $@ $(OBJS)
	@echo "$(GREEN) Compilación completada con éxito.$(RESET)"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "$(GOLD) Compilando:$(RESET) $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# --- Menú de ejecución ---
run: all
	@echo "$(CYAN) Iniciando simulación BCH...$(RESET)"
	@echo "$(GOLD)Ejecutando: ./$(TARGET) $(RUN_ARGS)$(RESET)"
	@./$(TARGET) $(RUN_ARGS)

clean:
	@echo "$(CYAN) Eliminando archivos de compilación...$(RESET)"
	@rm -rf $(BUILD_DIR) $(TARGET)
	@echo "$(GREEN) Completado.$(RESET)"

help:
	@echo "$(CYAN)========================================================================$(RESET)"
	@echo "   Comandos del Simulador BCH (TFG)        "
	@echo "$(CYAN)========================================================================$(RESET)"
	@echo "$(GOLD)make$(RESET)                  		- Compilar el simulador"
	@echo "$(GOLD)make clean$(RESET)            		- Eliminar binarios y temporales"
	@echo ""
	@echo "$(GREEN)--- MODO ESTADÍSTICO (AWGN forzado para Montecarlo) ---$(RESET)"
	@echo "$(GOLD)make run$(RESET)              		- Ejecutar barrido por defecto"
	@echo "$(GOLD)make run M=4 T=2 MIN=0 MAX=8$(RESET)	- Ejecutar BCH(15,7) de 0 a 8 dB"
	@echo ""
	@echo "$(GREEN)--- MODO CUALITATIVO (Procesamiento de Archivos e Imágenes) ---$(RESET)"
	@echo "$(GOLD)make run FILE=img.bmp MIN=4$(RESET) 	- Procesar archivo con AWGN a 4 dB"
	@echo "$(GOLD)make run FILE=img.bmp CHAN=bsc PROB=0.05$(RESET) - Procesar con canal BSC (5%)"
	@echo "$(CYAN)------------------------------------------------------------------------$(RESET)"

-include $(DEPS)