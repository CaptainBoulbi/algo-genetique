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

int map_valide(Map map)
{
    for (int y=0; y<map.len; y++) {
        for (int x=0; x<map.len; x++) {
            if (MAPAT(map, x, y) != MAPAT(map, y, x))
                return 0;
            if (x == y && MAPAT(map, x, y) != -1)
                return 0;
        }
    }
    return 1;
}

int main()
{
    Map map;
    map.len = 5;
    map.poids = malloc(sizeof(*map.poids) * map.len * map.len);

    MAPAT(map, 0, 0) = -1;
    MAPAT(map, 1, 0) = 5;
    MAPAT(map, 2, 0) = 7;
    MAPAT(map, 3, 0) = 3;
    MAPAT(map, 4, 0) = 5;
    MAPAT(map, 0, 1) = 5;
    MAPAT(map, 1, 1) = -1;
    MAPAT(map, 2, 1) = -1;
    MAPAT(map, 3, 1) = -1;
    MAPAT(map, 4, 1) = 9;
    MAPAT(map, 0, 2) = 7;
    MAPAT(map, 1, 2) = -1;
    MAPAT(map, 2, 2) = -1;
    MAPAT(map, 3, 2) = 2;
    MAPAT(map, 4, 2) = -1;
    MAPAT(map, 0, 3) = 3;
    MAPAT(map, 1, 3) = -1;
    MAPAT(map, 2, 3) = 2;
    MAPAT(map, 3, 3) = -1;
    MAPAT(map, 4, 3) = 1;
    MAPAT(map, 0, 4) = 5;
    MAPAT(map, 1, 4) = 9;
    MAPAT(map, 2, 4) = -1;
    MAPAT(map, 3, 4) = 1;
    MAPAT(map, 4, 4) = -1;

    map_print(map);
    if (!map_valide(map))
        printf("map not valide\n");

    return 0;
}
