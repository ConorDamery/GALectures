import "app" for App

class Utils {
	static orthographic(size, aspect, zn, zf) {
		var w = size * aspect * 0.5
		var h = size * 0.5
		App.projection(1/w, 0, 0, 0, 0, 1/h, 0, 0, 0, 0, -2 / (zf - zn), 0, 0, 0, -(zf + zn)/(zf - zn), 1)
	}

	static perspective(fov, aspect, zn, zf) {
		var f = 1 / (fov * 0.5).tan
		var rinv = 1 / (zn - zf)
		App.projection(f / aspect, 0, 0, 0, 0, f, 0, 0, 0, 0, (zf + zn) * rinv, -1, 0, 0, 2 * zf * zn * rinv, 0)
	}

	static drawGrid(width, height, depth, resolution, color) {
		App.begin(true, true, 1, 1)
		var ystep = height / resolution
    	for (y in 0..resolution) {
			var yPos = y * ystep
			App.vertex(-width / 2, yPos - height / 2, depth, color)
			App.vertex(width / 2, yPos - height / 2, depth, color)
		}

		var xstep = width / resolution
		for (x in 0..resolution) {
			var xPos = x * xstep
			App.vertex(xPos - width / 2, -height / 2, depth, color)
			App.vertex(xPos - width / 2, height / 2, depth, color)
		}
		App.end(App.lines)
	}
}