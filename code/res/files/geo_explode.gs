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
	uniform float time;
	uniform vec3 random;
	
	// Functions
	vec3 GetNormal()
	{
		vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
		vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
		return normalize(cross(a, b));
	} 

	vec4 explode(vec4 position, vec3 normal, vec3 rand)
	{
		float magnitude = 2.0;
		//vec3 direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude * rand; 
		vec3 direction = rand * normal * ((sin(time) + 1.0) / 2.0) * magnitude; 
		return position + vec4(direction, 0.0);
	}

	void main()
	{
		vec3 normal = GetNormal();

		// Create geometry
		for(int i = 0; i < 3; i++) 
		{
			gl_Position = explode(gl_in[i].gl_Position, normal, random);
			vert_Normal = _vert_Normal[i];
			fragPos = _fragPos[i];
			texCoord = _texCoord[i];
			EmitVertex();
		}
		EndPrimitive();
	};