#version 430 core

layout (location=0) in vec3 Position;
layout (location=2) in vec3 VertexPosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
	//Преобразование координат
    gl_Position = proj * view * (vec4(Position, 1.0) + model * vec4( VertexPosition, 1.0));
}