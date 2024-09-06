all:
	clang main.c -o algo-genetique -lraylib

run: all
	./algo-genetique
