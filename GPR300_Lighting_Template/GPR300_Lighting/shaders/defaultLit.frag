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
    vec3 intensityRGB = _Light.intensity * _Light.color * _Mat.color;

    //Ambient Light
    vec3 ambient = _Mat.ambientCoefficient * intensityRGB;

    //Diffuse Light
    //Direction to light
    vec3 lightDir = normalize(_Light.pos - WorldPos);
    vec3 diffuse = _Mat.diffuseCoefficient * clamp(dot(lightDir, WorldNormal), 0, 1) * intensityRGB;
    
    //Specular Light
    //What dot product to put in for specular (depending on if phong or blinn-phong it changes)
    float angle = 0;

    //Direction to viewer
    vec3 eyeDir = normalize(_CamPos - WorldPos);

    if(_Light.phong)    //Phong
    {
        angle = dot(normalize(reflect(WorldPos - _Light.pos, WorldNormal)), eyeDir);
    }
    else    //Blinn-Phong
    {
        vec3 sum = eyeDir + lightDir;
        vec3 halfVec = sum / length(sum);
        angle = dot(WorldNormal, halfVec);
    }
    
    vec3 specular = _Mat.specularCoefficient * pow(clamp(angle, 0, 1), _Mat.shininess) * intensityRGB;

    FragColor = vec4(ambient + diffuse + specular, 1.0f);
}
