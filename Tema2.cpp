
#include "lab_m1/Tema2/Tema2.h"

#include <vector>
#include <string>
#include <iostream>

using namespace std;
using namespace m1;

#include <GLFW/glfw3.h>

/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Tema2::Tema2()
{
}


Tema2::~Tema2()
{
}


void Tema2::Init()
{
    renderCameraTarget = false;

    projectionType = true;
    fov = 60.0f;

    cameraSpeed = 20.0f;
    right = 10.f;
    left = .01f;
    bottom = .01f;
    top = 10.f;

    camera = new implemented::Camera();
    camera->Set(glm::vec3(0, 2, 3.5f), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));


    {
        Shader *shader = new Shader("ShaderTank");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shaders", "VertexShaderTank.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shaders", "FragmentShaderTank.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    {
        Mesh* mesh = new Mesh("floor");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("sphere");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "sphere.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }


    {
        Mesh* mesh = new Mesh("tracks");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "my_models"), "senile.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }


    {
        Mesh* mesh = new Mesh("body");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "my_models"), "corp.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("turret");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "my_models"), "turela.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("cannon");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "my_models"), "tun.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    // a tank has 3 lives initially
    tankPlayer = Tank(3, 5, 100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  180, 180, 180, 180);

    // enemies from the right of the player
    enemies.push_back(Tank(3, 3, 100, 10, 0, 5, 10, 0, 5, 10, 0, 5, 10, 0, 5, 180, 180, 180, 180));
    enemies.push_back(Tank(3, 2, 100, 5, 0, 5, 5, 0, 5, 5, 0, 5, 5, 0, 5, 180, 180, 180, 180));
    enemies.push_back(Tank(3, 1, 100, 15, 0, 5, 15, 0, 5, 15, 0, 5, 15, 0, 5, 180, 180, 180, 180));

    // enemies from the left of the player
    enemies.push_back(Tank(3, 1, 100, -5, 0, 5, -5, 0, 5, -5, 0, 5, -5, 0, 5, 180, 180, 180, 180));
    enemies.push_back(Tank(3, 4, 100, -10, 0, 5, -10, 0, 5, -10, 0, 5, -10, 0, 5, 180, 180, 180, 180));
    enemies.push_back(Tank(3, 3, 100, -15, 0, 5, -15, 0, 5, -15, 0, 5, -15, 0, 5, 180, 180, 180, 180));


    for (int i = 0; i < enemies.size(); i++) {
        enemies[i].isMovingForward = true;
        enemies[i].dead = false;
    }
    // TODO(student): After you implement the changing of the projection
    // parameters, remove hardcodings of these parameters
    projectionMatrix = glm::perspective(RADIANS(fov), window->props.aspectRatio, 0.01f, 200.0f);

    lastSphereTime = 0;
    points_player = 0;

    gameEnd = false;
}


void Tema2::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}


