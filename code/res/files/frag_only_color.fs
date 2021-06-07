#version 330
	
	// Inputs
	in vec4 vert_Normal;
	in vec4 fragPos;
	in vec2 texCoord;
	
	// Outputs
	out vec4 out_Color;
	
	// Uniforms
	uniform vec4 objColor;
	
	void main() 
	{
		out_Color = objColor;
	};