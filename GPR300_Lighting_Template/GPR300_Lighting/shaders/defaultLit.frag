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

    bool phong;
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
    vec3 diffuse = _Mat.diffuseCoefficient * clamp(dot(normalize(_Light.pos - WorldPos), WorldNormal), 0, 1) * (_Light.intensity * _Light.color * _Mat.color);
    
    vec3 specular = vec3(0);

    if(_Light.phong)
    {
        specular = _Mat.specularCoefficient * pow(clamp(dot(normalize(reflect(WorldPos - _Light.pos, WorldNormal)), normalize(_CamPos - WorldPos)), 0, 1), _Mat.shininess) * (_Light.intensity * _Light.color * _Mat.color);
    }
    else
    {
        
    }
    

    FragColor = vec4(ambient + diffuse + specular, 1.0f);
}
