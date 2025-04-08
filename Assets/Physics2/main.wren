import "app" for App
import "pga2" for Point, Complex

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
		_shader = App.glLoadShader("Assets/Common/vertex2.glsl")

		_camScale = 2
		_camX = 0
		_camY = 0

		_angle = 0
    }

	update(dt) {
	}

	render() {
		if (App.guiBeginChild("Settings", 500, -1)) {
			_camScale = App.guiFloat("Cam Scale", _camScale, 1, 10)
			_angle = App.guiFloat("Angle", _angle, -180, 180)
		}
		App.guiEndChild()

		App.glClear(0.1, 0.1, 0.1, 1, 0, 0, 0)
		App.glSetShader(_shader)

		App.glUniform("Proj")
		App.glVec4f(_camX, _camY, App.winWidth / App.winHeight, _camScale)

        Util.glDrawGrid(20, 20, 0.5, 20, App.glGray)
	}
}

class Main {
	static init() { __state = State.new() }
	static update(dt) { __state.update(dt) }
	static render() { __state.render() }
}