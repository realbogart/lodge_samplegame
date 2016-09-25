#version 400

in vec2 texcoord;
out vec4 frag_color;

uniform vec4 color;
uniform sampler2D tex;

void main() {
    vec4 tex_color = texture(tex, vec2(texcoord.x, texcoord.y)) * color;
	
	if(tex_color.a < 0.01)
		discard;
	
	tex_color.a /= 16.0;
	
    frag_color = tex_color;
}
