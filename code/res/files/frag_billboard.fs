#version 330
	
	// Inputs
	in vec4 vert_Normal;
	in vec4 fragPos;
	in vec2 texCoord;
	
	// Outputs
	out vec4 out_Color;
	
	// Uniforms
	uniform vec4 objColor;	// Object color
	uniform vec4 dir_light; // Light pos
	uniform vec4 viewPos;	// Camera pos
	uniform sampler2D ourTexture; // Texture
	
		// Ambient Components
	uniform vec4 ambient_strength;
	uniform vec4 ambient_color;

		// Diffuse Components
	uniform vec4 diffuse_strength;
	uniform vec4 diffuse_color;

		// Specular Components
	uniform vec4 specular_strength;
	uniform vec4 specular_color;
	uniform float shininess;
	
		// Uniform booleans
	uniform bool isMatrix;
	uniform float displaceX;
	uniform float displaceY;
	
	void main() 
	{
		// ----- PHONG SHADING
		// --- Ambient
		vec4 ambientComp = ambient_color * ambient_strength;
		// --- Diffuse
		vec4 diffuseComp = max(dot(normalize(vert_Normal), normalize(dir_light)),0.0) * diffuse_strength * diffuse_color;
		// --- Specular
		vec4 viewDir = viewPos - fragPos;
		vec4 reflectDir = reflect(normalize(-dir_light), vert_Normal);
		vec4 specularComp = pow(max(dot(normalize(viewDir), reflectDir),0.0),shininess) * specular_strength * specular_color;
		
		// DISCARD EFFECT
		if(isMatrix)
		{
			if(mod(gl_FragCoord.x,displaceX) > 0.5 && mod(gl_FragCoord.y,displaceY) > 0.5)
			{
				discard;
			}
		}

		// --- Texture
		vec4 textureColor = texture(ourTexture,texCoord);

		if(textureColor.r <= 0.1 && textureColor.g <= 0.1 && textureColor.b <= 0.1)
		{
			discard;
		}

		out_Color = textureColor * objColor * (ambientComp + diffuseComp + specularComp);
	};