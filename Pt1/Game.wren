import "app" for App
import "utils" for Utils
import "mvec" for MVec

class Game {
	static init() {
	}

	static update(dt) {
		Utils.drawGrid(100, 100, -0.5, 100, 0x5FFFFFFF)

		var mouseX = (App.mouseX / App.width) - 0.5
		var mouseY = 0.5 - (App.mouseY / App.height)
		App.drawPoint3(mouseX, mouseY, -0.5, 0xFFFFFFFF)
		
		App.drawLine3(-0.5, -0.5, -5, 0.5, 0.5, -5, 0xFF0000FF)
		App.drawQuad3(-0.5, -0.5, -5, 0.5, 0.5, -5, 0x5F0000FF)
		
		Utils.setPerspective(1.22173, App.width / App.height, 0.1, 1000.0)
	}
}