void Tema2::Update(float deltaTimeSeconds)
{
    Shader *shader = shaders["ShaderTank"];
    int meshColorLocation = glGetUniformLocation(shader->GetProgramID(), "meshColor");
    glUseProgram(shader->GetProgramID());
    int colorAdjustmentLocation = glGetUniformLocation(shader->GetProgramID(), "colorAdjustment");

    // Render the floor
    {
        Shader* floorShader = shaders["ShaderTank"];
        int floorMeshColorLocation = glGetUniformLocation(floorShader->GetProgramID(), "meshColor");
        int floorColorAdjustmentLocation = glGetUniformLocation(floorShader->GetProgramID(), "colorAdjustment");

        glUseProgram(floorShader->GetProgramID());
        glUniform4f(floorMeshColorLocation, 0.9f, 0.9f, 0.9f, 1.0f);
        glUniform1f(floorColorAdjustmentLocation, 1.0f);

        glm::mat4 floorModelMatrix = glm::mat4(1.0f);
        floorModelMatrix = glm::scale(floorModelMatrix, glm::vec3(50.0f, 0.05f, 50.0f));

        RenderMesh(meshes["floor"], floorShader, floorModelMatrix);
    }

    {
        // render the tracks of the tank player
        glUniform4f(meshColorLocation, 0.9f, 0.8f, 0.7f, 1.0f); // send the color to the shader
        glUniform1f(colorAdjustmentLocation, tankPlayer.health); // send the health of the tank to the shader
        glm::mat4 modelMatrix = glm::mat4(1);

        modelMatrix = glm::translate(modelMatrix, glm::vec3(tankPlayer.x_tracks, tankPlayer.y_tracks, tankPlayer.z_tracks));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(tankPlayer.angularStep_tracks), glm::vec3(0, 1, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));
        RenderMesh(meshes["tracks"], shader, modelMatrix);

        // render the body of the tank player
        glUniform4f(meshColorLocation, 1.0f, 0.0f, 0.0f, 1.0f);
        glUniform1f(colorAdjustmentLocation, tankPlayer.health);
        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(tankPlayer.x_body, tankPlayer.y_body, tankPlayer.z_body));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(tankPlayer.angularStep_body), glm::vec3(0, 1, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));
        RenderMesh(meshes["body"], shader, modelMatrix);

        // render the turret of the tank player
        glUniform4f(meshColorLocation, 0.7f, 0.0f, 0.0f, 1.0f);
        glUniform1f(colorAdjustmentLocation, tankPlayer.health);
        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(tankPlayer.x_turret, tankPlayer.y_turret, tankPlayer.z_turret));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(tankPlayer.angularStep_turret), glm::vec3(0, 1, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));
        RenderMesh(meshes["turret"], shader, modelMatrix);

        // render the cannon of the tank player
        glUniform4f(meshColorLocation, 0.9f, 0.8f, 0.7f, 1.0f);
        glUniform1f(colorAdjustmentLocation, tankPlayer.health);
        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(tankPlayer.x_cannon, tankPlayer.y_cannon, tankPlayer.z_cannon));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(tankPlayer.angularStep_cannon), glm::vec3(0, 1, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));
        RenderMesh(meshes["cannon"], shader, modelMatrix);
    }

    // render the proiectile
    for (int i = 0; i < spheres.size(); i++) {
        glUniform4f(meshColorLocation, 0.0f, 0.0f, 0.0f, 1.0f);
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(spheres[i].position_x, spheres[i].position_y, spheres[i].position_z));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2, 0.2, 0.2));
        RenderMesh(meshes["sphere"], shader, modelMatrix);
    }

    // move the proiectile forward in the direction of the cannon
    for (int i = 0; i < spheres.size(); i++) {
        spheres[i].position_x += 5 * deltaTimeSeconds * sin(spheres[i].cannon_direction);
        spheres[i].position_z += 5 * deltaTimeSeconds * cos(spheres[i].cannon_direction);
    }

    // delete the proiectile when 3 seconds passed from the creation
    for (int i = 0; i < spheres.size(); i++) {
        if (glfwGetTime() - spheres[i].creation_time>= 3.0 || spheres[i].to_be_deleted == true) {
            spheres.erase(spheres.begin() + i);
        }
    }

    // render the enemies
    for (int i = 0; i < enemies.size(); i++) {
        glUniform1f(colorAdjustmentLocation, enemies[i].health);

        // tracks
        glUniform4f(meshColorLocation, 0.4f, 0.8f, 1.0f, 1.0f);
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(enemies[i].x_tracks, enemies[i].y_tracks, enemies[i].z_tracks));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(enemies[i].angularStep_tracks), glm::vec3(0, 1, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));
        RenderMesh(meshes["tracks"], shader, modelMatrix);

        // body
        glUniform4f(meshColorLocation, 0.4f, 0.6f, 0.9f, 1.0f);
        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(enemies[i].x_body, enemies[i].y_body, enemies[i].z_body));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(enemies[i].angularStep_body), glm::vec3(0, 1, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));
        RenderMesh(meshes["body"], shader, modelMatrix);

        // turret
        glUniform4f(meshColorLocation, 0.3f, 0.3f, 0.6f, 1.0f);
        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(enemies[i].x_turret, enemies[i].y_turret, enemies[i].z_turret));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(enemies[i].angularStep_turret), glm::vec3(0, 1, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));
        RenderMesh(meshes["turret"], shader, modelMatrix);

        // cannon
        glUniform4f(meshColorLocation, 0.4f, 0.8f, 1.0f, 1.0f);
        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(enemies[i].x_cannon, enemies[i].y_cannon, enemies[i].z_cannon));
        modelMatrix = glm::rotate(modelMatrix, RADIANS(enemies[i].angularStep_cannon), glm::vec3(0, 1, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));
        RenderMesh(meshes["cannon"], shader, modelMatrix);
    }

    // move the enemies forward or backward (depending on the direction they are moving)
    for (int i = 0; i < enemies.size(); i++) {
        if (enemies[i].dead == false && gameEnd == false) { // a dead enemy will not move
            enemies[i].z_body += enemies[i].tank_speed * deltaTimeSeconds * cos(enemies[i].angularStep_body);
            enemies[i].z_tracks += enemies[i].tank_speed * deltaTimeSeconds * cos(enemies[i].angularStep_body);
            enemies[i].z_turret += enemies[i].tank_speed * deltaTimeSeconds  * cos(enemies[i].angularStep_body);
            enemies[i].z_cannon += enemies[i].tank_speed * deltaTimeSeconds * cos(enemies[i].angularStep_body);
        }
    }

    // change the direction of the enemies when they reach a limit
    for (int i = 0; i < enemies.size(); i++) {
        if (enemies[i].z_body <= -20) {
            enemies[i].isMovingForward = false;
            enemies[i].angularStep_body = 0;
            enemies[i].angularStep_turret = 0;
            enemies[i].angularStep_cannon = 0;
            enemies[i].angularStep_tracks = 0;
        } else if (enemies[i].z_body >= 20) {
            enemies[i].isMovingForward = true;
            enemies[i].angularStep_body = 180;
            enemies[i].angularStep_turret = 180;
            enemies[i].angularStep_cannon = 180;
            enemies[i].angularStep_tracks = 180;
        } 
    }

    // Collision with proiectile
    for (int i = 0; i < enemies.size(); i++) {
        for (int j = 0; j < spheres.size(); j++) {
            float radius_tank = 1.2f;
            //  float radius_tank = 2.0f;
            // for each spheres, verify if it collide with a enemy
            if ((spheres[j].position_x >= enemies[i].x_body - radius_tank && spheres[j].position_x <= enemies[i].x_body + radius_tank)
                && (spheres[j].position_z >= enemies[i].z_body - radius_tank  && spheres[j].position_z <= enemies[i].z_body + radius_tank)
                && enemies[i].dead == false) {

                    spheres[j].to_be_deleted = true;
                    enemies[i].health--;
                    break;
            }
        }
    }

    // Collision between the tank player and the enemies
    for (int i = 0; i < enemies.size(); i++) {
        float radius_tank_player = 2.0f;
        float radius_enemy = 2.0f;

        glm::vec3 centerTankPlayer = glm::vec3(tankPlayer.x_body, tankPlayer.y_body, tankPlayer.z_body);
        glm::vec3 centerEnemy = glm::vec3(enemies[i].x_body, enemies[i].y_body, enemies[i].z_body);

        float distance = glm::distance(centerTankPlayer, centerEnemy);
        
        if (distance < radius_tank_player + radius_enemy) {

            float mod_p = radius_tank_player + radius_enemy - distance;
            glm::vec3 dif = (centerEnemy - centerTankPlayer);
            glm::vec3 normalizedDif = glm::normalize(dif);
            glm::vec3 p = mod_p * normalizedDif;

            tankPlayer.x_body -= p.x * 1.0f;
            tankPlayer.x_tracks -= p.x * 1.0f;
            tankPlayer.x_turret -= p.x * 1.0f;
            tankPlayer.x_cannon -= p.x * 1.0f;

            tankPlayer.z_body -= p.z * 1.0f;
            tankPlayer.z_tracks -= p.z * 1.0f;
            tankPlayer.z_turret -= p.z * 1.0f;
            tankPlayer.z_cannon -= p.z * 1.0f;

            enemies[i].x_body += p.x * 1.0f;
            enemies[i].x_tracks += p.x * 1.0f;
            enemies[i].x_turret += p.x * 1.0f;
            enemies[i].x_cannon += p.x * 1.0f;

            enemies[i].z_body += p.z * 1.0f;
            enemies[i].z_tracks += p.z * 1.0f;
            enemies[i].z_turret += p.z * 1.0f;
            enemies[i].z_cannon += p.z * 1.0f;
        }
        
    }

    // Verify if the enemy has no lives, mark it as dead and increase the points of the player
    for (int i = 0; i < enemies.size(); i++) {
        if (enemies[i].health == 0 && enemies[i].dead == false) {
            points_player += 1;
            enemies[i].dead = true;
        }
    }

    // After 90 seconds, the game ends
    if (glfwGetTime() >= 90.0f && gameEnd == false) {
        printf("Game End. You achieved %d points!!!\n", points_player);
        gameEnd = true;
    }
}


