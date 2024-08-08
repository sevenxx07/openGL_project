//----------------------------------------------------------------------------------------
/**
 * @file    render_stuff.cpp
 * @author  Sára Veselá
 * @date    16/5/2022
 * @brief   Loading and drawing of all models.
 */
 //----------------------------------------------------------------------------------------

#include <iostream>
#include "pgr.h"
#include "render_stuff.h"
#include "spline.h"

//init all geometry
CubeMapGeometry* skyboxGeometry = NULL;
MeshGeometry* groundGeometry = NULL;
MeshGeometry* waterGeometry = NULL;
MeshGeometry* treeGeometry = NULL;
MeshGeometry* plantGeometry = NULL;
MeshGeometry* benchGeometry = NULL;
MeshGeometry* hallGeometry = NULL;
MeshGeometry* eagleGeometry = NULL;
MeshGeometry* hatGeometry = NULL;
MeshGeometry* broomGeometry = NULL;
MeshGeometry* wandGeometry = NULL;
MeshGeometry* rockGeometry = NULL;
MeshGeometry* fireGeometry = NULL;
MeshGeometry* fireplaceGeometry = NULL;

//init shader programs
SCommonShaderProgram    shaderProgram;
SkyboxShaderProgram     skyboxShaderProgram;

float day = 0; //as a gameState day time

//Struct of water shader program
struct waterShaderProgram {
    // identifier for the shader program
    GLuint program;              // = 0;
    // vertex attributes locations
    GLint posLocation;           // = -1;
    GLint texCoordLocation;      // = -1;
    // uniforms locations
    GLint PVMmatrixLocation;     // = -1;
    GLint VmatrixLocation;       // = -1;
    GLint timeLocation;          // = -1;
    GLint texSamplerLocation;    // = -1;
    GLint frameDurationLocation; // = -1;

} waterShaderProgram;

/// Struct for fire shader
struct fireShaderProgram {
    // identifier for the shader program
    GLuint program;              // = 0;
    // vertex attributes locations
    GLint posLocation;           // = -1;
    GLint texCoordLocation;      // = -1;
    // uniforms locations
    GLint PVMmatrixLocation;     // = -1;
    GLint VmatrixLocation;       // = -1;
    GLint timeLocation;          // = -1;
    GLint texSamplerLocation;    // = -1;
    GLint frameDurationLocation; // = -1;
} fireShaderProgram;

//Sends matrices to shader - sets correct transformations
void setTransformUniforms(const glm::mat4 &modelMatrix, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix) {

    glm::mat4 PVMmatrix = projectionMatrix * viewMatrix * modelMatrix;

    glUniformMatrix4fv(shaderProgram.PVMmatrixLocation, 1, GL_FALSE, glm::value_ptr(PVMmatrix));
    glUniformMatrix4fv(shaderProgram.VmatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(shaderProgram.MmatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    const glm::mat4 modelRotationMatrix = glm::mat4(
    modelMatrix[0],
    modelMatrix[1],
    modelMatrix[2],
    glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
    );
    glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelRotationMatrix));

    glUniformMatrix4fv(shaderProgram.normalMatrixLocation, 1, GL_FALSE, glm::value_ptr(normalMatrix));

}

//Sends material specifics to shader - function that has our materials set by uniform
int setMaterialUniforms(const glm::vec3 &ambient, const glm::vec3 &diffuse, const glm::vec3 &specular, float shininess, GLuint texture) {

    glUniform3fv(shaderProgram.diffuseLocation,  1, glm::value_ptr(diffuse));
    glUniform3fv(shaderProgram.ambientLocation,  1, glm::value_ptr(ambient));
    glUniform3fv(shaderProgram.specularLocation, 1, glm::value_ptr(specular));
    glUniform1f(shaderProgram.shininessLocation, shininess);

    if(texture != 0) {
        glUniform1i(shaderProgram.useTextureLocation, 1);
        glUniform1i(shaderProgram.texSamplerLocation, 0);
        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, texture);
    }
    else {
        glUniform1i(shaderProgram.useTextureLocation, 0);
    }

    return 0;

}

//Drawing objects = all with specific parametrs 

void drawRock(Object* rock, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {

    glUseProgram(shaderProgram.program);

    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), rock->position);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(rock->size));
    modelMatrix = glm::rotate(modelMatrix, rock->rotationAngle, rock->direction);

    // send matrices to the vertex & fragment shader
    setTransformUniforms(modelMatrix, viewMatrix, projectionMatrix);

    setMaterialUniforms(
        rockGeometry->ambient,
        rockGeometry->diffuse,
        rockGeometry->specular,
        rockGeometry->shininess,
        rockGeometry->texture
    );

    // draw geometry

    glUniform1i(shaderProgram.useTextureLocation, 1);
    glBindVertexArray(rockGeometry->vertexArrayObject);
    glUniform1i(shaderProgram.texSamplerLocation, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rockGeometry->texture);
    glDrawElements(GL_TRIANGLES, rockGeometry->numTriangles * 3, GL_UNSIGNED_INT, 0);



    glUniform1i(shaderProgram.useTextureLocation, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    return;
}

