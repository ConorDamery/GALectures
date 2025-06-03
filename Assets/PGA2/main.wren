import "app" for App
import "pga2" for Line2, Point2, Motor2, MVec2

class Util {
	static winMouseX { (App.winWidth / App.winHeight) * ((2 * App.winMouseX / App.winWidth) - 1) }
	static winMouseY { 1 - (2 * App.winMouseY / App.winHeight) }

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

class Handle {
	construct new(x, y) {
		_x = x
		_y = y
		__sel = null
	}

	x { _x }
	y { _y }

	x=(v) { _x = v }
	y=(v) { _y = v }

	isOver(s, mx, my) {
		return (mx > x - s*0.05 && mx < x + s*0.05 &&
			 	my > y - s*0.05 && my < y + s*0.05)
	}

	update(s, dt) {
		var mx = s * Util.winMouseX
		var my = s * Util.winMouseY

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
	}
}

class State {
	construct new() {
		_shader = App.glLoadShader("Assets/Common/vertex2.glsl")

		_camScale = 2
		_camX = 0
		_camY = 0

		_a = Handle.new(-1, -1)
		_b = Handle.new(-1, 1)
		_c = Handle.new(1, 1)
    }

	update(dt) {
		_a.update(_camScale, dt)
		_b.update(_camScale, dt)
		_c.update(_camScale, dt)
	}

	render() {
		if (App.guiBeginChild("Settings", 500, App.guiContentAvailHeight() - 150)) {
			_camScale = App.guiFloat("Cam Scale", _camScale, 1, 10)
		}
		App.guiEndChild()

		App.glClear(0.1, 0.1, 0.1, 1, 0, 0, 0)
		App.glSetShader(_shader)

		App.glSetUniform("Proj")
		App.glSetVec4f(_camX, _camY, App.winWidth / App.winHeight, _camScale)

        Util.glDrawGrid(10, 10, 0.5, 10, App.glGray)

		var a = Point2.new(_a.x, _a.y, 1)
		var b = Point2.new(_b.x, _b.y, 1)
		var c = Point2.new(_c.x, _c.y, 1)

		App.glBegin(true, true, 1, 1)
		App.glAddVertex(a.x, a.y, 0.1, 0x5F00FF00)
		App.glAddVertex(b.x, b.y, 0.1, 0x5F00FF00)
		App.glAddVertex(c.x, c.y, 0.1, 0x5F00FF00)
		App.glEnd(App.glTriangles)

		var l = Line2.new(-0.5, 1, 1)

		var m = c & a
		var d = (l ^ m).normalized

		//var d = m ^ Line2.new(1, 0, 0)
		//var p = Point2.new(0, 0, 1).proj(m)
		//var mp = (p + d).normalized

		a.guiInspect("a")
		c.guiInspect("c")
		m.guiInspect("m")
		d.guiInspect("d")
		//p.guiInspect("p")
		//mp.guiInspect("mp")

		a.glDraw(0xFF00FF00)
		b.glDraw(0xFF00FF00)
		c.glDraw(0xFF00FF00)
		l.glDraw(0xFF00FFFF)
		m.glDraw(0xFFFFFFFF)
		d.glDraw(0xFFFFFF00)
	}
}

class Main {
	static init() { __state = State.new() }
	static update(dt) { __state.update(dt) }
	static render() { __state.render() }
}