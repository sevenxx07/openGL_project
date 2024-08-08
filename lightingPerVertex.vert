#version 140

struct Material {
    vec3  ambient;
    vec3  diffuse;
    vec3  specular;
    float shininess;

    bool  useTexture;
};

uniform sampler2D texSampler;

struct Light {
    vec3  ambient;
    vec3  diffuse;
    vec3  specular;
    vec3  position;
    vec3  spotDirection;
    float spotCosCutOff;
    float spotExponent;
};

in vec3 position;
in vec3 normal;
in vec2 texCoord;

uniform float time;
uniform Material material;

uniform mat4 PVMmatrix;
uniform mat4 Vmatrix;
uniform mat4 Mmatrix;
uniform mat4 normalMatrix;

uniform vec3 torchPosition;
uniform vec3 torchDirection;

uniform bool torchOn;
uniform float dayTime;
uniform bool lampsOn;
uniform vec3 lampLight;

smooth out vec2 texCoord_v;
smooth out vec4 color_v;

out float visibility;

vec4 spotLight(Light light, Material material, vec3 vertexPosition, vec3 vertexNormal) {

    vec3 ret = vec3(0.0);

    vec3 L = normalize(light.position - vertexPosition);
    vec3 R = reflect(-L, vertexNormal);
    vec3 V = normalize(-vertexPosition);
    float NdotL = max(0.0, dot(vertexNormal, L));
    float RdotV = max(0.0, dot(R, V));
    float spotCoef = max(0.0, dot(-L, light.spotDirection));

    ret += material.ambient * light.ambient;
    ret += material.diffuse * light.diffuse * NdotL;
    ret += material.specular * light.specular * pow(RdotV, material.shininess);

    if(spotCoef < light.spotCosCutOff)
    ret *= 0.0;
    else
    ret *= pow(spotCoef, light.spotExponent);

    return vec4(ret, 1.0);

}

vec4 pointLight(Light light, Material material, vec3 vertexPosition, vec3 vertexNormal) {

    vec3 ret = vec3(0.0);

    vec3 L = normalize(light.position - vertexPosition);
    vec3 R = reflect(-L, vertexNormal);
    vec3 V = normalize(-vertexPosition);
    float NdotL = max(0.0, dot(vertexNormal, L));
    float RdotV = max(0.0, dot(R, V));
    float dist = length(light.position - vertexPosition);
    float attentuation = 1.0f/(0.0f + 0.8f * dist + 10.0f * (dist * dist)); //constant 1.0, linear 0.7, quadric 1.8, distance 7

    ret += material.ambient * light.ambient * attentuation;
    ret += material.diffuse * light.diffuse * NdotL * attentuation;
    ret += material.specular * light.specular * pow(RdotV, material.shininess) * attentuation;

    return vec4(ret, 1.0);

}

vec4 directionalLight(Light light, Material material, vec3 vertexPosition, vec3 vertexNormal) {

    vec3 ret = vec3(0.0);

    vec3 L = normalize(light.position);
    vec3 R = reflect(-L, vertexNormal);
    vec3 V = normalize(-vertexPosition);
    float NdotL = max(0.0, dot(vertexNormal, L));
    float RdotV = max(0.0, dot(R, V));

    ret += material.ambient * light.ambient;
    ret += material.diffuse * light.diffuse * NdotL;
    ret += material.specular * light.specular * pow(RdotV, material.shininess);

    return vec4(ret, 1.0);

}


Light sun;                      //directional light moving on sky during daytime
float sunSpeed = 3.14/15;       //count sun speed so that at the end of day its in 180 degree angle
Light moon;                     //directional light moving on sky during nighttime

Light torch;                    //flashlight moving with camera
Light pointlight;               //point light as a lamp on castle

void setupLights() {

    sun.ambient  = vec3(0.0f);
    sun.specular = vec3(1.0f);
    sun.diffuse  = vec3(0.5f*dayTime + 0.5f, 0.5f, 0.25f);    //sun and moon are red in the morning and in the evening
    sun.position = (Vmatrix * vec4(cos(time * sunSpeed), sin(time * sunSpeed), 0.0, 0.0)).xyz; //sun and moon start on the opposite side

    torch.ambient       = vec3(0.2f);
    torch.diffuse       = vec3(1.0f, 1.0f, 1.0f);
    torch.specular      = vec3(1.0f);
    torch.spotCosCutOff = 0.95f;
    torch.spotExponent  = 0.0;

    torch.position = (Vmatrix * vec4(torchPosition, 1.0)).xyz;
    torch.spotDirection = normalize((Vmatrix * vec4(torchDirection, 0.0)).xyz);

    pointlight.ambient       = vec3(0.2f);
    pointlight.diffuse       = vec3(1.0f, 1.0f, 1.0f);
    pointlight.specular      = vec3(1.0f);
    pointlight.spotCosCutOff = 0.9f;
    pointlight.spotExponent  = 0.0;
    pointlight.position      = (Vmatrix*vec4(-4.0f, 0.14f, 1.3f, 1.0f)).xyz;

}

void main() {

    setupLights();

    vec3 vertexPosition = (Vmatrix * Mmatrix * vec4(position, 1.0)).xyz;
    vec3 vertexNormal   = normalize((Vmatrix * normalMatrix * vec4(normal, 0.0) ).xyz);

    vec3 globalAmbientLight = vec3(0.4f);
    vec4 outputColor = vec4(material.ambient * globalAmbientLight, 0.0);

    const float density = 1.0f;
    const float gradient = 2.0f;
    const float lower = -1.0f;			//lower border of fog
    const float upper = 3.0f;			//upper border of fog
    float dist = length(vertexPosition);
    visibility = (vertexPosition.y - lower)/(upper - lower);
    visibility = clamp(visibility, 0.0f, 1.0f);         //count vertex visibility in fog

    //sun is invisible at night, moon at day
    outputColor += directionalLight(sun, material, vertexPosition, vertexNormal)*(1-dayTime);
    if(torchOn){
        outputColor += spotLight(torch, material, vertexPosition, vertexNormal);
    }
    outputColor += pointLight(pointlight, material, vertexPosition, vertexNormal);

    gl_Position = PVMmatrix * vec4(position, 1);

    color_v = outputColor;
    texCoord_v = texCoord;

}
