# DO NOT CHANGE TS. USE A MAKEFILE THE WAY YOU'RE SUPPOSED TO.
# DO NOT INCLUDE SHIT YOU DON'T NEED TO.
# DO NOT, FOR FUCKS SAKE, INCLUDE A FUCKING IMPLEMENTATION. I GOT TERRIBLE REDEFINITION ERRORS.

CXX = g++

CXXFLAGS = -Wall -Wextra -Wswitch -ggdb -I./ $(shell llvm-config --cxxflags)

TARGET = compiler

OBJS = lexer.o parser.o codegen.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(CXXFLAGS) $(shell llvm-config --ldflags --system-libs --libs core native)

lexer.o: lexer.cpp lexer.hpp
	$(CXX) -c lexer.cpp $(CXXFLAGS)

parser.o: parser.cpp parser.hpp lexer.hpp
	$(CXX) -c parser.cpp $(CXXFLAGS)

codegen.o: codegen.cpp parser.hpp
	$(CXX) -c codegen.cpp $(CXXFLAGS)

clean:
	rm -rf $(TARGET) $(OBJS)

.PHONY: clean all
