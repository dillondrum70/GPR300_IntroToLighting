#version 450                          
layout (location = 0) in vec3 vPos;  
layout (location = 1) in vec3 vNormal;

out vec3 Normal;
out vec3 WorldPos;
out vec3 WorldNormal;

uniform mat4 _Model;
uniform mat4 _View;
uniform mat4 _Projection;

uniform mat4 _NormalMatrix;

void main(){    
    Normal = vNormal;
    WorldPos = vec3(_Model * vec4(vPos, 1));
    WorldNormal = normalize(mat3(_NormalMatrix) * Normal);
    gl_Position = _Projection * _View * _Model * vec4(vPos,1);
}
