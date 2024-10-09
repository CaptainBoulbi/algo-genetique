all:
	clang -Wall -Wextra main.c -o algo-genetique ./lib/libraylib.a -I./lib -lm

release:
	clang -Wall -Wextra -O3 main.c -o algo-genetique ./lib/libraylib.a -I./lib -lm

run: all
	./algo-genetique

win:
	x86_64-w64-mingw32-gcc -Wall -Wextra -O3 main.c -o algo-genetique ./lib/win-libraylib.a -I./lib -lm -lgdi32 -lwinmm

clean:
	rm -f algo-genetique algo-genetique.exe
