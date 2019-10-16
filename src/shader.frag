#version 450

layout( location = 0 ) in vec3 vert_normal;

layout( location = 0 ) out vec4 frag_color;

void main() {
	vec3 light = vec3(0, 2.5, -7);
	vec3 light_normal = normalize(light - gl_FragCoord.xyz);
	float shade = dot(vert_normal, light_normal);

	frag_color = vec4( 0.8, 0.4, 0.0, 1.0 ) * max(shade, 0.1);
}
