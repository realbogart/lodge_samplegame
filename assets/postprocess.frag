#version 400

in vec2 texcoord;
out vec4 frag_color;

uniform sampler2D texture_diffuse;

void main() {
	vec4 tex_color = texture(texture_diffuse, vec2(texcoord.x, texcoord.y));
	frag_color = tex_color;
	//float average = (tex_color.r + tex_color.g + tex_color.b + tex_color.a)/4.0; 
    //frag_color = vec4(average);
}
