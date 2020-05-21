#version 450

layout( set = 2, binding = 0) uniform sampler2D sampler_color;

//layout (input_attachment_index = 0, set = 2, binding = 1) uniform subpassInput inputColor;
layout (input_attachment_index = 1, set = 1, binding = 3) uniform subpassInput inputDepth;

/*layout( set = 1, binding = 2 ) uniform tex {
	int texture_index;
};

layout( location = 2 ) in vec2 in_uv;*/
layout( location = 3 ) in vec3 in_color;

/*layout( location = 10 ) in vec3 vert_normal;
layout( location = 11 ) in vec3 in_pos;*/

layout( location = 0 ) out vec4 frag_color;

void main() {
//	float depth = subpassLoad(inputDepth).r;
	frag_color = vec4(1, 1, 1, 0.05);
}
