#version 330 core
out vec4 FragColor;

uniform vec2 fragPos;

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0/3.0, 1.0/3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
    float hue = atan(fragPos.y-1, fragPos.x-1) / (2.0 * 3.1415926) + 0.5;
    float saturation = 1.0;
    float value = 1.0;

    vec3 color = hsv2rgb(vec3(hue, saturation, value));
    FragColor = vec4(color, 1.0);
}