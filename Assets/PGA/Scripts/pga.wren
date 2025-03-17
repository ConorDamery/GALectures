import "app" for App

// Based on: pga.glsl
// by Steven De Keninck ("Look, Ma, No Matrices!" https://enkimute.github.io/LookMaNoMatrices/)

//#define direction vec3    // [ e032, e013, e021 ] implied 0 e123
class Direction {
	construct new(e032, e013, e021) {
		_e032 = e032
		_e013 = e013
		_e021 = e021
	}

	e032 { _e032 }
	e013 { _e013 }
	e021 { _e021 }

	e032=(v) { _e032 = v }
	e013=(v) { _e013 = v }
	e021=(v) { _e021 = v }

	[i] {
		if (i == 0) {
			return e032
		} else if (i == 1) {
			return e013
		} else if (i == 2) {
			return e021
		} else {
        	Fiber.abort("Index out of bounds: " + i)
		}
	}

	glUniform(name) {
		App.glUniform(name)
		App.glVec3f(e032, e013, e021)
	}

	glDraw(color) {
		App.glBegin(true, true, 2, 1)
		App.glVertex(0, 0, 0, color)
		App.glVertex(e032, e013, e021, color)
		App.glEnd(App.glLines)
	}

	toString { "[%(e032), %(e013), %(e021)]" }
}

//#define point     vec3    // [ e032, e013, e021 ] implied 1 e123
class Point {
	construct new(e032, e013, e021) {
		_e032 = e032
		_e013 = e013
		_e021 = e021
	}

	e032 { _e032 }
	e013 { _e013 }
	e021 { _e021 }

	e032=(v) { _e032 = v }
	e013=(v) { _e013 = v }
	e021=(v) { _e021 = v }

	[i] {
		if (i == 0) {
			return e032
		} else if (i == 1) {
			return e013
		} else if (i == 2) {
			return e021
		} else {
        	Fiber.abort("Index out of bounds: " + i)
		}
	}

	glUniform(name) {
		App.glUniform(name)
		App.glVec3f(e032, e013, e021)
	}

	glDraw(color) {
		App.glBegin(true, true, 10, 1)
		App.glVertex(e032, e013, e021, color)
		App.glEnd(App.glPoints)
	}

	toString { "[%(e032), %(e013), %(e021)]" }
}

//#define line      mat2x3  // [ [e23, e31, e12], [e01, e02, e03] ]
class Line {
	construct new(e23, e31, e12, e01, e02, e03) {
		_e23 = e23
		_e31 = e31
		_e12 = e12
		_e01 = e01
		_e02 = e02
		_e03 = e03
	}

	e23 { _e23 }
	e31 { _e31 }
	e12 { _e12 }
	e01 { _e01 }
	e02 { _e02 }
	e03 { _e03 }

	e23=(v) { _e23 = v }
	e31=(v) { _e31 = v }
	e12=(v) { _e12 = v }
	e01=(v) { _e01 = v }
	e02=(v) { _e02 = v }
	e03=(v) { _e03 = v }
	
	[i] {
		if (i == 0) {
			return e23
		} else if (i == 1) {
			return e31
		} else if (i == 2) {
			return e12
		} else if (i == 1) {
			return e01
		} else if (i == 1) {
			return e02
		} else if (i == 1) {
			return e03
		} else {
        	Fiber.abort("Index out of bounds: " + i)
		}
	}

	glUniform(name) {
		App.glUniform(name)
		App.glMat2x3f(e23, e31, e12, e01, e02, e03)
	}

	glDraw(color) {
		App.glBegin(true, true, 2, 1)
		App.glVertex(-e23 + e01, -e31 + e02, -e12 + e03, color)
    	App.glVertex(e23 + e01, e31 + e02, e12 + e03, color)
		App.glEnd(App.glLines)
	}

	toString { "[%(e23), %(e31), %(e12) | %(e01), %(e02), %(e03)]" }
}