void drawFire(FireObject* fire, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    glUseProgram(fireShaderProgram.program);

    // just take 3x3 rotation part of the view transform
    glm::mat4 billboardRotationMatrix = glm::mat4(
        viewMatrix[0],
        viewMatrix[1],
        viewMatrix[2],
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
    );
    // inverse view rotation
    billboardRotationMatrix = glm::transpose(billboardRotationMatrix);

    glm::mat4 matrix = glm::translate(glm::mat4(1.0f), fire->position);
    matrix = glm::scale(matrix, glm::vec3(fire->size));
    matrix = matrix * billboardRotationMatrix; // make billboard to face the camera

    glm::mat4 PVMmatrix = projectionMatrix * viewMatrix * matrix;
    glUniformMatrix4fv(fireShaderProgram.PVMmatrixLocation, 1, GL_FALSE, glm::value_ptr(PVMmatrix));  // model-view-projection
    glUniformMatrix4fv(fireShaderProgram.VmatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));   // view
    glUniform1f(fireShaderProgram.timeLocation, fire->currentTime - fire->startTime);
    glUniform1i(fireShaderProgram.texSamplerLocation, 0);
    glUniform1f(fireShaderProgram.frameDurationLocation, fire->frameDuration);

    glBindVertexArray(fireGeometry->vertexArrayObject);
    glBindTexture(GL_TEXTURE_2D, fireGeometry->texture);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, fireGeometry->numTriangles);

    glBindVertexArray(0);
    glUseProgram(0);

    glDisable(GL_BLEND);
}

void drawEagle(MoveableObject* eagle, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {

    glUseProgram(shaderProgram.program);

    glm::mat4 modelMatrix = glm::rotate(modelMatrix, eagle->rotationAngle, eagle->rotdirection);
    modelMatrix = alignObject(eagle->position, eagle->direction, glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(eagle->size));

    // send matrices to the vertex & fragment shader
    setTransformUniforms(modelMatrix, viewMatrix, projectionMatrix);

    setMaterialUniforms(
        eagleGeometry->ambient,
        eagleGeometry->diffuse,
        eagleGeometry->specular,
        eagleGeometry->shininess,
        eagleGeometry->texture
    );

    // draw geometry
    glBindVertexArray(eagleGeometry->vertexArrayObject);
    glDrawElements(GL_TRIANGLES, eagleGeometry->numTriangles * 3, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);

    return;
}


void drawHall(Object* hall, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {

    glUseProgram(shaderProgram.program);

    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), hall->position);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(hall->size));
    modelMatrix = glm::rotate(modelMatrix, hall->rotationAngle, hall->direction);

    // send matrices to the vertex & fragment shader
    setTransformUniforms(modelMatrix, viewMatrix, projectionMatrix);

    int ret = setMaterialUniforms(
        hallGeometry->ambient,
        hallGeometry->diffuse,
        hallGeometry->specular,
        hallGeometry->shininess,
        hallGeometry->texture
    );


    // draw geometry
    glBindVertexArray(hallGeometry->vertexArrayObject);
    glDrawElements(GL_TRIANGLES, hallGeometry->numTriangles * 3, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);

    return;
}

void drawFireplace(Object* fireplace, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {

    glUseProgram(shaderProgram.program);

    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), fireplace->position);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(fireplace->size));
    modelMatrix = glm::rotate(modelMatrix, fireplace->rotationAngle, fireplace->direction);

    // send matrices to the vertex & fragment shader
    setTransformUniforms(modelMatrix, viewMatrix, projectionMatrix);

    int ret = setMaterialUniforms(
        fireplaceGeometry->ambient,
        fireplaceGeometry->diffuse,
        fireplaceGeometry->specular,
        fireplaceGeometry->shininess,
        fireplaceGeometry->texture
    );


    // draw geometry
    glBindVertexArray(fireplaceGeometry->vertexArrayObject);
    glDrawElements(GL_TRIANGLES, fireplaceGeometry->numTriangles * 3, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);

    return;
}

void drawBench(Object* bench, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {

    glUseProgram(shaderProgram.program);

    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), bench->position);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(bench->size));
    modelMatrix = glm::rotate(modelMatrix, bench->rotationAngle, bench->direction);

    // send matrices to the vertex & fragment shader
    setTransformUniforms(modelMatrix, viewMatrix, projectionMatrix);

    setMaterialUniforms(
        benchGeometry->ambient,
        benchGeometry->diffuse,
        benchGeometry->specular,
        benchGeometry->shininess,
        benchGeometry->texture
    );

    // draw geometry
    glBindVertexArray(benchGeometry->vertexArrayObject);
    glDrawElements(GL_TRIANGLES, benchGeometry->numTriangles * 3, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);

    return;
}

