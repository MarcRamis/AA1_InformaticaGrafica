#version 330
	layout (triangles) in;
	layout (triangle_strip, max_vertices = 6) out;
	
	// Inputs
	in vec4 vert_Normal[];

	// Uniforms
	uniform float moveWTime;
	
	void main()
	{
		for(int i = 0; i < 3; i++) 
		{ 
			vec4 offset = vec4(5.0,0.0,0.0,0.0);
			gl_Position = gl_in[i].gl_Position + offset;
			EmitVertex();
		}
		//EndPrimitive();
		//for(int i = 0; i < 3; i++) 
		//{ 
		//	gl_Position = gl_in[i].gl_Position + vec4(moveWTime, moveWTime, 0.0, 0.0);
		//	EmitVertex();
		//}
		//EndPrimitive(); 
	};