OBJS=lexer.o main.o parser.o

all: kaleidoscope

clean:
	rm -f $(OBJS) parser.cpp parser.hpp lexer.c

kaleidoscope: $(OBJS)
	gcc $(OBJS) -o $@ -lstdc++ -ljit

%.o: %.c
	gcc -g -c $< -o $@

%.o: %.cpp main.hpp
	g++ -g -std=c++11 -c $< -o $@

%.cpp: %.y main.hpp
	yacc -d -o $@ $<

%.c: %.l
	lex -o $@ $<

lexer.o: parser.hpp
parser.hpp: parser.cpp