void drawHat(Object* hat, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {

    glUseProgram(shaderProgram.program);

    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), hat->position);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(hat->size));
    modelMatrix = glm::rotate(modelMatrix, hat->rotationAngle, hat->direction);

    // send matrices to the vertex & fragment shader
    setTransformUniforms(modelMatrix, viewMatrix, projectionMatrix);

    setMaterialUniforms(
        hatGeometry->ambient,
        hatGeometry->diffuse,
        hatGeometry->specular,
        hatGeometry->shininess,
        hatGeometry->texture
    );

    // draw geometry
    glBindVertexArray(hatGeometry->vertexArrayObject);
    glDrawElements(GL_TRIANGLES, hatGeometry->numTriangles * 3, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);

    return;
}

void drawWand(Object* wand, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {

    glUseProgram(shaderProgram.program);

    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), wand->position);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(wand->size));
    modelMatrix = glm::rotate(modelMatrix, wand->rotationAngle, wand->direction);

    // send matrices to the vertex & fragment shader
    setTransformUniforms(modelMatrix, viewMatrix, projectionMatrix);

    setMaterialUniforms(
        wandGeometry->ambient,
        wandGeometry->diffuse,
        wandGeometry->specular,
        wandGeometry->shininess,
        wandGeometry->texture
    );

    // draw geometry
    glBindVertexArray(wandGeometry->vertexArrayObject);
    glDrawElements(GL_TRIANGLES, wandGeometry->numTriangles * 3, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);

    return;
}

void drawBroom(Object* broom, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {

    glUseProgram(shaderProgram.program);

    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), broom->position);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(broom->size));
    modelMatrix = glm::rotate(modelMatrix, broom->rotationAngle, broom->direction);

    // send matrices to the vertex & fragment shader
    setTransformUniforms(modelMatrix, viewMatrix, projectionMatrix);

    setMaterialUniforms(
        broomGeometry->ambient,
        broomGeometry->diffuse,
        broomGeometry->specular,
        broomGeometry->shininess,
        broomGeometry->texture
    );

    // draw geometry
    glBindVertexArray(broomGeometry->vertexArrayObject);
    glDrawElements(GL_TRIANGLES, broomGeometry->numTriangles * 3, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);

    return;
}


void drawPlant(Object* plant, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {

    glUseProgram(shaderProgram.program);

    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), plant->position);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(plant->size));

    // send matrices to the vertex & fragment shader
    setTransformUniforms(modelMatrix, viewMatrix, projectionMatrix);

    setMaterialUniforms(
        plantGeometry->ambient,
        plantGeometry->diffuse,
        plantGeometry->specular,
        plantGeometry->shininess,
        plantGeometry->texture
    );

    // draw geometry
    glBindVertexArray(plantGeometry->vertexArrayObject);
    glDrawElements(GL_TRIANGLES, plantGeometry->numTriangles * 3, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);

    return;
}

void drawTree(Object* tree, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {

    glUseProgram(shaderProgram.program);

    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), tree->position);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(tree->size));
    modelMatrix = glm::rotate(modelMatrix, tree->rotationAngle, tree->direction);

    // send matrices to the vertex & fragment shader
    setTransformUniforms(modelMatrix, viewMatrix, projectionMatrix);

    setMaterialUniforms(
        treeGeometry->ambient,
        treeGeometry->diffuse,
        treeGeometry->specular,
        treeGeometry->shininess,
        treeGeometry->texture
    );

    // draw geometry
    glBindVertexArray(treeGeometry->vertexArrayObject);
    glDrawElements(GL_TRIANGLES, treeGeometry->numTriangles * 3, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);

    return;
}

void drawWater(WaterObject* water, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    glUseProgram(waterShaderProgram.program);

    glm::mat4 matrix = glm::translate(glm::mat4(1.0f), water->position);
    matrix = glm::scale(matrix, glm::vec3(water->size * 22));
    matrix = glm::rotate(matrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));


    glm::mat4 PVMmatrix = projectionMatrix * viewMatrix * matrix;
    glUniformMatrix4fv(waterShaderProgram.PVMmatrixLocation, 1, GL_FALSE, glm::value_ptr(PVMmatrix));  // model-view-projection
    glUniformMatrix4fv(waterShaderProgram.VmatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));   // view
    glUniform1f(waterShaderProgram.timeLocation, water->currentTime - water->startTime);
    glUniform1i(waterShaderProgram.texSamplerLocation, 0);
    glUniform1f(waterShaderProgram.frameDurationLocation, water->frameDuration);

    glBindVertexArray(waterGeometry->vertexArrayObject);
    glBindTexture(GL_TEXTURE_2D, waterGeometry->texture);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, waterGeometry->numTriangles);

    glBindVertexArray(0);
    glUseProgram(0);

    glDisable(GL_BLEND);

    return;
}

void drawBase(GroundObject* ground, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {

    glUseProgram(shaderProgram.program);

    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), ground->position);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(ground->size));

    // send matrices to the vertex & fragment shader
    setTransformUniforms(modelMatrix, viewMatrix, projectionMatrix);

    setMaterialUniforms(
        groundGeometry->ambient,
        groundGeometry->diffuse,
        groundGeometry->specular,
        groundGeometry->shininess,
        groundGeometry->texture
    );

    // draw geometry
    glBindVertexArray(groundGeometry->vertexArrayObject);
    glDrawElements(GL_TRIANGLES, groundGeometry->numTriangles * 3, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);

    return;
}

