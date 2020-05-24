#version 450

layout( set = 2, binding = 0) uniform sampler2D sampler_color;
layout( set = 2, binding = 1) uniform material_att{
	float tranc;
};

//layout (input_attachment_index = 0, set = 2, binding = 1) uniform subpassInput inputColor;
layout (input_attachment_index = 1, set = 1, binding = 3) uniform subpassInput inputDepth;

/*layout( set = 1, binding = 2 ) uniform tex {
	int texture_index;
};

layout( location = 2 ) in vec2 in_uv;*/
layout( location = 3 ) in vec3 in_color;

layout( location = 10 ) in vec3 vert_normal;
layout( location = 11 ) in vec3 in_pos;

layout( location = 0 ) out vec4 frag_color;

void main() {
	float lod_bias = 0;
	vec3 light = vec3(0, 2.5 - 7, -7);
	float intensity = 0.75;
	vec3 light_normal = normalize(in_pos - light);
	float shade = dot(vert_normal, light_normal);

	frag_color = vec4((in_color * max(shade * intensity, 0.1)), tranc);
}
