#version 330

	// Inputs
	in vec3 in_Position;
	
	// Outputs
	out vec3 TexCoords;
	
	// Uniforms
	uniform mat4 objMat;
	uniform mat4 mvpMat;
	
	void main()
	{
		TexCoords = in_Position;
		vec4 pos = mvpMat * objMat * vec4(in_Position, 1.0);
		gl_Position = pos.xyww;
	};