void cleanupShaderPrograms() {

    pgr::deleteProgramAndShaders(shaderProgram.program);
    pgr::deleteProgramAndShaders(skyboxShaderProgram.program);
    pgr::deleteProgramAndShaders(fireShaderProgram.program);
    pgr::deleteProgramAndShaders(waterShaderProgram.program);

}

void initializeShaderPrograms() {

    std::vector<GLuint> shaderList;

    if (!day == 1) {

        shaderList.push_back(pgr::createShaderFromFile(GL_VERTEX_SHADER, "lightingPerVertex.vert"));
        shaderList.push_back(pgr::createShaderFromFile(GL_FRAGMENT_SHADER, "lightingPerVertex.frag"));

        shaderProgram.program = pgr::createProgram(shaderList);

        shaderProgram.posLocation = glGetAttribLocation(shaderProgram.program, "position");
        shaderProgram.normalLocation = glGetAttribLocation(shaderProgram.program, "normal");
        shaderProgram.texCoordLocation = glGetAttribLocation(shaderProgram.program, "texCoord");

        shaderProgram.PVMmatrixLocation = glGetUniformLocation(shaderProgram.program, "PVMmatrix");
        shaderProgram.VmatrixLocation = glGetUniformLocation(shaderProgram.program, "Vmatrix");
        shaderProgram.MmatrixLocation = glGetUniformLocation(shaderProgram.program, "Mmatrix");
        shaderProgram.normalMatrixLocation = glGetUniformLocation(shaderProgram.program, "normalMatrix");
        shaderProgram.timeLocation = glGetUniformLocation(shaderProgram.program, "time");

        shaderProgram.ambientLocation = glGetUniformLocation(shaderProgram.program, "material.ambient");
        shaderProgram.diffuseLocation = glGetUniformLocation(shaderProgram.program, "material.diffuse");
        shaderProgram.specularLocation = glGetUniformLocation(shaderProgram.program, "material.specular");
        shaderProgram.shininessLocation = glGetUniformLocation(shaderProgram.program, "material.shininess");

        shaderProgram.texSamplerLocation = glGetUniformLocation(shaderProgram.program, "texSampler");
        shaderProgram.useTextureLocation = glGetUniformLocation(shaderProgram.program, "material.useTexture");

        shaderProgram.torchPositionLocation = glGetUniformLocation(shaderProgram.program, "torchPosition");
        shaderProgram.torchDirectionLocation = glGetUniformLocation(shaderProgram.program, "torchDirection");
        shaderProgram.torchOnLocation = glGetUniformLocation(shaderProgram.program, "torchOn");
        shaderProgram.dayTimeLocation = glGetUniformLocation(shaderProgram.program, "dayTime");
        shaderProgram.fogColourLocation = glGetUniformLocation(shaderProgram.program, "fogColour");
        shaderProgram.fogOnLocation = glGetUniformLocation(shaderProgram.program, "fogOn");
        shaderList.clear();
    }
    

    shaderList.clear();

    shaderList.push_back(pgr::createShaderFromFile(GL_VERTEX_SHADER, "skybox.vert"));
    shaderList.push_back(pgr::createShaderFromFile(GL_FRAGMENT_SHADER, "skybox.frag"));

    skyboxShaderProgram.program = pgr::createProgram(shaderList);

    skyboxShaderProgram.screenCoordLocation = glGetAttribLocation(skyboxShaderProgram.program, "screenCoord");
    skyboxShaderProgram.skyboxSamplerLocation = glGetUniformLocation(skyboxShaderProgram.program, "skyboxSampler");
    skyboxShaderProgram.skyboxSampler2Location = glGetUniformLocation(skyboxShaderProgram.program, "skyboxSampler2");
    skyboxShaderProgram.inversePVmatrixLocation = glGetUniformLocation(skyboxShaderProgram.program, "inversePVmatrix");

    skyboxShaderProgram.fogColourLocation = glGetUniformLocation(skyboxShaderProgram.program, "fogColour");
    skyboxShaderProgram.fogActiveLocation = glGetUniformLocation(skyboxShaderProgram.program, "fogActive");
    skyboxShaderProgram.blendFactorLocation = glGetUniformLocation(skyboxShaderProgram.program, "blendFactor");

    shaderList.clear();


        shaderList.push_back(pgr::createShaderFromFile(GL_VERTEX_SHADER, "water.vert"));
        shaderList.push_back(pgr::createShaderFromFile(GL_FRAGMENT_SHADER, "water.frag"));

        waterShaderProgram.program = pgr::createProgram(shaderList);

        waterShaderProgram.posLocation = glGetAttribLocation(waterShaderProgram.program, "position");
        waterShaderProgram.texCoordLocation = glGetAttribLocation(waterShaderProgram.program, "texCoord");
        waterShaderProgram.PVMmatrixLocation = glGetUniformLocation(waterShaderProgram.program, "PVMmatrix");
        waterShaderProgram.timeLocation = glGetUniformLocation(waterShaderProgram.program, "time");
        waterShaderProgram.VmatrixLocation = glGetUniformLocation(waterShaderProgram.program, "Vmatrix");
        waterShaderProgram.texSamplerLocation = glGetUniformLocation(waterShaderProgram.program, "texSampler");
        waterShaderProgram.frameDurationLocation = glGetUniformLocation(waterShaderProgram.program, "frameDuration");
        
        shaderList.clear();

        // push vertex shader and fragment shader
        shaderList.push_back(pgr::createShaderFromFile(GL_VERTEX_SHADER, "dynamicTexture.vert"));
        shaderList.push_back(pgr::createShaderFromFile(GL_FRAGMENT_SHADER, "dynamicTexture.frag"));

        // create the program with two shaders
        fireShaderProgram.program = pgr::createProgram(shaderList);

        // get position and texture coordinates attributes locations
        fireShaderProgram.posLocation = glGetAttribLocation(fireShaderProgram.program, "position");
        fireShaderProgram.texCoordLocation = glGetAttribLocation(fireShaderProgram.program, "texCoord");
        // get uniforms locations
        fireShaderProgram.PVMmatrixLocation = glGetUniformLocation(fireShaderProgram.program, "PVMmatrix");
        fireShaderProgram.VmatrixLocation = glGetUniformLocation(fireShaderProgram.program, "Vmatrix");
        fireShaderProgram.timeLocation = glGetUniformLocation(fireShaderProgram.program, "time");
        fireShaderProgram.texSamplerLocation = glGetUniformLocation(fireShaderProgram.program, "texSampler");
        fireShaderProgram.frameDurationLocation = glGetUniformLocation(fireShaderProgram.program, "frameDuration");
}