//#define motor     mat2x4  // [ [s, e23, e31, e12], [e01, e02, e03, e0123] ] 
class Motor {
	construct new(s, e23, e31, e12, e01, e02, e03, e0123) {
		_s = s
		_e23 = e23
		_e31 = e31
		_e12 = e12
		_e01 = e01
		_e02 = e02
		_e03 = e03
		_e0123 = e0123
	}

	s { _s }
	e23 { _e23 }
	e31 { _e31 }
	e12 { _e12 }
	e01 { _e01 }
	e02 { _e02 }
	e03 { _e03 }
	e0123 { _e0123 }

	s=(v) { _s = v }
	e23=(v) { _e23 = v }
	e31=(v) { _e31 = v }
	e12=(v) { _e12 = v }
	e01=(v) { _e01 = v }
	e02=(v) { _e02 = v }
	e03=(v) { _e03 = v }
	e0123=(v) { _e0123 = v }

	// Identity motor
	static identity { Motor.new(1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0) }

	[i] {
		if (i == 0) {
			return s
		} else if (i == 1) {
			return e23
		} else if (i == 2) {
			return e31
		} else if (i == 3) {
			return e12
		} else if (i == 4) {
			return e01
		} else if (i == 5) {
			return e02
		} else if (i == 6) {
			return e03
		} else if (i == 7) {
			return e0123
		} else {
        	Fiber.abort("Index out of bounds: " + i)
		}
	}

	// Geometric product
	//*(b) { Point.new(0, 0, 0) }

	// Inner product
	//|(b) { Point.new(0, 0, 0) }

	// Outer product
	//^(b) { Point.new(0, 0, 0) }

	// Sandwich product
	//>>(b) { Point.new(0, 0, 0) }

	// Left contraction
	//<<(b) { Point.new(0, 0, 0) }

	// Regressive product
	//&(b) { Point.new(0, 0, 0) }

	// Reverse operator
	//~ { Point.new(0, 0, 0) }

	// Dual operator
	//! { Point.new(0, 0, 0) }

	// Grade selection
	//grade(i) { Point.new(0, 0, 0) }

	// Normalization
	//normalized { Point.new(0, 0, 0) }

	// Exponentiation
	//exp(x) { Point.new(0, 0, 0) }

	// Logarithm
	//log { Point.new(0, 0, 0) }

	glUniform(name) {
		App.glUniform(name)
		App.glMat2x4f(s, e23, e31, e12, e01, e02, e03, e0123)
	}

	toString { "[%(s), %(e23), %(e31), %(e12) | %(e01), %(e02), %(e03), %(e0123)]" }
}

class PGA {
	// Basis planes e1,e2,e3
	static e1 { Direction.new(1.0, 0.0, 0.0) }
	static e2 { Direction.new(0.0, 1.0, 0.0) }
	static e3 { Direction.new(0.0, 0.0, 1.0) }

	// Basis directions
	static e032 { Direction.new(1.0, 0.0, 0.0) }
	static e013 { Direction.new(0.0, 1.0, 0.0) }
	static e021 { Direction.new(0.0, 0.0, 1.0) }

	// Basis lines
	static e23 { Line.new(1.0, 0.0, 0.0, 0.0, 0.0, 0.0) }
	static e31 { Line.new(0.0, 1.0, 0.0, 0.0, 0.0, 0.0) }
	static e12 { Line.new(0.0, 0.0, 1.0, 0.0, 0.0, 0.0) }
	static e01 { Line.new(0.0, 0.0, 0.0, 1.0, 0.0, 0.0) }
	static e02 { Line.new(0.0, 0.0, 0.0, 0.0, 1.0, 0.0) }
	static e03 { Line.new(0.0, 0.0, 0.0, 0.0, 0.0, 1.0) }

	static checkType(name, p, type) {
		if (!(p is type)) {
			Fiber.abort("Param '%(name)' must be a %(type.name)!")
		}
	}

