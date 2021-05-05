		#version 330
	in vec3 in_Position;
	in vec3 in_Normal;
	out vec4 vert_Normal;
	out vec4 fragPos;
	uniform mat4 objMat;
	uniform mat4 mv_Mat;
	uniform mat4 mvpMat;
	void main() {
		gl_Position = mvpMat * objMat * vec4(in_Position, 1.0); 
		mat4 normalMat = transpose(inverse(objMat));
		vert_Normal = normalMat * vec4(in_Normal, 0.0);
		fragPos = vec4(objMat * vec4(in_Position, 1.0));
	};