//load single mesh
bool loadSingleMesh(const std::string& fileName, SCommonShaderProgram& shader, MeshGeometry** geometry) {
    Assimp::Importer importer;

    // Unitize object in size (scale the model to fit into (-1..1)^3)
    importer.SetPropertyInteger(AI_CONFIG_PP_PTV_NORMALIZE, 1);

    // Load asset from the file - you can play with various processing steps
    const aiScene* scn = importer.ReadFile(fileName.c_str(), 0
        | aiProcess_Triangulate             // Triangulate polygons (if any).
        | aiProcess_PreTransformVertices    // Transforms scene hierarchy into one root with geometry-leafs only. For more see Doc.
        | aiProcess_GenSmoothNormals        // Calculate normals per vertex.
        | aiProcess_JoinIdenticalVertices);

    // abort if the loader fails
    if (scn == NULL) {
        std::cerr << "assimp error: " << importer.GetErrorString() << std::endl;
        *geometry = NULL;
        return false;
    }

    // some formats store whole scene (multiple meshes and materials, lights, cameras, ...) in one file, we cannot handle that in our simplified example
    if (scn->mNumMeshes != 1) {
        std::cerr << "this simplified loader can only process files with only one mesh" << std::endl;
        *geometry = NULL;
        return false;
    }

    // in this phase we know we have one mesh in our loaded scene, we can directly copy its data to OpenGL ...
    const aiMesh* mesh = scn->mMeshes[0];

    *geometry = new MeshGeometry;

    // vertex buffer object, store all vertex positions and normals
    glGenBuffers(1, &((*geometry)->vertexBufferObject));
    glBindBuffer(GL_ARRAY_BUFFER, (*geometry)->vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float) * mesh->mNumVertices, 0, GL_STATIC_DRAW); // allocate memory for vertices, normals, and texture coordinates
    // first store all vertices
    glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * sizeof(float) * mesh->mNumVertices, mesh->mVertices);
    // then store all normals
    glBufferSubData(GL_ARRAY_BUFFER, 3 * sizeof(float) * mesh->mNumVertices, 3 * sizeof(float) * mesh->mNumVertices, mesh->mNormals);

    // just texture 0 for now
    float* textureCoords = new float[2 * mesh->mNumVertices];  // 2 floats per vertex
    float* currentTextureCoord = textureCoords;

    // copy texture coordinates
    aiVector3D vect;

    if (mesh->HasTextureCoords(0)) {
        // we use 2D textures with 2 coordinates and ignore the third coordinate
        for (unsigned int idx = 0; idx < mesh->mNumVertices; idx++) {
            vect = (mesh->mTextureCoords[0])[idx];
            *currentTextureCoord++ = vect.x;
            *currentTextureCoord++ = vect.y;
        }
    }

    // finally store all texture coordinates
    glBufferSubData(GL_ARRAY_BUFFER, 6 * sizeof(float) * mesh->mNumVertices, 2 * sizeof(float) * mesh->mNumVertices, textureCoords);

    // copy all mesh faces into one big array (assimp supports faces with ordinary number of vertices, we use only 3 -> triangles)
    unsigned int* indices = new unsigned int[mesh->mNumFaces * 3];
    for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
        indices[f * 3 + 0] = mesh->mFaces[f].mIndices[0];
        indices[f * 3 + 1] = mesh->mFaces[f].mIndices[1];
        indices[f * 3 + 2] = mesh->mFaces[f].mIndices[2];
    }

    // copy our temporary index array to OpenGL and free the array
    glGenBuffers(1, &((*geometry)->elementBufferObject));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (*geometry)->elementBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(unsigned) * mesh->mNumFaces, indices, GL_STATIC_DRAW);

    delete[] indices;

    // copy the material info to MeshGeometry structure
    const aiMaterial* mat = scn->mMaterials[mesh->mMaterialIndex];
    aiColor4D color;
    aiString name;
    aiReturn retValue = AI_SUCCESS;

    // Get returns: aiReturn_SUCCESS 0 | aiReturn_FAILURE -1 | aiReturn_OUTOFMEMORY -3
    mat->Get(AI_MATKEY_NAME, name); // may be "" after the input mesh processing. Must be aiString type!

    if ((retValue = aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &color)) != AI_SUCCESS)
        color = aiColor4D(0.0f, 0.0f, 0.0f, 0.0f);

    (*geometry)->diffuse = glm::vec3(color.r, color.g, color.b);

    if ((retValue = aiGetMaterialColor(mat, AI_MATKEY_COLOR_AMBIENT, &color)) != AI_SUCCESS)
        color = aiColor4D(0.0f, 0.0f, 0.0f, 0.0f);
    (*geometry)->ambient = glm::vec3(color.r, color.g, color.b);

    if ((retValue = aiGetMaterialColor(mat, AI_MATKEY_COLOR_SPECULAR, &color)) != AI_SUCCESS)
        color = aiColor4D(0.0f, 0.0f, 0.0f, 0.0f);
    (*geometry)->specular = glm::vec3(color.r, color.g, color.b);

    ai_real shininess, strength;
    unsigned int max;	// changed: to unsigned

    max = 1;
    if ((retValue = aiGetMaterialFloatArray(mat, AI_MATKEY_SHININESS, &shininess, &max)) != AI_SUCCESS)
        shininess = 1.0f;
    max = 1;
    if ((retValue = aiGetMaterialFloatArray(mat, AI_MATKEY_SHININESS_STRENGTH, &strength, &max)) != AI_SUCCESS)
        strength = 1.0f;
    (*geometry)->shininess = shininess * strength;

    (*geometry)->texture = 0;

    // load texture image
    if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        // get texture name 
        aiString path; // filename

        aiReturn texFound = mat->GetTexture(aiTextureType_DIFFUSE, 0, &path);
        std::string textureName = path.data;

        size_t found = fileName.find_last_of("/\\");
        // insert correct texture file path 
        if (found != std::string::npos) { // not found
          //subMesh_p->textureName.insert(0, "/");
            textureName.insert(0, fileName.substr(0, found + 1));
        }

        std::cout << "Loading texture file: " << textureName << std::endl;
        (*geometry)->texture = pgr::createTexture(textureName);
    }
    CHECK_GL_ERROR();

    glGenVertexArrays(1, &((*geometry)->vertexArrayObject));
    glBindVertexArray((*geometry)->vertexArrayObject);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (*geometry)->elementBufferObject); // bind our element array buffer (indices) to vao
    glBindBuffer(GL_ARRAY_BUFFER, (*geometry)->vertexBufferObject);

    glEnableVertexAttribArray(shader.posLocation);
    glVertexAttribPointer(shader.posLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

    if (!day == 1) {
        glEnableVertexAttribArray(shader.normalLocation);
        glVertexAttribPointer(shader.normalLocation, 3, GL_FLOAT, GL_FALSE, 0, (void*)(3 * sizeof(float) * mesh->mNumVertices));
    }
    else {
        glDisableVertexAttribArray(shader.colorLocation);
        // following line is problematic on AMD/ATI graphic cards
        // -> if you see black screen (no objects at all) than try to set color manually in vertex shader to see at least something
        glVertexAttrib3f(shader.colorLocation, color.r, color.g, color.b);
    }

    glEnableVertexAttribArray(shader.texCoordLocation);
    glVertexAttribPointer(shader.texCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, (void*)(6 * sizeof(float) * mesh->mNumVertices));
    CHECK_GL_ERROR();

    glBindVertexArray(0);

    (*geometry)->numTriangles = mesh->mNumFaces;

    return true;
}

