import "app" for App
import "utils" for Utils, P2

class C2 {
	construct new(r, i) {
        _r = r
        _i = i
    }

    r { _r }
    i { _i }
    r=(v) { _r = v }
    i=(v) { _i = v }

	~ { C2.new(r, -i) }

	*(z) { C2.new(r*z.r - i*z.i, r*z.i + i*z.r) }

	static exp(z, a) { C2.new(z * Utils.radians(a).cos, z * Utils.radians(a).sin) }
	
	draw(c) {
		App.begin(true, true, 1, 5)
		App.vertex(0, 0, 0, c)
		App.vertex(r, i, 0, c)
		App.end(App.lines)
		App.begin(true, true, 10, 1)
		App.vertex(r, i, 0, c)
		App.end(App.points)
	}

	debug(s) {
		r = App.debugFloat(s + "r", r)
		i = App.debugFloat(s + "i", i)
	}
}

class Ex {
	construct new() {
        _z1 = C2.new(1, 0)
        _z2 = C2.new(0, 1)
    }

	z1 { _z1 }
	z2 { _z2 }
	z1=(v) { _z1 = v }
    z2=(v) { _z2 = v }
	
	update(t, dt) {
		var z3 = z1 * z2
		
		z3.debug("z3: ")
		z3.draw(0xFF00FFFF)
	}
}

class Game {
	static init() {
		__time = 0
		__zoom = 5
		__animate = true
		__mode = 0
		__size = 0.1

		__ex = Ex.new()
		__p1 = P2.new(__ex.z1.r, __ex.z1.i)
		__p2 = P2.new(__ex.z2.r, __ex.z2.i)
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
		App.debugSeparator("")

		if (__mode == 0) {
			__p1.x = __ex.z1.r
			__p1.y = __ex.z1.i
			__p2.x = __ex.z2.r
			__p2.y = __ex.z2.i

			if ((__sel == null || __sel == __p1) && __p1.update(__size)) {
				__ex.z1.r = __p1.x
				__ex.z1.i = __p1.y
				__sel = __p1

			} else if ((__sel == null || __sel == __p2) && __p2.update(__size)) {
				__ex.z2.r = __p2.x
				__ex.z2.i = __p2.y
				__sel = __p2

			} else {
				__sel = null
			}

		} else if (__mode == 1 && __animate) {
			__ex.z1 = C2.exp(1, Utils.degrees(__time))

		} else if (__mode == 2 && __animate) {
			__ex.z2 = C2.exp(1, Utils.degrees(__time))
		}

		App.begin(true, true, 10, 1)
		App.vertex(Utils.mouseX, Utils.mouseY, 0, 0xFFFFFFFF)
		App.end(App.points)
		
		__ex.z1.debug("z1: ")
		App.debugSeparator("")
		__ex.z2.debug("z2: ")
		App.debugSeparator("")
		__ex.update(__time, dt)
		__ex.z2.draw(0xFF00FF00)
		__ex.z1.draw(0xFF0000FF)
	}
}