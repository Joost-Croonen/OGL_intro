#version 330 core

out vec4 FragColor; 

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in mat3 invTBN;
in vec3 tangentViewPos;
in vec3 tangentLightPos;
in vec3 tangentFragPos;

struct Material{
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
    sampler2D texture_height1;
    float shininess;
};

struct PointLight{
    vec3 position;
    vec3 color;
    float attenuation_power;
};

#define MAX_LIGHTS 10 
uniform Material material;
uniform PointLight light[MAX_LIGHTS];
uniform vec3 viewPos;
uniform int num_lights;
uniform float height_scale;  //0.025
uniform bool normal_mapping;

vec4 calc_light_parallax(int index)
{
    vec3 viewDir = normalize(tangentViewPos - tangentFragPos);
    // parallax TexCoords
    float height = 1.0 - texture(material.texture_height1, TexCoords).r;
    vec2 parallaxCoords = TexCoords - viewDir.xy / viewDir.z * (height_scale * height);
    if(parallaxCoords.x > 1.0 || parallaxCoords.y > 1.0 || parallaxCoords.x < 0.0 || parallaxCoords.y < 0.0)
        discard;
    // texture
    vec3 diffuseColor = texture(material.texture_diffuse1, parallaxCoords).rgb;
    // attentunation
    float dist = length(light[index].position - FragPos);
    float attenuation = 1.0 / pow(dist, light[index].attenuation_power);
    // diffuse
    vec3 vert_normal = invTBN * normalize(Normal);
    vec3 mapped_normal = texture(material.texture_normal1, parallaxCoords).rgb;
    mapped_normal = normalize(mapped_normal * 2.0 - 1.0);
    vec3 norm = normal_mapping ? mapped_normal : vert_normal;
    vec3 lightDir = normalize(tangentLightPos - tangentFragPos);
    vec3 diffuse = max(dot(lightDir, norm), 0.0) * diffuseColor;
    // specular
    vec3 halfVec = normalize(lightDir + viewDir);
    vec3 specular = pow(max(dot(norm, halfVec), 0.0), material.shininess) * diffuseColor;

    return vec4(attenuation * (diffuse + specular) * light[index].color, 1.0);
}


vec4 calc_light_steep_parallax(int index)
{
    const int numLayers = 10;
    const float layerDepth = 1.0f / numLayers;
    float currentLayerDepth = 0.0;
    
    vec3 viewDir = normalize(tangentViewPos - tangentFragPos);
    vec2 P = viewDir.xy * height_scale;
    vec2 deltaUV = P / numLayers;

    vec2 currentUV = TexCoords;
    float currentMapDepth = 1.0 - texture(material.texture_height1, currentUV).r;
    // parallax TexCoords
    while (currentLayerDepth < currentMapDepth)
    {
        currentUV -= deltaUV;
        currentMapDepth = 1.0 - texture(material.texture_height1, currentUV).r;
        currentLayerDepth += layerDepth;
    }
    vec2 parallaxCoords = currentUV;
    if(parallaxCoords.x > 1.0 || parallaxCoords.y > 1.0 || parallaxCoords.x < 0.0 || parallaxCoords.y < 0.0)
        discard;
    // texture
    vec3 diffuseColor = texture(material.texture_diffuse1, parallaxCoords).rgb;
    // attentunation
    float dist = length(light[index].position - FragPos);
    float attenuation = 1.0 / pow(dist, light[index].attenuation_power);
    // diffuse
    vec3 vert_normal = invTBN * normalize(Normal);
    vec3 mapped_normal = texture(material.texture_normal1, parallaxCoords).rgb;
    mapped_normal = normalize(mapped_normal * 2.0 - 1.0);
    vec3 norm = normal_mapping ? mapped_normal : vert_normal;
    vec3 lightDir = normalize(tangentLightPos - tangentFragPos);
    vec3 diffuse = max(dot(lightDir, norm), 0.0) * diffuseColor;
    // specular
    vec3 halfVec = normalize(lightDir + viewDir);
    vec3 specular = pow(max(dot(norm, halfVec), 0.0), material.shininess) * diffuseColor;

    return vec4(attenuation * (diffuse + specular) * light[index].color, 1.0);
}

