//Copyright (c) <2025> <Lost Empire Entertainment>

#version 330 core

uniform float transparency;
uniform vec3 color;

out vec4 FragColor;

void main()
{
    vec4 outputColor = vec4(color, 1.0);
    outputColor.a *= transparency;

    FragColor = outputColor;
}