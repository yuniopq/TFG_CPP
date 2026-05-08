# --- Variables ---
CXX      = g++
# Añadimos -O3, -march=native y -DNDEBUG para máximo rendimiento
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude -MMD -MP -O3 -march=native -DNDEBUG
TARGET = $(BUILD_DIR)/bch_test

# --- Directorios ---
SRC_DIR   = src
BUILD_DIR = build

# --- Búsqueda automática de archivos ---
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

# --- Reglas Principales ---
.PHONY: all clean run run-m4 run-m5 run-m6 run-error help

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "🔗 Enlazando el ejecutable optimizado..."
	@$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "🔨 Compilando $<..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "🧹 Limpiando..."
	@rm -rf $(BUILD_DIR) $(TARGET)
	@echo "✨ Todo limpio."

# =====================================================================
# --- MENÚ DE EJECUCIÓN ---
# =====================================================================

# 1. Ejecución estándar (Por defecto: m=4, t=2)
run: all
	@echo "🚀 Ejecutando test por defecto (m=4, t=2)...\n"
	@./$(TARGET)

# 2. Ejecuciones predefinidas (Añade las que más uses)
run-m4: all
	@echo "🚀 Ejecutando test para m=4, t=2 (BCH 15, 7)...\n"
	@./$(TARGET) 4 2

run-m5: all
	@echo "🚀 Ejecutando test para m=5, t=3 (BCH 31, 16)...\n"
	@./$(TARGET) 5 3

run-m6: all
	@echo "🚀 Ejecutando test para m=6, t=4 (BCH 63, 39)...\n"
	@./$(TARGET) 6 4

run-m7: all
	@echo "🚀 Ejecutando test para m=7, t=5 (BCH 127, ...)...\n"
	@./$(TARGET) 7 5

run-m8: all
	@echo "🚀 Ejecutando test para m=8, t=6 (BCH 255, ...)...\n"
	@./$(TARGET) 8 6

run-m15: all
	@echo "🚀 Ejecutando test GRANDE para m=15, t=100 (BCH 32767, ...)...\n"
	@./$(TARGET) 15 100

# 3. Test de error (Para comprobar que el control de excepciones funciona)
run-error: all
	@echo "🚀 Ejecutando test de error intencionado (m=4, t=9)...\n"
	@./$(TARGET) 4 9

# 4. Ejecución personalizada (Pasando variables desde la terminal)
# Uso: make run-custom m=5 t=2
run-custom: all
	@echo "🚀 Ejecutando test personalizado (m=$(m), t=$(t))...\n"
	@./$(TARGET) $(m) $(t)

# Menú de ayuda para no olvidarte de los comandos
help:
	@echo "========================================="
	@echo "   Comandos disponibles en el Makefile   "
	@echo "========================================="
	@echo "make           - Compila el proyecto"
	@echo "make clean     - Borra los archivos compilados"
	@echo "make run       - Ejecuta el test por defecto"
	@echo "make run-m4    - Ejecuta BCH(15, 7)"
	@echo "make run-m5    - Ejecuta BCH(31, 16)"
	@echo "make run-m6    - Ejecuta BCH(63, 39)"
	@echo "make run-m7    - Ejecuta BCH(127, ...)"
	@echo "make run-m8    - Ejecuta BCH(255, ...)"
	@echo "make run-m15   - Ejecuta BCH grande (32767, ...)"
	@echo "make run-error - Fuerza un error (t demasiado grande)"
	@echo "make run-custom m=X t=Y - Ejecuta con tus propios valores"
	@echo "========================================="

-include $(DEPS)