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
#define CLAMP(val, min, max) (MAX(MIN((val), (max)), (min)))

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

// map distance valide si la matrice est symétrique car la distance de la ville
// a -> b == b -> a
// et que la diagonale est a 0 car a -> a = 0
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

// convertit les coordonnées des villes en matrice de distances entre les villes
// [ (0, 0), (1, 1) ]
// est convertit en
// [ 0.0, 1.4 ]
// [ 1.4, 0.0 ]
// calcul de la distance = sqrtf((v1.x - v2.x)*(v1.x - v2.x) + (v1.y - v2.y)*(v1.y - v2.y))
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

// genere un individu avec un chemin aléatoire qui passe une fois par chaque ville
void individu_generate(int* ind, int len)
{
    // le chemin de l'individu sera [ 0, 1, 2, 3, ... ]
    // donc il possede une fois chaque ville
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

// vérifie que le chemin d'un individu ne possede qu'une seule fois chaque ville
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

// retourne la performance d'un individu qui est égale à la distance totale de son chemin
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

// corrige un individu en remplaçant un doublon par une ville manquante
void individu_fix(int *ind, int len)
{
    int ex[len];
    memset(ex, 0, len * sizeof(*ex));

    // liste combien de fois une ville existe sur le chemin
    // ex = [ 0, 1, 1, 2 ] la ville 3 est en double et la 0 n'est pas sur le chemin
    // un chemin juste = [ 1, 1, 1, 1 ]
    for (int i = 0; i < len; i++) {
        ex[ind[i]] += 1;
    }

    do {
        int doublon = 0; // la ville en doublon
        for (doublon = 0; doublon < len && ex[doublon] != 2; doublon++);

        int oublier = 0; // la ville qui n'est pas traversée
        for (oublier = 0; oublier < len && ex[oublier] != 0; oublier++);

        int index = 0; // l'index dans le chemin de la ville en doublon
        for (index = 0; index < len && ind[index] != doublon; index++);

        // on remplace le doublon par la vile qui n'est pas traversée
        ind[index] = oublier;
        ex[doublon]++;
        ex[oublier]--;

        // on repete jusqu'a ce qu'il n'y ait plus de doublons et que toutes les villes soient parcourues par le chemin
    } while (!individu_valide(ind, len));
}

// reproduit 2 individus séléctionnés par l'evolution et genere 2 enfants
// la reproduction se fait par bloc : on prend une partie d'un individu et le reste du deuxième individu
void reproduce(int *fort1, int *fort2, int *faible1, int *faible2, int len)
{
    // idx1 et idx2 sont les limites de la partie que nous allons prendre
    int idx1 = GetRandomValue(0, len-1);
    int idx2 = GetRandomValue(0, len-1);
    int swap = idx2;
    idx2 = MAX(idx1, idx2);
    idx1 = MIN(idx1, swap);

    // copie les forts vers les faibles
    for (int i = 0; i < idx1 ; i++) {
        faible1[i] = fort1[i];
        faible2[i] = fort2[i];
    }
    // copie les forts vers les faibles de maniere inversée
    for (int i = idx1; i < idx2; i++) {
        faible2[i] = fort1[i];
        faible1[i] = fort2[i];
    }
    // puis copie la suite de la meme maniere que la 1ere fois
    for (int i = idx2; i < len ; i++) {
        faible1[i] = fort1[i];
        faible2[i] = fort2[i];
    }

    // on repare les chemin en remplacant les doublons par les villes inexistantes
    individu_fix(faible1, len);
    individu_fix(faible2, len);

}

// la mutation d'un individu se fait en echangeant la place de 2 villes aléatoires sur son chemin
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

// genere une population aléatoire
void population_generate(int *pop, int len)
{
    for (int i = 0; i < NB_INDIVIDUS; ++i) {
        individu_generate(INDIVIDU(pop, i, len), len);
        if (!individu_valide(INDIVIDU(pop, i, len), len))
            printf("Individu non valide\n");
    }
}

// calcule la performance de la population complète 
void population_performance(float *perf, int *pop, MapDistance mapd)
{
    for (int i = 0; i < NB_INDIVIDUS; ++i) {
        perf[i] = individu_performance(INDIVIDU(pop, i, mapd.len), mapd.len, mapd);
    }
}

// trie les individus du plus performant vers le moins performant
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

// l'evolution se passe en prenant la meilleure moitie et en les reproduisant entre eux
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

// boucle principale du programme
int main()
{
    // setup
    SetRandomSeed(time(0));
    MapCoord mapc;
    mapc.len = 5;

    mapc.coord = calloc(sizeof(*mapc.coord), mapc.len);
    MapDistance mapd = {0};
    int *pop = calloc(sizeof(*pop), mapd.len*NB_INDIVIDUS);
    float perf[NB_INDIVIDUS];

    // definition des coordonées des villes (entre 0 et 1)
    mapc.coord[0] = (Vector2) {0.00,  0.00};
    mapc.coord[1] = (Vector2) {0.69,  0.12};
    mapc.coord[2] = (Vector2) {0.23,  0.36};
    mapc.coord[3] = (Vector2) {0.42,  0.96};
    mapc.coord[4] = (Vector2) {1.00,  1.00};

    Vector2 window = {1200, 600};

    InitWindow(window.x, window.y, "algo genetique");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        int ville_radius = 15;
        int map_thickness = 5;
        int offset = ville_radius + map_thickness;
        Rectangle map;
        Rectangle info;
        static int first_frame = 1;
        if (IsWindowResized() || first_frame) {
            first_frame = 0;
            window.x = GetScreenWidth();
            window.y = GetScreenHeight();

            map.width = MIN(window.x/2, window.y) - offset*2;
            map.height = MIN(window.x/2, window.y) - offset*2;
            map.x = offset;
            map.y = (window.y/2 - map.height/2);

            info.x = window.x - map.width;
            info.y = 0;
            info.width = window.x - info.x;
            info.height = window.y;
        }
        BeginDrawing();
        {
            ClearBackground(GRAY);
            Vector2 mouse = GetMousePosition();

            // next gen
            if (IsKeyPressed(KEY_SPACE)) {
                // affiche les coordonées + verifie si elles sont correctes
                printf("Coordonée ville :\n");
                map_coord_print(mapc);
                if (!map_coord_valide(mapc)) {
                    printf("mapc not valide\n");
                    return 0;
                }

                // convertit les coordonées des villes en matrices de distance entre les villes, exemple :
                // [ 0.0, 0.8, 0.3 ]
                // [ 0.8, 0.0, 0.4 ]
                // [ 0.3, 0.4, 0.0 ]
                mapd = map_coord_to_map_distance(mapc);

                // affiche la matrice des distances et verifie si elle est correcte
                printf("Matrice distance entre ville :\n");
                map_distance_print(mapd);
                if (!map_distance_valide(mapd)) {
                    printf("mapd not valide\n");
                    return 0;
                }

                // if (pop != NULL) free(pop);
                // pop = calloc(sizeof(*pop), mapd.len*NB_INDIVIDUS);

                // genere la population aléatoirement
                population_generate(pop, mapd.len);
            }
            // boucle d'évolution
            for (int c = 0; c < 10; c++) {
                // calcul des performances
                population_performance(perf, pop, mapd);
                // tri en fonction des performances
                population_tri(perf, pop, mapd.len);
                // fait évoluer en reproduisant les meilleurs individus
                population_evolution(pop, mapd.len);
            }

            // au bout de plusieurs itérations le chemin le plus court est le meilleur individu de sa génération (index 0)
            // printf("Le chemin le plus court :\n");
            // individu_print(INDIVIDU(pop, 0, mapd.len), mapd.len);
            // printf("Avec une performance de %.3f.\n", perf[0]);
            DrawText(TextFormat("performance: %.3f", perf[0]), info.x, info.y, 20, WHITE);

            for (int i = 0; i < mapd.len; i++) {
                int idx = (i+1)%mapd.len;
                Vector2 coord1;
                coord1.x = mapc.coord[INDIVIDU(pop, 0, mapd.len)[i]].x * map.width + map.x;
                coord1.y = mapc.coord[INDIVIDU(pop, 0, mapd.len)[i]].y * map.width + map.y;
                Vector2 coord2;
                coord2.x = mapc.coord[INDIVIDU(pop, 0, mapd.len)[idx]].x * map.width + map.x;
                coord2.y = mapc.coord[INDIVIDU(pop, 0, mapd.len)[idx]].y * map.width + map.y;
                DrawLineEx(coord1, coord2, 5, GOLD);
            }

            for (int i = 0; i < mapc.len; i++) {
                Vector2 coord;
                coord.x = mapc.coord[i].x * map.width + map.x;
                coord.y = mapc.coord[i].y * map.width + map.y;

                static int ville_click = -1;
                if (ville_click < 0 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointCircle(coord, mouse, ville_radius))
                    ville_click = i;
                else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
                    ville_click = -1;

                Color color = RED;
                if (ville_click == i) {
                    color = GOLD;
                    Vector2 coord;

                    coord.x = (mouse.x - map.x) / map.width;
                    coord.y = (mouse.y - map.y) / map.height;

                    coord.x = CLAMP(coord.x, 0, 1);
                    coord.y = CLAMP(coord.y, 0, 1);

                    mapc.coord[i] = coord;

                    coord.x = mapc.coord[i].x * map.width + map.x;
                    coord.y = mapc.coord[i].y * map.width + map.y;
                }
                DrawCircleV(coord, ville_radius, color);

                char text[4];
                snprintf(text, 4, "%d", i);
                float text_len = TextLength(text);
                DrawText(text, coord.x - text_len*2, coord.y - ville_radius/2, ville_radius*1.5, WHITE);
            }
            Rectangle m;
            m.x = map.x - offset;
            m.y = map.y - offset;
            m.width = map.width + offset*2;
            m.height = map.height + offset*2;

            DrawRectangleLinesEx(m, map_thickness, BLACK);
            // DrawRectangleLinesEx(info, map_thickness, BLUE);
        }
        EndDrawing();
    }

    return 0;
}
