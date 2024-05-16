import "app" for App

class Utils {
	static setPerspective(fov, aspect, zn, zf) {
		var f = 1 / (fov * 0.5).tan
		var rinv = 1 / (zn - zf)
		App.setCamera(f / aspect, 0, 0, 0, 0, f, 0, 0, 0, 0, (zf + zn) * rinv, -1, 0, 0, 2 * zf * zn * rinv, 0)
	}

	static drawGrid(width, height, depth, resolution, color) {
		var ystep = height / resolution
    	for (y in 0..resolution) {
			var yPos = y * ystep
			App.drawLine3(-width / 2, depth, yPos - height / 2, width / 2, depth, yPos - height / 2, color)
		}

		var xstep = width / resolution
		for (x in 0..resolution) {
			var xPos = x * xstep
			App.drawLine3(xPos - width / 2, depth, -height / 2, xPos - width / 2, depth, height / 2, color)
		}
	}
}