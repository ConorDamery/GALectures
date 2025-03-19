import "app" for App
import "complex" for Complex

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
		_shader = App.glCreateShader("Assets/Complex/Shaders/vertex.glsl")
        _scale = 2

        _a = Complex.new(1, 1)
        _b = Complex.new(-1, 1)
    }

	update(dt) {
	}

	render() {
		if (App.guiBeginChild("Settings", 500, -1)) {
			_scale = App.guiFloat("Scale", _scale).max(1).min(10)
            App.guiSeparator("Data")
            _a.r = App.guiFloat("A Real", _a.r)
            _a.i = App.guiFloat("A Img", _a.i)
            _b.r = App.guiFloat("B Real", _b.r)
            _b.i = App.guiFloat("B Img", _b.i)
		}
		App.guiEndChild()

		App.glClear(0.1, 0.1, 0.1, 1, 0, 0, 0)
		App.glSetShader(_shader)

		App.glUniform("Proj")
		App.glVec2f(App.winWidth / App.winHeight, _scale)

        Util.glDrawGrid(20, 20, 0.5, 20, App.glGray)

		_a.glDraw(0xFF00FFFF)
		_b.glDraw(0xFFFFFF00)

        var c = (_a * _b).exp
		c.glDraw(0xFFFFFFFF)
	}
}

class Main {
	static init() { __state = State.new() }
	static update(dt) { __state.update(dt) }
	static render() { __state.render() }
}