//Init all geometries we need 

void initfireGeometry(GLuint shader, MeshGeometry** geometry) {
    *geometry = new MeshGeometry;

    (*geometry)->texture = pgr::createTexture(FIRE_TEXTURE_NAME);
    CHECK_GL_ERROR();
    glGenVertexArrays(1, &((*geometry)->vertexArrayObject));
    glBindVertexArray((*geometry)->vertexArrayObject);

    glGenBuffers(1, &((*geometry)->vertexBufferObject)); \
        glBindBuffer(GL_ARRAY_BUFFER, (*geometry)->vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fireVertexData), fireVertexData, GL_STATIC_DRAW);

    glEnableVertexAttribArray(fireShaderProgram.posLocation);
    // vertices of triangles - start at the beginning of the array (interlaced array)
    glVertexAttribPointer(fireShaderProgram.posLocation, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);

    glEnableVertexAttribArray(fireShaderProgram.texCoordLocation);
    // texture coordinates are placed just after the position of each vertex (interlaced array)
    glVertexAttribPointer(fireShaderProgram.texCoordLocation, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);

    (*geometry)->numTriangles = fireNumQuadVertices;
    CHECK_GL_ERROR();
}

void initSkyboxGeometry(GLuint shader, CubeMapGeometry** geometry){

    *geometry = new CubeMapGeometry;

    glGenVertexArrays(1, &((*geometry)->vertexArrayObject));
    glBindVertexArray((*geometry)->vertexArrayObject);

    // buffer for far plane rendering
    glGenBuffers(1, &((*geometry)->vertexBufferObject));
    glBindBuffer(GL_ARRAY_BUFFER, (*geometry)->vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    glGenTextures(1, &((*geometry)->texture));
    glGenTextures(1, &((*geometry)->texture2));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, (*geometry)->texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, (*geometry)->texture2);


    glEnableVertexAttribArray(skyboxShaderProgram.screenCoordLocation);
    glVertexAttribPointer(skyboxShaderProgram.screenCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);
    glUseProgram(0);
    CHECK_GL_ERROR();

    (*geometry)->numTriangles = 2;

    glActiveTexture(GL_TEXTURE0);

    GLuint targets[] = {
      GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
      GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
      GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };

    for (int i = 0; i < 6; i++) {
        std::string texName = std::string(SKYBOX_PREFIX_DAY) + std::to_string(i+1) + ".png";
        
        if (!pgr::loadTexImage2D(texName, targets[i])) {
            pgr::dieWithError("Skybox cube map 1 loading failed!");
        }
    }

    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    glActiveTexture(GL_TEXTURE1);

    for (int i = 0; i < 6; i++) {
        std::string texName = std::string(SKYBOX_PREFIX_NIGHT) + std::to_string(i + 1) + ".jpg";
        
        if (!pgr::loadTexImage2D(texName, targets[i])) {
            pgr::dieWithError("Skybox cube map 2 loading failed!");
        }
    }

    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glActiveTexture(GL_TEXTURE0);

}

