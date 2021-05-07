#version 330
	layout (points) in;
	layout (triangle_strip, max_vertices = 4) out;
	
	// Inputs
	in vec4 _vert_Normal[];
	in vec4 _fragPos[];
	in vec2 _texCoord[];

	// Outputs
	out vec4 vert_Normal;
	out vec4 fragPos;
	out vec2 texCoord;

	void main()
	{
		gl_Position = gl_in[0].gl_Position + vec4(-10, -10, 0.0, 0.0); 
		vert_Normal = vec4(0.0,0.0,1.0,0.0);
		fragPos = gl_in[0].gl_Position + vec4(-10, -10, 0.0, 0.0); 
		texCoord = vec2(0.0,1.0);
		EmitVertex();

		gl_Position = gl_in[0].gl_Position + vec4( 10, -10, 0.0, 0.0);  
		vert_Normal = vec4(0.0,0.0,1.0,0.0);
		fragPos = gl_in[0].gl_Position + vec4(10, -10, 0.0, 0.0); 
		texCoord = vec2(1.0,1.0);
		EmitVertex();

		gl_Position = gl_in[0].gl_Position + vec4(-10,  10, 0.0, 0.0);
		vert_Normal = vec4(0.0,0.0,1.0,0.0);
		fragPos = gl_in[0].gl_Position + vec4(-10, 10, 0.0, 0.0); 
		texCoord = vec2(0.0,0.0);
		EmitVertex();
		
		gl_Position = gl_in[0].gl_Position + vec4( 10,  10, 0.0, 0.0);  
		vert_Normal = vec4(0.0,0.0,1.0,0.0);
		fragPos = gl_in[0].gl_Position + vec4(10, 10, 0.0, 0.0); 
		texCoord = vec2(1.0,0.0);
		EmitVertex();
		
		EndPrimitive();  
	};