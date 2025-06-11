import "app" for App
import "physics2" for World, Vec2, Body

class State {
	construct new() {
		_shader = App.glLoadShader("Assets/Common/vertex2.glsl")
        _camScale = 10
		_timeScale = 1
		_timeReverse = false
		_camX = 0
		_camY = 0
		
		World.init()
		_world = World.new(Vec2.new(0, -10))

		var floor = Body.new()
		floor.set(Vec2.new(10, 1), Num.largest)
		floor.position = Vec2.new(-5, -5)
		_world.addBody(floor)

		var floor2 = Body.new()
		floor2.set(Vec2.new(20, 1), Num.largest)
		floor2.position = Vec2.new(0, -10)
		_world.addBody(floor2)

		/*var b = Body.new()
		b.set(Vec2.new(1, 1), 1)
		b.angularVelocity = 0
		_world.addBody(b)*/

		/*var rng = Random.new()
		var planet_radius = 5
		var spawn_radius = planet_radius + 5

		_planet = Body.new(0, 0, 1000, planet_radius)
		_bodies.add(_planet)
		for (i in 1..500) {
			var angle = rng.float(0, 2 * Num.pi)
			var r = rng.float(planet_radius * planet_radius, spawn_radius * spawn_radius).sqrt
			var x = angle.cos * r
			var y = angle.sin * r
			_bodies.add(Body.new(x, y, 0.001, 0.25))
		}*/
    }

	update(dt) {
		if (App.winButton(App.eWinButtonRight)) {
			//_planet.x = _camScale * State.mouseX
			//_planet.y = _camScale * State.mouseY
		}

		_world.step(_timeScale * dt * (_timeReverse ? -1 : 1), 10)
	}

	render() {
		if (App.guiBeginChild("Settings", 500, -1)) {
			_camScale = App.guiFloat("Cam Scale", _camScale, 1, 50)
			_timeScale = App.guiFloat("Time Scale", _timeScale, 0, 2)
			_timeReverse = App.guiBool("Time Reverse", _timeReverse)

			if (App.guiButton("Add Body")) {
				var b = Body.new()
				b.set(Vec2.new(1, 1), 1)
				b.angularVelocity = 0
				_world.addBody(b)
			}
		}
		App.guiEndChild()

		App.glClear(0.1, 0.1, 0.1, 1, 0, 0, 0)
		App.glSetShader(_shader)

		App.glSetUniform("Proj")
		App.glSetVec4f(_camX, _camY, App.winWidth / App.winHeight, _camScale)

        State.glDrawGrid(20, 20, 0.5, 20, App.glGray)

		_world.glDraw()
	}

	static mouseX { (App.winWidth / App.winHeight) * ((2 * App.winMouseX / App.winWidth) - 1) }
	static mouseY { 1 - (2 * App.winMouseY / App.winHeight) }

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