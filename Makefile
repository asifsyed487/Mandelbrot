all: mandel mandelmovie run

mandel: mandel.o bitmap.o
	gcc mandel.o bitmap.o -o mandel -lpthread

mandel.o: mandel.c
	gcc -Wall -g -c mandel.c -o mandel.o

bitmap.o: bitmap.c
	gcc -Wall -g -c bitmap.c -o bitmap.o

mandelmovie: mandelmovie.c
	gcc mandelmovie.c -o mandelmovie -lm

run: mandelmovie
	./mandelmovie -n 10 -p 5
        

clean:
	rm -f mandel.o bitmap.o mandel

