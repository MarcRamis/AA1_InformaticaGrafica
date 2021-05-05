#version 330
	
	// Inputs
	in vec4 vert_Normal;
	
	// Outputs
	//out vec4 out_Color;
	
	// Uniforms
	uniform vec4 objColor;	// Object color
	
	// Ambient Components
	uniform vec4 ambient_strength;
	uniform vec4 ambient_color;

	void main() 
	{
		// ----- PHONG SHADING
		// --- Ambient
		vec4 ambientComp = ambient_color * ambient_strength;
		// --- Diffuse
		//vec4 diffuseComp = dot(vert_Normal, normalize(dir_light)) * diffuse_strength * diffuse_color;
		// --- Specular
		
		//if(mod(gl_FragCoord.x,5) > 0.5 && mod(gl_FragCoord.y,5) > 0.5)
		//{
		//	discard;
		//}
		out_Color = objColor * (ambientComp); 
	};