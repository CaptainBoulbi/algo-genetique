#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "raylib.h"
#include "raymath.h"

#define MAPDISTAT(map, x, y) ((map).poids[(x) + (y)*((map).len)])

#define INDIVIDU(pop, i, l) ((pop) + sizeof(*(pop)) * (l) * (i))

#define NB_INDIVIDUS 30

typedef struct MapCoord {
    Vector2 *coord;
    int len;
} MapCoord;

typedef struct MapDistance {
    float *poids;
    int len;
} MapDistance;

void map_distance_print(MapDistance map)
{
    int len = map.len - 1;
    printf("[\n    [ ");
    for (int y=0; y<len; y++) {
        for (int x=0; x<len; x++) {
            printf("%.3f, ", MAPDISTAT(map, x, y));
        }
        printf("%.3f ],\n    [ ", MAPDISTAT(map, len, y));
    }
    for (int x=0; x<len; x++) {
        printf("%.3f, ", MAPDISTAT(map, x, len));
    }
    printf("%.3f ],\n]\n", MAPDISTAT(map, len, len));
}

int map_distance_valide(MapDistance map)
{
    for (int y=0; y<map.len; y++) {
        for (int x=0; x<map.len; x++) {
            if (MAPDISTAT(map, x, y) != MAPDISTAT(map, y, x))
                return 0;
            if (x == y && MAPDISTAT(map, x, y) != 0)
                return 0;
        }
    }
    return 1;
}

void map_coord_print(MapCoord map)
{
    printf("[ ");
    for (int i=0; i<map.len-1; i++) {
        printf("(%.3f, %.3f), ", map.coord[i].x, map.coord[i].y);
    }
    printf("(%.3f, %.3f) ]\n", map.coord[map.len-1].x, map.coord[map.len-1].y);
}

int map_coord_valide(MapCoord map)
{
    for (int i=0; i<map.len; i++) {
        if (map.coord[i].x < 0.0 || map.coord[i].x > 1.0 || map.coord[i].y < 0.0 || map.coord[i].y > 1.0)
            return 0;
    }
    return 1;
}

MapDistance map_coord_to_map_distance(MapCoord mapc)
{
    MapDistance mapd;
    mapd.len = mapc.len;
    mapd.poids = calloc(sizeof(*mapd.poids), mapd.len * mapd.len);

    for (int y = 0; y < mapc.len; y++) {            
        for (int x = y+1; x < mapc.len; x++) {
            float distance = Vector2Distance(mapc.coord[x], mapc.coord[y]);
            MAPDISTAT(mapd, x, y) = distance;
            MAPDISTAT(mapd, y, x) = distance;
        }
    }
    return mapd;
}

void individu_print(int* ind, int len)
{
    printf("[ ");
    for (int i = 0; i < len-1; ++i)
    {
        printf("%d, ", ind[i]);
    }
    printf("%d ]\n", ind[len-1]);
}

void individu_generate(int* ind, int len)
{
    for (int i=0; i<len; ++i)
    {
        ind[i]=i;
    }

    for (int i=0; i<len; ++i)
    {
        int index = GetRandomValue(0, len-1);
        int swap = ind[index];
        ind[index] = ind[i];
        ind[i] = swap;
    }
}

int individu_valide(int* ind, int len)
{
    for (int y = 0; y < len; ++y)
    {
        for (int i = y+1; i < len; ++i)
        {
            if (ind[y] == ind[i])
                return 0;
        }
    }

    return 1;
}

float individu_performance(int* ind, int len, MapDistance map)
{
    float perf = 0.0f;
    for (int i = 0; i < len; ++i)
    {
        //perf += qlq chose;
    }
}

int main()
{
    SetRandomSeed(time(0));
    MapCoord mapc;
    mapc.len = 5;
    mapc.coord = calloc(sizeof(*mapc.coord), mapc.len);

    mapc.coord[0] = (Vector2) {0.12,  0.69};
    mapc.coord[1] = (Vector2) {0.1,   0.69};
    mapc.coord[2] = (Vector2) {0.2,   0.69};
    mapc.coord[3] = (Vector2) {0.42,  0.69};
    mapc.coord[4] = (Vector2) {0.4,   0.69};

    map_coord_print(mapc);

    if (!map_coord_valide(mapc)) {
        printf("mapc not valide\n");
        return 0;
    }

    MapDistance mapd = map_coord_to_map_distance(mapc);
    map_distance_print(mapd);

    if (!map_distance_valide(mapd)) {
        printf("mapd not valide\n");
        return 0;
    }

    int *pop = calloc(sizeof(*pop), mapd.len*NB_INDIVIDUS);

    for (int i = 0; i < NB_INDIVIDUS; ++i)
    {
        individu_generate(INDIVIDU(pop, i, mapd.len), mapd.len);
        individu_print(INDIVIDU(pop, i, mapd.len), mapd.len);
        
        if (!individu_valide(INDIVIDU(pop, i, mapd.len), mapd.len))
            printf("Individu non valide\n");
    }

    float perf[NB_INDIVIDUS];

    for (int i = 0; i < NB_INDIVIDUS; ++i)
     {
         perf[i] = individu_performance(INDIVIDU(pop, i, mapd.len), mapd.len, mapd);
     } 

    return 0;
}