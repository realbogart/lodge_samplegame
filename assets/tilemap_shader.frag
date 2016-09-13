#version 400

in vec2 texcoord;
in vec3 vertex;

out vec4 frag_color;

uniform vec3 lightpos;

uniform sampler2D diffuse;
uniform sampler2D normal;
uniform sampler2D depth;

uniform sampler2D palette;

vec4 map_color(vec4 color) 
{
    vec4 map = floor(color * 16.0);
    int i = int(map.r + map.g * 16 + map.b * 16 * 16);

	int x = i / 64;
	int y = int(mod(i, 64));
	
    return texture2D(palette, floor(1.0/vec2(x, y)));
}

vec4 interpolate(sampler2D sampler, vec2 texcoord)
{
	float offset = 1.0 / 1024.0;
	
	vec4 middle = 		texture(sampler, vec2(texcoord.x, texcoord.y));
	vec4 top_left = 	texture(sampler, vec2(texcoord.x - offset, texcoord.y + offset));
	vec4 top = 			texture(sampler, vec2(texcoord.x, texcoord.y + offset));
	vec4 top_right = 	texture(sampler, vec2(texcoord.x + offset, texcoord.y + offset));
	vec4 left = 		texture(sampler, vec2(texcoord.x - offset, texcoord.y));
	vec4 right = 		texture(sampler, vec2(texcoord.x + offset, texcoord.y));
	vec4 bottom_left = 	texture(sampler, vec2(texcoord.x + offset, texcoord.y - offset));
	vec4 bottom = 		texture(sampler, vec2(texcoord.x, texcoord.y - offset));
	vec4 bottom_right = texture(sampler, vec2(texcoord.x + offset, texcoord.y - offset));
	
	return middle;
	//return (middle + top + bottom) / 3.0;
	//return (middle + top_left + top + top_right + left + right + bottom_left + bottom + bottom_right) / 9.0;
}

float pixelate(float value)
{
	int wholepixels = int(value / 1.0);
	return float(wholepixels);
}

vec3 pixelate(vec3 p)
{
	p.x = pixelate(p.x);
	p.y = pixelate(p.y);
	p.z = pixelate(p.z);
	
	return p;
}

void main() 
{
	vec4 color_diffuse = texture(diffuse, vec2(texcoord.x, texcoord.y));
	vec4 color_normal = texture(normal, vec2(texcoord.x, texcoord.y));
	
	vec4 color_depth = texture(depth, vec2(texcoord.x, texcoord.y));
	//vec4 color_depth = interpolate(depth, texcoord);
	
	float offset_depth = (color_depth.x - 0.5) / 0.00392157;
	
	//vec3 worldpos = pixelate(vertex.xyz);
	vec3 worldpos = vertex.xyz;
	worldpos.z = offset_depth;
	worldpos.y -= offset_depth;
	vec3 light_pos = lightpos;
	
	light_pos.z = 20.0;
	light_pos.y -= 8.0;
	
	//light_pos = pixelate(light_pos);
	vec3 normal = vec3(color_normal.xyz);
	normal -= vec3(0.5, 0.5, 0.5);
	normal = normalize(normal);
	
	vec3 light_dir = light_pos - worldpos;
	float light_dist = length(light_dir);
	
	light_dir = normalize(light_dir);
	
	float light_angle = max(0.0, dot(normal, light_dir));
	
	//float light_att = 50.0 / (1.0 + 0.1*light_dist + 0.05*light_dist*light_dist);
	float light_radius = 100.0;
	float light_att = clamp(1.0 - (light_dist*light_dist)/(light_radius*light_radius), 0.0, 1.0);
	light_att *= light_att;
	
	//float light_intensity = light_radius * light_att;
	
	//frag_color = color_depth;
    //frag_color = color_diffuse;
    frag_color = light_angle * light_att * color_diffuse;
	frag_color.w = color_diffuse.w;
	
	//frag_color = map_color(frag_color);
	//float debug = worldpos.y;
	//frag_color = vec4(debug, debug, debug, 1.0);
}
