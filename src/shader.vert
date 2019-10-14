#version 450

layout( location = 0 ) in vec4 app_position;

layout( set = 0, binding = 0 ) uniform MVPBuffer {
	mat4 mvp;
};

void main(){
	gl_Position = mvp * app_position;
}
