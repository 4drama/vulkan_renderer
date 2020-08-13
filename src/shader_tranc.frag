#version 450

layout( set = 2, binding = 0) uniform sampler2D sampler_color;
layout( set = 2, binding = 1) uniform material_att{
	float tranc;
};

//layout (input_attachment_index = 0, set = 2, binding = 1) uniform subpassInput inputColor;
//layout (input_attachment_index = 1, set = 1, binding = 3) uniform subpassInput inputDepth;

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

	vec3 camera_normal = normalize(vec3(0, 0, -1) - in_pos);
	vec3 reflect = light_normal - (2 * vert_normal) * dot(light_normal, vert_normal);
	float shininess = 110;
	
	vec3 material_specular = vec3(255, 255, 255) * 0.1;
	vec3 light_specular = vec3(255, 255, 255) * 0.1;
	
	vec3 specular = material_specular * light_specular * pow(max(0, dot(camera_normal, reflect)), shininess);
	
	vec3 material_ambient = in_color * 0.025;
	vec3 light_ambient = vec3(255, 255, 255) * 0.025;
	
	vec3 ambient_color = material_ambient * light_ambient;
	float f;
	if(shade > 0){
		f = 1;
	} else {
		f = 0;
	}

	vec3 light_deffuse = vec3(255, 255, 255) * 0.0045;
	vec3 deffuse = in_color;
	
	frag_color = vec4(ambient_color + f * (light_deffuse * deffuse * max(shade * intensity, 0) + specular), tranc);
}
