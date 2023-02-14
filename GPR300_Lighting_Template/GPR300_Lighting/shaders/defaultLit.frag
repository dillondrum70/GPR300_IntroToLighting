#version 450                          
out vec4 FragColor;

in struct Vertex
{
    vec3 Normal;
    vec3 WorldPos;
    vec3 WorldNormal;
}vert_out;

//Uniforms from application

struct Attenuation
{
    float constant;
    float linear;
    float quadratic;
};

uniform Attenuation _Attenuation;

struct PointLight
{
    vec3 pos;
    vec3 color;
    float intensity;
};

const int MAX_POINT_LIGHTS = 8;
uniform PointLight _PointLight[MAX_POINT_LIGHTS];
uniform int _UsedPointLights;

struct DirectionalLight
{
    vec3 dir;
    vec3 color;
    float intensity;
};

uniform DirectionalLight _DirectionalLight;
uniform bool _DirectionalEnabled;

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

float calculatePhong(vec3 eyeDir, vec3 lightDir, vec3 worldNormal)
{
    return dot(normalize(reflect(lightDir, worldNormal)), eyeDir);
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

float calculateAttenuationFactor(float dist, float constant, float linear, float quadratic)
{
    return 1 / (constant + (linear * dist) + (quadratic * dist * dist));
}

void pointLights(inout vec3 diffuse, inout vec3 specular)
{
    for(int i = 0; i < _UsedPointLights; i++)
    {
        vec3 intensityRGB = _PointLight[i].intensity * _PointLight[i].color * _Mat.color;   //Material color and light intensity/color
        float dist = distance(vert_out.WorldPos, _PointLight[i].pos);    //distance between candidate point and light
        vec3 lightDir = normalize(_PointLight[i].pos - vert_out.WorldPos);  //Direction to light
        float attenuationFactor = calculateAttenuationFactor(dist, _Attenuation.constant, _Attenuation.linear, _Attenuation.quadratic);   //Factor of how much light makes it based on distance

        //Diffuse Light
        diffuse += calculateDiffuse(_Mat.diffuseCoefficient, lightDir, vert_out.WorldNormal, intensityRGB)
        * attenuationFactor;
    
        //Specular Light
        float angle = 0;    //What dot product to put in for specular (depending on if phong or blinn-phong it changes)
        vec3 eyeDir = normalize(_CamPos - vert_out.WorldPos);   //Direction to viewer

        if(_Phong)    //Phong
        {
            angle = calculatePhong(eyeDir, -lightDir, vert_out.WorldNormal);
        }
        else    //Blinn-Phong
        {
            angle = calculateBlinnPhong(eyeDir, lightDir, vert_out.WorldNormal);
        }
    
        specular += calculateSpecular(_Mat.specularCoefficient, angle, _Mat.shininess, intensityRGB)
        * attenuationFactor;
    }
}

void directionalLight(inout vec3 diffuse, inout vec3 specular)
{
    vec3 intensityRGB = _DirectionalLight.intensity * _DirectionalLight.color * _Mat.color;   //Material color and light intensity/color
    vec3 lightDir = normalize(_DirectionalLight.dir);
    
    //Diffuse Light
    diffuse += calculateDiffuse(_Mat.diffuseCoefficient, lightDir, vert_out.WorldNormal, intensityRGB);
    
    //Specular Light
    float angle = 0;    //What dot product to put in for specular (depending on if phong or blinn-phong it changes)
    vec3 eyeDir = normalize(_CamPos - vert_out.WorldPos);   //Direction to viewer

    if(_Phong)    //Phong
    {
        angle = calculatePhong(eyeDir, -lightDir, vert_out.WorldNormal);
    }
    else    //Blinn-Phong
    {         
        angle = calculateBlinnPhong(eyeDir, lightDir, vert_out.WorldNormal);
    }
    
    specular += calculateSpecular(_Mat.specularCoefficient, angle, _Mat.shininess, intensityRGB);
}

void main()
{   
    //Ambient Light
    vec3 ambient = _Mat.ambientCoefficient * _Mat.color;

    vec3 diffuse = vec3(0);
    vec3 specular = vec3(0);

    //Point Light diffuse and specular
    pointLights(diffuse, specular);

    //Directional light diffuse and specular
    if(_DirectionalEnabled)
    {
        directionalLight(diffuse, specular);
    }

    FragColor = vec4(ambient + diffuse + specular, 1.0f);
}
