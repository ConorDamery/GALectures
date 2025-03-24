import "app" for App
import "pga2" for Point, Rotor, Motor

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

		_angle = 0

		_bone0 = Motor.new(-1, 0, 0, 0)
		_bone1 = Motor.new( 1, 0, 0, 0)
    }

	update(dt) {
	}

	render() {
		if (App.guiBeginChild("Settings", 500, -1)) {
			_camScale = App.guiFloat("Cam Scale", _camScale, 1, 10)
			_angle = App.guiFloat("Angle", _angle, -180, 180)

			_bone0.s = App.guiFloat("X1", _bone0.s)
			_bone0.e12 = App.guiFloat("Y1", _bone0.e12)
			_bone1.s = App.guiFloat("X2", _bone1.s)
			_bone1.e12 = App.guiFloat("Y2", _bone1.e12)
		}
		App.guiEndChild()

		App.glClear(0.1, 0.1, 0.1, 1, 0, 0, 0)
		App.glSetShader(_shader)

		App.glUniform("Proj")
		App.glVec4f(_camX, _camY, App.winWidth / App.winHeight, _camScale)

        Util.glDrawGrid(20, 20, 0.5, 20, App.glGray)

		// Skinning 2D demo
		App.glSetShader(_vshader)

		App.glUniform("Proj")
		App.glVec4f(_camX, _camY, App.winWidth / App.winHeight, _camScale)

		var a = -0.5 * _angle * Num.pi / 180.0
		App.glUniform("Model")
		App.glVec4f(a.cos, a.sin, 0, 0)

		_bone0.glUniform("Bones[0]")
		_bone1.glUniform("Bones[1]")

		App.glUniform("Tex")
		App.glTex2D(0, _texture)

		App.glBegin(true, true, 10, 1)
		for (i in 0...10) {
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