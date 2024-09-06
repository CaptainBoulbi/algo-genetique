#include <stdlib.h>
#include <stdio.h>

#define MAPAT(map, x, y) ((map).poids[(x) + (y)*((map).len)])

typedef struct Map {
    float *poids;
    // char *names;
    int len;
} Map;

void map_print(Map map)
{
    int len = map.len - 1;
    printf("[\n    [ ");
    for (int y=0; y<len; y++) {
        for (int x=0; x<len; x++) {
            printf("%.3f, ", MAPAT(map, x, y));
        }
        printf("%.3f ],\n    [ ", MAPAT(map, len, y));
    }
    for (int x=0; x<len; x++) {
        printf("%.3f, ", MAPAT(map, x, len));
    }
    printf("%.3f ],\n]\n", MAPAT(map, len, len));
}

int main()
{
    Map map;
    map.len = 5;
    map.poids = malloc(sizeof(*map.poids) * map.len * map.len);

    MAPAT(map, 0, 0) = 1;
    MAPAT(map, 1, 0) = 2;
    MAPAT(map, 2, 0) = 3;
    MAPAT(map, 3, 0) = 4;
    MAPAT(map, 4, 0) = 5;
    MAPAT(map, 0, 1) = 6;
    MAPAT(map, 1, 1) = 7;
    MAPAT(map, 2, 1) = 8;
    MAPAT(map, 3, 1) = 9;
    MAPAT(map, 4, 1) = 10;
    MAPAT(map, 0, 2) = 11;
    MAPAT(map, 1, 2) = 12;
    MAPAT(map, 2, 2) = 13;
    MAPAT(map, 3, 2) = 14;
    MAPAT(map, 4, 2) = 15;
    MAPAT(map, 0, 3) = 16;
    MAPAT(map, 1, 3) = 17;
    MAPAT(map, 2, 3) = 18;
    MAPAT(map, 3, 3) = 19;
    MAPAT(map, 4, 3) = 20;
    MAPAT(map, 0, 4) = 21;
    MAPAT(map, 1, 4) = 22;
    MAPAT(map, 2, 4) = 23;
    MAPAT(map, 3, 4) = 24;
    MAPAT(map, 4, 4) = 25;

    map_print(map);

    return 0;
}
