#version 330
	in vec3 in_Position;
	uniform mat4 objMat;
	uniform mat4 mv_Mat;
	uniform mat4 mvpMat;
	void main()
	{
		gl_Position = mvpMat * objMat * vec4(in_Position, 1.0);
	};