#version 330 core

layout(location=0) in vec3 pos;
layout(location=1) in vec3 in_normal;
layout(location=2) in vec2 tex;

uniform mat4 m;
uniform mat4 v;
uniform mat4 p;

uniform float morhping_progress;

out vec3 Normal;
out vec3 position;
out vec2 vert_tex;

void main() {
    position = vec3(m * vec4(pos, 1.0));
    position = mix(position, normalize(position),  morhping_progress);
    vert_tex = tex;

    Normal = vec3(m * vec4(in_normal, 1.0));
    Normal = mix(Normal, normalize(position), morhping_progress);
    gl_Position = p * v * vec4(position, 1.0);
}