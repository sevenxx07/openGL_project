#version 140

struct Material {
  vec3  ambient;
  vec3  diffuse;
  vec3  specular;
  float shininess;

  bool  useTexture;
};

uniform sampler2D texSampler;

uniform Material material;

smooth in vec4 color_v;
smooth in vec2 texCoord_v;
out vec4       color_f;

in float visibility;        //fog factor
uniform vec3 fogColour;     //for colour
uniform bool fogOn;         //is fog showing

void main() {

    color_f = color_v;

    if(material.useTexture){
        color_f =  color_v * texture(texSampler, texCoord_v);
    }

    if(fogOn){
         color_f = mix(vec4(fogColour, 1.0f), color_f, visibility);  //mix colour with fog according to visibility
    }

}
