import "app" for App
import "gnum" for Complex

class Util {
    static glDrawGrid(width, height, depth, resolution, color) {
		App.glBegin(true, true, 1, 1)
		var ystep = height / resolution
    	for (y in 0..resolution) {
			var yPos = y * ystep
            var c = (yPos - height / 2) == 0 ? App.glRed : color
			App.glAddVertex(-width / 2, yPos - height / 2, depth, c)
			App.glAddVertex(width / 2, yPos - height / 2, depth, c)
		}

		var xstep = width / resolution
		for (x in 0..resolution) {
			var xPos = x * xstep
            var c = (xPos - width / 2) == 0 ? App.glGreen : color
			App.glAddVertex(xPos - width / 2, -height / 2, depth, c)
			App.glAddVertex(xPos - width / 2, height / 2, depth, c)
		}
		App.glEnd(App.glLines)
	}
}

class State {
	construct new() {
		_shader = App.glLoadShader("Assets/PGA2/vertex2.glsl")
        _camScale = 2
		_camX = 0
		_camY = 0

        _c = Complex.new(0.5, Num.pi/4)
		_mode = 0
    }

	update(dt) {
	}

	render() {
		if (App.guiBeginChild("Settings", 500, -1)) {
			_camScale = App.guiFloat("Cam Scale", _camScale).max(1).min(10)
            App.guiSeparator("Data")
            _c.r = App.guiFloat("c.s", _c.r)
            _c.i = App.guiFloat("c.e12", _c.i)
			
			App.guiSeparator("Formula")
			_mode = App.guiInt("Mode", _mode, 0, 3)

			if (_mode == 0) {
				App.guiText("= c * p")
			} else if (_mode == 1) {
				App.guiText("= c.exp * p")
			} else if (_mode == 2) {
				App.guiText("= c * p * ~c")
			} else if (_mode == 3) {
				App.guiText("= c.exp * p * ~c.exp")
			}
		}
		App.guiEndChild()

		App.glClear(0.1, 0.1, 0.1, 1, 0, 0, 0)
		App.glSetShader(_shader)

		App.glSetUniform("Proj")
		App.glSetVec4f(_camX, _camY, App.winWidth / App.winHeight, _camScale)

        Util.glDrawGrid(20, 20, 0.5, 20, App.glGray)

		_c.glDraw(0xFF00FFFF)

		//System.print((_c*~_c).toString)

		/*if (_mode == 0) {
			var c = _c * _p
			c.glDraw(0xFFFFFFFF)

			App.glBegin(true, true, 1, 2)
			for (i in -100..100) {
				var z = Complex.new(_c.r, i * 0.1)
				var x = z * _p
				App.glAddVertex(x.x, x.y, 0, 0xA0FFFFFF)
			}
			App.glEnd(App.glLines)
			
		} else if (_mode == 1) {
			var c = _c.exp * _p
			c.glDraw(0xFFFFFFFF)

			App.glBegin(true, true, 1, 2)
			for (i in -180..180) {
				var z = Complex.new(_c.r, i * Num.pi / 180.0)
				var x = z.exp * _p
				App.glAddVertex(x.x, x.y, 0, 0xA0FFFFFF)
			}
			App.glEnd(App.glLines)
			
		} else if (_mode == 2) {
			var c = _c * _p * ~_c
			c.glDraw(0xFFFFFFFF)

			App.glBegin(true, true, 1, 2)
			for (i in -100..100) {
				var z = Complex.new(_c.r, i * 0.1)
				var x = z * _p * ~z
				App.glAddVertex(x.x, x.y, 0, 0xA0FFFFFF)
			}
			App.glEnd(App.glLines)	
		} else if (_mode == 3) {
			var c = _c.exp * _p * ~_c.exp
			c.glDraw(0xFFFFFFFF)

			App.glBegin(true, true, 1, 2)
			for (i in -180..180) {
				var z = Complex.new(_c.r, i * Num.pi / 180.0)
				var x = z.exp * _p * ~z.exp
				App.glAddVertex(x.x, x.y, 0, 0xA0FFFFFF)
			}
			App.glEnd(App.glLines)	
		}*/
	}
}

class Main {
	static init() { __state = State.new() }
	static update(dt) { __state.update(dt) }
	static render() { __state.render() }
}