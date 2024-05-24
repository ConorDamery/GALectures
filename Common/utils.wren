import "app" for App

class Utils {
	static mouseX { 2.5 * ((App.width / App.height) * ((2 * App.mouseX / App.width) - 1)) }
	static mouseY { 2.5 * (1 - (2 * App.mouseY / App.height)) }

	static radians(x) { Num.pi * x / 180 }
	static degrees(x) { x / Num.pi * 180 }

	static setup(size) {
		App.view(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, -5, 1)
		var aspect = App.width / App.height
		Utils.orthographic(size, aspect, 0.1, 1000.0)

		Utils.drawGrid(10, 10, -1, 10, 0x5FFFFFFF)
	}

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

class P2 {
	construct new(x, y) {
        _x = x
        _y = y
		_s = false
    }

    x { _x }
    y { _y }
    x=(v) { _x = v }
    y=(v) { _y = v }

	update(size) {
		var hover = Utils.mouseX > (x - size) && Utils.mouseX < (x + size) && Utils.mouseY > (y - size) && Utils.mouseY < (y + size)
		if (App.getButton(0)) {
			if (hover) {
				_s = true
			}
		} else {
			_s = false
		}

		if (_s) {
			x = Utils.mouseX
			y = Utils.mouseY
		}

		if (!_s) {
			App.begin(true, true, 1, 1)
			App.vertex(x - size, y - size, 0, 0xFFFFFFFF)
			App.vertex(x + size, y - size, 0, 0xFFFFFFFF)
			App.vertex(x + size, y - size, 0, 0xFFFFFFFF)
			App.vertex(x + size, y + size, 0, 0xFFFFFFFF)
			App.vertex(x + size, y + size, 0, 0xFFFFFFFF)
			App.vertex(x - size, y + size, 0, 0xFFFFFFFF)
			App.vertex(x - size, y + size, 0, 0xFFFFFFFF)
			App.vertex(x - size, y - size, 0, 0xFFFFFFFF)
			App.end(App.lines)
		}

		return _s
	}
}