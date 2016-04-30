OBJS=lexer.o main.o parser.o

all: kaleidoscope

clean:
	rm -f $(OBJS) parser.c parser.h lexer.c

kaleidoscope: $(OBJS)
	gcc $(OBJS) -o $<

%.o: %.c
	gcc -c $< -o $@

%.c: %.y
	yacc -d -o $@ $<

%.c: %.l
	lex -o $@ $<

lexer.o: parser.h
parser.h: parser.c

