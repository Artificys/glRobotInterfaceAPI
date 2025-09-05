#version 330 core

in vec3 vNormal;
in vec3 vWorldPos;

out vec4 FragColor;

uniform vec3 uCameraPos;
uniform vec3 uColor;

const vec3 Ind = vec3(0.5);

void main()
{
    vec3 N = normalize(vNormal);

    // Simple overhead light
    vec3 L = normalize(vec3(0.0, 0.0, 1.0));  

    // Indirect side lights
    vec3 L1 = normalize(vec3(-0.8, -0.5, 0.05));   // right side
    vec3 L2 = normalize(vec3(0.8, 0.5, 0.1));  // left side
    vec3 L3 = normalize(vec3(0.8, -0.5, 0.05));   // right side
    vec3 L4 = normalize(vec3(-0.8, 0.5, 0.1));  // left side

    vec3 V = normalize(uCameraPos - vWorldPos);
    vec3 H = normalize(L + V);

    // Lighting
    float diff = max(dot(N, L), 0.0);
    float diff1 = max(dot(N, L1), 0.0);
    float diff2 = max(dot(N, L2), 0.0);
    float diff3 = max(dot(N, L3), 0.0);
    float diff4 = max(dot(N, L4), 0.0);

    float spec = pow(max(dot(N, H), 0.0), 32.0); // lower power = softer highlight

    vec3 ambient = 0.2 * uColor; // always lit a little
    vec3 diffuse = 0.8 * diff * uColor; // main contribution
    vec3 indirect1 = Ind * diff1 * uColor; // side light 1
    vec3 indirect2 = Ind * diff2 * uColor; // side light 2
    vec3 indirect3 = Ind * diff3 * uColor; // side light 3
    vec3 indirect4 = Ind * diff4 * uColor; // side light 4
    vec3 highlight = 0.25 * spec * vec3(1.0); // white specular

    vec3 finalColor = ambient + diffuse + indirect1 + indirect2 + indirect3 + indirect4 + highlight;
    FragColor = vec4(finalColor, 1.0);
}
