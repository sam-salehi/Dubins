CXX      := g++
CXXFLAGS := -std=c++17 -Wall
SFML     := $(shell pkg-config --cflags --libs sfml-graphics 2>/dev/null)

ifeq ($(SFML),)
  SFML_PREFIX := $(shell brew --prefix sfml 2>/dev/null)
  ifneq ($(SFML_PREFIX),)
    SFML := -I$(SFML_PREFIX)/include -L$(SFML_PREFIX)/lib -lsfml-graphics -lsfml-window -lsfml-system
  else
    SFML := -I/opt/homebrew/opt/sfml/include -L/opt/homebrew/opt/sfml/lib -lsfml-graphics -lsfml-window -lsfml-system
  endif
endif

SRCS := main.cpp dublin.cpp dubins_paths.cpp
TARGET := viz

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(SFML)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
