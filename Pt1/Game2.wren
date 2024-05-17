import "app" for App
import "utils" for Utils

class MV2 {
	construct new(r, i, j, ij) {
        _r = r
        _i = i
		_j = j
		_ij = ij
    }

	// Members
    r { _r }
    i { _i }
	j { _j }
    ij { _ij }
	r=(v) { _r = v }
    i=(v) { _i = v }
    j=(v) { _j = v }
    ij=(v) { _ij = v }
	
	// Geometric product
	*(z) {
		var tr = r*z.r + i*z.i + j*z.j - ij*z.ij
		var ti = r*z.i + i*z.r - j*z.ij + ij*z.j
		var tj = r*z.j + i*z.ij + j*z.r - ij*z.i
		var tij = r*z.ij + i*z.j - j*z.i + ij*z.r
		return MV2.new(tr, ti, tj, tij)
	}

	// Negate operator
	- { MV2.new(r, -i, -j, -ij) }

	// Meet operator
	|(z) {
		var t = this * z
		var id = 1 / (z * z).r
    	return MV2.new(t.r * id, t.i * id, t.j * id, t.ij * id)
	}
	
	// Utils
	draw(c) {
		var a = ij.sign * ij.abs.sqrt
		App.begin(true, false, 1, 1)
		App.vertex(0, 0, 0, c)
		App.vertex(a, 0, 0, c)
		App.vertex(0, a, 0, c)
		App.vertex(a, a, 0, c)
		App.vertex(0, a, 0, c)
		App.vertex(a, 0, 0, c)
		App.end(App.triangles)
		App.begin(true, false, 1, 5)
		App.vertex(0, 0, 0, c)
		App.vertex(i, j, 0, c)
		App.end(App.lines)
		App.begin(true, false, 10, 1)
		App.vertex(i, j, 0, c)
		App.vertex(r, 0, 0, c)
		App.end(App.points)
	}
	
	debug(s) {
		r = App.debugFloat(s + "r", r)
		i = App.debugFloat(s + "i", i)
		j = App.debugFloat(s + "j", j)
		ij = App.debugFloat(s + "ij", ij)
	}
}

class Game {
	static init() {
		__time = 0
		__zoom = 5
		__animate = true
		__mode = 0
		__angle = 45
	}

	static update(dt) {
		Utils.setup(__zoom)
		__time = __time + dt
		
		__zoom = App.debugFloat("zoom", __zoom)
		__mode = App.debugFloat("mode", __mode)
		__animate = App.debugBool("animate", __animate)
		__angle = App.debugFloat("angle", __angle)
		
		var z1 = MV2.new(0, 1, 0, 0)
		if (__animate) {
			z1.i = __time.cos
			z1.j = __time.sin
		} else {
			z1.i = Utils.mouseX * __zoom / 2
			z1.j = Utils.mouseY * __zoom / 2
		}

		var z2 = MV2.new(0, 0, 0, 0)
		var z3 = MV2.new(0, 0, 0, 0)
		
		if (__mode == 0) {
			var a = Num.pi * (__angle / 180)
			z2 = MV2.new((a/2).cos, 0, 0, (a/2).sin)
			z3 = -z2 * z1 * z2

		} else if (__mode == 1) {
			z2 = MV2.new(0, 0, 1, 0)
			z3 = z2 * z1 * z2

		} else if (__mode == 2) {
			z2 = MV2.new(0, 0, 1, 0)
			z3 = z1 | z2
		}

		z1.debug("z1")
		z2.debug("z2")
		z3.debug("z3")

		z2.draw(0xFF00FF00)
		z3.draw(0xFF00FFFF)
		z1.draw(0xFF0000FF)
	}
}