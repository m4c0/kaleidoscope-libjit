OBJS=lexer.o main.o parser.o

all: kaleidoscope

clean:
	rm -f $(OBJS) parser.cpp parser.h lexer.c

kaleidoscope: $(OBJS)
	gcc $(OBJS) -o $@ -lstdc++

%.o: %.c
	gcc -c $< -o $@

%.o: %.cpp main.hpp
	g++ -c $< -o $@

%.cpp: %.y main.hpp
	yacc -d -o $@ $<

%.c: %.l
	lex -o $@ $<

lexer.o: parser.h
parser.h: parser.cpp

