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
		__animate = true
		__z1 = C2.new(1, 0)
		__z2 = C2.new(0, 1)
	}

	static update(dt) {
		__time = __time + dt

		App.view(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, -5, 1)
		var aspect = App.width / App.height
		Utils.orthographic(5, aspect, 0.1, 1000.0)

		Utils.drawGrid(10, 10, -1, 10, 0x5FFFFFFF)
		
		__animate = App.debugBool("animate", __animate)
		__z1.debug("z1")
		__z2.debug("z2")
		
		if (__animate) {
			__z1.r = __time.cos
			__z1.i = __time.sin
		} else {
			var mouseX = (2 * App.mouseX / App.width) - 1
			var mouseY = 1 - (2 * App.mouseY / App.height)
			__z1.r = mouseX * 2.5 * aspect
			__z1.i = mouseY * 2.5
		}
		
		var z3 = __z2 * __z1

		__z1.draw(0xFF0000FF)
		__z2.draw(0xFF00FF00)
		z3.draw(0xFF00FFFF)
	}
}