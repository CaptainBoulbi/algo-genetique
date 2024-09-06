#include "raylib.h"

int main()
{
    InitWindow(800, 600, "algo genetique");
    while (!WindowShouldClose()) {
        BeginDrawing();
        {
            ClearBackground(RED);
        }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
