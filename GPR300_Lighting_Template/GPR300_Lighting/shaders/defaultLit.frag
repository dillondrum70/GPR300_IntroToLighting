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
uniform vec3 _CamPos;

void main()
{   
    vec3 ambient = _Mat.ambientCoefficient * _Light.intensity * _Light.color * _Mat.color;
    vec3 diffuse = _Mat.diffuseCoefficient * clamp(dot(normalize(_Light.pos - WorldPos), WorldNormal), 0, 1) * (_Light.intensity * _Light.color);
    
    vec3 specular = _Mat.specularCoefficient * pow(dot(normalize(reflect(WorldPos - _Light.pos, WorldNormal)), normalize(_CamPos - WorldPos)), _Mat.shininess) * _Light.intensity;

    FragColor = vec4(ambient + diffuse,1.0f);
}
