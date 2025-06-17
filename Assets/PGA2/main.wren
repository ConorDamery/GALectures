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
	construct new(x, y) {
		_x = x
		_y = y
		__sel = null
	}

	x { _x }
	y { _y }

	x=(v) { _x = v }
	y=(v) { _y = v }

	static sel { __sel }  

	isOver(s, mx, my) {
		return (mx > x - s*0.05 && mx < x + s*0.05 &&
			 	my > y - s*0.05 && my < y + s*0.05)
	}

	update(s, mx, my, dt) {
		if (__sel == null &&
			App.winButton(App.eWinButtonLeft) &&
			isOver(s, mx, my)) {
			__sel = this
		}

		if (__sel == this) {
			x = mx
			y = my

			if (!App.winButton(App.eWinButtonLeft)) {
				__sel = null
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

		_a = Handle.new(-1, -1)
		_b = Handle.new(-1, 1)
		_c = Handle.new(1, 1)
		_d = Handle.new(1, 0)

		_handles = [_a, _b, _c, _d]
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
		}
		App.guiEndChild()

		App.glClear(0.1, 0.1, 0.1, 1, 0, 0, 0)
		App.glSetShader(_shader)

		App.glSetUniform("Proj")
		App.glSetVec4f(_camX, _camY, App.winWidth / App.winHeight, _camScale)

        State.glDrawGrid(10, 10, 0.5, 10, App.glGray)

		var a = Point2.new(_a.x, _a.y, 1)
		var b = Point2.new(_b.x, _b.y, 1)
		var c = Point2.new(_c.x, _c.y, 1)
		var d = Point2.new(_d.x, _d.y, 1)

		var l = Line2.new(-0.5, 1, 1)

		var m = c & a
		var n = l ^ m

		var r = m * l
		//System.print(r.type)
		r.glDraw(0xFFFF00FF)

		var t = l>>b
		t.glDraw(0xFFFFFFFF)
		t.guiInspect("t")

		App.glBegin(true, true, 1, 1)
		App.glAddVertex(a.x, a.y, 0.1, 0x5F00FF00)
		App.glAddVertex(b.x, b.y, 0.1, 0x5F00FF00)
		App.glAddVertex(c.x, c.y, 0.1, 0x5F00FF00)
		App.glEnd(App.glTriangles)
		
		a.glDraw(0xFF00FF00)
		b.glDraw(0xFF00FF00)
		c.glDraw(0xFF00FF00)
		d.glDraw(0xFF0000FF)

		l.glDraw(0xFF00FFFF)
		m.glDraw(0xFFFFFFFF)
		n.glDraw(0xFFFFFF00)
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