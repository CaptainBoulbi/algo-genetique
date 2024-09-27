all:
	clang -Wall -Wextra main.c -o algo-genetique /usr/local/lib/libraylib.a -lm

release:
	clang -Wall -Wextra -O3 main.c -o algo-genetique /usr/local/lib/libraylib.a -lm

run: all
	./algo-genetique

win:
	x86_64-w64-mingw32-gcc -Wall -Wextra -O3 main.c -o algo-genetique /usr/local/lib/win-libraylib.a -I /usr/local/include -lm -lgdi32 -lwinmm

clean:
	rm -f algo-genetique algo-genetique.exe
