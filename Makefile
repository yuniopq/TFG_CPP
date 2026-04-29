# --- Variables ---
CXX      = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude
TARGET   = bch_test

# --- RUTAS ACTUALIZADAS ---
# main.cpp está en src/
# Los demás están en src/core/
SRCS = src/main.cpp \
       src/core/GaloisField.cpp \
       src/core/Polynomial.cpp \
       src/core/BCH_Codec.cpp

OBJS = $(SRCS:.cpp=.o)

# --- Reglas ---
all: $(TARGET)
	@echo "✅ Proyecto compilado."

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Esta regla es clave: permite compilar .cpp en subcarpetas
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
	@echo "🧹 Limpio."

run: all
	./$(TARGET)