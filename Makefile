.PHONY: clean all

all: main

main: main.cpp
	g++ -Wall -Wextra -ggdb parser.cpp -o parser -I./

clean:
	rm -rf main
