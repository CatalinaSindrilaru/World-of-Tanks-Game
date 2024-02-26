#pragma once

#include "components/simple_scene.h"
#include "lab_m1/Tema2/lab_camera.h"

struct Tank {
   bool isMovingForward;
   bool dead;
   int health;
   float tank_speed;
   float tank_rotation_speed;
   // position body
   float x_body;
   float y_body;
   float z_body;
   // position tracks
   float x_tracks;
   float y_tracks;
   float z_tracks;
   // position turret
   float x_turret;
   float y_turret;
   float z_turret;
   // position cannon
   float x_cannon;
   float y_cannon;
   float z_cannon;

   // angular rotation
   float angularStep_body;
   float angularStep_turret;
   float angularStep_cannon;
   float angularStep_tracks;

   // constructor 
   Tank(
      int health,
      float tank_speed,
      float tank_rotation_speed,
      float x_body,
      float y_body,
      float z_body,
      float x_tracks,
      float y_tracks,
      float z_tracks,
      float x_turret,
      float y_turret,
      float z_turret,
      float x_cannon,
      float y_cannon,
      float z_cannon,
      float angularStep_body,
      float angularStep_turret,
      float angularStep_cannon,
      float angularStep_tracks
   )
      : 
        health(health),
        tank_speed(tank_speed),
        tank_rotation_speed(tank_rotation_speed),
        x_body(x_body),
        y_body(y_body),
        z_body(z_body),
        x_tracks(x_tracks),
        y_tracks(y_tracks),
        z_tracks(z_tracks),
        x_turret(x_turret),
        y_turret(y_turret),
        z_turret(z_turret),
        x_cannon(x_cannon),
        y_cannon(y_cannon),
        z_cannon(z_cannon),
        angularStep_body(angularStep_body),
        angularStep_turret(angularStep_turret),
        angularStep_cannon(angularStep_cannon),
        angularStep_tracks(angularStep_tracks) {}

   Tank() 
   : health(0), 
     tank_speed(0.0f),
     tank_rotation_speed(0.0f),
     x_body(0.0f),
     y_body(0.0f),
     z_body(0.0f),
     x_tracks(0.0f),
     y_tracks(0.0f),
     z_tracks(0.0f),
     x_turret(0.0f),
     y_turret(0.0f),
     z_turret(0.0f),
     x_cannon(0.0f),
     y_cannon(0.0f),
     z_cannon(0.0f),
     angularStep_body(0.0f),
     angularStep_turret(0.0f),
     angularStep_cannon(0.0f),
     angularStep_tracks(0.0f) {}
};

struct Sphere {
   float position_x;
   float position_y;
   float position_z;

   float cannon_direction;
   float creation_time;
   bool to_be_deleted;
};

namespace m1
{
    class Tema2 : public gfxc::SimpleScene
    {
     public:
        Tema2();
        ~Tema2();

        void Init() override;

     private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void RenderMesh(Mesh *mesh, Shader *shader, const glm::mat4 &modelMatrix) override;

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

     protected:
        implemented::Camera *camera;
        glm::mat4 projectionMatrix;
        bool renderCameraTarget;
        bool projectionType;
        GLfloat right;
        GLfloat left;
        GLfloat bottom;
        GLfloat top;
        GLfloat fov;
        float cameraSpeed;
        
        Tank tankPlayer;
        std::vector<Sphere> spheres;
        std::vector<Tank> enemies;

        double lastSphereTime;
        int points_player;

        bool gameEnd;

    };
}   // namespace m1
