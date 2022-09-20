#version 330 core
uniform sampler2D imageY;
uniform sampler2D imageU;
uniform sampler2D imageV;

in vec2 TexCoords;

out vec4 color;
void main(){
    vec3 yuv;
    vec3 rgb;
    yuv.x=texture2D(imageY,TexCoords).r;
    /* 这里减0.5，范围就变成了-0.5到1.5 */
    yuv.y=texture2D(imageU,TexCoords).r-.5;
    yuv.z=texture2D(imageV,TexCoords).r-.5;
    rgb=mat3(
        1.,1.,1.,
        0.,-.39465,2.03211,
        1.13983,-.58060,0.
    )*yuv;
    
    /* 按YCbCr转RGB的公式进行数据转换
    float r = y + 1.403 * (cr - 0.5);
    float g = y - 0.343 * (cb - 0.5) - 0.714 * (cr - 0.5);
    float b = y + 1.770 * (cb - 0.5); */
    color=vec4(rgb,1.);
}