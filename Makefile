fat: fat.c
	gcc -w -std=c99 -o fat.exe fat.c -g

clean:
	rm -vf *.exe *.o
