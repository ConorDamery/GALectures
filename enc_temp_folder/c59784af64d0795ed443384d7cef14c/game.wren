import "app" for App
import "utils" for Utils

class Game {
	static init() {
		__time = 0
		__zoom = 5
	}

	static update(dt) {
		Utils.setup(__zoom)
		__time = __time + dt
	}
}