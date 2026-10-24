#include "Voxel65.hpp"
#include "ShaderUnmanaged.hpp"
#include "raylib.h"
#include <cstdio>

raylib::Window *w;
raylib::Camera3D camera;
raylib::Shader *shader;
int ambientLoc;
int viewPosLoc;
Vector4 ambient = {0.3, 0.3, 0.3, 0.3};
World *world;
raylib::Texture2D crosshair;
bool cursorLocked = false;

void update() {
    // Update
    
    // Chunk Update
    //chunk.updateModel();

    // Camera Update
    Vector3 movement = Vector3{0.0f, 0.0f, 0.0f};
    Vector3 rotation = Vector3{0.0f, 0.0f, 0.0f};
    if (IsKeyDown(KEY_W)) movement.x += CAMERA_MOVE_SPEED;
    if (IsKeyDown(KEY_A)) movement.y -= CAMERA_MOVE_SPEED;
    if (IsKeyDown(KEY_S)) movement.x -= CAMERA_MOVE_SPEED;
    if (IsKeyDown(KEY_D)) movement.y += CAMERA_MOVE_SPEED;
    if (IsKeyDown(KEY_SPACE)) movement.z += CAMERA_MOVE_SPEED;
    if (IsKeyDown(KEY_LEFT_SHIFT)) movement.z -= CAMERA_MOVE_SPEED;
    
    if (IsKeyDown(KEY_ESCAPE)) cursorLocked = false;

    if (!cursorLocked && w->IsFocused()) {
      DisableCursor();
      cursorLocked = true;
    }
    if (cursorLocked) {
      Vector2 mousePositionDelta = GetMouseDelta();
      rotation.x = mousePositionDelta.x*CAMERA_MOUSE_MOVE_SENSITIVITY*RAD2DEG;
      rotation.y = mousePositionDelta.y*CAMERA_MOUSE_MOVE_SENSITIVITY*RAD2DEG;
    }
    //SetMousePosition(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
    camera.Update(movement, rotation, 0.0f);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) world->breakBlock(camera);

    world->update(camera);

    // Shader Update
    shader->SetValue(ambientLoc, &ambient, SHADER_UNIFORM_VEC4);
    shader->SetValue(viewPosLoc, &camera.position, SHADER_UNIFORM_VEC3);

    // Draw
    BeginDrawing();
    {
      ClearBackground(BLACK);
      rlSetClipPlanes(3.0, 96.0);
      camera.BeginMode();
      {
        world->draw();
      }
      camera.EndMode();
      
      crosshair.Draw(Vector2{SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f}, 0.0f, 2.0f, WHITE);

      DrawFPS(10, 10);
      DrawText(TextFormat("World Size (Chunks): %i\n", world->size()), 10, 40, 20, RED);
      DrawText(TextFormat("X: %.02f Y: %.02f Z: %.02f\n", camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z), 10, 70, 20, BLUE);
    }
    EndDrawing();
}

int main() {
  
  // Initialization
  SetTraceLogLevel(LOG_WARNING);
  w = new raylib::Window(SCREEN_WIDTH, SCREEN_HEIGHT, "Voxel65");

  camera.SetPosition(Vector3{0.0f, CHUNK_HEIGHT, 0.0f});
  camera.SetTarget(Vector3{1.0f, CHUNK_HEIGHT-1.0f, 1.0f});
  camera.SetUp(Vector3{0.0f, 1.0f, 0.0f});
  camera.SetFovy(45.0f);
  camera.SetProjection(CAMERA_PERSPECTIVE);

  shader = new raylib::Shader("resources/shaders/vert.glsl", "resources/shaders/frag.glsl");
  ambientLoc = shader->GetLocation("ambient");
  viewPosLoc = shader->GetLocation("viewPos");
  shader->SetValue(ambientLoc, &ambient, SHADER_UNIFORM_VEC4);
  shader->SetValue(viewPosLoc, &camera.position, SHADER_UNIFORM_VEC3);

  Texture2D texture = LoadTexture("resources/Grass.png");

  Material material = LoadMaterialDefault();
  material.shader = *shader;
  material.maps[MATERIAL_MAP_DIFFUSE].texture = texture;

  world = new World(material);

  crosshair.Load("resources/Crosshair.png");

  //SetMousePosition(0, 0);
  //HideCursor();

#if defined (PLATFORM_WEB)
  SetExitKey(KEY_NULL);
  emscripten_set_main_loop(update, 0, 1);
#else
  SetTargetFPS(60);

  // Main game loop
  while (!w->ShouldClose()) // Detect window close button or ESC key
  {
    update();
  }
#endif

  UnloadTexture(texture);

  return 0;
}