void initWaterGeometry(GLuint shader, MeshGeometry** geometry) {

    *geometry = new MeshGeometry;

    (*geometry)->texture = pgr::createTexture(WATER_TEXTURE_NAME);

    glGenVertexArrays(1, &((*geometry)->vertexArrayObject));
    glBindVertexArray((*geometry)->vertexArrayObject);

    glGenBuffers(1, &((*geometry)->vertexBufferObject)); \
        glBindBuffer(GL_ARRAY_BUFFER, (*geometry)->vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(waterVertexData), waterVertexData, GL_STATIC_DRAW);

    glEnableVertexAttribArray(waterShaderProgram.posLocation);
    // vertices of triangles - start at the beginning of the array (interlaced array)
    glVertexAttribPointer(waterShaderProgram.posLocation, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);

    glEnableVertexAttribArray(waterShaderProgram.texCoordLocation);
    // texture coordinates are placed just after the position of each vertex (interlaced array)
    glVertexAttribPointer(waterShaderProgram.texCoordLocation, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);

    (*geometry)->numTriangles = waterNumQuadVertices;
}

void initRockGeometry(SCommonShaderProgram& shader, MeshGeometry** geometry) {

    *geometry = new MeshGeometry;

    glGenVertexArrays(1, &((*geometry)->vertexArrayObject));
    glBindVertexArray((*geometry)->vertexArrayObject);

    glGenBuffers(1, &((*geometry)->vertexBufferObject));
    glBindBuffer(GL_ARRAY_BUFFER, (*geometry)->vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float) * cliff_rock_two_objNVertices, cliff_rock_two_objVertices, GL_STATIC_DRAW);
    (*geometry)->texture = pgr::createTexture("data/rock_hardcoded/rock_texture.png");

    // copy our temporary index array to opengl and free the array
    glGenBuffers(1, &((*geometry)->elementBufferObject));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (*geometry)->elementBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(unsigned int) * cliff_rock_two_objNTriangles, cliff_rock_two_objTriangles, GL_STATIC_DRAW);

    glEnableVertexAttribArray(shader.posLocation);
    // vertices of triangles - start at the beginning of the array
    glVertexAttribPointer(shader.posLocation, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);

    glEnableVertexAttribArray(shader.normalLocation);
    // normal of vertex starts after the color (interlaced array)
    glVertexAttribPointer(shader.normalLocation, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    (*geometry)->ambient = glm::vec3(1.0f, 1.0f, 1.0f);
    (*geometry)->diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
    (*geometry)->specular = glm::vec3(1.0f, 1.0f, 1.0f);
    (*geometry)->shininess = 10.0f;
    glEnableVertexAttribArray(shader.texCoordLocation);
    glVertexAttribPointer(shader.texCoordLocation, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);

    (*geometry)->numTriangles = cliff_rock_two_objNTriangles;
}


