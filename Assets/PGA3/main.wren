import "app" for App
import "util" for Util
import "pga3" for Direction, Point, Line, Rotor, Translator, Motor, PGA

class State {
	construct new() {
		_shader = App.glLoadShader("Assets/PGA3/vertex3.glsl")
		_showOrigin = true
		_showGrid = true
		_lookSpeed = 0.1
		_moveSpeed = 10.0
		
		_mx = App.winMouseX
		_my = App.winMouseY

		_campx = 0
		_campy = 0
		_campz = -1
		_camrx = 0
		_camry = 0
		_view = Motor.identity

		_deg = 45
		_seg = 32
		_x = 0
		_y = 1
		_z = 5
		_slerp = false
	}

	update(dt) {
		var dmx = _mx - App.winMouseX
		var dmy = _my - App.winMouseY
		_mx = App.winMouseX
		_my = App.winMouseY

		if (App.winButton(App.winButtonRight)) {
			App.winCursor(App.winCursorDisabled)
			_camry = _camry - dmx * _lookSpeed
			_camrx = _camrx + dmy * _lookSpeed
			_camrx = _camrx > 89 ? 89 : _camrx < -89 ? -89 : _camrx
		} else {
			App.winCursor(App.winCursorNormal)
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
		_campx = _campx + move.x * dt * _moveSpeed
		_campy = _campy + move.y * dt * _moveSpeed
		_campz = _campz + move.z * dt * _moveSpeed

		var t = Translator.new(-_campx, -_campy, -_campz, 0.0)
		_view = r * t
	}

	render() {
		if (App.guiBeginChild("Settings", -1, 40)) {
			_showOrigin = App.guiBool("Show Origin", _showOrigin)
			App.guiSameLine()
			_showGrid = App.guiBool("Show Grid", _showGrid)
			App.guiSameLine()
			App.guiPushItemWidth((App.guiContentAvailWidth() / 4).max(200))
			_lookSpeed = App.guiFloat("Look Speed", _lookSpeed)
			App.guiSameLine()
			_moveSpeed = App.guiFloat("Move Speed", _moveSpeed)
			App.guiPopItemWidth()
		}
		App.guiEndChild()

		if (App.guiBeginChild("Params", (App.guiContentAvailWidth() * 0.25).max(300), -1)) {
			_deg = App.guiFloat("Deg", _deg).min(360).max(0)
			_seg = App.guiFloat("Seg", _seg).max(0)
			_x = App.guiFloat("X", _x)
			_y = App.guiFloat("Y", _y)
			_z = App.guiFloat("Z", _z)
			
			_slerp = App.guiBool("Slerp", _slerp)
		}
		App.guiEndChild()

		App.glClear(0.1, 0.1, 0.1, 1, 0, 0, 0)
		App.glSetShader(_shader)

		Util.glPerspective("Proj", 70.0, App.winWidth / App.winHeight, 0.1, 1000.0)
		_view.glSetUniform("View")

		App.glSetUniform("Tint")
		App.glSetVec4f(1, 1, 1, 1)

		if (_showOrigin) {
			Util.glDrawOrigin(0, 0, 0)
		}
		if (_showGrid) {
			Util.glDrawGrid(5, 1, -1, 1)
		}
		
		var p = Point.new(1, 0, 0, 1)
		p.glDraw(App.glRed)

		var l = Line.new(_x, _y, _z, 0, 0, 0)
		l.glDraw(0xFFFFFFFF)

		var r = l.exp_r(-0.5 * _deg * Num.pi / 180.0) // Rotation around z-axis
		var rp = r >> p // Apply rotation
		rp.glDraw(App.glBlue)
		
		/*if (!_slerp) {
			App.glBegin(true, true, 1, 2)
			App.glAddVertex(0, 0, 0, App.glGreen)
			//l = l.motor.normalized.line
			var s = _deg / _seg.floor
			for (i in 0..._seg.floor+1) {
				var a = i * s
				var r = l.exp_r(-0.5 * a * Num.pi / 180.0)
				var rp = r >> p
				App.glAddVertex(rp.x, rp.y, rp.z, App.glGreen)
			}
			App.glEnd(App.glLines)

			App.glSetUniform("Tint")
			App.glSetVec4f(1, 1, 1, 0.1)
			App.glEnd(App.glTriangleFan)

		} else {
			App.glBegin(true, true, 1, 2)
			App.glAddVertex(0, 0, 0, App.glGreen)
			var g = r.motor.log
			for (i in 0..._seg.floor+1) {
				var t = i / _seg.floor
				var r = (g * t).exp
				var rp = r >> p
				App.glAddVertex(rp.x, rp.y, rp.z, App.glGreen)
			}
			App.glEnd(App.glLines)

			App.glSetUniform("Tint")
			App.glSetVec4f(1, 1, 1, 0.1)
			App.glEnd(App.glTriangleFan)
		}*/
	}
}

class Main {
	static init() { __state = State.new() }
	static update(dt) { __state.update(dt) }
	static render() { __state.render() }
}