void Tema2::FrameEnd()
{
    DrawCoordinateSystem(camera->GetViewMatrix(), projectionMatrix);
}


void Tema2::RenderMesh(Mesh * mesh, Shader * shader, const glm::mat4 & modelMatrix)
{
    if (!mesh || !shader || !shader->program)
        return;

    // Render an object using the specified shader and the specified position
    shader->Use();
    glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
    glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    mesh->Render();
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Tema2::OnInputUpdate(float deltaTime, int mods)
{
    glm::vec3 forward = glm::normalize(glm::vec3(sin(RADIANS(tankPlayer.angularStep_body)), 0, cos(RADIANS(tankPlayer.angularStep_body))));

    // move the tank forward when W is pressed
    if (window->KeyHold(GLFW_KEY_W) && gameEnd == false) {
        // change x and z coordinates
        tankPlayer.x_body += forward.x * tankPlayer.tank_speed * deltaTime;
        tankPlayer.x_tracks += forward.x * tankPlayer.tank_speed * deltaTime;
        tankPlayer.x_turret += forward.x * tankPlayer.tank_speed * deltaTime;
        tankPlayer.x_cannon += forward.x * tankPlayer.tank_speed * deltaTime;

        tankPlayer.z_body += forward.z * tankPlayer.tank_speed * deltaTime;
        tankPlayer.z_tracks += forward.z * tankPlayer.tank_speed * deltaTime;
        tankPlayer.z_turret += forward.z * tankPlayer.tank_speed * deltaTime;
        tankPlayer.z_cannon += forward.z * tankPlayer.tank_speed * deltaTime;
    }

    // move the tank backward when S is pressed
    if (window->KeyHold(GLFW_KEY_S) && gameEnd == false) {
        // change x and z coordinates
        tankPlayer.x_body -= forward.x * tankPlayer.tank_speed * deltaTime;
        tankPlayer.x_tracks -= forward.x * tankPlayer.tank_speed * deltaTime;
        tankPlayer.x_turret -= forward.x * tankPlayer.tank_speed * deltaTime;
        tankPlayer.x_cannon -= forward.x * tankPlayer.tank_speed * deltaTime;

        tankPlayer.z_body -= forward.z * tankPlayer.tank_speed * deltaTime;
        tankPlayer.z_tracks -= forward.z * tankPlayer.tank_speed * deltaTime;
        tankPlayer.z_turret -= forward.z * tankPlayer.tank_speed * deltaTime;
        tankPlayer.z_cannon -= forward.z * tankPlayer.tank_speed * deltaTime;
    }

    // rotate the tank to the left when A is pressed
    if (window->KeyHold(GLFW_KEY_A) && gameEnd == false) {
        tankPlayer.angularStep_body += tankPlayer.tank_rotation_speed * deltaTime;
        tankPlayer.angularStep_turret += tankPlayer.tank_rotation_speed * deltaTime;
        tankPlayer.angularStep_cannon += tankPlayer.tank_rotation_speed * deltaTime;
        tankPlayer.angularStep_tracks += tankPlayer.tank_rotation_speed * deltaTime;
    }

    // rotate the tank to the right when D is pressed
    if (window->KeyHold(GLFW_KEY_D) && gameEnd == false) {
        tankPlayer.angularStep_body -= tankPlayer.tank_rotation_speed * deltaTime;
        tankPlayer.angularStep_turret -= tankPlayer.tank_rotation_speed * deltaTime;
        tankPlayer.angularStep_cannon -= tankPlayer.tank_rotation_speed * deltaTime;
        tankPlayer.angularStep_tracks -= tankPlayer.tank_rotation_speed * deltaTime;
    }

    // shoot a proiectile when the left mouse button is pressed
    if (window->MouseHold(GLFW_MOUSE_BUTTON_LEFT) && gameEnd == false) {
        if (glfwGetTime() - lastSphereTime >= 1.0) {
            float sphereDistance = 7.2 / 2.0;

            float sphereHeight = 0.9;

            float angle = RADIANS(tankPlayer.angularStep_cannon);

            Sphere sphere;
            sphere.position_x = tankPlayer.x_cannon +  sphereDistance * sin(angle);
            sphere.position_y = tankPlayer.y_cannon + sphereHeight;
            sphere.position_z = tankPlayer.z_cannon + sphereDistance * cos(angle);

            sphere.cannon_direction = angle;
            sphere.creation_time = glfwGetTime();
            sphere.to_be_deleted = false;

            spheres.push_back(sphere);

            lastSphereTime = glfwGetTime();
        }
    }

    // move the camera in all the directions when the right mouse button is pressed
    if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT) && gameEnd == false)
    {
        float cameraSpeed = 2.0f;

        if (window->KeyHold(GLFW_KEY_I)) {
            camera->TranslateForward(deltaTime * cameraSpeed);

        }

        if (window->KeyHold(GLFW_KEY_J)) {
            camera->TranslateRight(-deltaTime * cameraSpeed);
        }

        if (window->KeyHold(GLFW_KEY_K)) {
            camera->TranslateForward(-deltaTime * cameraSpeed);
        }

        if (window->KeyHold(GLFW_KEY_L)) {
            camera->TranslateRight(deltaTime * cameraSpeed);
        }

        if (window->KeyHold(GLFW_KEY_U)) {
            camera->TranslateUpward(-deltaTime * cameraSpeed);
        }

        if (window->KeyHold(GLFW_KEY_O)) {
            camera->TranslateUpward(deltaTime * cameraSpeed);
        }
    }
}


