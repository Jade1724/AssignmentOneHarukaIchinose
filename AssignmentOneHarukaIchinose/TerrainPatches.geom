#version 330

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 mvpMatrix;
uniform vec4 lightPos;
uniform float waterLevel;
uniform float snowLevel;
//uniform vec3 eyePos;
//
in ES_OUT
{
    vec2 tcoord;
    float height;
} es_in[];

out vec4 lighting;
out vec2 geomTcord;
out vec4 texWeight;

vec3 normal()
{
    vec3 a = vec3(gl_in[0].gl_Position.xyz) - vec3(gl_in[2].gl_Position.xyz);
    vec3 b = vec3(gl_in[1].gl_Position.xyz) - vec3(gl_in[2].gl_Position.xyz);
    return normalize(cross(a, b));
}

void main()
{
	vec4 white = vec4(1.0);
	vec4 grey = vec4(0.3);

	for (int i = 0; i < 3; i++) {
	
		vec3 lgtVec = normalize(lightPos.xyz - gl_in[i].gl_Position.xyz);
		vec4 ambOut = grey;
		float diffTerm = max(dot(lgtVec, normal()), 0);
		vec4 diffOut = white * diffTerm;

		geomTcord = es_in[i].tcoord;
		lighting = ambOut + diffOut;

//		vec4 avg = (    gl_in[0].gl_Position
//                +   gl_in[1].gl_Position
//                +   gl_in[2].gl_Position
//                +   gl_in[3].gl_Position) * 0.25;

//		float dist = distance(eyePos, avg.xyz);
//		float t = (dist + 4.0) / (-20.0 + 4.0);      // Interpolation parameter for fog
//		lighting = (1-t)*lighting + t*white;
//		lighting = vec4(posnEye.z, 0, 0, 1);

		float height = es_in[i].height;
		float blendingRatio;
		float grassThreshold = 2;
		float grassRockThreshold = 6.0;
		float rockThreshold = 8;

		if (height < waterLevel) {
			texWeight = vec4(1, 0, 0, 0);
		}
		else if (height > waterLevel && height <= grassThreshold) {
			texWeight = vec4(0, 1, 0, 0);
		}
		else if (height > grassThreshold && height <= grassRockThreshold) {
			blendingRatio = (height - grassThreshold)/(grassRockThreshold - grassThreshold);
			texWeight = vec4(0, 1 - blendingRatio, blendingRatio, 0);
		}
		else if (height > grassRockThreshold && height < snowLevel) {
			blendingRatio = (height - grassRockThreshold) / (snowLevel - grassRockThreshold);
			texWeight = vec4(0, 0, 1 - blendingRatio, blendingRatio);
		}

		if (height > snowLevel && height > waterLevel) {
			texWeight = vec4(0, 0, 0, 1);
		}

		gl_Position = mvpMatrix * gl_in[i].gl_Position;

		EmitVertex();
	}

	EndPrimitive();
}