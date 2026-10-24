#include "Voxel65.hpp"
#include "Texture.hpp"
#include "raylib.h"

int main() {
  
  // Initialization
  SetTraceLogLevel(LOG_WARNING);
  raylib::Window w(SCREEN_WIDTH, SCREEN_HEIGHT, "Voxel65");

  raylib::Camera3D camera;
  camera.SetPosition(Vector3{0.0f, CHUNK_HEIGHT, 0.0f});
  camera.SetTarget(Vector3{1.0f, CHUNK_HEIGHT-1.0f, 1.0f});
  camera.SetUp(Vector3{0.0f, 1.0f, 0.0f});
  camera.SetFovy(45.0f);
  camera.SetProjection(CAMERA_PERSPECTIVE);

  raylib::Shader shader("src/resources/shaders/vert.glsl", "src/resources/shaders/frag.glsl");
  int ambientLoc = shader.GetLocation("ambient");
  int viewPosLoc = shader.GetLocation("viewPos");
  Vector4 ambient = {0.3, 0.3, 0.3, 0.3};
  shader.SetValue(ambientLoc, &ambient, SHADER_UNIFORM_VEC4);
  shader.SetValue(viewPosLoc, &camera.position, SHADER_UNIFORM_VEC3);

  Texture2D texture = LoadTexture("src/resources/Grass.png");

  Material material = LoadMaterialDefault();
  material.shader = shader;
  material.maps[MATERIAL_MAP_DIFFUSE].texture = texture;

  World world(material);

  raylib::Texture2D crosshair("src/resources/Crosshair.png");

  SetMousePosition(0, 0);
  HideCursor();
  SetTargetFPS(60);

  // Main game loop
  while (!w.ShouldClose()) // Detect window close button or ESC key
  {
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
    Vector2 mousePositionDelta = GetMouseDelta();
    rotation.x = mousePositionDelta.x*CAMERA_MOUSE_MOVE_SENSITIVITY*RAD2DEG;
    rotation.y = mousePositionDelta.y*CAMERA_MOUSE_MOVE_SENSITIVITY*RAD2DEG;
    SetMousePosition(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
    camera.Update(movement, rotation, 0.0f);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) world.breakBlock(camera);

    world.update(camera);

    // Shader Update
    shader.SetValue(ambientLoc, &ambient, SHADER_UNIFORM_VEC4);
    shader.SetValue(viewPosLoc, &camera.position, SHADER_UNIFORM_VEC3);

    // Draw
    BeginDrawing();
    {
      ClearBackground(BLACK);
      camera.BeginMode();
      {
        world.draw();
      }
      camera.EndMode();
      
      crosshair.Draw(Vector2{SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f}, 0.0f, 2.0f, WHITE);

      DrawFPS(10, 10);
      DrawText(TextFormat("World Size (Chunks): %i\n", world.size()), 10, 40, 20, RED);
      DrawText(TextFormat("X: %.02f Y: %.02f Z: %.02f\n", camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z), 10, 70, 20, BLUE);
    }
    EndDrawing();
  }

  UnloadTexture(texture);

  return 0;
}

