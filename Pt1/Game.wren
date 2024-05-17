import "app" for App
import "utils" for Utils

class Game {
	static init() {
		__time = 0
	}

	static update(dt) {
		__time = __time + dt

		App.view(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, -5, 1)
		var aspect = App.width / App.height
		Utils.orthographic(5, aspect, 0.1, 1000.0)

		Utils.drawGrid(10, 10, -1, 10, 0x5FFFFFFF)
	}
}