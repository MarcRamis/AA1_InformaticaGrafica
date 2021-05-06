#version 330

	// Inputs
	in vec3 in_Position;
	in vec3 in_Normal;
	in vec2 in_TexCoord;

	// Outputs
	out vec4 _vert_Normal;
	out vec4 _fragPos;
	out vec2 _texCoord;
	
	// Uniforms
	uniform mat4 objMat;
	uniform mat4 mv_Mat;
	uniform mat4 mvpMat;
	
	void main()
	{
		gl_Position = mvpMat * objMat * vec4(in_Position, 1.0);

		mat4 normalMat = transpose(inverse(objMat));
		_vert_Normal = normalMat * vec4(in_Normal, 0.0);
		
		_fragPos = vec4(objMat * vec4(in_Position, 1.0));
		
		_texCoord = in_TexCoord;
	};