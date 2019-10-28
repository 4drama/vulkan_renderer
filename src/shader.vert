#version 450

layout( location = 0 ) in vec3 app_position;
layout( location = 1 ) in vec3 app_normal;

layout( location = 2 ) in vec2 in_uv;
layout( location = 3 ) in vec3 in_color;

layout( set = 0, binding = 0 ) uniform MVPBuffer {
	mat4 mvp;
	mat4 mv;
};

layout( location = 2 ) out vec2 out_uv;
layout( location = 3 ) out vec3 out_color;

layout (location = 10) out vec3 out_normal;
layout (location = 11) out vec3 out_pos;

void main(){
	gl_Position = mvp * vec4(app_position, 1);

	out_normal = vec4(mv * vec4(app_normal, 0)).xyz;
	out_pos = vec4(mv * vec4(app_position, 1)).xyz;

	out_uv = in_uv;
	out_color = in_color;
}
