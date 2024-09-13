all:
	clang -Wall -Wextra main.c -o algo-genetique -lraylib

run: all
	./algo-genetique

win:
	x86_64-w64-mingw32-gcc -Wall -Wextra main.c -o algo-genetique -L/usr/local/lib -I/usr/local/include -lraylib-win -lgdi32 -lwinmm
