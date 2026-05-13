# --- Colores para terminal ---
CYAN  = \033[0;36m
GREEN = \033[0;32m
GOLD  = \033[0;33m
RESET = \033[0m

# --- Variables de Compilación ---
CXX      = g++
# -O3 y -march=native son clave para el rendimiento de la lógica polinómica 
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude -MMD -MP -O3 -march=native -DNDEBUG -fopenmp
TARGET   = bch_sim

# --- Directorios ---
SRC_DIR   = src
BUILD_DIR = build
RES_DIR   = results

# --- Búsqueda automática ---
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

# --- Valores por defecto para ejecución ---
M    ?= 7
T    ?= 10
MIN  ?= 0
MAX  ?= 10
STEP ?= 1
FILE ?= # Vacío por defecto

# --- Reglas Principales ---
.PHONY: all clean help setup

all: setup $(TARGET)

setup:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(RES_DIR)/csv

$(TARGET): $(OBJS)
	@echo "$(CYAN)🔗 Enlazando ejecutable optimizado:$(RESET) $@"
	@$(CXX) $(CXXFLAGS) -o $@ $(OBJS)
	@echo "$(GREEN)✨ Compilación completada con éxito.$(RESET)"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "$(GOLD)🔨 Compilando:$(RESET) $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# --- Menú de Ejecución Inteligente ---

# Ejecución general: usa los valores por defecto o los pasados por terminal
# Ejemplo: make run M=4 T=2 FILE=imagen.bmp
run: all
	@echo "$(CYAN)🚀 Iniciando Simulación BCH...$(RESET)"
	@./$(TARGET) $(M) $(T) $(MIN) $(MAX) $(STEP) $(FILE)

# Limpieza de compilación sin borrar resultados generados
clean:
	@echo "$(CYAN)🧹 Limpiando archivos temporales...$(RESET)"
	@rm -rf $(BUILD_DIR) $(TARGET)
	@echo "$(GREEN)Done.$(RESET)"

help:
	@echo "$(CYAN)=========================================$(RESET)"
	@echo "   Comandos de Simulación BCH (TFG)      "
	@echo "$(CYAN)=========================================$(RESET)"
	@echo "$(GOLD)make$(RESET)                - Compila el simulador"
	@echo "$(GOLD)make run$(RESET)            - Ejecuta con M=7 T=10"
	@echo "$(GOLD)make run M=4 T=2$(RESET)    - Ejecuta BCH(15,7)"
	@echo "$(GOLD)make run FILE=img.bmp$(RESET) - Procesa un archivo real"
	@echo "$(GOLD)make clean$(RESET)          - Borra solo build y ejecutable"
	@echo "$(CYAN)-----------------------------------------$(RESET)"

-include $(DEPS)