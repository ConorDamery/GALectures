import "app" for App
import "pga2" for Line2, Point2, Rotor2, Trans2, Motor2, MVec2

class Cam {
	construct new() {
		_t = Trans2.new(0, 0)
		_s = 2
		_m = Motor2.new()
	}

	m { _m }
	p { _p }
	m=(v) { _m = v }
	p=(v) { _p = v }

	project(p) {
		
	}

	glSetUniform() {
		App.glSetUniform("Mtr")
		App.glSetVec4f(_m.s, _m.e01, _m.e02, _m.e12)
		App.glSetUniform("Prj")
		App.glSetVec4f(_t.e01, _t.e02, App.winWidth / App.winHeight, _s)
	}
}

class Handle {
	construct new(p) {
		_p = p
	}

	p { _p }
	p=(v) { _p = v }

	static sel { __sel }
	static sel=(v) { __sel = v }

	isOver(s, mx, my) {
		return (mx > p.x - s*0.05 && mx < p.x + s*0.05 &&
			 	my > p.y - s*0.05 && my < p.y + s*0.05)
	}

	update(s, mx, my, dt) {
		if (Handle.sel == null &&
			App.winButton(App.eWinButtonLeft) &&
			isOver(s, mx, my)) {
			Handle.sel = this
		}

		if (Handle.sel == this) {
			p.x = mx
			p.y = my

			if (!App.winButton(App.eWinButtonLeft)) {
				Handle.sel = null
			}
		}
	}
}

class State {
	construct new() {
		_shader = App.glLoadShader("Assets/PGA2/vertex2.glsl")

		_camScale = 2
		_camX = 0
		_camY = 0
		_mx = _camScale * State.winMouseX
		_my = _camScale * State.winMouseY

		_a = Handle.new(Point2.new(0, 0, 1))
		_b = Handle.new(Point2.new(-1, 1, 1))
		_c = Handle.new(Point2.new(1, 1, 1))
		_d = Handle.new(Point2.new(1, 0, 1))

		_e = Handle.new(Point2.new(-1, -1, 1))
		_f = Handle.new(Point2.new(1, 0, 1))
		_g = Handle.new(Point2.new(1, 1, 1))

		_handles = [_a, _b, _c, _d, _e, _f, _g]
    }

	update(dt) {
		var mx = _camScale * State.winMouseX
		var my = _camScale * State.winMouseY
		var dmx = _mx - mx
		var dmy = _my - my
		_mx = mx
		_my = my

		for (i in _handles) {
			i.update(_camScale, mx, my, dt)
		}

		if (Handle.sel == null && App.winButton(App.eWinButtonLeft)) {
			//_camX = _camX + 
		}
	}

	render() {
		if (App.guiBeginChild("Settings", 500, App.guiContentAvailHeight() - 150)) {
			_camScale = App.guiFloat("Cam Scale", _camScale, 1, 10)
			App.guiSeparator("Points")
			_a.p.guiInspect("a")
			_b.p.guiInspect("b")
			_c.p.guiInspect("c")
			_d.p.guiInspect("d")
		}
		App.guiEndChild()

		App.glClear(0.1, 0.1, 0.1, 1, 0, 0, 0)
		App.glSetShader(_shader)

		App.glSetUniform("Proj")
		App.glSetVec4f(_camX, _camY, App.winWidth / App.winHeight, _camScale)

        State.glDrawGrid(10, 10, 0.5, 10, App.glGray)

		var a = _a.p
		var b = _b.p
		var c = _c.p
		var d = _d.p

		a.glDraw(0xFFFF0000)
		b.glDraw(0xFFFF0000)
		c.glDraw(0xFFFF0000)
		d.glDraw(0xFFFF0000)

		var l1 = a & b
		var l2 = c & d

		l1.guiInspect("l1")
		l2.guiInspect("l2")

		l1.glDraw(0xFF00FFFF)
		l2.glDraw(0xFFFFFFFF)

		var n = l1 ^ l2

		n.glDraw(0xFFFFFF00)

		var m = l1 * l2
		App.guiText(m.type.toString)
		//m.glDraw(0xFFFF00FF)

		App.glBegin(true, true, 1, 1)

		var e = _e.p
		var f = _f.p
		var g = _g.p
		App.glAddVertex(e.x, e.y, 0.1, 0x5F00FF00)
		App.glAddVertex(f.x, f.y, 0.1, 0x5F00FF00)
		App.glAddVertex(g.x, g.y, 0.1, 0x5F00FF00)

		e = m>>e
		f = m>>f
		g = m>>g
		if (e is Point2 && f is Point2 && g is Point2) {
			App.glAddVertex(e.x, e.y, 0.1, 0x5F0000FF)
			App.glAddVertex(f.x, f.y, 0.1, 0x5F0000FF)
			App.glAddVertex(g.x, g.y, 0.1, 0x5F0000FF)
		}
		
		App.glEnd(App.glTriangles)
	}

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

class Main {
	static init() { __state = State.new() }
	static update(dt) { __state.update(dt) }
	static render() { __state.render() }
}