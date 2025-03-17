import "app" for App
import "pga" for Direction, Point, Line, Motor, PGA

class Util {
	static glPerspective(name, fov, aspect, near, far) {
		var t = ((fov * Num.pi / 180.0) * 0.5).tan * near
		var b = -t
		var r = t * aspect
		var l = -r
		App.glUniform(name)
		App.glMat3x2f(r, l, t, b, near, far)
	}
}

class State {
	construct new() { init() }

	init() {
		_shader = App.glCreateShader("/Pt2/Shaders/basic.glsl")
		_deg = 0
		_x = 1
		_y = 0
	}

	update(dt) {
	}

	render() {
		if (App.guiBeginChild("Params", 0.25, 1.0)) {
			_deg = App.guiFloat("Deg", _deg)
			_x = App.guiFloat("X", _x)
			_y = App.guiFloat("Y", _y)
			App.guiEndChild()
		}

		App.glClear(0.1, 0.1, 0.1, 1, 0, 0, 0)
		App.glSetShader(_shader)

		Util.glPerspective("Proj", 70.0, App.winWidth / App.winHeight, 0.1, 1000.0)

		App.glUniform("View")
		App.glMat2x4f(0, 0, 0, 0, 0, 0, 0, 0)

		var p = Point.new(_x, _y, 5.0)
		p.glDraw(0xFF0000FF)

		App.glBegin(true, true, 1, 2)
		for (i in -180...180) {
			var r = PGA.exp_r(i * Num.pi / 180.0, Line.be12)
			var rp = PGA.sw_mp(r, p)
			App.glVertex(rp[0], rp[1], rp[2], 0xFF00FF00)
		}
		App.glEnd(App.glLineStrip)

		var r = PGA.exp_r(_deg * Num.pi / 180.0, Line.be12) // 90-degree rotation around z-axis
		var rp = PGA.sw_mp(r, p) // Apply rotation
		rp.glDraw(0xFFFF0000)
	}
}

class Main {
	static init() { __state = State.new() }
	static update(dt) { __state.update(dt) }
	static render() { __state.render() }
}