# DO NOT CHANGE TS. USE A MAKEFILE THE WAY YOU'RE SUPPOSED TO.
# DO NOT INCLUDE SHIT YOU DON'T NEED TO.
# DO NOT, FOR FUCKS SAKE, INCLUDE A FUCKING IMPLEMENTATION. I GOT TERRIBLE REDEFINITION ERRORS.

CXX = g++

CXXFLAGS = -Wall -Wextra -Wswitch -ggdb -I./ 

TARGET = compiler

OBJS = lexer.o parser.o codegen.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(CXXFLAGS)

lexer.o: lexer.cpp lexer.h
	$(CXX) -c lexer.cpp $(CXXFLAGS)

parser.o: parser.cpp parser.hpp lexer.h
	$(CXX) -c parser.cpp $(CXXFLAGS)

codegen.o: codegen.cpp parser.hpp
	$(CXX) -c codegen.cpp $(CXXFLAGS)

clean:
	rm -rf $(TARGET) $(OBJS)

.PHONY: clean all
