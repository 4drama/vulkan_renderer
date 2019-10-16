#version 450

layout( location = 0 ) in vec4 app_position;
layout( location = 1 ) in vec4 app_normal;

layout( set = 0, binding = 0 ) uniform MVPBuffer {
	mat4 mvp;
	mat4 mv;
};

layout (location = 0) out vec3 out_normal;

void main(){
	gl_Position = mvp * app_position;
	vec4 normal = mv * app_normal;
	out_normal = normal.xyz;
}
