#version 330 core
layout(location=0)in vec4 vertex;
/* layout(location=1)in vec2 texture; */

out vec2 TexCoords;
void main(){
    TexCoords=vec2(vertex.z,1.-vertex.w);
    //TexCoords=vertex.zw;
    gl_Position=vec4(vec2(vertex.xy),0.,1.);
}