	// point sw_mp(motor a, point b)
	static sw_mp(a, b) {
		checkType("a", a, Motor)
		checkType("b", b, Point)
		
		var t = Direction.new(
			(b.e013 * a.e12 - b.e021 * a.e31) - a.e01,
			(b.e021 * a.e23 - b.e032 * a.e12) - a.e02,
			(b.e032 * a.e31 - b.e013 * a.e23) - a.e03
		)

		return Point.new(
			(a.s * t.e032 + (t.e013 * a.e12 - t.e021 * a.e31) - a.e23 * a.e0123) * 2.0 + b.e032,
			(a.s * t.e013 + (t.e021 * a.e23 - t.e032 * a.e12) - a.e31 * a.e0123) * 2.0 + b.e013,
			(a.s * t.e021 + (t.e032 * a.e31 - t.e013 * a.e23) - a.e12 * a.e0123) * 2.0 + b.e021
		)
	}

	// point swx_mp(motor a, point b)
	static swx_mp(a, b) {
		checkType("a", a, Motor)
		checkType("b", b, Point)

		var t = Direction.new(
			(b.e013 * a.e12 - b.e021 * a.e31) - a.e01,
			(b.e021 * a.e23 - b.e032 * a.e12) - a.e02,
			(b.e032 * a.e31 - b.e013 * a.e23) - a.e03
		)

		return Point.new(
			a.s * t.e032 + (t.e013 * a.e12 - t.e021 * a.e31) - a.e23 * a.e0123,
			a.s * t.e013 + (t.e021 * a.e23 - t.e032 * a.e12) - a.e31 * a.e0123,
			a.s * t.e021 + (t.e032 * a.e31 - t.e013 * a.e23) - a.e12 * a.e0123
		)
	}

	// direction sw_md(motor a, direction b)
	static sw_md(a, b) {
		checkType("a", a, Motor)
		checkType("b", b, Direction)

		var t = Direction.new(
			(b.e013 * a.e021 - b.e021 * a.e013),
			(b.e021 * a.e032 - b.e032 * a.e021),
			(b.e032 * a.e013 - b.e013 * a.e032)
		)

		return Direction.new(
			(a.s * t.e032 + (t.e013 * a.e021 - t.e021 * a.e013)) * 2.0 + b.e032,
			(a.s * t.e013 + (t.e021 * a.e032 - t.e032 * a.e021)) * 2.0 + b.e013,
			(a.s * t.e021 + (t.e032 * a.e013 - t.e013 * a.e032)) * 2.0 + b.e021
		)
	}

	// direction sw_mx(motor a)
	static sw_mx(a) {
		checkType("a", a, Motor)

		return Direction.new(
			0.5 - a.s * a.s - a.e032 * a.e032, 
			a.e032 * a.e013 - a.s * a.e021, 
			a.s * a.e013 + a.e032 * a.e021
		)
	}

	// direction sw_my(motor a)
	static sw_my(a) {
		checkType("a", a, Motor)

		return Direction.new(
			a.s * a.e0123 + a.e013 * a.e032,
			0.5 - a.e013 * a.e013 - a.e0123 * a.e0123,
			a.e0123 * a.e032 - a.s * a.e013
		)
	}

	// direction sw_mz(motor a)
	static sw_mz(a) {
		checkType("a", a, Motor)

		return Direction.new(
			a.e013 * a.e0123 - a.e032 * a.s,
			a.e032 * a.e0123 + a.e013 * a.s,
			0.5 - a.e032 * a.e032 - a.e013 * a.e013
		)
	}

	// point sw_mo(motor a)
	static sw_mo(a) {
		checkType("a", a, Motor)

		return Point.new(
			2.0 * ((a.e013 * a.e02 - a.e021 * a.e01) - a.s * a.e01 - a.e0123 * a.e013),
			2.0 * ((a.e021 * a.e03 - a.e032 * a.e01) - a.s * a.e02 - a.e0123 * a.e032),
			2.0 * ((a.e032 * a.e01 - a.e013 * a.e03) - a.s * a.e03 - a.e0123 * a.e021)
		)
	}

