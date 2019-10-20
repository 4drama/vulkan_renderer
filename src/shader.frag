#version 450

layout( location = 0 ) in vec3 vert_normal;
layout( location = 1 ) in vec3 in_pos;

layout( location = 0 ) out vec4 frag_color;

void main() {
	vec3 light = vec3(0, -7, -7);
	float intensity = 0.75;
//	vec3 pos = gl_FragCoord.xyz - vec3(500, 400, 300);
	vec3 light_normal = normalize(in_pos - light);
	float shade = dot(vert_normal, light_normal);

	frag_color = vec4( 0.8, 0.4, 0.0, 1.0 ) * max(shade * intensity, 0.1);
}