void drawSkybox(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {

    glUseProgram(skyboxShaderProgram.program);

    glm::mat4 matrix = projectionMatrix * viewMatrix;
    glm::mat4 viewRotation = viewMatrix;
    viewRotation[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glm::mat4 inversePVmatrix = glm::inverse(projectionMatrix * viewRotation);

    glUniformMatrix4fv(skyboxShaderProgram.inversePVmatrixLocation, 1, GL_FALSE, glm::value_ptr(inversePVmatrix));
    glUniform1i(skyboxShaderProgram.skyboxSamplerLocation, 0);
    glUniform1i(skyboxShaderProgram.skyboxSampler2Location, 1);

    glBindVertexArray(skyboxGeometry->vertexArrayObject);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxGeometry->texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxGeometry->texture2);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, skyboxGeometry->numTriangles + 2);
    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(0);
    glUseProgram(0);

}

void initializeModels() {

    if (loadSingleMesh(GROUND_MODEL_NAME, shaderProgram, &groundGeometry) != true) {
        std::cerr << "initializeModels(): Ground model loading failed." << std::endl;
    }
    CHECK_GL_ERROR();
    if (loadSingleMesh(PLANT_MODEL_NAME, shaderProgram, &plantGeometry) != true) {
        std::cerr << "initializeModels(): Ground model loading failed." << std::endl;
    }
    CHECK_GL_ERROR();
    if (loadSingleMesh(TREE_MODEL_NAME, shaderProgram, &treeGeometry) != true) {
        std::cerr << "initializeModels(): Tree model loading failed." << std::endl;
    }
    CHECK_GL_ERROR();
    if (loadSingleMesh(BENCH_MODEL_NAME, shaderProgram, &benchGeometry) != true) {
        std::cerr << "initializeModels(): Bench model loading failed." << std::endl;
    }
    CHECK_GL_ERROR();
    if (loadSingleMesh(HALL_MODEL_NAME, shaderProgram, &hallGeometry) != true) {
        std::cerr << "initializeModels(): Hall model loading failed." << std::endl;
    }
    CHECK_GL_ERROR();
    if (loadSingleMesh(EAGLE_MODEL_NAME, shaderProgram, &eagleGeometry) != true) {
        std::cerr << "initializeModels(): Eagle model loading failed." << std::endl;
    }
    if (loadSingleMesh(HAT_MODEL_NAME, shaderProgram, &hatGeometry) != true) {
        std::cerr << "initializeModels(): Hat model loading failed." << std::endl;
    }
    CHECK_GL_ERROR();
    if (loadSingleMesh(BROOM_MODEL_NAME, shaderProgram, &broomGeometry) != true) {
        std::cerr << "initializeModels(): Broom model loading failed." << std::endl;
    }
    CHECK_GL_ERROR();
    if (loadSingleMesh(WAND_MODEL_NAME, shaderProgram, &wandGeometry) != true) {
        std::cerr << "initializeModels(): Torch model loading failed." << std::endl;
    }
    CHECK_GL_ERROR();
    if (loadSingleMesh(FIREPLACE_MODEL_NAME, shaderProgram, &fireplaceGeometry) != true) {
        std::cerr << "initializeModels(): Fireplace model loading failed." << std::endl;
    }
    CHECK_GL_ERROR();
    initSkyboxGeometry(skyboxShaderProgram.program, &skyboxGeometry);
    initWaterGeometry(waterShaderProgram.program, &waterGeometry);
    initRockGeometry(shaderProgram, &rockGeometry);
    initfireGeometry(fireShaderProgram.program, &fireGeometry);
    CHECK_GL_ERROR();

}

void cleanupGeometry(MeshGeometry *geometry) {

    glDeleteVertexArrays(1, &(geometry->vertexArrayObject));
    glDeleteBuffers(1, &(geometry->elementBufferObject));
    glDeleteBuffers(1, &(geometry->vertexBufferObject));

    if (geometry->texture != 0) {
        glDeleteTextures(1, &(geometry->texture));
    }

}

// Deletes all geometries
void cleanupModels() {

    cleanupGeometry(plantGeometry);
    cleanupGeometry(treeGeometry);
    cleanupGeometry(benchGeometry);
    cleanupGeometry(hallGeometry);
    cleanupGeometry(eagleGeometry);
    cleanupGeometry(hatGeometry);
    cleanupGeometry(broomGeometry);

    cleanupGeometry(fireGeometry);
    cleanupGeometry(fireplaceGeometry);

    glDeleteTextures(1, &(skyboxGeometry->texture2));
    cleanupGeometry(skyboxGeometry);
    cleanupGeometry(groundGeometry);

    cleanupGeometry(waterGeometry);

}
