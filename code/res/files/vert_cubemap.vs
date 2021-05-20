#version 330

	// Inputs
	in vec3 in_Position;
	
	// Outputs
	out vec3 TexCoords;
	
	// Uniforms
	uniform mat4 mv_Mat;
	uniform mat4 mvpMat;
	
	void main()
	{
		TexCoords = in_Position;
		gl_Position = mvpMat * mv_Mat * vec4(in_Position, 1.0);
	};