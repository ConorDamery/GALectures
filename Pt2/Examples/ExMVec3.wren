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
		
		// GA product
		if (__mode == 0) {

			var z1 = MVec2.new(0, 1, 0, 0)
			var z2 = MVec2.new(0, __time.cos, __time.sin, 0)
			var z3 = z1 * z2
			
			z1.debug("z1")
			z2.debug("z2")
			z3.debug("z3")
			
			z1.draw(0xFF0000FF)
			z2.draw(0xFF00FF00)
			z3.draw(0xFFFF0000)
		
		// Rotations Cos & Sin
		} else if (__mode == 1) {

			var z1 = MVec2.new(0, 1, 0, 0)
			var z2 = MVec2.new(__time.cos, 0, 0, __time.sin)
			var z3 = z1 * z2
			
			z1.debug("z1")
			z2.debug("z2")
			z3.debug("z3")
			
			z1.draw(0xFF0000FF)
			z2.draw(0xFF00FF00)
			z3.draw(0xFFFF0000)
		
		// Rotations Line & Exp
		} else if (__mode == 2) {

			var p1 = MVec2.new(0, 1, 0, 0)
			var p2 = MVec2.new(0, 0, 1, 0)
			var p3 = MVec2.new(0, 1, 1, 0)
			var l1 = p1 ^ p2 ^ p3//MVec2.inf
			l1.debug("l1")
			var a = MVec2.scalar(Num.pi * (__angle / 180))
			var r = MVec2.exp(a * l1.dual / MVec2.scalar(2))
			var l2 = r * l1 / r
			
			l2.debug("l2")
			l2.draw(0xFF00FFFF)
		
		// Reflections
		} else if (__mode == 3) {
			//z2 = MVec2.new(0, 0, 1, 0)
			//z3 = z2 * z1 * z2
		
		// Projections
		} else if (__mode == 4) {

			//z2 = MVec2.new(0, 0, 1, 0)
			//z3 = z1 | z2
		
		// Intersections
		} else if (__mode == 5) {

			//z2 = MVec2.new(0, 0, 1, 0)
			//z3 = (z1 ^ MVec2.new(0, 0, 0, 1)) | (z2 ^ MVec2.new(0, 0, 0, 1))
		}
	}
}