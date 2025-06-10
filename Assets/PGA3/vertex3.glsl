#include "Assets/PGA3/pga3.glsl"

#ifdef VERT
precision highp float;
layout (location = 0) in vec3 Position;
layout (location = 1) in vec4 Color;

uniform mat3x2 Proj;
uniform motor View;
uniform vec4 Tint;

out vec4 Frag_Color;

void main()
{
	Frag_Color = Color * Tint;
	vec3 p = sw_mp(reverse_m(View), Position);
	gl_Position = project(Proj, p);
	//gl_Position = project(Proj, Position);
}
#endif

#ifdef FRAG
precision mediump float;
layout (location = 0) out vec4 Out_Color;

in vec4 Frag_Color;

void main()
{
	Out_Color = Frag_Color;
}
#endif