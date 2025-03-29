import "app" for App
import "pga2" for Point2, Rotor2, Motor2

class Util {
	static winMouseX { (App.winWidth / App.winHeight) * ((2 * App.winMouseX / App.winWidth) - 1) }
	static winMouseY { 1 - (2 * App.winMouseY / App.winHeight) }

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

	static glVertex(x, y, z, c, u, v, bi, bw) {
		App.glVertex(x, y, z, 0, c, bw, bi, 0, u, v, 0, 0, 0, 0, 0, 0)
	}

	static gl4B2UI(r, g, b, a) {
		return (r.floor << 0) | (g.floor << 8) | (b.floor << 16) | (a.floor << 24)
	}

	static gl4F2UI(r, g, b, a) {
		return gl4B2UI(r * 255, g * 255, b * 255, a * 255)
	}
}

class Handle {
	construct new(x, y) {
		_lm = Motor2.new(1, -0.5 * x, -0.5 * y, 0)
		_wm = lm
		_isOver = false

		__sel = null
	}

	lm { _lm }
	wm { _wm }
	isOver { _isOver }

	lm=(v) { _lm = v }
	wm=(v) { _wm = v }

	update(pm, s, dt) {
		var mp = ~wm >> Point2.new(s * Util.winMouseX, s * Util.winMouseY)
		_isOver = mp.x > -s*0.05 && mp.x < s*0.05 && mp.y > -s*0.05 && mp.y < s*0.05

		if (__sel == null && _isOver &&
			App.winButton(App.winButtonLeft)) {
			__sel = this
		}

		if (__sel == this) {
			var a = Num.pi * 0
			var r = Motor2.new(a.cos, 0, 0, a.sin)
			var t = Motor2.new(1, -0.5 * mp.x, -0.5 * mp.y, 0)
			lm = lm * t * r

			if (!App.winButton(App.winButtonLeft)) {
				__sel = null
			}
		}

		wm = pm * lm
	}
}

class State {
	construct new() {
		_shader = App.glCreateShader("Assets/Common/vertex2.glsl")

		_vshader = App.glCreateShader("Assets/Skinning/skinning.glsl")
		var img = App.glCreateImage("Assets/App/GASandbox.png", true)
		_texture = App.glCreateTexture(
			img, App.glTexFmtRGBA8,
			App.glTexFltLinear, App.glTexFltLinear,
			App.glTexWrpRepeat, App.glTexWrpRepeat,
			true)
        App.glDestroyImage(img)

		_camScale = 10
		_camX = 0
		_camY = 0

		_handles = [Handle.new(0, 0), Handle.new(1, 2), Handle.new(-2, 3)]
    }

	update(dt) {
		var m = Motor2.new(1, 0, 0, 0)
		for (i in _handles) {
			i.update(m, _camScale, dt)
			m = i.wm
		}
	}

	render() {
		if (App.guiBeginChild("Settings", 500, -1)) {
			_camScale = App.guiFloat("Cam Scale", _camScale, 1, 20)
			App.guiText("mx: %(Util.winMouseX)")
			App.guiText("my: %(Util.winMouseY)")
		}
		App.guiEndChild()

		App.glClear(0.1, 0.1, 0.1, 1, 0, 0, 0)
		App.glSetShader(_shader)

		App.glUniform("Proj")
		App.glVec4f(_camX, _camY, App.winWidth / App.winHeight, _camScale)

        Util.glDrawGrid(20, 20, 0.5, 20, App.glGray)

		App.glBegin(true, true, 20, 2)
		for (i in _handles) {
			var p = i.wm >> Point2.new(0, 0)
			App.glVertex(p.x, p.y, 0, i.isOver ? 0xFF00FFFF : 0xFFFFFFFF)
		}
		App.glEnd(App.glPoints | App.glLineStrip)

		// Skinning 2D demo
		App.glSetShader(_vshader)

		App.glUniform("Proj")
		App.glVec4f(_camX, _camY, App.winWidth / App.winHeight, _camScale)

		App.glUniform("Model")
		App.glVec4f(1, 0, 0, 0)

		for (i in 0..._handles.count) {
			_handles[i].wm.glUniform("Bones[%(i)]")
		}

		App.glUniform("Tex")
		App.glTex2D(0, _texture)

		App.glBegin(true, true, 10, 1)
		for (i in 0..10) {
			var bw = i / 10
			Util.glVertex(-1, 0, 0, 0xFFFFFFFF, 0, bw, Util.gl4B2UI(0, 1, 2, 255), Util.gl4F2UI(1 - bw, bw, 0, 0))
			Util.glVertex( 1, 0, 0, 0xFFFFFFFF, 1, bw, Util.gl4B2UI(0, 1, 2, 255), Util.gl4F2UI(1 - bw, bw, 0, 0))
		}
		for (i in 0..10) {
			var bw = i / 10
			Util.glVertex(-1, 0, 0, 0xFFFFFFFF, 0, bw, Util.gl4B2UI(0, 1, 2, 255), Util.gl4F2UI(0, 1 - bw, bw, 0))
			Util.glVertex( 1, 0, 0, 0xFFFFFFFF, 1, bw, Util.gl4B2UI(0, 1, 2, 255), Util.gl4F2UI(0, 1 - bw, bw, 0))
		}
		App.glEnd(App.glTriangleStrip)
	}
}

class Main {
	static init() { __state = State.new() }
	static update(dt) { __state.update(dt) }
	static render() { __state.render() }
}