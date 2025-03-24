import "app" for App
import "pga2" for Point2, Motor2

class Util {
    static glDrawGrid(width, height, depth, resolution, color) {
		App.glBegin(true, true, 1, 1)
		var ystep = height / resolution
    	for (y in 0..resolution) {
			var yPos = y * ystep
            var c = (yPos - height / 2) == 0 ? App.glRed : color
			App.glVertex(-width / 2, yPos - height / 2, depth, c)
			App.glVertex(width / 2, yPos - height / 2, depth, c)
		}

		var xstep = width / resolution
		for (x in 0..resolution) {
			var xPos = x * xstep
            var c = (xPos - width / 2) == 0 ? App.glGreen : color
			App.glVertex(xPos - width / 2, -height / 2, depth, c)
			App.glVertex(xPos - width / 2, height / 2, depth, c)
		}
		App.glEnd(App.glLines)
	}
}

class State {
	construct new() {
		_shader = App.glCreateShader("Assets/Common/vertex2.glsl")

		_camScale = 2
		_camX = 0
		_camY = 0

		_angle = 0
		_x = 0
		_y = 0
    }

	update(dt) {
	}

	render() {
		if (App.guiBeginChild("Settings", 500, App.guiContentAvailHeight() - 150)) {
			_camScale = App.guiFloat("Cam Scale", _camScale, 1, 10)
			_angle = App.guiFloat("Angle", _angle, -180, 180)
			_x = App.guiFloat("X", _x)
			_y = App.guiFloat("Y", _y)
		}
		App.guiEndChild()

		App.glClear(0.1, 0.1, 0.1, 1, 0, 0, 0)
		App.glSetShader(_shader)

		App.glUniform("Proj")
		App.glVec4f(_camX, _camY, App.winWidth / App.winHeight, _camScale)

        Util.glDrawGrid(20, 20, 0.5, 20, App.glGray)

		var a = Num.pi * _angle / -360
		var r = Motor2.new(a.cos, a.sin, 0, 0)
		var t = Motor2.new(1, 0, -0.5 * _x, -0.5 * _y)
		var m = r * t
		App.guiText(" R: %(r)")
		App.guiText(" T: %(t)")
		App.guiText(" M: %(m)")
		App.guiText("~M: %(~m)")

		var p = Point2.new(0, 0)
		var pm = m * p * ~m
		var pp = Point2.new(pm.e01, pm.e02)
		pp.glDraw(0xFFFFFFFF)
		App.guiText(" P: %(pp)")

		System.print(" R: %(r)")
		System.print(" T: %(t)")
		System.print(" M: %(m)")
		System.print("~M: %(~m)")
		System.print(" MP: %(m * p)")
		System.print(" M~M: %(m * ~m)")
		System.print(" MP~M: %(m * p * ~m)")
		System.print(" M>>P: %(m >> pp)")

		App.glBegin(true, true, 1, 2)
		for (i in -180...180) {
			var ia = Num.pi * i / -360
			var ir = Motor2.new(ia.cos, ia.sin, 0, 0)
			var it = Motor2.new(1, 0, -0.5 * _x, -0.5 * _y)
			var im = ir * it

			//var ip = Motor2.new(0, 1, 0, 0)
			//var ipm = im * ip * ~im
			//var ipp = Point2.new(ipm.e01, ipm.e02)

			var ip = Point2.new(0, 0)
			var ipp = im >> ip
			App.glVertex(ipp.x, ipp.y, 0, 0xFFFFFFFF)
		}
		App.glEnd(App.glLines)
	}
}

class Main {
	static init() { __state = State.new() }
	static update(dt) { __state.update(dt) }
	static render() { __state.render() }
}