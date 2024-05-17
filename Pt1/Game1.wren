import "app" for App
import "utils" for Utils

class C2 {
	construct new(r, i) {
        _r = r
        _i = i
    }

    r { _r }
    i { _i }
    r=(v) { _r = v }
    i=(v) { _i = v }

	*(z) { C2.new(r*z.r - i*z.i, r*z.i + i*z.r) }
	
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

class Game {
	static init() {
		__time = 0
		__zoom = 5
		__animate = true
	}

	static update(dt) {
		Utils.setup(__zoom)
		__time = __time + dt
		
		__animate = App.debugBool("animate", __animate)
		
		var z1 = C2.new(1, 0)
		if (__animate) {
			z1.r = __time.cos
			z1.i = __time.sin
		} else {
			z1.r = Utils.mouseX * 2.5
			z1.i = Utils.mouseY * 2.5
		}
		
		var z2 = C2.new(0, 1)
		var z3 = z2 * z1

		z1.debug("z1")
		z2.debug("z2")
		z3.debug("z3")

		z1.draw(0xFF0000FF)
		z2.draw(0xFF00FF00)
		z3.draw(0xFF00FFFF)
	}
}