//----------------------------------------------------------------------------------------
/**
 * @file    main.cpp
 * @author  Sára Veselá
 * @date    16/5/2022
 * @brief   Implemenatation of Hogwart grounds.
 */
//----------------------------------------------------------------------------------------

#include <time.h>
#include <tuple>
#include "pgr.h"
#include "render_stuff.h"
#include "spline.h"
#include <iostream>
#include "glm/ext.hpp"

//init shader program
extern SCommonShaderProgram shaderProgram;
extern SkyboxShaderProgram skyboxShaderProgram;


struct GameState {

  int windowWidth;
  int windowHeight;

  int cameraMode;               
  //camera 1 - static, start position
  //camera 2 - static 2
  //camera 3 - free camera with broomstick
  //camera 4 - dynamic camera - birds view
  bool keyMap[KEYS_COUNT];
  
  bool torchOn;
  bool fogOn;

  float elapsedTime;

  glm::vec3 skyColour;          //colour of fog, changes from grey to black at night
  float dayTime;                //0 if day, 1 if night, in between at dawn and dusk

} gameState;

/**
 * @brief Struct with all objects or lists of objects in game
*/
struct GameObjects {

  CameraObject*     camera;
  Object*           tree1;
  Object*           tree2;
  Object*           tree3;
  Object*           tree4;
  Object*           tree5;
  Object*           plant;
  Object*           plant1;
  Object*           plant2;
  Object*           plant3;
  Object*           plant4;
  MoveableObject*   eagle;
  GroundObject*     ground;
  WaterObject*      water;
  FireObject*       fire;

  Object*           bench1;
  Object*           bench2;
  Object*           hall;
  Object*           hat;
  Object*           broom;
  Object*           wand;
  Object*           fireplace;
  Object*           rock;

} gameObjects;

//GUI menu 
static int window;
static int value = 0;

//Function to create a menu using glut
void menu(int num) {
    if (num == 0) {
        glutDestroyWindow(window);
        exit(0);
    }
    else {
        value = num;
    }
    glutPostRedisplay();
}

static int menu_id;