	// motor reverse_m(motor R)
	static reverse_m(R) {
		checkType("R", R, Motor)

		return Motor.new(R.s, -R.e23, -R.e31, -R.e12, -R.e01, -R.e02, -R.e03, R.e0123)
	}

	// motor exp_r(float angle, line B)
	static exp_r(angle, r) {
		checkType("angle", angle, Num)
		checkType("r", r, Line)

		var c = angle.cos
		var s = angle.sin
		return Motor.new(c, s * r.e23, s * r.e31, s * r.e12, 0, 0, 0, 0)
	}

	// motor exp_t(float dist, line B)
	static exp_t(dist, B) {
		checkType("dist", dist, Num)
		checkType("B", B, Line)

		return Motor.new(1.0, 0, 0, 0, dist * B.e01, dist * B.e02, dist * B.e03, 0)
	}

	// motor exp_b(line B)
	static exp_b(B) {
		checkType("B", B, Line)
		
		var l = B.e23 * B.e23 + B.e31 * B.e31 + B.e12 * B.e12
		if (l == 0.0) { 
			return Motor.new(1.0, 0, 0, 0, B.e01, B.e02, B.e03, 0) 
		}

		var a = l.sqrt
		var m = B.e23 * B.e01 + B.e31 * B.e02 + B.e12 * B.e03
		var c = a.cos
		var s = a.sin / a
		var t = (m / l) * (c - s)

		return Motor.new(c, s * B.e23, s * B.e31, s * B.e12, 
			s * B.e01 + t * B.e31, s * B.e02 + t * B.e12, s * B.e03 + t * B.e23, m * s)
	}

	// line log_m(motor M)
	static log_m(M) {
		checkType("M", M, Motor)

		if (M.s == 1.0) { 
			return Line.new(0.0, 0.0, 0.0, M.e01, M.e02, M.e03) 
		}

		var a = 1.0 / (1.0 - M.s * M.s)
		var b = M.s.acos * a.sqrt
		var c = a * M.e0123 * (1.0 - M.s * b)

		return Line.new(b * M.e23, b * M.e31, b * M.e12, 
			b * M.e01 + c * M.e31, b * M.e02 + c * M.e12, b * M.e03 + c * M.e23)
	}

	// motor gp_rt(motor a, motor b)
	static gp_rt(a, b) {
		checkType("a", a, Motor)
		checkType("b", b, Motor)

		return Motor.new(
			a.s, a.e23, a.e31, a.e12,
			a.s * b.e01 + (b.e02 * a.e13 - b.e03 * a.e12),
			a.s * b.e02 + (b.e03 * a.e23 - b.e01 * a.e13),
			a.s * b.e03 + (b.e01 * a.e12 - b.e02 * a.e23),
			b.e01 * a.e23 + b.e02 * a.e31 + b.e03 * a.e12
		)
	}

	// motor gp_tr(motor a, motor b)
	static gp_tr(a, b) {
		checkType("a", a, Motor)
		checkType("b", b, Motor)

		return Motor.new(
			b.s, b.e23, b.e31, b.e12,
			b.s * a.e01 - (a.e02 * b.e13 - a.e03 * b.e12),
			b.s * a.e02 - (a.e03 * b.e23 - a.e01 * b.e13),
			b.s * a.e03 - (a.e01 * b.e12 - a.e02 * b.e23),
			a.e01 * b.e23 + a.e02 * b.e31 + a.e03 * b.e12
		)
	}

