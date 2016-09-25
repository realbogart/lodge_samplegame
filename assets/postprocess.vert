#version 400 core
precision highp float;

uniform mat4 projection;

in vec3 vertex_in;
in vec2 texcoord_in;

out vec2 texcoord;

void main() {
   texcoord = texcoord_in;
   gl_Position = vec4(vertex_in, 1.0);
}
