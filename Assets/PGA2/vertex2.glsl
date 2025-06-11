#ifdef VERT
precision highp float;
layout (location = 0) in vec4 Pos;
layout (location = 1) in vec4 Col0;
layout (location = 2) in vec4 Col1;
layout (location = 3) in vec4 Idx0;
layout (location = 4) in vec4 Idx1;
layout (location = 5) in mat2x4 Extra;

uniform vec4 Proj;

out vec4 Frag_Color;

void main()
{
	Frag_Color = Col0;
	vec4 position = vec4((Pos.x - Proj.x) / Proj.z, Pos.y - Proj.y, Pos.z, Proj.w);
	gl_Position = vec4(position.xyz / Pos.w, position.w);
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