#version 140

in vec3 position;           ///< vertex position in world space
in vec2 texCoord;           ///< incoming texture coordinates

uniform mat4 PVMmatrix;     

smooth out vec2 texCoord_v; ///< outgoing vertex texture coordinates

void main() {

  gl_Position = PVMmatrix * vec4(position, 1);  

  // outputs entering the fragment shader
  texCoord_v = texCoord;
}
