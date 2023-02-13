#version 450                          
out vec4 FragColor;

in struct Vertex
{
    vec3 Normal;
    vec3 WorldPos;
    vec3 WorldNormal;
}vert_out;

//Uniforms from application

struct PointLight
{
    vec3 pos;
    vec3 color;
    float intensity;
};

const int MAX_POINT_LIGHTS = 8;
uniform PointLight _PointLight[MAX_POINT_LIGHTS];
uniform int _UsedPointLights;

struct Material
{
    vec3 color;

    float ambientCoefficient;
    float diffuseCoefficient;
    float specularCoefficient;
    float shininess;
};

uniform Material _Mat;
uniform vec3 _CamPos;
uniform bool _Phong;

vec3 calculateDiffuse(float coefficient, vec3 lightDir, vec3 worldNormal, vec3 intensity)
{
    return coefficient * clamp(dot(lightDir, worldNormal), 0, 1) * intensity;
}

float calculatePhong(vec3 worldPos, vec3 lightPos, vec3 worldNormal, vec3 eyeDir)
{
    return dot(normalize(reflect(worldPos - lightPos, worldNormal)), eyeDir);
}

float calculateBlinnPhong(vec3 eyeDir, vec3 lightDir, vec3 worldNormal)
{
    vec3 sum = eyeDir + lightDir;
    vec3 halfVec = sum / length(sum);
    return dot(worldNormal, halfVec);
}

vec3 calculateSpecular(float coefficient, float angle, float shininess, vec3 intensity)
{
    return coefficient * pow(clamp(angle, 0, 1), shininess) * intensity;
}


void main()
{   
    //Ambient Light
    vec3 ambient = _Mat.ambientCoefficient * _Mat.color;

    vec3 diffuse = vec3(0);
    vec3 specular = vec3(0);

    for(int i = 0; i < _UsedPointLights; i++)
    {
        //Material color and light intensity/color
        vec3 intensityRGB = _PointLight[i].intensity * _PointLight[i].color * _Mat.color;

        //Direction to light
        vec3 lightDir = normalize(_PointLight[i].pos - vert_out.WorldPos);

        //Diffuse Light
        diffuse += calculateDiffuse(_Mat.diffuseCoefficient, lightDir, vert_out.WorldNormal, intensityRGB);
    
        //Specular Light
        //What dot product to put in for specular (depending on if phong or blinn-phong it changes)
        float angle = 0;

        //Direction to viewer
        vec3 eyeDir = normalize(_CamPos - vert_out.WorldPos);

        if(_Phong)    //Phong
        {
            angle = calculatePhong(vert_out.WorldPos, _PointLight[i].pos, vert_out.WorldNormal, eyeDir);
        }
        else    //Blinn-Phong
        {
            angle = calculateBlinnPhong(eyeDir, lightDir, vert_out.WorldNormal);
        }
    
        specular += calculateSpecular(_Mat.specularCoefficient, angle, _Mat.shininess, intensityRGB);
    }

    

    FragColor = vec4(ambient + diffuse + specular, 1.0f);
}
