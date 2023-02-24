# shader vertex
# version 330 core


layout(location = 0) in vec4 aPos;

uniform mat4 Trans;



void main()
{
	gl_Position = Trans * aPos;
};




# shader fragment
# version 330 core


out vec4 sph_colors;


void main()
{
	sph_colors = vec4(1.0f, 0.0f, 0.0f, 1.0f);
};