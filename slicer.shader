# shader vertex
# version 330 core


layout(location = 0) in vec4 aPos;
layout(location = 2) in vec3 texcoords;

uniform mat4 M;
uniform mat4 VP;
uniform vec3 S;				// Sx, Sy, Sz = size of the volume

out vec3 vertex_tex;

void main()
{
	vec4 w = M * aPos;
	vec4 t = vec4(w.x / S.x, w.y / S.y, w.z / S.z, 1.0f);

	gl_Position = VP * w;
	vertex_tex = t.xyz;
};




# shader fragment
# version 330 core

in vec3 vertex_tex;

out vec4 colors;

uniform sampler3D ourText;

void main()
{
	colors = texture(ourText, vertex_tex);
};