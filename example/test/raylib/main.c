#include "include/raylib.h"

// -lGL -lc -lm -lpthread -ldl -lrt

int main(void)
{
  InitWindow(800, 600, "test");
  SetTargetFPS(60);

  while (!WindowShouldClose())
  {
    ClearBackground((Color){ 23, 23, 23, 255 });
    BeginDrawing();
    {
      DrawFPS(10, 10);
    }
    EndDrawing();
  }

  CloseWindow();

  return 0;
}
