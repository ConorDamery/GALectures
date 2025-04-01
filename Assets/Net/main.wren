import "app" for App
import "pga2" for Line2, Point2, Motor2

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
}

class Player {
	construct new(id, local, x, y) {
		_id = id
		_local = local
		_x = x
		_y = y
	}

	id { _id }
	local { _local }

	x { _x }
	y { _y }
	x=(v) { _x = v }
	y=(v) { _y = v }

	update(dt) {
		var dx = 0
		var dy = 0
		if (App.winKey(App.winKeyW)) {
			dy = dy + 1
		} else if (App.winKey(App.winKeyS)) {
			dy = dy - 1
		}

		if (App.winKey(App.winKeyA)) {
			dx = dx - 1
		} else if (App.winKey(App.winKeyD)) {
			dx = dx + 1
		}

		x = x + dx * dt
		y = y + dy * dt

		var relay = App.netCreatePacket(16)
		App.netSetUInt(relay, 0, 101)
		App.netSetUInt(relay, 4, id)
		App.netSetFloat(relay, 8, x)
		App.netSetFloat(relay, 12, y)
		App.netSend(relay, App.netPktReliable)
	}

	render() {
		App.glBegin(true, true, 1, 1)
		App.glVertex(x - 0.1, y - 0.1, 0.1, 0xFF00FF00)
		App.glVertex(x - 0.1, y + 0.1, 0.1, 0xFF00FF00)
		App.glVertex(x + 0.1, y - 0.1, 0.1, 0xFF00FF00)
		App.glVertex(x + 0.1, y + 0.1, 0.1, 0xFF00FF00)
		App.glEnd(App.glTriangles)
	}

	netcode(server, packet) {
		var eid = App.netGetUInt(packet, 0)

		if (server) {
			if (eid == 101) {
				var pid = App.netGetUInt(packet, 4)
				var px = App.netGetFloat(packet, 8)
				var py = App.netGetFloat(packet, 12)

				var relay = App.netCreatePacket(16)
				App.netSetUInt(relay, 0, 102)
				App.netSetUInt(relay, 4, pid)
				App.netSetFloat(relay, 8, px)
				App.netSetFloat(relay, 12, py)
				App.netBroadcast(relay, App.netPktReliable)
			}
		
		} else {
			if (eid == 102) {
				var pid = App.netGetUInt(packet, 4)
				if (pid == id) {
					x = App.netGetFloat(packet, 8)
					y = App.netGetFloat(packet, 12)
				}
			}
		}
	}
}

class State {
	construct new() {
		if (!App.isHeadless) {
			_shader = App.glCreateShader("Assets/Common/vertex2.glsl")
		
		} else {
			App.netStartServer()
		}

		_camScale = 2
		_camX = 0
		_camY = 0

		_players = {}
    }

	update(dt) {
		for (i in _players) {
			if (i.value.local) {
				i.value.update(dt)
			}
		}
	}

	render() {
		if (App.guiBeginChild("Settings", 500, App.guiContentAvailHeight() - 150)) {
			_camScale = App.guiFloat("Cam Scale", _camScale, 1, 10)

			if (App.guiButton("Start Server")) {
				App.netStartServer()
			}

			if (App.guiButton("Stop Server")) {
				App.netStopServer()
			}

			if (App.guiButton("Start Client")) {
				App.netStartClient()
			}

			if (App.guiButton("Stop Client")) {
				App.netStopClient()
			}
		}
		App.guiEndChild()

		App.glClear(0.1, 0.1, 0.1, 1, 0, 0, 0)
		App.glSetShader(_shader)

		App.glUniform("Proj")
		App.glVec4f(_camX, _camY, App.winWidth / App.winHeight, _camScale)

        Util.glDrawGrid(10, 10, 0.5, 10, App.glGray)

		for (i in _players) {
			i.value.render()
		}
	}

	netcode(server, event, peer, channel, packet) {
		if (event == App.netEvConnect) {
			if (server) {
				var pid = peer
				var x = 0.0
				var y = 0.0

				var relay = App.netCreatePacket(16)
				App.netSetUInt(relay, 0, 100)
				App.netSetUInt(relay, 4, pid)
				App.netSetFloat(relay, 8, x)
				App.netSetFloat(relay, 12, y)
				App.netBroadcast(relay, App.netPktReliable)

				_players[pid] = Player.new(pid, true, x, y)
				System.print("Player added from server %(pid)")

			} else {
				System.print("Player added from client local %(peer)")
			}

		} else if (event == App.netEvReceive) {
			var eid = App.netGetUInt(packet, 0)

			if (eid == 100 && !App.netIsServer) {
				var pid = App.netGetUInt(packet, 4)
				var x = App.netGetFloat(packet, 8)
				var y = App.netGetFloat(packet, 12)
				_players[pid] = Player.new(pid, true, x, y)
				System.print("Player added from client %(pid)")

			} else {
				for (i in _players) {
					i.value.netcode(server, packet)
				}
			}
		}
	}
}

class Main {
	static init() { __state = State.new() }
	static update(dt) { __state.update(dt) }
	static render() { __state.render() }
	static netcode(server, event, peer, channel, packet) { __state.netcode(server, event, peer, channel, packet) }
}