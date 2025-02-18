//Copyright (c) <2025> <Lost Empire Entertainment>

#version 330 core
out vec4 FragColor;

struct Material 
{
    sampler2D diffuse;
    sampler2D specular;    
    float shininess;
}; 

struct DirLight
{
    vec3 direction;
    float intensity;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    bool enabled;
};
struct PointLight
{
    vec3 position;
    float intensity;
    float distance;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    bool enabled;
};
struct SpotLight
{
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    float intensity;
    float distance;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    bool enabled;
};

#define MAX_POINT_LIGHTS 16
uniform int pointLightCount;
uniform PointLight pointLights[MAX_POINT_LIGHTS];

#define MAX_SPOT_LIGHTS 16
uniform int spotLightCount;
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

#define MAX_DIR_LIGHTS 1
uniform int dirLightCount;
uniform DirLight dirLights[MAX_DIR_LIGHTS];

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;
  
uniform bool isTransparent;
uniform float transparency;
uniform vec3 viewPos;
uniform DirLight dirLight;
uniform SpotLight spotLight;
uniform Material material;

float GetAlpha(sampler2D tex, vec2 coords);
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    //properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = vec3(0.0);
    if (dirLightCount > 0)
    {   
        if (dirLight.enabled)
        {
            result += CalcDirLight(dirLight, norm, viewDir);
        }
    }

    if (pointLightCount > 0)
    {
        for (int i = 0; i < pointLightCount; i++)
        {
            if (pointLights[i].enabled)
            {
                result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
            }
        }
    }
    if (spotLightCount > 0)
    {
        for (int i = 0; i < spotLightCount; i++)
        {
            if (spotLights[i].enabled)
            {
                result += CalcSpotLight(spotLights[i], norm, FragPos, viewDir);
            }
        }
    }

    float alpha = GetAlpha(material.diffuse, TexCoords);

    if (alpha < 0.1) discard;

    FragColor = vec4(result, alpha);
}

float GetAlpha(sampler2D tex, vec2 coords)
{
	if (isTransparent)
	{
		return texture(tex, coords).a * transparency;
	}
	else
	{
		return texture(tex, coords).a;
	}
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    //diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    //specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    //combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    ambient *= light.intensity;
    diffuse *= light.intensity;
    specular *= light.intensity;
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    //diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    //specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    //attenuation
    float distance = length(light.position - fragPos);
    distance /= light.distance;
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    //combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    ambient *= attenuation * light.intensity;
    diffuse *= attenuation * light.intensity;
    specular *= attenuation * light.intensity;
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    //diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    //specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    //attenuation
    float distance = length(light.position - fragPos);
    distance /= light.distance;
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    //spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    intensity *= light.intensity;
    //combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}