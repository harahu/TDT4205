CFLAGS=-std=gnu99
LDLIBS=-lm
pencil: pencil.c scanner.c
test: pencil
	cat triangle.test | ./pencil | ps2pdf - > triangle.pdf
clean:
	-rm -f pencil triangle.pdf
