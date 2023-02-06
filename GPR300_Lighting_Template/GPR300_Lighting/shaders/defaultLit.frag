#version 450                          
out vec4 FragColor;

in vec3 WorldNormal;
in vec3 WorldPos;

//Uniforms from application


void main(){         
    FragColor = vec4(abs(WorldNormal),1.0f);
}
