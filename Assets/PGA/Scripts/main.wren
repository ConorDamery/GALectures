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
	construct new() {
		_shader = App.glCreateShader("Assets/PGA/Shaders/vertex.glsl")
		_deg = 90
		_x = 1
		_y = 0
		_z = 5
	}

	update(dt) {
	}

	render() {
		if (App.guiBeginChild("Params", 0.25, 1.0)) {
			_deg = App.guiFloat("Deg", _deg)
			_x = App.guiFloat("X", _x)
			_y = App.guiFloat("Y", _y)
			_z = App.guiFloat("Z", _z)
		}
		App.guiEndChild()

		App.glClear(0.1, 0.1, 0.1, 1, 0, 0, 0)
		App.glSetShader(_shader)

		Util.glPerspective("Proj", 70.0, App.winWidth / App.winHeight, 0.1, 1000.0)
		
		var p = Point.new(_x, _y, _z)
		p.glDraw(App.glRed)

		var r = PGA.exp_r(_deg * Num.pi / 180.0, PGA.e12) // Rotation around z-axis
		var rp = PGA.sw_mp(r, p) // Apply rotation
		rp.glDraw(App.glBlue)

		App.glBegin(true, true, 1, 2)
		for (i in -180...180) {
			var r = PGA.exp_r(i * Num.pi / 180.0, PGA.e12)
			var rp = PGA.sw_mp(r, p)
			App.glVertex(rp[0], rp[1], rp[2], App.glGreen)
		}
		App.glEnd(App.glLines)
	}
}

class Main {
	static init() { __state = State.new() }
	static update(dt) { __state.update(dt) }
	static render() { __state.render() }
}