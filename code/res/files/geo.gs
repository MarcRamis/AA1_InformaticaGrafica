#version 330
	layout (triangles) in;
	layout (triangle_strip, max_vertices = 3) out;
	
	// Inputs
	in vec4 _vert_Normal[];
	in vec4 _fragPos[];
	in vec2 _texCoord[];

	// Outputs
	out vec4 vert_Normal;
	out vec4 fragPos;
	out vec2 texCoord;

	// Uniforms
	uniform float moveWTime;
	
	void main()
	{
		// Create geometry
		for(int i = 0; i < 3; i++) 
		{
			gl_Position = gl_in[i].gl_Position;
			vert_Normal = _vert_Normal[i];
			fragPos = _fragPos[i];
			texCoord = _texCoord[i];
			EmitVertex();
		}
		EndPrimitive();

		// DUPLICATE
		//for(int i = 0; i < 3; i++) 
		//{ 
		//	// MOVE IN TIME
		//	gl_Position = gl_in[i].gl_Position + vec4(moveWTime, moveWTime, 0.0, 0.0);
		//
		//	vert_Normal = _vert_Normal[i];
		//	fragPos = _fragPos[i];
		//	texCoord = _texCoord[i];
		//
		//	EmitVertex();
		//}
		//EndPrimitive(); 
	};