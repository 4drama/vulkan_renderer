#version 450

layout (input_attachment_index = 0, set = 1, binding = 0) uniform subpassInput sampler_position;
layout (input_attachment_index = 1, set = 1, binding = 1) uniform subpassInput sampler_normal;
layout (input_attachment_index = 2, set = 1, binding = 2) uniform subpassInput sampler_deffuse;

layout( location = 0 ) out vec4 frag_color;

void main(){
	vec3 position = subpassLoad(sampler_position).xyz;
	vec3 normal = subpassLoad(sampler_normal).rgb;
	vec4 deffuse = subpassLoad(sampler_deffuse);

	vec3 light = vec3(0, 2.5 - 7, -7);
	float intensity = 0.75;
	vec3 light_normal = normalize(position - light);
	float shade = dot(normal, light_normal);

	if(normal != vec3(0, 0, 0))
		frag_color = (deffuse * max(shade * intensity, 0.1)).bgra;
	else
		frag_color = deffuse;
}
