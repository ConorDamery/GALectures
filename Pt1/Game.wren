import "app" for App
import "mvec" for MVec

class Game {
	static init() {
	}

	static update(dt) {
		App.drawLine2(0, 0, 1, 1, 0xFFFFFFFF)
		App.drawQuad2(0, 0, 1, 1, 0xFFFFFFFF)
		App.drawLine3(0, 0, 0, 1, 1, 1, 0xFFFFFFFF)
		App.drawQuad3(0, 0, 0, 1, 1, 1, 0xFFFFFFFF)

		var aspect = App.getWidth() / App.getHeight()
		var f = 1 / (70 * 0.5).tan()
		var zn = 0.1
		var zf = 100.0
		var rinv = 1.0f / (zn - zf)
		App.setCamera(f / aspect, 0, 0, 0, 0, f, 0, 0, 0, 0, (zf + zn) * rinv, (2 * zf * zn) * rinv, 0, 0, -1, 0)
	}
}