#version 400

in vec2 texcoord;
out vec4 frag_color;

uniform sampler2D diffuse;
uniform sampler2D normal;
uniform sampler2D depth;

void main() {
    frag_color = texture(normal, vec2(texcoord.x, texcoord.y)) * texture(diffuse, vec2(texcoord.x, texcoord.y));
}
