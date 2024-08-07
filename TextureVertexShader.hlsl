#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aTexCoord;

out vec3 ourColor;
out vec2 TexCoord;

uniform vec3 offset;
uniform float zoom;

void main()
{

    vec3 finalPos = aPos + offset;
    finalPos = finalPos * zoom;

    gl_Position = vec4(finalPos, 1.0);
    ourColor = aColor;
    TexCoord = aTexCoord;
}