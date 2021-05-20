#version 330
	
	// Inputs
	in vec3 TexCoords;
	
	// Outputs
	out vec4 out_Color;

	// Uniforms
	uniform samplerCube skybox;

	void main() 
	{
		// --- Texture
		out_Color = texture(skybox, TexCoords);
	};