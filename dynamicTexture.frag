#version 140

smooth in vec3 position_v;    ///< camera space fragment position
smooth in vec2 texCoord_v;    ///< fragment texture coordinates

uniform float time;						///< used for simulation of moving lights (such as sun) and to select proper animation frame
uniform mat4 Vmatrix;					///< view (camera) transfor
uniform sampler2D texSampler;			///< sampler for texture access
uniform ivec2 pattern = ivec2(8, 2);	///< there are 8 frames in the row, 2 rows total
uniform float frameDuration = 0.3f;		///< one frame lasts 0.3s

out vec4 color_f;

void main() {

  int frame = int(time / frameDuration);

  // sample proper frame of the texture to get a fragment color 
  vec2 offset = vec2(1.0) / vec2(pattern);

  vec2 texCoordBase = texCoord_v / vec2(pattern);
  vec2 texCoord = texCoordBase + vec2(frame % pattern.x, (frame / pattern.x)) * offset;

  color_f = texture(texSampler, texCoord);
}
