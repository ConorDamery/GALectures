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

	foreign -
	foreign ~
	foreign dual
	foreign inv

	foreign +(z)
	foreign -(z)
	foreign *(z)
	foreign %(z)
	foreign ^(z)
	foreign /(z)
	foreign |(z)

	foreign grade(i)

	foreign static exp(z)
	foreign static inf

	static scalar(s) { MVec2.new(s, 0, 0, 0) }
	static vector(x, y) { MVec2.new(0, x, y, 0) }
	static bivector(xy) { MVec2.new(0, 0, 0, xy) }

	foreign draw(c)
	foreign debug(s)

	toString { "[%(e0), %(e1), %(e2), %(e12)]" }
}