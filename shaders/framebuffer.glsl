#shader vertex
#version 330 core
layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 iTexCoords;

out vec2 texCoords;

void main() {
    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
    texCoords = iTexCoords;
}

#shader fragment
#version 330 core
out vec4 FragColor;
in vec2 texCoords;

uniform sampler2D screen_texture;
// uniform sampler2D bloomTexture;
const float gamma = 1.0;

void main() {
    vec4 frag = texture(screen_texture, texCoords);
    // vec3 bloomColor = texture(bloomTexture, texCoords).rgb;
    FragColor = vec4(frag.rgb, 1);
    // FragColor.rgb = frag.rgb;
    // FragColor.a = 1;

    // FragColor = frag;
    // FragColor.rgb = pow(frag.rgb, vec3(1.0 / gamma));
}