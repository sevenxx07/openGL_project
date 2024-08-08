#version 140

uniform samplerCube skyboxSampler;
uniform samplerCube skyboxSampler2;
in vec3 texCoord_v;
out vec4 color_f;

const float lower = -1.0f;			//lower border of fog
const float upper = 5.0f;			//upper border of fog
uniform vec3 fogColour;

uniform bool fogActive;

uniform float blendFactor;			//blend between day and night

void main() {

	vec4 tex1 = texture(skyboxSampler, texCoord_v);
	vec4 tex2 = texture(skyboxSampler2, texCoord_v);
	color_f = mix(tex1, tex2, blendFactor);
  
    if(fogActive){
		float factor = (texCoord_v.y - lower)/(upper - lower);
		factor = clamp(factor, 0.0, 1.0);
		color_f = mix(vec4(fogColour, 1.0), color_f, factor);
    }
}
