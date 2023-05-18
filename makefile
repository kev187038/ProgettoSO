CC=gcc
LIBS=-lglut -lGLU -lGL -lm
INCLUDES=-I/usr/include/GL
CCOPTS=	-Wall -03	-std=gnu99	$(INCLUDES)
main.o:	main.c	h_files/mmu.h	h_files/utils.h
	gcc	-std=gnu99	-o	main	main.c	functions/utils.c

clean:
	rm -f main
