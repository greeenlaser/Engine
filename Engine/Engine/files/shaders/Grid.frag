//Copyright (c) <2025> <Lost Empire Entertainment>

#version 330 core

uniform float transparency;
uniform vec3 color;
uniform float fadeDistance;
uniform float maxDistance;
uniform vec3 center;
in vec3 fragPos;

out vec4 FragColor;

void main()
{
    float distance = length(fragPos.xz - center.xz);
    float height = abs(fragPos.y - center.y);

    if (distance > maxDistance 
        || height > 50.0)
    {
        discard;
    }

    float fadeFactor = 1.0;
    if (distance > fadeDistance)
    {
        fadeFactor = 1.0 - (distance - fadeDistance) 
            / (maxDistance - fadeDistance);
        fadeFactor = clamp(fadeFactor, 0.0, 1.0);
    }

    if (fadeFactor <= 0.0)
    {
        discard;
    }

    vec3 finalColor = color;
    float finalAlpha = fadeFactor * transparency;

    FragColor = vec4(finalColor, finalAlpha);
}
