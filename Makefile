# Replace this with the path you get from `brew info sfml`
SFML_PATH = /opt/homebrew/Cellar/sfml/2.6.1

# Replace this with the path you get from `brew info rtaudio`
RTAUDIO_PATH = /opt/homebrew/Cellar/rtaudio/6.0.1

# Replace this with the path you get from `brew info fftw`
FFTW_PATH = /opt/homebrew/Cellar/fftw/3.3.10_1

# Replace "src" with the name of the folder where all your cpp code is
# cppFileNames := $(shell find . -maxdepth 1 -type f -name "*.cpp")
cppFileNames := $(shell find . -maxdepth 1 -type f -name "main.cpp")

all: compile

compile:	
	mkdir -p bin
	g++ -std=c++14 $(cppFileNames) ./helpers/matrix.cpp -I$(SFML_PATH)/include -I$(RTAUDIO_PATH)/include/rtaudio -I$(FFTW_PATH)/include -o bin/app -L$(SFML_PATH)/lib -L$(RTAUDIO_PATH)/lib -L$(FFTW_PATH)/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lsfml-network -lrtaudio -lfftw3 -lfftw3f