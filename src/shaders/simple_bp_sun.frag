#version 330 core
in vec3 Normals;
in vec3 WorldPos;

out vec4 FragColor;

uniform vec3 worldLightDir; // Must be normalized World-Space vector pointing TO the light

void main()
{
    // 1. Re-normalize interpolated vector
    vec3 N = normalize(Normals);

    // 2. Handle 2-sided lighting
    if (!gl_FrontFacing) {
        N = -N;
    }

    // 3. Lambertian Diffuse
    vec3 L = normalize(worldLightDir);
    float diff = max(dot(N, L), 0.0);

    // Ambient + Diffuse
    vec3 ambient = vec3(0.2);
    vec3 color = (ambient + diff) * vec3(0.1, 0.8, 0.2);

    FragColor = vec4(color, 1.0);
    FragColor = vec4(N * 0.5 + 0.5, 1.0);
}