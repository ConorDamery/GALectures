import "app" for App
import "utils" for Utils
import "mvec2" for MVec2

class Game {
	static init() {
		__time = 0
		__zoom = 5
	}

	static update(dt) {
		Utils.setup(__zoom)
		__time = __time + dt

		var z1 = MVec2.new(0, 1, 0, 0)
		var z2 = MVec2.new(0, 0, 1, 0)
		var z3 = z1 * z2

		z1.debug("z1")
		z2.debug("z2")
		z3.debug("z3")

		z3.draw(0xFF00FFFF)
		z2.draw(0xFF00FF00)
		z1.draw(0xFF0000FF)
	}
}