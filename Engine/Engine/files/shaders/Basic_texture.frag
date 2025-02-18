//Copyright (c) <2025> <Lost Empire Entertainment>

#version 330 core

uniform sampler2D diffuse;
in vec2 TexCoords;

uniform float transparency;
uniform vec3 color;

out vec4 FragColor;

void main()
{
    vec4 textureColor = texture(diffuse, TexCoords);
    vec4 outputColor = vec4(textureColor.rgb * color, textureColor.a * transparency);
    FragColor = outputColor;
}