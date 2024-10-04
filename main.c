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
#define MUTATION_POURCENTAGE 25

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

// map distance valide si la matrice est simétrique car la distance de la ville
// a -> b == b -> a
// et que la diagonal est a 0 car a -> a = 0
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

// map coord valide si les coordonée sont entre 0 et 1
int map_coord_valide(MapCoord map)
{
    for (int i=0; i<map.len; i++) {
        if (map.coord[i].x < 0.0 || map.coord[i].x > 1.0 || map.coord[i].y < 0.0 || map.coord[i].y > 1.0)
            return 0;
    }
    return 1;
}

// convertit les coordonnées des villes en matrice de distance entre les villes
// [ (0, 0), (1, 1) ]
// est convertit en
// [ 0.0, 1.4 ]
// [ 1.4, 0.0 ]
MapDistance map_coord_to_map_distance(MapCoord mapc)
{
    MapDistance mapd;
    mapd.len = mapc.len;
    mapd.poids = calloc(sizeof(*mapd.poids), mapd.len * mapd.len);

    // recupere seulement les valeur pour calculer la partir haute de la matrice
    // et copie la valeur pour la parti basse, vue qu'elle doit etre symétrique
    for (int y = 0; y < mapc.len; y++) {
        for (int x = y+1; x < mapc.len; x++) {
            float distance = Vector2Distance(mapc.coord[x], mapc.coord[y]);
            // copie symétriquement
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

// genere un individu avec un chemin aléatoire qui passe par toute les villes une fois
void individu_generate(int* ind, int len)
{
    // le chemin de l'individu sera [ 0, 1, 2, 3, ... ]
    // donc il possede toute les villes une fois
    for (int i=0; i<len; ++i) {
        ind[i]=i;
    }

    // puis on melange l'ordre des villes
    for (int i=0; i<len; ++i) {
        int index = GetRandomValue(0, len-1);
        int swap = ind[index];
        ind[index] = ind[i];
        ind[i] = swap;
    }
}

// verifie que le chemin d'un individu ne possede pas plusieurs fois la meme ville
int individu_valide(int* ind, int len)
{
    for (int y = 0; y < len; ++y) {
        for (int i = y+1; i < len; ++i) {
            if (ind[y] == ind[i])
                return 0;
        }
    }

    return 1;
}

// retourne la performance d'un individu qui est égale a la distance total de sont chemin
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

// corige un individu en remplaçant un doublon par une ville manquante
void individu_fix(int *ind, int len)
{
    int ex[len];
    memset(ex, 0, len * sizeof(*ex));

    // liste combien de fois une ville existe sur le chemin
    // ex = [ 0, 1, 1, 2 ] la ville 3 est en double et la 0 n'est pas sur le chemin
    // doit etre égale ex = [ 1, 1, 1, 1 ]
    for (int i = 0; i < len; i++) {
        ex[ind[i]] += 1;
    }

    do {
        for (int i = 0; i < len; i++) {
            if (ex[i] != 2)
                continue;
            // a partir d'ici i est une ville qui existe 2 fois
            for (int y = 0; y < len; y++) {
                if (ex[y] != 0)
                    continue;
                // a partir d'ici y est une ville qui n'est pas sur le chemin
                for (int j = 0; j < len; j++) {
                    if (ind[j] != i)
                        continue;
                    // a partir d'ici j est l'emplacement a l'aquelle le doublon ce trouve
                    // donc a l'emplacement de j on replace le doublon i par la ville y
                    ind[j] = y;
                    ex[i]++;
                    ex[y]--;
                    break;
                }
                break;
            }
            break;
        }
        // on repete jusqu'a ce qu'il n'ya plus de doublon et que toute les villes sont parcouru par le chemin
    } while (!individu_valide(ind, len));
}

// reproduit 2 individu séléctionner par l'evolution et en genere 2 enfant
// on les reproduit en prenant une parti de l'un et l'ajoute a l'autre
void reproduce(int *fort1, int *fort2, int *faible1, int *faible2, int len)
{
    // idx1 et idx2 sont les limite de la parti que nous allons prendre
    int idx1 = GetRandomValue(0, len-1);
    int idx2 = GetRandomValue(0, len-1);
    int swap = idx2;
    idx2 = MAX(idx1, idx2);
    idx1 = MIN(idx1, swap);

    // copie les fort ver les faible
    for (int i = 0; i < idx1 ; i++) {
        faible1[i] = fort1[i];
        faible2[i] = fort2[i];
    }
    // copie les fort ver les faible de maniere inverser
    for (int i = idx1; i < idx2; i++) {
        faible2[i] = fort1[i];
        faible1[i] = fort2[i];
    }
    // puis copie la suite de la meme manier que la 1er fois
    for (int i = idx2; i < len ; i++) {
        faible1[i] = fort1[i];
        faible2[i] = fort2[i];
    }

    // on repare les chemin en remplacant les doublon par les ville inexistante
    individu_fix(faible1, len);
    individu_fix(faible2, len);

}

// la mutation d'un individu ce fait en echanger 2 ville aléatoir sur son chemin
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

void population_generate(int *pop, int len)
{
    for (int i = 0; i < NB_INDIVIDUS; ++i) {
        individu_generate(INDIVIDU(pop, i, len), len);
        if (!individu_valide(INDIVIDU(pop, i, len), len))
            printf("Individu non valide\n");
    }
}

void population_performance(float *perf, int *pop, MapDistance mapd)
{
    for (int i = 0; i < NB_INDIVIDUS; ++i) {
        perf[i] = individu_performance(INDIVIDU(pop, i, mapd.len), mapd.len, mapd);
    }
}

// tri la population du plus performant vert le moins performant
// tri a bulle
void population_tri(float *perf, int *pop, int len)
{
    for (int i = 0; i < NB_INDIVIDUS - 1; i++) {
        for (int y = 0; y < NB_INDIVIDUS - i - 1; y++) {
            if (perf[y] > perf[y+1]) {
                // deplace la performance
                float swap = perf[y];
                perf[y] = perf[y+1];
                perf[y+1] = swap;
                // puis deplace l'individu
                for (int j = 0; j < len; j++) {
                    int swap = INDIVIDU(pop, y, len)[j];
                    INDIVIDU(pop, y, len)[j] = INDIVIDU(pop, y+1, len)[j];
                    INDIVIDU(pop, y+1, len)[j] = swap;
                }
            }
        }
    }
}

// l'evolution ce passe en prenant la meilleur moitier et en les reproduire entre eu
// il y'a une chance que l'enfant fasse une mutation
void population_evolution(int *pop, int len)
{
    for (int i = 0; i < NB_INDIVIDUS/4; i+=2) {
        int offset = NB_INDIVIDUS/2;
        int *fort1 = INDIVIDU(pop, i, len);
        int *fort2 = INDIVIDU(pop, i+1, len);
        int *faible1 = INDIVIDU(pop, offset+i, len);
        int *faible2 = INDIVIDU(pop, offset+i+1, len);
        reproduce(fort1, fort2, faible1, faible2, len);
        if (GetRandomValue(0, 100) < MUTATION_POURCENTAGE)
            mutate(faible1, len);
        if (GetRandomValue(0, 100) < MUTATION_POURCENTAGE)
            mutate(faible2, len);
    }
}

int main()
{
    // setup
    SetRandomSeed(time(0));
    MapCoord mapc;
    mapc.len = 5;
    mapc.coord = calloc(sizeof(*mapc.coord), mapc.len);

    // definie les coordonée des ville doit etre entre 0 et 1
    mapc.coord[0] = (Vector2) {0.12,  0.96};
    mapc.coord[1] = (Vector2) {0.69,  0.12};
    mapc.coord[2] = (Vector2) {0.23,  0.36};
    mapc.coord[3] = (Vector2) {0.42,  0.96};
    mapc.coord[4] = (Vector2) {0.55,  0.02};

    // affiche les coordonée + verifie si elle sont correcte
    printf("Coordonée ville :\n");
    map_coord_print(mapc);
    if (!map_coord_valide(mapc)) {
        printf("mapc not valide\n");
        return 0;
    }

    // convertit les coordonée des villes en matrices de distance entre les villes, exemple :
    // [ 0.0, 0.8, 0.3 ]
    // [ 0.8, 0.0, 0.4 ]
    // [ 0.3, 0.4, 0.0 ]
    MapDistance mapd = map_coord_to_map_distance(mapc);

    // affiche la matric de distance et verifie si elle est correcte
    printf("Matrice distance entre ville :\n");
    map_distance_print(mapd);
    if (!map_distance_valide(mapd)) {
        printf("mapd not valide\n");
        return 0;
    }

    int *pop = calloc(sizeof(*pop), mapd.len*NB_INDIVIDUS);

    // genere la population aléatoirement
    population_generate(pop, mapd.len);

    float perf[NB_INDIVIDUS];

    // boucle évolution
    for (int c = 0; c < 50000; c++) {
        // calcul performance
        population_performance(perf, pop, mapd);
        // tri en fonction de performance
        population_tri(perf, pop, mapd.len);
        // fait évoluer en reproduisant les meilleurs
        population_evolution(pop, mapd.len);
    }

    // au bout de plusieurs itération le chemin le plus court est l'individu 0
    printf("Le chemin le plus court :\n");
    individu_print(INDIVIDU(pop, 0, mapd.len), mapd.len);
    printf("Avec une performance de %.3f.\n", perf[0]);

    return 0;
}
