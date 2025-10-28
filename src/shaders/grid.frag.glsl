#version 330 core

in vec3 fragPos;

uniform vec3 uCameraPos;
uniform vec3 uColor;

out vec4 FragColor;

void main() {
    float dist = length(fragPos.xy); // distance from origin in XY plane
    float fadeStart = uCameraPos.z * 5.0;
    float fadeEnd = uCameraPos.z * 10.0;
    float opacity_falloff = smoothstep(1.0, 0.0, (dist - fadeStart) / (fadeEnd - fadeStart));

    FragColor = vec4(uColor, 1.0) * opacity_falloff;
}
