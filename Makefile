fat: fat.c
	gcc -std=c99 -o fat.exe fat.c -g

clean:
	rm -vf *.exe *.o

backup: fat.c functions.h Makefile
	git add fat.c functions.h Makefile
	git commit -m "Additional work on functionality."
	git push