# --- Terminal colors ---
CYAN  = \033[0;36m
GREEN = \033[0;32m
GOLD  = \033[0;33m
RESET = \033[0m

# --- Build variables ---
CXX      = g++
# -O3 and -march=native are key for polynomial logic performance
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude -MMD -MP -O3 -march=native -DNDEBUG -fopenmp
TARGET   = bch_sim

# --- Directories ---
SRC_DIR   = src
BUILD_DIR = build
RES_DIR   = results

# --- Automatic source discovery ---
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

# --- Default run values ---
M    ?= 7
T    ?= 10
MIN  ?= 0
MAX  ?= 10
STEP ?= 1
FILE ?= # Empty by default

# --- Main Rules ---
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

# --- Smart Run Menu ---

# General run: uses defaults or values passed on the command line
# Example: make run M=4 T=2 FILE=image.bmp
run: all
	@echo "$(CYAN)🚀 Starting BCH simulation...$(RESET)"
	@./$(TARGET) $(M) $(T) $(MIN) $(MAX) $(STEP) $(FILE)

# Build cleanup without deleting generated results
clean:
	@echo "$(CYAN)🧹 Cleaning temporary files...$(RESET)"
	@rm -rf $(BUILD_DIR) $(TARGET)
	@echo "$(GREEN)Done.$(RESET)"

help:
	@echo "$(CYAN)==========================================================$(RESET)"
	@echo "   BCH Simulation Commands (TFG)         "
	@echo "$(CYAN)==========================================================$(RESET)"
	@echo "$(GOLD)make$(RESET)                	- Build the simulator"
	@echo "$(GOLD)make run$(RESET)            	- Run with M=7 T=10"
	@echo "$(GOLD)make run M=4 T=2$(RESET)    	- Run BCH(15,7)"
	@echo "$(GOLD)make run FILE=img.bmp$(RESET)	- Process a real file"
	@echo "$(GOLD)make clean$(RESET)          	- Delete only build and executable"
	@echo "$(CYAN)----------------------------------------------------------$(RESET)"

-include $(DEPS)