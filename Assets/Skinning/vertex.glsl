#include "Assets/Common/pga2.glsl"

#ifdef VERT
precision highp float;
layout (location = 0) in vec4 Pos;
layout (location = 1) in vec4 Col0;
layout (location = 2) in vec4 Col1;
layout (location = 3) in ivec4 Idx0;
layout (location = 4) in ivec4 Idx1;
layout (location = 5) in mat2x4 Extra;

uniform vec4 Proj;
uniform motor Model;
uniform motor Bones[2];

out vec4 Frag_Color;
out vec2 Frag_UV; 

void main()
{
	Frag_Color = Col0;
	Frag_UV = Extra[0].xy;
	
	/*vec3 pos = vec3(Pos.xy, 0);
	for (int i = 0; i < 4; i++)
	{
		int bi = Idx0[i];
		if (bi == 255) continue;

		float bw = Col1[bi];
		//motor m = gp_mm(Bones[bi], gp_mm(motor(1, 0, Pos.xy), reverse_m(Bones[bi])));
		motor m = gp_mm(Bones[bi], motor(1, 0, Pos.xy));
		m = gp_mm(reverse_m(Bones[bi]), m);
		pos.xy += bw * m.zw;
		//pos.xy += bw * sw_mp(Bones[bi], Pos.xy);
		pos.z += bw;
	}
	pos.xy /= pos.z;*/

	vec2 pos = sw_mp(Bones[0], Pos.xy);

	//pos.xy = sw_mp(Model, pos.xy);
	gl_Position = vec4((pos.x - Proj.x) / Proj.z, pos.y - Proj.y, Pos.z, Proj.w);
}
#endif

#ifdef FRAG
precision mediump float;
layout (location = 0) out vec4 Out_Color;

uniform sampler2D Tex;

in vec4 Frag_Color;
in vec2 Frag_UV;

void main()
{
	Out_Color = Frag_Color * texture(Tex, Frag_UV);
}
#endif