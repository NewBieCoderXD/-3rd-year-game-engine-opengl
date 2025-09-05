#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in mat4 trans;

void main()
{
    gl_Position = trans * vec4(aPos, 0.0, 1.0);
}