import "app" for App

class Main {
	static init() {
		__shader = App.createShader("/Pt2/Shaders/basic.glsl")
		__fov = 60
	}

	static update(dt) {
	}

	static render() {
		App.clear(0.1, 0.1, 0.1, 1, 0, 0, 0)
		App.setShader(__shader)

		__fov = App.debugFloat("Fov", __fov)
		
		App.uniform("Proj")
		perspective(__fov, App.width / App.height, 0.1, 1000.0)

		App.begin(true, true, 100, 1)
		App.vertex(-1, -1, 5, 0xFF00FFFF)
		App.vertex(1, -1, 5, 0xFF00FFFF)
		App.vertex(1, 1, 5, 0xFFFFFFFF)
		App.vertex(-1, 1, 5, 0xFFFFFFFF)
		App.end(App.points)
	}

	static perspective(fov, aspect, near, far) {
		var t = ((fov * Num.pi / 180.0) * 0.5).tan * near
		var b = -t
		var r = t * aspect
		var l = -r
		App.mat3x2f(r, l, t, b, near, far)
	}
}