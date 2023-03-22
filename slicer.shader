# shader vertex
# version 330 core


layout(location = 0) in vec4 aPos;
layout(location = 2) in vec3 texcoords;

uniform mat4 MVP;
uniform float slider;
uniform mat4 view;
uniform int axis;


out vec3 vertex_tex;

void main()
{
	gl_Position = MVP * aPos;
	// based on eye, choose the vertex position
	if (axis == 2) {
		vertex_tex = vec3(texcoords.x, texcoords.y, slider);
	}
	else if (axis == 1)
	{
		vertex_tex = vec3(texcoords.x, slider, texcoords.y);
	}
	else
	{
		vertex_tex = vec3(slider, texcoords.x, texcoords.y);
	}
};




# shader fragment
# version 330 core

in vec3 vertex_tex;

out vec4 colors;

uniform sampler3D volumeTexture;

void main()
{
	float lineWidthHalf = 0.002f;
	colors = texture(volumeTexture, vertex_tex);

	// X axis
	if (abs(vertex_tex.y - 0.5f) < lineWidthHalf && abs(vertex_tex.z - 0.5f) < lineWidthHalf && vertex_tex.x > 0.5f) {
		colors = vec4(8.0f, 0.0f, 0.0f, 1.0f);
	}
	// Y axis
	if (abs(vertex_tex.x - 0.5f) < lineWidthHalf && abs(vertex_tex.z - 0.5f) < lineWidthHalf && vertex_tex.y > 0.5f) {
		colors = vec4(0.0f, 0.6f, 0.0f, 1.0f);
	}
	// Z axis
	if (abs(vertex_tex.x - 0.5f) < lineWidthHalf && abs(vertex_tex.y - 0.5f) < lineWidthHalf && vertex_tex.z > 0.5f) {
		colors = vec4(0.0f, 0.2f, 0.6f, 1.0f);
	}
};