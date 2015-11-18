fat: fat.c
	gcc -o fat.exe fat.c

clean:
	rm -vf *.exe *.o
