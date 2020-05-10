#version 450

layout( set = 2, binding = 0) uniform sampler2D sampler_color;

layout( location = 2 ) in vec2 in_uv;

layout( location = 10 ) in vec3 vert_normal;
layout( location = 11 ) in vec3 in_pos;

layout( location = 0 ) out vec4 frag_color;

void main() {
	float lod_bias = 0;
	vec4 color = texture(sampler_color, in_uv, lod_bias);

	vec3 light = vec3(0, 2.5 - 7, -7);
	float intensity = 0.75;
	vec3 light_normal = normalize(in_pos - light);
	float shade = dot(vert_normal, light_normal);

	frag_color = (color * max(shade * intensity, 0.1)).bgra;
}
