#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "raylib.h"
#include "raymath.h"

#define MAPDISTAT(map, x, y) ((map).poids[(x) + (y)*((map).len)])

#define INDIVIDU(pop, i, l) ((pop) + sizeof(*(pop)) * (l) * (i))

#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define NB_INDIVIDUS 32

#define MUTATION_POURCENTAGE 80

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
        int y = (i+1)%len;
        perf += MAPDISTAT(map, ind[i], ind[y]);
    }
    return perf;
}

void individu_fix(int *ind, int len)
{
    int ex[len];
    memset(ex, 0, len * sizeof(*ex));

    for (int i = 0; i < len; i++) {
        ex[ind[i]] += 1;
    }

    do {
        for (int i = 0; i < len; i++) {
            if (ex[i] != 2)
                continue;
            for (int y = 0; y < len; y++) {
                if (ex[y] != 0)
                    continue;
                for (int j = 0; j < len; j++) {
                    if (ind[j] != i)
                        continue;
                    ind[j] = y;
                    ex[i]++;
                    ex[y]--;
                    break;
                }
                break;
            }
            break;
        }
    } while (!individu_valide(ind, len));
}

void reproduce(int *fort1, int *fort2, int *faible1, int *faible2, int len)
{
    int idx1 = GetRandomValue(0, len-1);
    int idx2 = GetRandomValue(0, len-1);
    int swap = idx2;
    idx2 = MAX(idx1, idx2);
    idx1 = MIN(idx1, swap);

    for (int i = 0; i < idx1 ; i++) {
        faible1[i] = fort1[i];
        faible2[i] = fort2[i];
    }
    for (int i = idx1; i < idx2; i++) {
        faible2[i] = fort1[i];
        faible1[i] = fort2[i];
    }
    for (int i = idx2; i < len ; i++) {
        faible1[i] = fort1[i];
        faible2[i] = fort2[i];
    }

    individu_fix(faible1, len);
    individu_fix(faible2, len);

}

void mutate(int *ind, int len)
{
    int idx1 = GetRandomValue(0, len-1);
    int idx2;
    do {
        idx2 = GetRandomValue(0, len-1);
    } while (idx1 == idx2);
    int swap = ind[idx1];
    ind[idx1] = ind[idx2];
    ind[idx2] = swap;
}

int main()
{
    SetRandomSeed(time(0));
    MapCoord mapc;
    mapc.len = 5;
    mapc.coord = calloc(sizeof(*mapc.coord), mapc.len);

    mapc.coord[0] = (Vector2) {0.12,  0.96};
    mapc.coord[1] = (Vector2) {0.69,  0.12};
    mapc.coord[2] = (Vector2) {0.23,  0.36};
    mapc.coord[3] = (Vector2) {0.42,  0.96};
    mapc.coord[4] = (Vector2) {0.55,  0.02};

    printf("Coordonée ville :\n");
    map_coord_print(mapc);

    if (!map_coord_valide(mapc)) {
        printf("mapc not valide\n");
        return 0;
    }

    printf("Matrice distance entre ville :\n");
    MapDistance mapd = map_coord_to_map_distance(mapc);
    map_distance_print(mapd);

    if (!map_distance_valide(mapd)) {
        printf("mapd not valide\n");
        return 0;
    }

    int *pop = calloc(sizeof(*pop), mapd.len*NB_INDIVIDUS);

    printf("Population aléatoirs :\n");
    for (int i = 0; i < NB_INDIVIDUS; ++i)
    {
        individu_generate(INDIVIDU(pop, i, mapd.len), mapd.len);
        // individu_print(INDIVIDU(pop, i, mapd.len), mapd.len);
        
        if (!individu_valide(INDIVIDU(pop, i, mapd.len), mapd.len))
            printf("Individu non valide\n");
    }

    float perf[NB_INDIVIDUS];

    for (int c = 0; c < 50000; c++) {

    for (int i = 0; i < NB_INDIVIDUS; ++i) {
        perf[i] = individu_performance(INDIVIDU(pop, i, mapd.len), mapd.len, mapd);
    }

    for (int i = 0; i < NB_INDIVIDUS - 1; i++) {
        for (int y = 0; y < NB_INDIVIDUS - i - 1; y++) {
            if (perf[y] > perf[y+1]) {
                float swap = perf[y];
                perf[y] = perf[y+1];
                perf[y+1] = swap;
                for (int j = 0; j < mapd.len; j++) {
                    int swap = INDIVIDU(pop, y, mapd.len)[j];
                    INDIVIDU(pop, y, mapd.len)[j] = INDIVIDU(pop, y+1, mapd.len)[j];
                    INDIVIDU(pop, y+1, mapd.len)[j] = swap;
                }
            }
        }
    }

    for (int i = 0; i < NB_INDIVIDUS/4; i+=2) {
        int offset = NB_INDIVIDUS/2;
        int *fort1 = INDIVIDU(pop, i, mapd.len);
        int *fort2 = INDIVIDU(pop, i+1, mapd.len);
        int *faible1 = INDIVIDU(pop, offset+i, mapd.len);
        int *faible2 = INDIVIDU(pop, offset+i+1, mapd.len);
        reproduce(fort1, fort2, faible1, faible2, mapd.len);
        if (GetRandomValue(0, 100) < MUTATION_POURCENTAGE)
            mutate(faible1, mapd.len);
        if (GetRandomValue(0, 100) < MUTATION_POURCENTAGE)
            mutate(faible2, mapd.len);
    }

    }

    printf("Le chemin le plus court :\n[ ");
    for (int i = 0; i < mapd.len-1; i++) {
        printf("%d, ", INDIVIDU(pop, 0, mapd.len)[i]);
    }
    printf("%d ]\n", INDIVIDU(pop, 0, mapd.len)[mapd.len-1]);
    printf("Avec une performance de %.3f.\n", perf[0]);

    return 0;
}
