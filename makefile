SOURCE = $(wildcard src/*.cpp)
OUTFILE = main

all: run

clean:
	rm -f $(OUTFILE)

build: clean
	clear && g++ -std=c++17 -g $(SOURCE) -o $(OUTFILE)

run: build
	clear && ./$(OUTFILE)