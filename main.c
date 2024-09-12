#include <stdlib.h>
#include <stdio.h>

#include "raylib.h"

#define MAPDISTAT(map, x, y) ((map).poids[(x) + (y)*((map).len)])

#define MAPCOORDAT(map, x, y) ((map).coord[(x) + (y)*((map).len)])

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
            if (x == y && MAPDISTAT(map, x, y) != -1)
                return 0;
        }
    }
    return 1;
}

void map_coord_print(MapCoord map)
{
    int len = map.len - 1;
    printf("[\n    [ ");
    for (int y=0; y<len; y++) {
        for (int x=0; x<len; x++) {
            printf("(%.3f, %.3f), ", MAPCOORDAT(map, x, y).x, MAPCOORDAT(map, x, y).y);
        }
        printf("(%.3f, %.3f) ],\n    [ ", MAPCOORDAT(map, len, y).x, MAPCOORDAT(map, len, y).y);
    }
    for (int x=0; x<len; x++) {
        printf("(%.3f, %.3f), ", MAPCOORDAT(map, x, len).x, MAPCOORDAT(map, x, len).y);
    }
    printf("(%.3f, %.3f) ],\n]\n", MAPCOORDAT(map, len, len).x, MAPCOORDAT(map, len, len).y);
}

int map_coord_valide(MapCoord map)
{
    for (int y=0; y<map.len; y++) {
        for (int x=0; x<map.len; x++) {
            float xx = MAPCOORDAT(map, x, y).x;
            float yy = MAPCOORDAT(map, x, y).y;
            if (xx < 0.0 || xx > 1.0 || yy < 0.0 || yy > 1.0)
                return 0;
        }
    }
    return 1;
}

int main()
{
    MapCoord mapc;
    mapc.len = 5;
    mapc.coord = malloc(sizeof(*mapc.coord) * mapc.len * mapc.len);

    MAPCOORDAT(mapc, 0, 0) = (Vector2) {0.12,  0.69};
    MAPCOORDAT(mapc, 1, 0) = (Vector2) {0.1,   0.69};
    MAPCOORDAT(mapc, 2, 0) = (Vector2) {0.2,   0.69};
    MAPCOORDAT(mapc, 3, 0) = (Vector2) {0.42,  0.69};
    MAPCOORDAT(mapc, 4, 0) = (Vector2) {0.4,   0.69};
    MAPCOORDAT(mapc, 0, 1) = (Vector2) {0.2,   0.69};
    MAPCOORDAT(mapc, 1, 1) = (Vector2) {0.69,  0.69};
    MAPCOORDAT(mapc, 2, 1) = (Vector2) {0.6,   0.69};
    MAPCOORDAT(mapc, 3, 1) = (Vector2) {0.9,   0.69};
    MAPCOORDAT(mapc, 4, 1) = (Vector2) {0.412, 0.69};

    map_coord_print(mapc);

    if (!map_coord_valide(mapc))
        printf("mapc not valide\n");

    return 0;
}