	// motor gp_rm(motor a, motor b)
	static gp_rm(a, b) {
		checkType("a", a, Motor)
		checkType("b", b, Motor)

		return Motor.new(
			a.s * b.s - (a.e23 * b.e23 + a.e31 * b.e31 + a.e12 * b.e12),
			a.s * b.e23 + b.s * a.e23 + (b.e31 * a.e12 - b.e12 * a.e31),
			a.s * b.e31 + b.s * a.e31 + (b.e12 * a.e23 - b.e23 * a.e12),
			a.s * b.e12 + b.s * a.e12 + (b.e23 * a.e31 - b.e31 * a.e23),
			a.s * b.e01 + b.s * a.e01 + (b.e02 * a.e31 - b.e03 * a.e12) + (b.e01 * a.e23 - a.e01 * b.e23) - (b.e0123 * a.e23 + a.e0123 * b.e23),
			a.s * b.e02 + b.s * a.e02 + (b.e03 * a.e23 - b.e01 * a.e31) + (b.e02 * a.e31 - a.e02 * b.e31) - (b.e0123 * a.e31 + a.e0123 * b.e31),
			a.s * b.e03 + b.s * a.e03 + (b.e01 * a.e12 - b.e02 * a.e23) + (b.e03 * a.e12 - a.e03 * b.e12) - (b.e0123 * a.e12 + a.e0123 * b.e12),
			a.s * b.e0123 + b.s * a.e0123 + (a.e23 * b.e03 + a.e31 * b.e02 + a.e12 * b.e01) + (a.e01 * b.e31 + a.e02 * b.e12 + a.e03 * b.e23)
		)
	}

	// motor gp_mr(motor a, motor b)
	static gp_mr(a, b) {
		checkType("a", a, Motor)
		checkType("b", b, Motor)

		return Motor.new(
			b.s * a.s - (b.e23 * a.e23 + b.e31 * a.e31 + b.e12 * a.e12),
			b.s * a.e23 + a.s * b.e23 + (a.e31 * b.e12 - a.e12 * b.e31),
			b.s * a.e31 + a.s * b.e31 + (a.e12 * b.e23 - a.e23 * b.e12),
			b.s * a.e12 + a.s * b.e12 + (a.e23 * b.e31 - a.e31 * b.e23),
			b.s * a.e01 + a.s * b.e01 + (a.e02 * b.e31 - a.e03 * b.e12) - (a.e01 * b.e23 - b.e01 * a.e23) - (a.e0123 * b.e23 + b.e0123 * a.e23),
			b.s * a.e02 + a.s * b.e02 + (a.e03 * b.e23 - a.e01 * b.e31) - (a.e02 * b.e31 - b.e02 * a.e31) - (a.e0123 * b.e31 + b.e0123 * a.e31),
			b.s * a.e03 + a.s * b.e03 + (a.e01 * b.e12 - a.e02 * b.e23) - (a.e03 * b.e12 - b.e03 * a.e12) - (a.e0123 * b.e12 + b.e0123 * a.e12),
			b.s * a.e0123 + a.s * b.e0123 + (b.e23 * a.e03 + b.e31 * a.e02 + b.e12 * a.e01) + (b.e01 * a.e31 + b.e02 * a.e12 + b.e03 * a.e23)
		)
	}

	// motor gp_tm(motor a, motor b)
	static gp_tm(a, b) {
		checkType("a", a, Motor)
		checkType("b", b, Motor)

		return Motor.new(
			b.s, b.e23, b.e31, b.e12,
			b.e01 + b.s * a.e01 - (a.e02 * b.e31 - a.e03 * b.e12),
			b.e02 + b.s * a.e02 - (a.e03 * b.e23 - a.e01 * b.e31),
			b.e03 + b.s * a.e03 - (a.e01 * b.e12 - a.e02 * b.e23),
			a.e01 * b.e23 + a.e02 * b.e31 + a.e03 * b.e12 + b.e0123
		)
	}

	// motor gp_mt(motor a, motor b)
	static gp_mt(a, b) {
		checkType("a", a, Motor)
		checkType("b", b, Motor)

		return Motor.new(
			a.s, a.e23, a.e31, a.e12,
			a.e01 + a.s * b.e01 + (b.e02 * a.e31 - b.e03 * a.e12),
			a.e02 + a.s * b.e02 + (b.e03 * a.e23 - b.e01 * a.e31),
			a.e03 + a.s * b.e03 + (b.e01 * a.e12 - b.e02 * a.e23),
			b.e01 * a.e23 + b.e02 * a.e31 + b.e03 * a.e12 + a.e0123
		)
	}

