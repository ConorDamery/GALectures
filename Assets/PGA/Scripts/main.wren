import "app" for App
import "pga" for Direction, Point, Line, Rotor, Translator, Motor, PGA

class Util {
	static glPerspective(name, fov, aspect, near, far) {
		var t = ((fov * Num.pi / 180.0) * 0.5).tan * near
		var b = -t
		var r = t * aspect
		var l = -r
		App.glUniform(name)
		App.glMat3x2f(r, l, t, b, near, far)
	}

	static glDrawOrigin(x, y, z) {
		var s = 0.5
		App.glBegin(true, true, 1, 1)
		App.glVertex(x-s, y, z, App.glRed)
		App.glVertex(x+s, y, z, App.glRed)
		App.glVertex(x, y-s, z, App.glGreen)
		App.glVertex(x, y+s, z, App.glGreen)
		App.glVertex(x, y, z-s, App.glBlue)
		App.glVertex(x, y, z+s, App.glBlue)
		App.glEnd(App.glLines)
	}

	static glDrawGrid(size, x, y, z) {
		App.glBegin(true, true, 1, 1)
		var gridSpacing = 0.5

		// X plane
		x = x * size
		var xRange = (size / gridSpacing).floor
		for (i in -xRange..xRange) {
			App.glVertex(x, i * gridSpacing, -size, App.glGray)
			App.glVertex(x, i * gridSpacing,  size, App.glGray)
			App.glVertex(x, -size, i * gridSpacing, App.glGray)
			App.glVertex(x,  size, i * gridSpacing, App.glGray)
		}

		// Y plane
		y = y * size
		var yRange = (size / gridSpacing).floor
		for (i in -yRange..yRange) {
			App.glVertex(i * gridSpacing, y, -size, App.glGray)
			App.glVertex(i * gridSpacing, y,  size, App.glGray)
			App.glVertex(-size, y, i * gridSpacing, App.glGray)
			App.glVertex( size, y, i * gridSpacing, App.glGray)
		}

		// Z plane
		z = z * size
		var zRange = (size / gridSpacing).floor
		for (i in -zRange..zRange) {
			App.glVertex(i * gridSpacing, -size, z, App.glGray)
			App.glVertex(i * gridSpacing,  size, z, App.glGray)
			App.glVertex(-size, i * gridSpacing, z, App.glGray)
			App.glVertex( size, i * gridSpacing, z, App.glGray)
		}

		App.glEnd(App.glLines)
	}
}

class State {
	construct new() {
		_shader = App.glCreateShader("Assets/PGA/Shaders/vertex.glsl")
		
		_mx = App.winMouseX
		_my = App.winMouseY

		_campx = 0
		_campy = 0
		_campz = 0
		_camrx = 0
		_camry = 0
		_view = Motor.identity

		_deg = 90
		_x = 1
		_y = 0
		_z = 0
	}

	update(dt) {
		var dmx = _mx - App.winMouseX
		var dmy = _my - App.winMouseY
		_mx = App.winMouseX
		_my = App.winMouseY

		if (App.winButton(App.winButtonRight)) {
			_camry = _camry - dmx * 0.1
			_camrx = _camrx + dmy * 0.1
			_camrx = _camrx > 89 ? 89 : _camrx < -89 ? -89 : _camrx
		}

		var rx = PGA.e23.exp_r(0.5 * _camrx * Num.pi / 180.0)
		var ry = PGA.e31.exp_r(-0.5 * _camry * Num.pi / 180.0)
		var r = rx * ry

		var move = Direction.new(
			(App.winKey(App.winKeyD) ? 1 : 0) + (App.winKey(App.winKeyA) ? -1 : 0),
			(App.winKey(App.winKeyE) ? 1 : 0) + (App.winKey(App.winKeyQ) ? -1 : 0),
			(App.winKey(App.winKeyW) ? 1 : 0) + (App.winKey(App.winKeyS) ? -1 : 0)
		)
		move = r >> move.normalized
		_campx = _campx + move.x * dt * 10.0
		_campy = _campy + move.y * dt * 10.0
		_campz = _campz + move.z * dt * 10.0

		var t = Translator.new(-_campx, -_campy, -_campz)
		_view = r * t
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
		_view.glUniform("View")

		App.glUniform("Tint")
		App.glVec4f(1, 1, 1, 1)

		Util.glDrawOrigin(0, 0, 0)
		Util.glDrawGrid(5, 1, -1, 1)
		
		var p = Point.new(_x, _y, _z)
		p.glDraw(App.glRed)

		var r = PGA.e12.exp_r(-0.5 * _deg * Num.pi / 180.0) // Rotation around z-axis
		var rp = r >> p // Apply rotation
		rp.glDraw(App.glBlue)
		
		App.glBegin(true, true, 1, 2)
		for (i in -45...45) {
			var r = PGA.e12.exp_r(2 * i * Num.pi / 180.0)
			var rp = r >> p
			App.glVertex(rp[0], rp[1], rp[2], App.glGreen)
		}
		App.glEnd(App.glLines)

		App.glUniform("Tint")
		App.glVec4f(1, 1, 1, 0.1)
		App.glEnd(App.glTriangleFan)
	}
}

class Main {
	static init() { __state = State.new() }
	static update(dt) { __state.update(dt) }
	static render() { __state.render() }
}