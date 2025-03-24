#ifndef PGA2_GUARD
#define PGA2_GUARD

#define motor     vec4
#define point     vec2

const float PI = 3.141592653;

point sw_mp(motor a, point b)
{
	return point(
		a.x*a.x*b.y + a.x*a.y*b.x - a.x*a.z + a.y*a.w - a.x*a.z + a.x*a.y*b.x + a.y*a.y*b.y + a.y*a.w,
		a.x*a.x*b.x + a.x*a.y*b.y + a.x*a.w + a.y*a.z + a.x*a.w - a.x*a.y*b.y - a.y*a.y*b.x + a.y*a.z
	);
}

motor reverse_m(motor a)
{
	return motor(a.x, -a.yzw); 
}

motor gp_mm(motor a, motor b)
{
	return motor(
		a.x*b.x - a.y*b.y,
		a.x*b.y + a.y*b.x,
		a.x*b.z + a.w*b.y + a.z*b.x - a.y*b.w,
		a.x*b.w + a.w*b.x - a.z*b.y + a.y*b.z
	);
}

#endif // PGA2_GUARD