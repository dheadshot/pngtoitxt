pngtoitxt: main.o pngfuncs.o
	gcc -o pngtoitxt main.o pngfuncs.o -lpng
	chmod +x pngtoitxt

main.o: main.c main.h pngfuncs.h
	gcc ${CFLAGS} -c main.c

pngfuncs.o: pngfuncs.c pngfuncs.h
	gcc ${CFLAGS} -c pngfuncs.c
