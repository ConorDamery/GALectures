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

class NetServer {
	static playerJoin { 100 }
	static playerUpdate { 200 }
}

class NetClient {
	static playerJoin { 101 }
	static playerUpdate { 201 }
}

class Player {
	construct new(id, x, y) {
		_id = id
		_x = x
		_y = y
	}

	id { _id }
	local { App.netIsClient(_id) }

	x { _x }
	y { _y }
	x=(v) { _x = v }
	y=(v) { _y = v }

	update(dt) {
		if (!local) {
			return
		}

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

		var relay = App.netCreatePacket("NETPKT", 16)
		App.netSetUInt(relay, 0, NetClient.playerUpdate)
		App.netSetUInt(relay, 4, id)
		App.netSetFloat(relay, 8, x)
		App.netSetFloat(relay, 12, y)
		App.netSend(id, relay, App.netPktReliable)
	}

	render() {
		App.glBegin(true, true, 1, 1)
		App.glVertex(x - 0.1, y - 0.1, 0.1, 0xFF00FF00)
		App.glVertex(x - 0.1, y + 0.1, 0.1, 0xFF00FF00)
		App.glVertex(x + 0.1, y - 0.1, 0.1, 0xFF00FF00)
		App.glVertex(x + 0.1, y + 0.1, 0.1, 0xFF00FF00)
		App.glEnd(App.glTriangleStrip)
	}

	netcode(server, packet) {
		var eid = App.netGetUInt(packet, 0)

		if (server) {
			if (eid == NetClient.playerUpdate) {
				var pid = App.netGetUInt(packet, 4)
				var px = App.netGetFloat(packet, 8)
				var py = App.netGetFloat(packet, 12)

				var relay = App.netCreatePacket("NETPKT", 16)
				App.netSetUInt(relay, 0, NetServer.playerUpdate)
				App.netSetUInt(relay, 4, pid)
				App.netSetFloat(relay, 8, px)
				App.netSetFloat(relay, 12, py)
				App.netBroadcast(relay, App.netPktReliable)
			}
		
		} else {
			if (eid == NetServer.playerUpdate) {
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
			i.value.update(dt)
		}
	}

	render() {
		if (App.guiBeginChild("Settings", 500, App.guiContentAvailHeight() - 150)) {
			_camScale = App.guiFloat("Cam Scale", _camScale, 1, 10)

			if (App.guiButton("Start Server")) {
				App.netStartServer("any", 7777, 32, 2)
			}

			if (App.guiButton("Stop Server")) {
				App.netStopServer()
			}

			if (App.guiButton("Connect Client")) {
				var client = App.netConnectClient("127.0.0.1", 7777, 1, 2)
			}

			//if (App.guiButton("Stop Client")) {
			//	App.netStopClient()
			//}
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
			if (!server) {
				var relay = App.netCreatePacket("NETPKT", 8)
				App.netSetUInt(relay, 0, NetClient.playerJoin)
				App.netSetUInt(relay, 4, App.)
				App.netSend(relay, App.netPktReliable)
			}
		}
		
		if (event == App.netEvReceive) {
			var eid = App.netGetUInt(packet, 0)

			if (eid == NetServer.playerJoin) {
				var pid = App.netGetUInt(packet, 4)
				var x = App.netGetFloat(packet, 8)
				var y = App.netGetFloat(packet, 12)

				if (!_players.containsKey(pid)) {
					_players[pid] = Player.new(pid, x, y)
					System.print("Player added from client %(peer)")
				}

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