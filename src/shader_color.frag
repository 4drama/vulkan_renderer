#version 450

layout( location = 3 ) in vec3 in_color;

layout( location = 10 ) in vec3 vert_normal;
layout( location = 11 ) in vec3 in_pos;

layout( location = 0 ) out vec4 frag_color;

layout( location = 1 ) out vec4 position;
layout( location = 2 ) out vec4 normal;
layout( location = 3 ) out vec4 albedo;

void main() {
	float lod_bias = 0;
	vec4 in2_color = vec4(in_color, 255);

	vec3 light = vec3(0, 2.5 - 7, -7);
	float intensity = 0.75;
	vec3 light_normal = normalize(in_pos - light);
	float shade = dot(vert_normal, light_normal);

	vec4 result_color = (in2_color * max(shade * intensity, 0.1)).bgra;
	frag_color = result_color;

	position = vec4(in_pos, 1);
	normal = vec4(vert_normal, 1);
	albedo = in2_color;
}
