#version 400

in vec2 texcoord;
in vec3 vertex;

out vec4 frag_color;

uniform vec3 lightpos;

uniform sampler2D diffuse;
uniform sampler2D normal;
uniform sampler2D depth;

void main() {
	vec4 color_diffuse = texture(diffuse, vec2(texcoord.x, texcoord.y));
	vec4 color_normal = texture(normal, vec2(texcoord.x, texcoord.y));
	vec4 color_depth = texture(depth, vec2(texcoord.x, texcoord.y));
	
	float offset_depth = (color_depth.x - 0.5) / 0.00392157;
	
	vec3 worldpos = vertex.xyz;
	worldpos.z = offset_depth;
	worldpos.y -= offset_depth;
	vec3 light_pos = lightpos;
	
	light_pos.z = 33.0f;
	vec3 normal = vec3(color_normal.xyz);
	normal -= vec3(0.5, 0.5, 0.5);
	normal = normalize(normal);
	
	vec3 light_dir = light_pos - worldpos;
	float light_dist = length(light_dir);
	light_dir = normalize(light_dir);
	
	float light_angle = max(0.0, dot(normal, light_dir));
	
	float light_att = 20.0 / (1.0 + 0.1*light_dist + 0.01*light_dist*light_dist);
	//float light_radius = 100.0;
	//float light_att = clamp(1.0 - (light_dist*light_dist)/(light_radius*light_radius), 0.0, 1.0);
	//light_att *= light_att;
	
	//float light_intensity = light_radius * light_att;
	
	//frag_color = color_depth;
    frag_color = light_angle * light_att * color_diffuse;
	frag_color.w = color_diffuse.w;
	
	//float debug = worldpos.y;
	//frag_color = vec4(debug, debug, debug, 1.0);
}
