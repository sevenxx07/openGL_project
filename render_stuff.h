//----------------------------------------------------------------------------------------
/**
 * @file    render_stuff.h
 * @author  Sára Veselá
 * @date    16/5/2022
 * @brief   Defined geometry, shader and object structures.
 */
 //----------------------------------------------------------------------------------------

#ifndef __RENDER_STUFF_H
#define __RENDER_STUFF_H

#include "data.h"
#include "cliff_rock_two_obj.h"

//Struct with VBO, VAO, EBO, unique id, material specifics and texture
typedef struct MeshGeometry {
  GLuint        vertexBufferObject;
  GLuint        elementBufferObject;
  GLuint        vertexArrayObject;
  unsigned int  numTriangles;
  // id is used to recognize, which geometry belongs to which object
  std::string	id;

  //material specifics
  glm::vec3     ambient;
  glm::vec3     diffuse;
  glm::vec3     specular;
  float         shininess;
  GLuint        texture;
} MeshGeometry;

//MeshGeometry with one added texture pointer for multitexturing
typedef struct CubeMapGeometry : MeshGeometry{

	GLuint		  texture2;

} CubeMapGeometry;


//Struct with important object information
typedef struct Object {

  glm::vec3 position;
  glm::vec3 direction;		//rotation axis
  float		rotationAngle;
  float     size;

  float startTime;
  float currentTime;

  std::string id;

 
} Object;

//Struct for camera, added speed and yaw and pitch angles of view
typedef struct CameraObject : public Object {

  float speed;		//how fast camera is moving

  float yaw;		//view angle to side
  float pitch;		//view angle up and down

} CameraObject;

//Struct for objects that is moving
typedef struct MoveableObject : public Object {
	glm::vec3 initPosition;
	float speed;
	glm::vec3 rotdirection;
} MoveableObject;

//Object for ground
typedef struct GroundObject : public Object {

}GroundObject;

//water object = moveable texture
typedef struct WaterObject : public Object {
	int    textureFrames;
	float  frameDuration;
	float  speed;

};

//fire object = dynamic texture
typedef struct FireObject : public Object {
	int    textureFrames;
	float  frameDuration;

};
//Shader program for common objects
typedef struct _commonShaderProgram {
  
  GLuint program;

  GLint posLocation;
  GLint colorLocation;
  GLint normalLocation;
  GLint texCoordLocation;

  GLint PVMmatrixLocation;
  GLint VmatrixLocation;
  GLint MmatrixLocation;
  GLint normalMatrixLocation;

  GLint timeLocation;

  GLint diffuseLocation;
  GLint ambientLocation;
  GLint specularLocation;
  GLint shininessLocation;

  GLint useTextureLocation;
  GLint texSamplerLocation;

  GLint torchPositionLocation;		//camera position
  GLint torchDirectionLocation;		//view direction
  GLint torchOnLocation;			//is torch switched on

  GLint dayTimeLocation;			//is it day or night
  GLint fogColourLocation;			//fog colour changing according to day time
  GLint fogOnLocation;				//is fog showing

} SCommonShaderProgram;

//Shader for skybox
typedef struct SkyboxShaderProgram {
	
	GLuint program;
	
	GLint screenCoordLocation;
	GLint inversePVmatrixLocation;

	GLint skyboxSamplerLocation;		//day texture
	GLint skyboxSampler2Location;		//night texture

	GLint fogActiveLocation;
	GLint fogColourLocation;			//fog colour
	GLint blendFactorLocation;			//blend between day and night

} SkyboxShaderProgram;

void drawWand(Object* wand, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
void drawFireplace(Object* fireplace, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
void drawFire(FireObject* fire, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
void drawRock(Object* rock, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
void drawBroom(Object* broom, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
void drawEagle(MoveableObject* eagle, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
void drawHall(Object* hall, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
void drawBench(Object* bench, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
void drawHat(Object* hat, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
void drawTree(Object* tree, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
void drawPlant(Object* plant, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
void drawSkybox(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
void drawBase(GroundObject* base, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
void drawWater(WaterObject* water, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);

void initializeShaderPrograms();
void cleanupShaderPrograms();

void initializeModels();
void cleanupModels();

#endif
