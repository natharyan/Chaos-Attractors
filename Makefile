.PHONY: all compile clean

# Path to SFML (override with SFML_PATH variable if needed)
SFML_PATH ?= /opt/homebrew/Cellar/sfml/3.0.1

# Find source files (keep simple: main + include implementations)
cppFileNames := $(shell find ./src -maxdepth 1 -type f -name "main.cpp")

all: compile

compile:
	mkdir -p bin

	# Clear LIBRARY_PATH and CPLUS_INCLUDE_PATH to avoid stale system search paths
	LIBRARY_PATH= CPLUS_INCLUDE_PATH= g++ -std=c++17 $(cppFileNames) ./src/includes/audio_player.cpp ./src/includes/visualization.cpp ./src/includes/matrix.cpp ./src/includes/attractors/lorenz.cpp ./src/includes/attractors/aizawa.cpp ./src/includes/attractors/thomas.cpp ./src/includes/attractors/halvorsen.cpp ./src/includes/attractors/sprott.cpp -I$(SFML_PATH)/include -o bin/app -L$(SFML_PATH)/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lsfml-network

run: compile
	./bin/app

clean:
	rm -rf bin
