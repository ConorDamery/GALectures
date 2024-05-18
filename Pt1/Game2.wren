import "app" for App
import "utils" for Utils
import "mvec2" for MVec2

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
		
		var z1 = MVec2.new(0, 1, 0, 0)
		if (__animate) {
			z1.e1 = __time.cos
			z1.e2 = __time.sin
		} else {
			z1.e1 = Utils.mouseX * __zoom / 2
			z1.e2 = Utils.mouseY * __zoom / 2
		}

		var z2 = MVec2.new(0, 0, 0, 0)
		var z3 = MVec2.new(0, 0, 0, 0)
		
		// GA product
		if (__mode == 0) {
			z2 = MVec2.new(0, 0, 1, 0)
			z3 = z1 * z2
		
		// Rotations
		} else if (__mode == 1) {
			var a = Num.pi * (__angle / 180)
			z2 = MVec2.new((a/2).cos, 0, 0, (a/2).sin)
			z3 = -z2 * z1 * z2
		
		// Reflections
		} else if (__mode == 2) {
			z2 = MVec2.new(0, 0, 1, 0)
			z3 = z2 * z1 * z2
		
		// Projections
		} else if (__mode == 3) {
			z2 = MVec2.new(0, 0, 1, 0)
			z3 = z1 | z2
		
		// Intersections
		} else if (__mode == 4) {
			z2 = MVec2.new(0, 0, 1, 0)
			z3 = (z1 ^ MVec2.new(0, 0, 0, 1)) | (z2 ^ MVec2.new(0, 0, 0, 1))
		}

		z1.debug("z1")
		z2.debug("z2")
		z3.debug("z3")

		z2.draw(0xFF00FF00)
		z3.draw(0xFF00FFFF)
		z1.draw(0xFF0000FF)
	}
}