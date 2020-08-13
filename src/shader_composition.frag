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

	vec3 camera_normal = normalize(vec3(0, 0, -1) - position);
	vec3 reflect = light_normal - (2 * normal) * dot(light_normal, normal);
	float shininess = 10;	// 0 - 128
	
	vec3 material_specular = vec3(deffuse.r, deffuse.g, deffuse.b) * 0.1;
	vec3 light_specular = vec3(255, 255, 255) * 0.1;
	
	vec4 specular = vec4(material_specular * light_specular * pow(max(0, dot(camera_normal, reflect)), shininess), 1);
	
	vec4 material_ambient = deffuse * 0.025;
	vec4 light_ambient = vec4(255, 255, 255, 1) * 0.025;
	
	vec4 ambient_color = material_ambient * light_ambient;
	float f;
	if(shade > 0){
		f = 1;
	} else {
		f = 0;
	}
	
	vec4 light_deffuse = vec4(255, 255, 255, 1) * 0.0045;
	if(normal != vec3(0, 0, 0))
		frag_color = ambient_color + f * ((light_deffuse * deffuse * max(shade * intensity, 0) + specular).bgra);
	else
		frag_color = ambient_color + deffuse + specular;
}