void Tema2::OnKeyPress(int key, int mods)
{
}


void Tema2::OnKeyRelease(int key, int mods)
{

}


void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // rotate the turret and the cannon when the right mouse button is pressed
    if (!window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT)) {
        float sensitivity = 0.4f;
        tankPlayer.angularStep_turret -= sensitivity * deltaX;
        tankPlayer.angularStep_cannon -= sensitivity * deltaX;
    }

    // rotate the camera only if MOUSE_RIGHT button is pressed
    if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT) && gameEnd == false)
    {
        float sensivityOX = 0.001f;
        float sensivityOY = 0.001f;

        if (window->GetSpecialKeyState() == 0) {
            printf("here1\n");
            renderCameraTarget = false;
            camera->RotateFirstPerson_OX(-deltaY * sensivityOX);
            camera->RotateFirstPerson_OY(-deltaX * sensivityOY);

        }

        if (window->GetSpecialKeyState() & GLFW_MOD_CONTROL) {
            renderCameraTarget = true;
            printf("here2\n");
            camera->RotateThirdPerson_OX(-deltaY * sensivityOX);
            camera->RotateThirdPerson_OY(-deltaX * sensivityOY);
        }
    }
}


void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
}


void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
}


void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void Tema2::OnWindowResize(int width, int height)
{
}