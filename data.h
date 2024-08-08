//----------------------------------------------------------------------------------------
/**
 * @file    data.h
 * @author  Sára Veselá
 * @date    16/5/2022
 * @brief   Setting all constant data - paths, vertex arrays etc.
 */
 //----------------------------------------------------------------------------------------

#ifndef __DATA_H
#define __DATA_H

#define WINDOW_WIDTH   750
#define WINDOW_HEIGHT  750
#define WINDOW_TITLE   "Hogwart grounds"

enum { KEY_LEFT_ARROW, KEY_RIGHT_ARROW, KEY_UP_ARROW, KEY_DOWN_ARROW, KEYS_COUNT };

#define BENCH_SIZE      0.5f
#define TREE_SIZE       1.1f
#define PLANT_SIZE      0.20f
#define GRASS_SIZE      0.10f
#define HAT_SIZE        0.35f
#define BROOM_SIZE      0.3f
#define WAND_SIZE       0.1f
#define HALL_SIZE       4.0f
#define GROUND_SIZE     10.0f;
#define EAGLE_SIZE      0.4f;

#define BILLBOARD_SIZE   0.1f
#define FIRE_BILLBOARD_SIZE   0.1f

#define BENCH_MODEL_NAME        "data/bench/bench.obj"
#define TREE_MODEL_NAME         "data/trees/10447_Pine_Tree_v1_L3b.obj"
#define PLANT_MODEL_NAME        "data/plant/kapradi.obj"
#define HAT_MODEL_NAME          "data/hat/hat2.obj"
#define BROOM_MODEL_NAME        "data/broom/brooml.obj"
#define WAND_MODEL_NAME         "data/torch/torch.obj"
#define SKYBOX_PREFIX_DAY       "data/skyboxd/"
#define SKYBOX_PREFIX_NIGHT     "data/skyboxn/"
#define HALL_MODEL_NAME         "data/hall/compHall.obj"
#define EAGLE_MODEL_NAME        "data/bird/eagle.obj"
#define ROCK_MODEL_NAME         "cliff_rock_two_obj.h"
#define FIREPLACE_MODEL_NAME    "data/Fireplace/fireplace.obj"
#define FIRE_TEXTURE_NAME       "data/Fireplace/fire.png"

#define GROUND_MODEL_NAME       "data/ground/base2.obj"

#define WATER_TEXTURE_NAME      "data/water.png"

#define VIEW_ANGLE_DELTA 2.0f
#define DAY_LENGTH       30

#define SKYBOX_SPEED        1.0f
#define CAMERA_SPEED        1.0f
#define DELTA_SPEED         3.3f
#define REACH               0.5f
#define ACCELERATION        1.5f         

#define SCENE_WIDTH  1.0f
#define SCENE_HEIGHT 1.0f
#define SCENE_DEPTH  1.0f

#define CAMERA_ELEVATION_MAX 25.0f


const std::string colorVertexShaderSrc(
    "#version 140\n"
    "uniform mat4 PVMmatrix;\n"
    "in vec3 position;\n"
    "in vec3 color;\n"
    "smooth out vec4 theColor;\n"
    "void main() {\n"
    "  gl_Position = PVMmatrix * vec4(position, 1.0);\n"
    "  theColor = vec4(color, 1.0);\n"
    "}\n"
);

const std::string colorFragmentShaderSrc(
    "#version 140\n"
    "smooth in vec4 theColor;\n"
    "out vec4 outputColor;\n"
    "void main() {\n"
    "  outputColor = theColor;\n"

    "}\n"
);


//vertices for skybox
static const float skyboxVertices[] = {
    -1.0f, -1.0f,
     1.0f, -1.0f,
    -1.0f,  1.0f,
     1.0f,  1.0f
};

//vetices for water
const int waterNumQuadVertices = 4;
const float waterVertexData[waterNumQuadVertices * 5] = {

    // x      y     z     u     v
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
     1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
     1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
};

//vertices for fire
const int fireNumQuadVertices = 4;
const float fireVertexData[fireNumQuadVertices * 5] = {
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
     1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
     1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
};


#endif
