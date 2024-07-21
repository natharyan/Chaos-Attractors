# Replace this with the path you get from `brew info sfml`
SFML_PATH = /opt/homebrew/Cellar/sfml/2.6.1

# Replace "src" with the name of the folder where all your cpp code is
cppFileNames := $(shell find . -maxdepth 1 -type f -name "*.cpp")

all: compile

compile:	
	mkdir -p bin
	g++ -std=c++14 $(cppFileNames) -I$(SFML_PATH)/include -o bin/app -L$(SFML_PATH)/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lsfml-network