	// motor gp_tt(motor a, motor b)
	static gp_tt(a, b) {
		checkType("a", a, Motor)
		checkType("b", b, Motor)

		return Motor.new(
			1.0, 0, 0, 0,
			a.e01 + b.e01,
			a.e02 + b.e02,
			a.e03 + b.e03,
			a.e0123 + b.e0123
		)
	}

	// motor gp_rr(motor a, motor b)
	static gp_rr(a, b) {
		checkType("a", a, Motor)
		checkType("b", b, Motor)

		return Motor.new(
			a.s * b.s - (a.e23 * b.e23 + a.e31 * b.e31 + a.e12 * b.e12), 
			a.s * b.e23 + b.s * a.e23 + (a.e31 * b.e12 - a.e12 * b.e31), 
			a.s * b.e31 + b.s * a.e31 + (a.e12 * b.e23 - a.e23 * b.e12), 
			a.s * b.e12 + b.s * a.e12 + (a.e23 * b.e31 - a.e31 * b.e23),
			0, 0, 0, 0
		)
	}

	// motor gp_mm(motor a, motor b)
	static gp_mm(a, b) {
		checkType("a", a, Motor)
		checkType("b", b, Motor)

		return Motor.new(
			a.s * b.s - (a.e23 * b.e23 + a.e31 * b.e31 + a.e12 * b.e12), 
			a.s * b.e23 + b.s * a.e23 + (a.e31 * b.e12 - a.e12 * b.e31), 
			a.s * b.e31 + b.s * a.e31 + (a.e12 * b.e23 - a.e23 * b.e12), 
			a.s * b.e12 + b.s * a.e12 + (a.e23 * b.e31 - a.e31 * b.e23),
			a.s * b.e01 + b.s * a.e01 + (a.e31 * b.e02 - a.e12 * b.e03) + (a.e01 * b.e23 - b.e01 * a.e23) - (b.e0123 * a.e23 + a.e0123 * b.e23), 
			a.s * b.e02 + b.s * a.e02 + (a.e12 * b.e01 - a.e23 * b.e03) + (a.e02 * b.e31 - b.e02 * a.e31) - (b.e0123 * a.e31 + a.e0123 * b.e31), 
			a.s * b.e03 + b.s * a.e03 + (a.e23 * b.e02 - a.e31 * b.e01) + (a.e03 * b.e12 - b.e03 * a.e12) - (b.e0123 * a.e12 + a.e0123 * b.e12), 
			a.s * b.e0123 + b.s * a.e0123 + (a.e23 * b.e03 + a.e31 * b.e02 + a.e12 * b.e01) + (a.e01 * b.e31 + a.e02 * b.e12 + a.e03 * b.e23)
		)
	}

	// motor normalize_m(motor a)
	static normalize_m(a) {
		checkType("a", a, Motor)

		var s = 1.0 / (a.s * a.s + a.e23 * a.e23 + a.e31 * a.e31 + a.e12 * a.e12).sqrt
		var d = (a.e0123 * a.s - (a.e01 * a.e23 + a.e02 * a.e31 + a.e03 * a.e12)) * s * s

		return Motor.new(
			a.s * s,
			a.e23 * s,
			a.e31 * s,
			a.e12 * s,
			a.e01 * s + a.e23 * (s * d),
			a.e02 * s + a.e31 * (s * d),
			a.e03 * s + a.e12 * (s * d),
			a.e0123 * s - a.s * (s * d)
		)
	}

	// motor gp_vv(vec3 a, vec3 b)
	/*static gp_vv(a, b) {
		return Motor.new(
			a.x * b.x + a.y * b.y + a.z * b.z,
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x,
			0, 0, 0, 0
		)
	}*/

	// motor sqrt_m(motor R)
	static sqrt_m(r) {
		checkType("r", r, Motor)

		return normalize_m(Motor.new(r.s + 1.0, r.e23, r.e31, r.e12, r.e01, r.e02, r.e03, r.e0123))
	}
}