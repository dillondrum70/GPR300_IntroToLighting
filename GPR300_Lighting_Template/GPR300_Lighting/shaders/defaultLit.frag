#version 450                          
out vec4 FragColor;

in vec3 Normal;
in vec3 WorldNormal;
in vec3 WorldPos;

//Uniforms from application

struct LightProperties
{
    vec3 pos;
    vec3 color;
    float intensity;
};

struct MaterialProperties
{
    vec3 color;

    float ambientCoefficient;
    float diffuseCoefficient;
    float specularCoefficient;
    float shininess;
};

uniform LightProperties _Light;
uniform MaterialProperties _Mat;

void main()
{   
    vec3 ambient = _Mat.ambientCoefficient * _Light.intensity * _Light.color * _Mat.color;
    vec3 diffuse = _Mat.diffuseCoefficient * dot(normalize(_Light.pos - WorldPos), WorldNormal) * (_Light.intensity * _Light.color);
    //vec3 specular = _Light.specularCoefficient *

    FragColor = vec4(ambient + diffuse,1.0f);
}
