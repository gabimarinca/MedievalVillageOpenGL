#version 410 core

layout(location = 0) in vec3 position;

uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * vec4(position, 1.0);
    gl_PointSize = 4.0; 
}