vec4 calc_light_steep_parallax_interp(int index)
{
    const int numLayers = 5;
    const float layerDepth = 1.0f / numLayers;
    float currentLayerDepth = 0.0;
    
    vec3 viewDir = normalize(tangentViewPos - tangentFragPos);
    vec2 P = viewDir.xy * height_scale;
    vec2 deltaUV = P / numLayers;

    vec2 currentUV = TexCoords;
    float currentMapDepth = 1.0 - texture(material.texture_height1, currentUV).r;
    float lastMapDepth = currentMapDepth;
    // parallax TexCoords
    while (currentLayerDepth < currentMapDepth)
    {
        currentUV -= deltaUV;
        lastMapDepth = currentMapDepth;
        currentMapDepth = 1.0 - texture(material.texture_height1, currentUV).r;
        currentLayerDepth += layerDepth;
    }
    float deltaHeight = lastMapDepth - currentMapDepth;
    float mixWeight = 1.0 - (currentMapDepth - currentLayerDepth) / (layerDepth + deltaHeight);
    vec2 parallaxCoords = mix(currentUV, currentUV + deltaUV, mixWeight);
    if(parallaxCoords.x > 1.0 || parallaxCoords.y > 1.0 || parallaxCoords.x < 0.0 || parallaxCoords.y < 0.0)
        discard;
    // texture
    vec3 diffuseColor = texture(material.texture_diffuse1, parallaxCoords).rgb;
    // attentunation
    float dist = length(light[index].position - FragPos);
    float attenuation = 1.0 / pow(dist, light[index].attenuation_power);
    // diffuse
    vec3 vert_normal = invTBN * normalize(Normal);
    vec3 mapped_normal = texture(material.texture_normal1, parallaxCoords).rgb;
    mapped_normal = normalize(mapped_normal * 2.0 - 1.0);
    vec3 norm = normal_mapping ? mapped_normal : vert_normal;
    vec3 lightDir = normalize(tangentLightPos - tangentFragPos);
    vec3 diffuse = max(dot(lightDir, norm), 0.0) * diffuseColor;
    // specular
    vec3 halfVec = normalize(lightDir + viewDir);
    vec3 specular = pow(max(dot(norm, halfVec), 0.0), material.shininess) * diffuseColor;

    return vec4(attenuation * (diffuse + specular) * light[index].color, 1.0);
}

vec4 calc_light_steep_parallax_refined(int index)
{
    const int numLayers = 4;
    float layerDepth = 1.0f / numLayers;
    float currentLayerDepth = 0.0;
    
    vec3 viewDir = normalize(tangentViewPos - tangentFragPos);
    vec2 P = viewDir.xy * height_scale;
    vec2 deltaUV = P / numLayers;

    vec2 currentUV = TexCoords;
    float currentMapDepth = 1.0 - texture(material.texture_height1, currentUV).r;
    float lastMapDepth = currentMapDepth;
    // parallax TexCoords
    while (currentLayerDepth < currentMapDepth)
    {
        currentUV -= deltaUV;
        lastMapDepth = currentMapDepth;
        currentMapDepth = 1.0 - texture(material.texture_height1, currentUV).r;
        currentLayerDepth += layerDepth;
    }
    layerDepth /= numLayers;
    deltaUV /= numLayers;
    while (currentLayerDepth > currentMapDepth)
    {
        currentUV += deltaUV;
        lastMapDepth = currentMapDepth;
        currentMapDepth = 1.0 - texture(material.texture_height1, currentUV).r;
        currentLayerDepth -= layerDepth;
    }
    float deltaHeight = currentMapDepth - lastMapDepth;
    float mixWeight = 1.0 - (currentMapDepth - currentLayerDepth) / (layerDepth + deltaHeight);
    vec2 parallaxCoords = mix(currentUV, currentUV + deltaUV, mixWeight);
    if(parallaxCoords.x > 1.0 || parallaxCoords.y > 1.0 || parallaxCoords.x < 0.0 || parallaxCoords.y < 0.0)
        discard;
    // texture
    vec3 diffuseColor = texture(material.texture_diffuse1, parallaxCoords).rgb;
    // attentunation
    float dist = length(light[index].position - FragPos);
    float attenuation = 1.0 / pow(dist, light[index].attenuation_power);
    // diffuse
    vec3 vert_normal = invTBN * normalize(Normal);
    vec3 mapped_normal = texture(material.texture_normal1, parallaxCoords).rgb;
    mapped_normal = normalize(mapped_normal * 2.0 - 1.0);
    vec3 norm = normal_mapping ? mapped_normal : vert_normal;
    vec3 lightDir = normalize(tangentLightPos - tangentFragPos);
    vec3 diffuse = max(dot(lightDir, norm), 0.0) * diffuseColor;
    // specular
    vec3 halfVec = normalize(lightDir + viewDir);
    vec3 specular = pow(max(dot(norm, halfVec), 0.0), material.shininess) * diffuseColor;

    return vec4(attenuation * (diffuse + specular) * light[index].color, 1.0);
}

void main()
{
    vec4 color = vec4(0);
    for(int i = 0; i < min(num_lights, MAX_LIGHTS); ++i)
        color += calc_light_steep_parallax_refined(i);
    //float gamma = 2.2;
    //color = vec4(pow(color.rgb, vec3(1.0/gamma)), 1.0);
    FragColor = color;
    //vec3 mapped_normal = texture(material.texture_normal1, TexCoords).rgb;
    //mapped_normal = normalize(mapped_normal * 2.0 - 1.0);
    //
    //float height = texture(material.texture_height1, TexCoords).r;
    //FragColor = vec4(vec3(height), 1.0);
}
