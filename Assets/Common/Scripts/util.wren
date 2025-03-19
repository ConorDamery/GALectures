import "app" for App

class Util {
	static mouseX { 2.5 * ((App.winWidth / App.winHeight) * ((2 * App.winMouseX / App.winWidth) - 1)) }
	static mouseY { 2.5 * (1 - (2 * App.winMouseY / App.winHeight)) }

	static radians(x) { Num.pi * x / 180 }
	static degrees(x) { x / Num.pi * 180 }

	static glPerspective(name, fov, aspect, near, far) {
		var t = ((fov * Num.pi / 180.0) * 0.5).tan * near
		var b = -t
		var r = t * aspect
		var l = -r
		App.glUniform(name)
		App.glMat3x2f(r, l, t, b, near, far)
	}

	static glDrawOrigin(x, y, z) {
		var s = 1
		App.glBegin(true, true, 1, 1)
		App.glVertex(x-s, y, z, 0x0F0000FF)
		App.glVertex(x+s, y, z, App.glRed)
		App.glVertex(x, y-s, z, 0x0F00FF00)
		App.glVertex(x, y+s, z, App.glGreen)
		App.glVertex(x, y, z-s, 0x0FFF0000)
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