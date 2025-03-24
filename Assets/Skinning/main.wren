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
		_x = x
		_y = y
		_m = Motor2.new(1, 0, 0, 0)

		__sel = null
	}

	x { _x }
	y { _y }
	m { _m }

	x=(v) { _x = v }
	y=(v) { _y = v }
	m=(v) { _m = v }

	isOver(s, mx, my) {
		return (mx > x - s*0.05 && mx < x + s*0.05 &&
			 	my > y - s*0.05 && my < y + s*0.05)
	}

	update(pm, s, mx, my, dt) {
		if (__sel == null &&
			App.winButton(App.winButtonLeft) &&
			isOver(s, mx, my)) {
			__sel = this
		}

		if (__sel == this) {
			x = mx
			y = my

			if (!App.winButton(App.winButtonLeft)) {
				__sel = null
			}
		}

		//var p = ~pm >> Point2.new(x, y)
		//m = Motor2.new(1, 0, 0.5 * p.x, 0.5 * p.y)
		m = Motor2.new(1, 0, 0.5 * x, 0.5 * y)
	}
}

class State {
	construct new() {
		_shader = App.glCreateShader("Assets/Common/vertex2.glsl")

		_vshader = App.glCreateShader("Assets/Skinning/vertex.glsl")
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

		_mx = 0
		_my = 0

		_handles = [Handle.new(0, 2.5), Handle.new(0, 5.0)]
    }

	update(dt) {
		_mx = _camScale * Util.winMouseX
		_my = _camScale * Util.winMouseY

		var m = Motor2.new(1, 0, 0, 0)
		for (i in _handles) {
			i.update(m, _camScale, _mx, _my, dt)
			m = m * i.m
		}
	}

	render() {
		if (App.guiBeginChild("Settings", 500, -1)) {
			_camScale = App.guiFloat("Cam Scale", _camScale, 1, 20)
			App.guiText("mx: %(_mx.floor), my: %(_my.floor)")
		}
		App.guiEndChild()

		App.glClear(0.1, 0.1, 0.1, 1, 0, 0, 0)
		App.glSetShader(_shader)

		App.glUniform("Proj")
		App.glVec4f(_camX, _camY, App.winWidth / App.winHeight, _camScale)

        Util.glDrawGrid(20, 20, 0.5, 20, App.glGray)

		var t = _handles[0].m >> Point2.new(0, 5)
		App.glBegin(true, true, 10, 1)
		App.glVertex(t.x, t.y, 0, 0xFFFFFFFF)
		App.glEnd(App.glPoints)

		App.glBegin(true, true, 20, 2)
		App.glVertex(0, 0, 0, 0xFFFFFFFF)
		for (i in _handles) {
			var c = i.isOver(_camScale, _mx, _my) ? 0xFF00FFFF : 0xFFFFFFFF
			App.glVertex(i.x, i.y, 0, c)
		}
		App.glEnd(App.glPoints | App.glLineStrip)

		// Skinning 2D demo
		App.glSetShader(_vshader)

		App.glUniform("Proj")
		App.glVec4f(_camX, _camY, App.winWidth / App.winHeight, _camScale)

		App.glUniform("Model")
		App.glVec4f(1, 0, 0, 0)

		var m = Motor2.new(1, 0, 0, 0)
		for (i in 0..._handles.count) {
			m = m * _handles[i].m
			m.glUniform("Bones[%(i)]")
		}

		App.glUniform("Tex")
		App.glTex2D(0, _texture)

		App.glBegin(true, true, 10, 1)
		for (i in 0..10) {
			var bw = i / 10
			Util.glVertex(-1, i * 0.5, 0, 0xFFFFFFFF, 0, i / 10, Util.gl4B2UI(0, 1, 255, 255), Util.gl4F2UI(bw, 1 - bw, 0, 0))
			Util.glVertex( 1, i * 0.5, 0, 0xFFFFFFFF, 1, i / 10, Util.gl4B2UI(0, 1, 255, 255), Util.gl4F2UI(bw, 1 - bw, 0, 0))
		}
		App.glEnd(App.glTriangleStrip)
	}
}

class Main {
	static init() { __state = State.new() }
	static update(dt) { __state.update(dt) }
	static render() { __state.render() }
}