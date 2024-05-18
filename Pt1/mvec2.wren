foreign class MVec2 {
	construct new(e0, e1, e2, e12) {}

	foreign e0
	foreign e1
	foreign e2
	foreign e12

	foreign e0=(v)
	foreign e1=(v)
	foreign e2=(v)
	foreign e12=(v)

	foreign *(z)
	foreign %(z)
	foreign ^(z)
	foreign dual
	foreign -
	foreign ~
	foreign |(z)
	foreign grade(i)

	foreign draw(c)
	foreign debug(s)

	toString { "[%(e0), %(e1), %(e2), %(e12)]" }
}