#version 330 core
layout(location = 0) out vec4 brightColorMap;

uniform sampler2D sceneMap;

in vec2 screenTexCoord;

void main() {
    // TODO: extract the bright color with color components greater than 1.0
    vec3 color = texture(sceneMap, screenTexCoord).rgb;
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        brightColorMap = vec4(color, 1.0);
    else
        brightColorMap = vec4(0.0, 0.0, 0.0, 1.0);
}