//Creates menu
void createMenu(void) {
    const int submenu_camera = glutCreateMenu(menu);
    glutAddMenuEntry("Free", 2);
    glutAddMenuEntry("Static 1", 3);
    glutAddMenuEntry("Static 2", 4);
    
    const int submenu_flashlight = glutCreateMenu(menu);
    glutAddMenuEntry("On", 6);
    glutAddMenuEntry("Off", 7);

    const int submenu_fog = glutCreateMenu(menu);
    glutAddMenuEntry("On", 8);
    glutAddMenuEntry("Off", 9);

    menu_id = glutCreateMenu(menu);
    glutAddSubMenu("Camera", submenu_camera);
    glutAddSubMenu("Light", submenu_flashlight);
    glutAddSubMenu("Fog", submenu_fog);
    glutAddMenuEntry("Restart", 1);
    glutAddMenuEntry("Quit", 0);     
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

//change position of camera according to pressed keys = Counts new position of camera from keyboard input and view direction
//forwards is up arrow or W pressed, backwards is down arrow or S pressed, right is right arrow or D pressed, left is left arrow or A pressed, timeDelta ensures stable FPS
//return vector in which camera is headed with correct size according to speed
glm::vec3 changeCameraPosition(bool forwards, bool backwards, bool right, bool left, float timeDelta) {

    if (((forwards && backwards) || !(forwards || backwards)) && ((right && left) || !(right || left))) {
        gameObjects.camera->speed = 0;
        return glm::vec3(0.0f, 0.0f, 0.0f);
    }
    if (forwards) {
        gameObjects.camera->speed = CAMERA_SPEED;
        glm::vec3 v = glm::vec3(gameObjects.camera->direction.x, 0, gameObjects.camera->direction.z);
        return timeDelta * v * CAMERA_SPEED;
    }
    else if(backwards){
        gameObjects.camera->speed = -CAMERA_SPEED;
        glm::vec3 v = glm::vec3(gameObjects.camera->direction.x, 0, gameObjects.camera->direction.z);
        return - timeDelta * v * CAMERA_SPEED;
    }
    else if (left) {
        gameObjects.camera->speed = -CAMERA_SPEED;
        return - glm::normalize(glm::cross(gameObjects.camera->direction, glm::vec3(0.0f, 1.0f, 0.0f))) * CAMERA_SPEED * timeDelta;
    }
    else if (right) {
        gameObjects.camera->speed = CAMERA_SPEED;
        return glm::normalize(glm::cross(gameObjects.camera->direction, glm::vec3(0.0f, 1.0f, 0.0f))) * CAMERA_SPEED * timeDelta;
    }
    return glm::vec3(0.0f, 0.0f, 0.0f);

}

//checking collision with pond/circle
//returns true if no collision
bool checkCollisionPond(glm::vec3 newCameraPosition) {
    float r = 3.0f;                             //radius of circle
    glm::vec3 s = glm::vec3(3.5f, 0.1f, -3.5f); //middle of circle
    float dist = abs(glm::distance(s, newCameraPosition));
    if (dist > r) {
        return true;
    }
    else {
        return false;
    }
}

//checks collision = dont go behind the ground
//returns true if no collision
bool checkCollisionBorder(glm::vec3 newCameraPosition) {
    if (abs(newCameraPosition.x) > 9.5f) {
        return false;
    }
    else if (abs(newCameraPosition.z) > 9.5f) {
        return false;
    }
    return true;
}

// Deletes all objects in game
void cleanUpObjects() {
    if (gameObjects.tree1 != NULL) {
        delete gameObjects.tree1;
        gameObjects.tree1 = NULL;
    }
    if (gameObjects.tree2 != NULL) {
        delete gameObjects.tree2;
        gameObjects.tree2 = NULL;
    }
    if (gameObjects.tree3 != NULL) {
        delete gameObjects.tree3;
        gameObjects.tree3 = NULL;
    }
    if (gameObjects.tree5 != NULL) {
        delete gameObjects.tree5;
        gameObjects.tree5 = NULL;
    }
    if (gameObjects.tree4 != NULL) {
        delete gameObjects.tree4;
        gameObjects.tree4 = NULL;
    }
    if (gameObjects.plant != NULL) {
        delete gameObjects.plant;
        gameObjects.plant = NULL;
    }
    if (gameObjects.plant1 != NULL) {
        delete gameObjects.plant1;
        gameObjects.plant1 = NULL;
    }
    if (gameObjects.plant2 != NULL) {
        delete gameObjects.plant2;
        gameObjects.plant2 = NULL;
    }
    if (gameObjects.plant3 != NULL) {
        delete gameObjects.plant3;
        gameObjects.plant3 = NULL;
    }
    if (gameObjects.plant4 != NULL) {
        delete gameObjects.plant4;
        gameObjects.plant4 = NULL;
    }
    if (gameObjects.camera != NULL) {
        delete gameObjects.camera;
        gameObjects.camera = NULL;
    }
    if (gameObjects.bench1 != NULL) {
        delete gameObjects.bench1;
        gameObjects.bench1 = NULL;
    }
    if (gameObjects.bench2 != NULL) {
        delete gameObjects.bench2;
        gameObjects.bench2 = NULL;
    }
    if (gameObjects.hall != NULL) {
        delete gameObjects.hall;
        gameObjects.hall = NULL;
    }
    
    if (gameObjects.broom != NULL) {
        delete gameObjects.broom;
        gameObjects.broom = NULL;
    }
    if (gameObjects.hat != NULL) {
        delete gameObjects.hat;
        gameObjects.hat = NULL;
    }
    if (gameObjects.ground != NULL) {
        delete gameObjects.ground;
        gameObjects.ground = NULL;
    }
    if (gameObjects.water != NULL) {
        delete gameObjects.water;
        gameObjects.water = NULL;
    }
    if (gameObjects.eagle != NULL) {
        delete gameObjects.eagle;
        gameObjects.eagle = NULL;
    }
    if (gameObjects.rock != NULL) {
        delete gameObjects.rock;
        gameObjects.rock = NULL;
    }
    if (gameObjects.fire != NULL) {
        delete gameObjects.fire;
        gameObjects.fire = NULL;
    }

    if (gameObjects.fireplace != NULL) {
        delete gameObjects.fireplace;
        gameObjects.fireplace = NULL;
    }

}

//setting normal objects = loads given parametrs into object
void setObject(glm::vec3 position, glm::vec3 direction, float rotationAngle, float size, std::string id, Object *obj, glm::vec3 size3D, glm::vec3 point) {

    obj->position = position;
    obj->rotationAngle = rotationAngle;
    obj->direction = direction;
    obj->size = size;
    obj->id = id;

    obj->startTime = gameState.elapsedTime;
    obj->currentTime = gameState.elapsedTime;

}

GroundObject* createGround(void) {
    GroundObject* newGround = new GroundObject;

    newGround->size = GROUND_SIZE;
    newGround->position = glm::vec3(0.0f, 0.0f, 0.0f);

    return newGround;
}

WaterObject* createWater(void) {
    WaterObject* newWater = new WaterObject;

    newWater->speed = 0.0f;

    newWater->startTime = gameState.elapsedTime;
    newWater->currentTime = newWater->startTime;

    newWater->size = BILLBOARD_SIZE * 4.4;
    newWater->direction = glm::vec3(0.0f, 0.0f, 1.0f);

    newWater->frameDuration = 0.1f;
    newWater->textureFrames = 16;

    newWater->position = glm::vec3(0.0f, -0.55f, 0.0f);

    return newWater;
}

MoveableObject* createEagle(void) {
    MoveableObject* newEagle = new MoveableObject;

    newEagle->startTime = gameState.elapsedTime;
    newEagle->currentTime = newEagle->startTime;

    newEagle->size = EAGLE_SIZE;
    newEagle->initPosition = glm::vec3(-7.0f, 4.0f, 5.0f);
    newEagle->position = newEagle->initPosition;
    newEagle->rotationAngle = 180.0f;
    newEagle->rotdirection = glm::vec3(1.0f, 0.0f, 0.0f);
    newEagle->speed = 1;

    return newEagle;
}
FireObject* createFire(const glm::vec3& position) {
    FireObject* newFire = new FireObject;

    newFire->startTime = gameState.elapsedTime;
    newFire->currentTime = newFire->startTime;

    newFire->size = FIRE_BILLBOARD_SIZE;
    newFire->direction = glm::vec3(0.0f, 0.0f, 1.0f);

    newFire->frameDuration = 0.3f;
    newFire->textureFrames = 16;

    newFire->position = position;

    return newFire;
}

Object* generateBenche(glm::vec3 position, float rad) {
    
    Object* obj = new Object;
    setObject(position, glm::vec3(0.0f, 1.0f, 0.0f), glm::radians(rad), BENCH_SIZE, BENCH_MODEL_NAME, obj, glm::vec3(0, 0, 0), glm::vec3(0, 0, 0));
    return obj;
}

Object* generateTree(glm::vec3 position, float rad) {
    Object* obj = new Object;
    setObject(position, glm::vec3(1.0f, 0.0f, 0.0f), glm::radians(rad), TREE_SIZE, TREE_MODEL_NAME, obj, glm::vec3(0, 0, 0), glm::vec3(0, 0, 0));
    return obj;

}

Object* createPlant(glm::vec3 position, float rad){
    Object* obj = new Object;
    setObject(position, glm::vec3(0.0f, 1.0f, 0.0f), glm::radians(rad), PLANT_SIZE, PLANT_MODEL_NAME, obj, glm::vec3(0, 0, 0), glm::vec3(0, 0, 0));
    return obj;

}

Object* createHall(glm::vec3 position, float rad) {
    Object* obj = new Object;
    setObject(position, glm::vec3(1.0f, 0.0f, 0.0f), glm::radians(rad), HALL_SIZE, HALL_MODEL_NAME, obj, glm::vec3(0, 0, 0), glm::vec3(0, 0, 0));
    return obj;
}

Object* createHat(glm::vec3 position, float rad) {
    Object* obj = new Object;
    setObject(position, glm::vec3(1.0f, 0.0f, 0.0f), glm::radians(rad), HAT_SIZE, HAT_MODEL_NAME, obj, glm::vec3(0, 0, 0), glm::vec3(0, 0, 0));
    return obj;
}

Object* createBroom(glm::vec3 position, float rad) {
    Object* obj = new Object;
    setObject(position, glm::vec3(1.0f, 0.0f, 0.0f), glm::radians(rad), BROOM_SIZE, BROOM_MODEL_NAME, obj, glm::vec3(0, 0, 0), glm::vec3(0, 0, 0));
    return obj;
}

Object* createWand(glm::vec3 position, float rad) {
    Object* obj = new Object;
    setObject(position, glm::vec3(1.0f, 0.0f, 0.0f), glm::radians(rad), WAND_SIZE, WAND_MODEL_NAME, obj, glm::vec3(0, 0, 0), glm::vec3(0, 0, 0));
    return obj;
}

Object* createRock(glm::vec3 position, float rad) {
    Object* obj = new Object;
    setObject(position, glm::vec3(1.0f, 0.0f, 0.0f), glm::radians(rad), WAND_SIZE*0.01f, ROCK_MODEL_NAME, obj, glm::vec3(0, 0, 0), glm::vec3(0, 0, 0));
    return obj;
}

Object* createFireplace(glm::vec3 position, float rad) {
    Object* obj = new Object;
    setObject(position, glm::vec3(1.0f, 0.0f, 0.0f), glm::radians(rad), PLANT_SIZE, FIREPLACE_MODEL_NAME, obj, glm::vec3(0, 0, 0), glm::vec3(0, 0, 0));
    return obj;
}

void startGame() {

    gameState.skyColour = glm::vec3(0.5f, 0.5f, 0.5f);
    gameState.dayTime = 0; 
    gameState.cameraMode = 1; 
    gameState.elapsedTime = 0.001f * (float)glutGet(GLUT_ELAPSED_TIME); 
    gameState.fogOn = false;

    //static camera 1
    if(gameObjects.camera == NULL)
        gameObjects.camera = new CameraObject;

    gameObjects.camera->yaw = 270.0f;
    gameObjects.camera->pitch = 0;
    gameObjects.camera->speed = 0.0f;
 
    setObject(glm::vec3(-0.2f, 1.0f, -1.3f), glm::vec3(cos(glm::radians(gameObjects.camera->yaw)), 0.0f, sin(glm::radians(gameObjects.camera->yaw))), 0.0f, 0.0f, "none", gameObjects.camera, glm::vec3(0, 0, 0), glm::vec3(0.0f, 0.06f, 1.0f));

    gameObjects.bench1 = generateBenche(glm::vec3(0.0f, -0.1f, -1.5f), 120.0f);
    gameObjects.bench2 = generateBenche(glm::vec3(1.8f, -0.1f, -0.3f), 170.0f);
    Object* tree1 = generateTree(glm::vec3(-3.8f, 0.8f, -6.3f), 270.0f);
    Object* tree2 = generateTree(glm::vec3(-4.5f, 0.8f, -5.0f), 270.0f);
    Object* tree3 = generateTree(glm::vec3(-2.2f, 0.8f, -7.4f), 270.0f);
    Object* tree4 = generateTree(glm::vec3(-1.5, 0.8f, -6.7f), 270.0f);
    Object* tree5 = generateTree(glm::vec3(-0.5f, 0.8f, -7.0f), 270.0f);
    GroundObject* ground = createGround();
    gameObjects.plant = createPlant(glm::vec3(2.5f, 0.0f, -0.4f), 10.0f);
    gameObjects.plant1 = createPlant(glm::vec3(1.2f, -0.18f, -0.5f), 200.0f);
    gameObjects.plant2 = createPlant(glm::vec3(2.49f, -0.05f, 0.0f), 250.0f);
    gameObjects.plant3 = createPlant(glm::vec3(2.53f, 0.05f, 0.45f), 90.0f);
    gameObjects.plant4 = createPlant(glm::vec3(2.8f, 0.1f, -0.3f), 180.0f);
    gameObjects.hall = createHall(glm::vec3(-6.0f, 3.05f, 1.0f), 270.0f);
    gameObjects.eagle = createEagle();
    gameObjects.hat = createHat(glm::vec3(1.5f, 0.05f, 2.3f), 340.0f);
    gameObjects.broom = createBroom(glm::vec3(-8.0f, -0.15f, 4.3f), 0.0f);
    gameObjects.wand = createWand(glm::vec3(-4.0f, 0.1f, 1.3f), 0.0f);
    gameObjects.rock = createRock(glm::vec3(1.0f, -0.4f, -1.3f), 270.0f);
    gameObjects.fire = createFire(glm::vec3(-8.3f, -0.05f, 6.0f));
    gameObjects.fireplace = createFireplace(glm::vec3(-8.3f, -0.15f, 6.0f), 0.0f);

    gameObjects.ground = ground;
    gameObjects.water = createWater();
    gameObjects.tree1 = tree1;
    gameObjects.tree2 = tree2;
    gameObjects.tree3 = tree3;
    gameObjects.tree4 = tree4;
    gameObjects.tree5 = tree5;
   
}

void restartGame() {
    cleanUpObjects();
    startGame();
}

// Called when mouse is moving while no mouse buttons are pressed.
void passiveMouseMotionCallback(int mouseX, int mouseY) {

    if (mouseY != gameState.windowHeight / 2 && mouseX != gameState.windowWidth / 2) {
        float xoffset = mouseX - gameState.windowWidth / 2;
        float yoffset = gameState.windowHeight / 2 - mouseY;

        float sensitivity = 0.05f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        gameObjects.camera->yaw += xoffset;
        gameObjects.camera->pitch += yoffset;

        if (gameObjects.camera->pitch > 89.0f)
            gameObjects.camera->pitch = 89.0f;
        if (gameObjects.camera->pitch < -89.0f)
            gameObjects.camera->pitch = -89.0f;

        glutWarpPointer(gameState.windowWidth / 2, gameState.windowHeight / 2);

        glutPostRedisplay();
    }

}

//setting up camera position and return viewMatrix and ProjectionMatrix
std::tuple<glm::mat4, glm::mat4> setupCamera() {

    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;

    //free camera
    if (gameState.cameraMode == 3) {
        glm::vec3 cameraPosition = gameObjects.camera->position;
        
        glm::vec3 cameraUpVector = glm::vec3(0.0f, 1.0f, 0.0f);

        glm::vec3 front;
        front.x = cos(glm::radians(gameObjects.camera->yaw)) * cos(glm::radians(gameObjects.camera->pitch));
        front.y = sin(glm::radians(gameObjects.camera->pitch));
        front.z = sin(glm::radians(gameObjects.camera->yaw)) * cos(glm::radians(gameObjects.camera->pitch));
        glm::vec3 cameraViewDirection = glm::normalize(front);

        glm::vec3 cameraCenter = cameraPosition + cameraViewDirection;

        gameObjects.camera->direction = cameraViewDirection;

        viewMatrix = glm::lookAt(
            cameraPosition,
            cameraCenter,
            cameraUpVector
        );

        projectionMatrix = glm::perspective(glm::radians(60.0f), gameState.windowWidth / (float)gameState.windowHeight, 0.1f, 10.0f);
        glutPassiveMotionFunc(passiveMouseMotionCallback);
        glutMotionFunc(passiveMouseMotionCallback);
        glutWarpPointer(gameState.windowWidth / 2, gameState.windowHeight / 2);
    }
    else {
        glutPassiveMotionFunc(NULL);
        glutMotionFunc(NULL);
    }

    //first static view (view from starting position)
    if (gameState.cameraMode == 1) {

        glm::vec3 cameraPosition = glm::vec3(0.0f, 0.1f, -1.4f);

        glm::vec3 cameraUpVector = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 cameraCenter;

        glm::vec3 cameraViewDirection = glm::vec3(0.4f, 0, -1);

        cameraCenter = cameraPosition + cameraViewDirection;

        gameObjects.camera->direction = cameraViewDirection;
        gameObjects.camera->position = cameraPosition;
        gameObjects.camera->pitch = 0;
        gameObjects.camera->yaw = 270;

        viewMatrix = glm::lookAt(
            cameraPosition,
            cameraCenter,
            cameraUpVector
        );

        projectionMatrix = glm::perspective(glm::radians(60.0f), gameState.windowWidth / (float)gameState.windowHeight, 0.1f, 10.0f);
    }

    //second static view 
    if (gameState.cameraMode == 2) {
        glm::vec3 cameraPosition = glm::vec3(-8.0f, 0.1f, 5.0f);

        glm::vec3 cameraUpVector = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 cameraCenter;

        glm::vec3 cameraViewDirection = glm::vec3(0.9, 0.5, -1);

        cameraCenter = cameraPosition + cameraViewDirection;

        gameObjects.camera->direction = cameraViewDirection;
        gameObjects.camera->position = cameraPosition;
        gameObjects.camera->pitch = 0;
        gameObjects.camera->yaw = 45;

        viewMatrix = glm::lookAt(
            cameraPosition,
            cameraCenter,
            cameraUpVector
        );

        projectionMatrix = glm::perspective(glm::radians(60.0f), gameState.windowWidth / (float)gameState.windowHeight, 0.1f, 10.0f);
    }
    return std::make_tuple(viewMatrix, projectionMatrix);

}

void drawWindowContents() {

    glm::mat4 viewMatrix, projectionMatrix;
    std::tie(viewMatrix, projectionMatrix) = setupCamera();
    

    glUseProgram(shaderProgram.program);
    glUniform1f(shaderProgram.timeLocation, gameState.elapsedTime);

    glUniform3fv(shaderProgram.torchDirectionLocation, 1, glm::value_ptr(gameObjects.camera->direction));
    glUniform3fv(shaderProgram.torchPositionLocation, 1, glm::value_ptr(gameObjects.camera->position));
    glUniform1i(shaderProgram.torchOnLocation, gameState.torchOn);
    glUniform1f(shaderProgram.dayTimeLocation, gameState.dayTime);
    glUniform3fv(shaderProgram.fogColourLocation, 1, glm::value_ptr(gameState.skyColour));
    glUniform1i(shaderProgram.fogOnLocation, gameState.fogOn);
    glUseProgram(0);

    glUseProgram(skyboxShaderProgram.program);
    glUniform3fv(skyboxShaderProgram.fogColourLocation, 1, glm::value_ptr(gameState.skyColour));
    glUniform1f(skyboxShaderProgram.blendFactorLocation, gameState.dayTime);
    glUniform1f(skyboxShaderProgram.fogActiveLocation, gameState.fogOn);
    
    glUseProgram(0);

    CHECK_GL_ERROR();
    
   
    drawSkybox(viewMatrix, projectionMatrix);
    drawFire(gameObjects.fire, viewMatrix, projectionMatrix);
  

    glEnable(GL_STENCIL_TEST);
    // set the stencil operations - if the stencil test and depth test are passed than

    // value in the stencil buffer is replaced with the object ID (byte 1..255, 0 ... background)
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    //draw all objects 
    glStencilFunc(GL_ALWAYS, 1, -1);
    drawBase(gameObjects.ground, viewMatrix, projectionMatrix);
    glStencilFunc(GL_ALWAYS, 2, -1);
    drawWater(gameObjects.water, viewMatrix, projectionMatrix);
    glStencilFunc(GL_ALWAYS, 3, -1);
    drawPlant(gameObjects.plant, viewMatrix, projectionMatrix);
    drawPlant(gameObjects.plant1, viewMatrix, projectionMatrix);
    drawPlant(gameObjects.plant2, viewMatrix, projectionMatrix);
    drawPlant(gameObjects.plant3, viewMatrix, projectionMatrix);
    drawPlant(gameObjects.plant4, viewMatrix, projectionMatrix);
    glStencilFunc(GL_ALWAYS, 4, -1);
    drawTree(gameObjects.tree1, viewMatrix, projectionMatrix);
    drawTree(gameObjects.tree2, viewMatrix, projectionMatrix);
    drawTree(gameObjects.tree3, viewMatrix, projectionMatrix);
    drawTree(gameObjects.tree4, viewMatrix, projectionMatrix);
    drawTree(gameObjects.tree5, viewMatrix, projectionMatrix);
    glStencilFunc(GL_ALWAYS, 5, -1);
    drawBench(gameObjects.bench1, viewMatrix, projectionMatrix);
    drawBench(gameObjects.bench2, viewMatrix, projectionMatrix);
    glStencilFunc(GL_ALWAYS, 6, -1);
    drawHall(gameObjects.hall, viewMatrix, projectionMatrix);
    glStencilFunc(GL_ALWAYS, 7, -1);
    drawEagle(gameObjects.eagle, viewMatrix, projectionMatrix);
    glStencilFunc(GL_ALWAYS, 8, -1);
    drawHat(gameObjects.hat, viewMatrix, projectionMatrix);
    glStencilFunc(GL_ALWAYS, 9, -1);
    drawBroom(gameObjects.broom, viewMatrix, projectionMatrix);
    glStencilFunc(GL_ALWAYS, 10, -1);
    drawWand(gameObjects.wand, viewMatrix, projectionMatrix);
    glStencilFunc(GL_ALWAYS, 11, -1);
    drawRock(gameObjects.rock, viewMatrix, projectionMatrix);
    glStencilFunc(GL_ALWAYS, 12, -1);
    drawFireplace(gameObjects.fireplace, viewMatrix, projectionMatrix);
}

// Called to update the display. You should call glutSwapBuffers after all of your
// rendering to display what you rendered.
void displayCallback() {

    GLbitfield mask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;

    glClear(mask);

    //menu click
    if (value == 1) {
        restartGame();
    }
    else if (value == 2) {
        gameState.cameraMode = 3;
        glutPassiveMotionFunc(passiveMouseMotionCallback);
    }
    else if (value == 3) {
        gameState.cameraMode = 1;
        glutPassiveMotionFunc(NULL);
    }
    else if (value == 4) {
        gameState.cameraMode = 2;
        glutPassiveMotionFunc(NULL);
    }
    else if (value == 5) {
        //birds view
    }
    else if (value == 6) {
        gameState.torchOn = true;
    }
    else if (value == 7) {
        gameState.torchOn = false;
    }
    else if (value == 8) {
        gameState.fogOn = true;
    }
    else if (value == 9) {
        gameState.fogOn = false;
    }

    drawWindowContents();

    glutSwapBuffers();

}

// Called whenever the window is resized. The new window size is given, in pixels.
// This is an opportunity to call glViewport or glScissor to keep up with the change in size.

void reshapeCallback(int newWidth, int newHeight) {

    gameState.windowWidth = newWidth;
    gameState.windowHeight = newHeight;

    glViewport(0, 0, (GLsizei) newWidth, (GLsizei) newHeight);
}


void updateObjects(float elapsedTime) {
    
    float timeDelta = elapsedTime - gameObjects.camera->currentTime;
    gameObjects.camera->currentTime = elapsedTime;
    if (gameState.cameraMode == 3) {
        glm::vec3 newPosition = changeCameraPosition(gameState.keyMap[KEY_UP_ARROW], gameState.keyMap[KEY_DOWN_ARROW], gameState.keyMap[KEY_RIGHT_ARROW], gameState.keyMap[KEY_LEFT_ARROW], timeDelta);//gameObjects.camera->position + timeDelta * gameObjects.camera->speed * gameObjects.camera->direction;
        if (checkCollisionPond(gameObjects.camera->position + newPosition) && checkCollisionBorder(gameObjects.camera->position + newPosition)) {
            gameObjects.camera->position += newPosition;
        }
    }

    int time = (int) elapsedTime;
    //tracks day and night changes
    if (time % DAY_LENGTH < DAY_LENGTH / 2 && gameState.dayTime > 0.0) {             //day is rising
        gameState.dayTime -= 0.01;
        gameState.dayTime = roundf(gameState.dayTime * 100) / 100;
        gameState.skyColour += glm::vec3(0.005, 0.005, 0.005);
        if (gameState.dayTime == -0.0) { gameState.dayTime = 0.0; }
    }
    else if (time % DAY_LENGTH >= DAY_LENGTH/2 && gameState.dayTime < 1.0) {        //night is falling
        gameState.dayTime += 0.01;
        gameState.dayTime = roundf(gameState.dayTime * 100) / 100;
        gameState.skyColour -= glm::vec3(0.005, 0.005, 0.005);
        if (gameState.dayTime == -0.0) { gameState.dayTime = 0.0; }
    }
    gameObjects.water->currentTime = elapsedTime;
    
    gameObjects.eagle->currentTime = elapsedTime;
    gameObjects.fire->currentTime = elapsedTime;
    //Make the curve and have the eagle path it
    float curveParam = gameObjects.eagle->speed * (gameObjects.eagle->currentTime - gameObjects.eagle->startTime);
    gameObjects.eagle->position = gameObjects.eagle->initPosition + evaluateClosedCurve(curveData, curveSize, curveParam);
    gameObjects.eagle->direction = glm::normalize(evaluateClosedCurve_1stDerivative(curveData, curveSize, curveParam));
    glm::vec3 derivative = evaluateClosedCurve_1stDerivative(curveData, curveSize, curveParam);

}

// Callback responsible for the scene update
void timerCallback(int) {

    gameState.elapsedTime = 0.001f * (float)glutGet(GLUT_ELAPSED_TIME);
    
    updateObjects(gameState.elapsedTime);

    glutTimerFunc(33, timerCallback, 0);

    glutPostRedisplay();

}

void mouseCallback(int buttonPressed, int buttonState, int mouseX, int mouseY) {

    if ((buttonPressed == GLUT_LEFT_BUTTON) && (buttonState == GLUT_DOWN)) {

        //Using stencils to find out where our mouse clicked
        unsigned char itemID = 0;
        glReadPixels(mouseX, gameState.windowHeight - 1 - mouseY, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, &itemID);

        if (itemID == 0) {
            // background was clicked
            std::cout << "Clicked on background" << std::endl;
        }
        else if (itemID == 1) {
            std::cout << "Clicked on ground" << std::endl;
        }
        else if (itemID == 2) {
            std::cout << "Clicked on water" << std::endl;
        }
        else if (itemID == 3) {
            std::cout << "Clicked on plant" << std::endl;

        }
        else if (itemID == 4) {
            std::cout << "Clicked on tree" << std::endl;
        }
        else if (itemID == 5) {
            std::cout << "Clicked on bench" << std::endl;
        }
        else if (itemID == 6) {
            std::cout << "Clicked on hall" << std::endl;
        }
        else if (itemID == 7) {
            std::cout << "Clicked on eagle" << std::endl;
        }
        else if (itemID == 8) {
            std::cout << "Clicked on hat" << std::endl;

        }
        else if (itemID == 9) {
            std::cout << "Clicked on broom" << std::endl;
        }
        else if (itemID == 10) {
            std::cout << "Clicked on torch" << std::endl;
        }
        else if (itemID == 11) {
            std::cout << "Clicked on rock" << std::endl;
        }
    }
    
}

// Called whenever a key on the keyboard was pressed. The key is given by the "keyPressed"
// parameter, which is in ASCII. It's often a good idea to have the escape key (ASCII value 27)
// to call glutLeaveMainLoop() to exit the program.
void keyboardCallback(unsigned char keyPressed, int mouseX, int mouseY) {
  
    switch(keyPressed) {
        case 27: // escape
    #ifndef __APPLE__
            glutLeaveMainLoop();
    #else
            exit(0);
    #endif
            break;
        case 'u':
            gameState.cameraMode = 1;                   //first static view
            break;
        case 'i':
            gameState.cameraMode = 2;                   //second static view
            break;
        case 'o':
            gameState.cameraMode = 3;                   //free camera
            break;
        case 'l':
            gameState.torchOn = !gameState.torchOn;     //switch on/off torch
            break;
        case 'f':
            gameState.fogOn = !gameState.fogOn;         //switch on/off fog
            break;
        case 'r':
            restartGame();         //switch on/off fog
            break;
        case 'd':
            std::cout << glm::to_string(gameObjects.camera->position) << std::endl;;         //switch on/off fog
            break;
        
    }
}

// Called whenever a key on the keyboard was released. The key is given by
// the "keyReleased" parameter, which is in ASCII. 
void keyboardUpCallback(unsigned char keyReleased, int mouseX, int mouseY) {

    switch (keyReleased) {

    default:
        ;
    }
}

// The special keyboard callback is triggered when keyboard function or directional
// keys are pressed.
void specialKeyboardCallback(int specKeyPressed, int mouseX, int mouseY) {

        switch (specKeyPressed) {
        case GLUT_KEY_RIGHT:
            gameState.keyMap[KEY_RIGHT_ARROW] = true;
            break;
        case GLUT_KEY_LEFT:
            gameState.keyMap[KEY_LEFT_ARROW] = true;
            break;
        case GLUT_KEY_UP:
            gameState.keyMap[KEY_UP_ARROW] = true;
            break;
        case GLUT_KEY_DOWN:
            gameState.keyMap[KEY_DOWN_ARROW] = true;
            break;
        default:
            break; // printf("Unrecognized special key pressed\n");
        }
}
// The special keyboard callback is triggered when keyboard function or directional
// keys are released.
void specialKeyboardUpCallback(int specKeyReleased, int mouseX, int mouseY) {
    
        switch (specKeyReleased) {
        case GLUT_KEY_RIGHT:
            gameState.keyMap[KEY_RIGHT_ARROW] = false;
            break;
        case GLUT_KEY_LEFT:
            gameState.keyMap[KEY_LEFT_ARROW] = false;
            break;
        case GLUT_KEY_UP:
            gameState.keyMap[KEY_UP_ARROW] = false;
            break;
        case GLUT_KEY_DOWN:
            gameState.keyMap[KEY_DOWN_ARROW] = false;
            break;
        default:
            break; // printf("Unrecognized special key released\n");
        }
    
}

// Called after the window and OpenGL are initialized. Called exactly once, before the main loop.
void initializeApplication() {

    srand ((unsigned int)time(NULL));

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    initializeShaderPrograms();
    initializeModels();

    gameObjects.camera = NULL;

    startGame();

}

void finalizeApplication(void) {

    cleanUpObjects();

    cleanupModels();

    cleanupShaderPrograms();

}

int main(int argc, char** argv) {
    glutInit(&argc, argv);

#ifndef __APPLE__
    glutInitContextVersion(pgr::OGL_VER_MAJOR, pgr::OGL_VER_MINOR);
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);
#else
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);
#endif

    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow(WINDOW_TITLE);

    //createMenu();

    /*---------------------------------------*/
    //glutSetCursor(GLUT_CURSOR_NONE);

    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);
    glutKeyboardFunc(keyboardCallback);
    glutKeyboardUpFunc(keyboardUpCallback);
    glutSpecialFunc(specialKeyboardCallback);
    glutSpecialUpFunc(specialKeyboardUpCallback);
   
    glutMouseFunc(mouseCallback);

    glutTimerFunc(33, timerCallback, 0);


    if (!pgr::initialize(pgr::OGL_VER_MAJOR, pgr::OGL_VER_MINOR)) {
        pgr::dieWithError("pgr init failed, required OpenGL not supported?");
    }

    initializeApplication();

#ifndef __APPLE__
    glutCloseFunc(finalizeApplication);
#else
    glutWMCloseFunc(finalizeApplication);
#endif

    glutMainLoop();

    return 0;

}
