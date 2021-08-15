#version 330

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 mvMatrix;
uniform mat4 mvpMatrix;
uniform mat4 norMatrix;
uniform vec4 lightPos;
uniform float waterLevel;

in vec2 tcoord[];
out vec4 lighting;
out vec2 geomTcord;

vec3 GetNormal()
{
    vec3 a = vec3(gl_in[0].gl_Position.xyz) - vec3(gl_in[2].gl_Position.xyz);
    vec3 b = vec3(gl_in[1].gl_Position.xyz) - vec3(gl_in[2].gl_Position.xyz);
    return normalize(cross(a, b));
}


void main()
{
	vec4 white = vec4(1.0);
	vec4 grey = vec4(0.2);


	for (int i = 0; i < 3; i++) {
		vec4 posnEye = mvMatrix * gl_in[i].gl_Position;
		vec4 normalEye = vec4(GetNormal(), 0);
		vec4 lgtVec = normalize(lightPos - posnEye);
//		vec4 viewVec = normalize(vec4(-posnEye.xyz, 0)); 
//		vec4 halfVec = normalize(lgtVec + viewVec); 

		vec4 ambOut = grey;
		float shininess = 100.0;
		float diffTerm = dot(lgtVec, normalEye);
		vec4 diffOut = vec4(1) * diffTerm;
//		float specTerm = max(dot(halfVec, normalEye), 0);
//		vec4 specOut = white *  pow(specTerm, shininess);

		geomTcord = tcoord[i];
		lighting = ambOut + diffOut;

		gl_Position = gl_in[i].gl_Position;

		EmitVertex();
	}

	EndPrimitive();
}