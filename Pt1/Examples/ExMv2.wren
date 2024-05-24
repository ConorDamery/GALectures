import "app" for App
import "utils" for Utils, P2

class Mv2 {
	construct new(e0, e1, e2, e12) {
        _e0 = e0
        _e1 = e1
		_e2 = e2
		_e12 = e12
    }

    e0 { _e0 }
    e1 { _e1 }
	e2 { _e2 }
	e12 { _e12 }
    e0=(v) { _e0 = v }
    e1=(v) { _e1 = v }
	e2=(v) { _e2 = v }
	e12=(v) { _e12 = v }
	
	~ { Mv2.new(e0, e1, e2, -e12) }

	*(z) {
		var re0 = e0 * z.e0 + e1 * z.e1 + e2 * z.e2 - e12 * z.e12
		var re1 = e0 * z.e1 + e1 * z.e0 - e2 * z.e12 + e12 * z.e2
		var re2 = e0 * z.e2 + e1 * z.e12 + e2 * z.e0 - e12 * z.e1
		var re12 = e0 * z.e12 + e1 * z.e2 - e2 * z.e1 + e12 * z.e0
		return Mv2.new(re0, re1, re2, re12)
	}

	static exp(z, a) { Mv2.new(z * Utils.radians(a).cos, 0, 0, z * Utils.radians(a).sin) }
	
	draw(v, c) {
		if (v) {
			App.begin(true, true, 1, 5)
			App.vertex(0, 0, 0, c)
			App.vertex(e0, e12, 0, c)
			App.vertex(0, 0, 0, c)
			App.vertex(e1, e2, 0, c)
			App.end(App.lines)
			App.begin(true, true, 10, 1)
			App.vertex(e0, e12, 0, c)
			App.vertex(e1, e2, 0, c)
			App.end(App.points)
		} else {
			var a = e12.sign * e12.abs.sqrt
			var c2 = 0x5F000000 + (c & 0x00FFFFFF)
			App.begin(true, true, 1, 1)
			App.vertex(0, 0, -1, c2)
			App.vertex(a, 0, -1, c2)
			App.vertex(0, a, -1, c2)
			App.vertex(a, a, -1, c2)
			App.vertex(0, a, -1, c2)
			App.vertex(a, 0, -1, c2)
			App.end(App.triangles)
			App.begin(true, true, 1, 5)
			App.vertex(0, 0, 0, c)
			App.vertex(e1, e2, 0, c)
			App.end(App.lines)
			App.begin(true, true, 10, 1)
			App.vertex(e1, e2, 1, c)
			App.vertex(e0, 0, 1, c)
			App.end(App.points)	
		}
	}

	debug(s) {
		e0 = App.debugFloat(s + "e0", e0)
		e1 = App.debugFloat(s + "e1", e1)
		e2 = App.debugFloat(s + "e2", e2)
		e12 = App.debugFloat(s + "e12", e12)
	}
}

class Ex {
	construct new() {
        _z1 = Mv2.new(1, 0, 0, 0)
        _z2 = Mv2.new(0, 0, 0, 1)
    }

	z1 { _z1 }
	z2 { _z2 }
	z1=(v) { _z1 = v }
	z2=(v) { _z2 = v }
	
	update(v, dt) {
		
		var z3 = z1 * z2
		
		z3.debug("z3: ")
		z3.draw(v, 0xFF00FFFF)
	}
}

class Game {
	static init() {
		__time = 0
		__zoom = 5
		__animate = true
		__mode = 0
		__size = 0.1
		__vectors = true

		__ex = Ex.new()
		__p11 = P2.new(0, 0)
		__p12 = P2.new(0, 0)
		__p21 = P2.new(0, 0)
		__p22 = P2.new(0, 0)
		__sel = null
	}

	static update(dt) {
		Utils.setup(__zoom)

		App.debugSeparator("Settings")
		__zoom = App.debugFloat("zoom", __zoom)
		__time = __time + dt
		
		__animate = App.debugBool("animate", __animate)
		__mode = App.debugInt("mode", __mode, 0, 2)
		__size = App.debugFloat("size", __size)
		__vectors = App.debugBool("vectors", __vectors)
		
		if (App.debugButton("Reset e0 e12")) {
			__ex.z1.e0 = 1
			__ex.z1.e1 = 0
			__ex.z1.e2 = 0
			__ex.z1.e12 = 0
			__ex.z2.e0 = 0
			__ex.z2.e1 = 0
			__ex.z2.e2 = 0
			__ex.z2.e12 = 1
		}
		if (App.debugButton("Reset e1 e2")) {
			__ex.z1.e0 = 0
			__ex.z1.e1 = 1
			__ex.z1.e2 = 0
			__ex.z1.e12 = 0
			__ex.z2.e0 = 0
			__ex.z2.e1 = 0
			__ex.z2.e2 = 1
			__ex.z2.e12 = 0
		}
		App.debugSeparator("")

		if (__mode == 0) {
				__p11.x = __ex.z1.e0
				__p11.y = __ex.z1.e12
				__p12.x = __ex.z1.e1
				__p12.y = __ex.z1.e2
				__p21.x = __ex.z2.e0
				__p21.y = __ex.z2.e12
				__p22.x = __ex.z2.e1
				__p22.y = __ex.z2.e2

				if ((__sel == null || __sel == __p11) && __p11.update(__size)) {
					__ex.z1.e0 = __p11.x
					__ex.z1.e12 = __p11.y
					__sel = __p11

				} else if ((__sel == null || __sel == __p12) && __p12.update(__size)) {
					__ex.z1.e1 = __p12.x
					__ex.z1.e2 = __p12.y
					__sel = __p12

				} else if ((__sel == null || __sel == __p21) && __p21.update(__size)) {
					__ex.z2.e0 = __p21.x
					__ex.z2.e12 = __p21.y
					__sel = __p21

				} else if ((__sel == null || __sel == __p22) && __p22.update(__size)) {
					__ex.z2.e1 = __p22.x
					__ex.z2.e2 = __p22.y
					__sel = __p22

				} else {
					__sel = null
				}

		} else if (__mode == 1 && __animate) {
			__ex.z1 = Mv2.exp(1, Utils.degrees(__time))

		} else if (__mode == 2 && __animate) {
			__ex.z2 = Mv2.exp(1, Utils.degrees(__time))
		}

		App.begin(true, true, 10, 1)
		App.vertex(Utils.mouseX, Utils.mouseY, 0, 0xFFFFFFFF)
		App.end(App.points)
		
		__ex.z1.debug("z1: ")
		App.debugSeparator("")
		__ex.z2.debug("z2: ")
		App.debugSeparator("")
		__ex.update(__vectors, dt)
		__ex.z2.draw(__vectors, 0xFF00FF00)
		__ex.z1.draw(__vectors, 0xFF0000FF)
	}
}