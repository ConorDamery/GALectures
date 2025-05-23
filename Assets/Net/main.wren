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

class NetServer {
	static playerJoin { 100 }
	static playerUpdate { 200 }
}

class NetClient {
	static playerJoin { 101 }
	static playerUpdate { 201 }
}

class Player {
	construct new(pid, client) {
		_pid = pid
		_client = client

		_x = 0
		_y = 0
	}
	
	pid { _pid }
	isLocal { App.netIsClient(_client) }

	x { _x }
	y { _y }
	x=(v) { _x = v }
	y=(v) { _y = v }

	update(dt) {
		if (!isLocal) {
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

		var relay = App.netCreatePacket(NetClient.playerUpdate, 12)
		App.netSetUInt(relay, 0, pid)
		App.netSetFloat(relay, 4, x)
		App.netSetFloat(relay, 8, y)
		App.netSend(_client, relay, App.netPktReliable)
	}

	render() {
		App.glBegin(true, true, 1, 1)
		App.glAddVertex(x - 0.1, y - 0.1, 0.1, 0xFF00FF00)
		App.glAddVertex(x - 0.1, y + 0.1, 0.1, 0xFF00FF00)
		App.glAddVertex(x + 0.1, y - 0.1, 0.1, 0xFF00FF00)
		App.glAddVertex(x + 0.1, y + 0.1, 0.1, 0xFF00FF00)
		App.glEnd(App.glTriangleStrip)
	}

	netcode(server, packet) {
		var eid = App.netPacketId(packet)

		if (server) {
			if (eid == NetClient.playerUpdate) {
				var cpid = App.netGetUInt(packet, 0)
				var cpx = App.netGetFloat(packet, 4)
				var cpy = App.netGetFloat(packet, 8)

				var relay = App.netCreatePacket(NetServer.playerUpdate, 12)
				App.netSetUInt(relay, 0, cpid)
				App.netSetFloat(relay, 4, cpx)
				App.netSetFloat(relay, 8, cpy)
				App.netBroadcast(relay, App.netPktReliable)
			}
		
		} else {
			if (eid == NetServer.playerUpdate) {
				var cpid = App.netGetUInt(packet, 0)
				if (!isLocal && pid == cpid) {
					x = App.netGetFloat(packet, 4)
					y = App.netGetFloat(packet, 8)
				}
			}
		}
	}
}

class State {
	construct new() {
		if (!App.isHeadless) {
			_shader = App.glLoadShader("Assets/Common/vertex2.glsl")

			var model = App.glLoadModel("Assets/Common/Models/Box.glb")

			//var audio = App.sfxLoadAudio("Assets/Common/Audio/48000-stereo.ogg")
			//var channel = App.sfxCreateChannel(1)

			//System.print("Audio: %(audio), Channel: %(channel)")
			//App.sfxPlay(audio, channel, false)
		
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

			if (!App.netIsServer()) {
				if (App.guiButton("Start Server")) {
					App.netStartServer("any", 7777, 32, 2)
				}
			} else {
				if (App.guiButton("Stop Server")) {
					App.netStopServer()
				}
			}

			if (App.guiButton("Connect Client")) {
				var client = App.netConnectClient("127.0.0.1", 7777, 1, 2)
			}

			//if (App.guiButton("Stop Client")) {
			//	App.netStopClient()
			//}
		}
		App.guiEndChild()

		App.glClear(0.1, 0.1, 0.1, 1, 1, 0, App.glClearAll)
		App.glSetShader(_shader)

		App.glSetUniform("Proj")
		App.glSetVec4f(_camX, _camY, App.winWidth / App.winHeight, _camScale)

        Util.glDrawGrid(10, 10, 0.5, 10, App.glGray)

		for (i in _players) {
			i.value.render()
		}
	}

	netcode(server, client, event, peer, channel, packet) {
		if (server) {
			if (event == App.netEvReceive) {
				var eid = App.netPacketId(packet)

				// Relay player joining, we broadcast the list of all players
				if (eid == NetClient.playerJoin) {
					var pid = App.netGetUInt(packet, 0)

					// When we are a server we still need to add a player
					// (it will already be there if in server/client mode)
					if (!_players.containsKey(pid)) {
						_players[pid] = Player.new(pid, -1)
					}

					var size = 4 + _players.count * 4
					var relay = App.netCreatePacket(NetServer.playerJoin, size)
					App.netSetUInt(relay, 0, _players.count)
					var offset = 4
					for (i in _players) {
						App.netSetUInt(relay, offset, i.value.pid)
						offset = offset + 4
					}
					App.netBroadcast(relay, App.netPktReliable)

					System.print("Player %(pid) joined, broadcasting lobby.")
				}
			}

		} else {
			// Our client has connected, join player with unique ID
			if (event == App.netEvConnect) {
				var pid = App.netMakeUuid()
				_players[pid] = Player.new(pid, client)

				var relay = App.netCreatePacket(NetClient.playerJoin, 4)
				App.netSetUInt(relay, 0, pid)
				App.netSend(client, relay, App.netPktReliable)

				System.print("Client connected, request player %(pid) join. %(client)")
			}
			
			if (event == App.netEvReceive) {
				var eid = App.netPacketId(packet)

				// Server sent player list, update local players
				if (eid == NetServer.playerJoin) {
					var count = App.netGetUInt(packet, 0)
					var offset = 4
					for (i in 0...count) {
						var pid = App.netGetUInt(packet, offset)
						if (!_players.containsKey(pid)) {
							_players[pid] = Player.new(pid, -1)
							System.print("Remote player %(pid) joined.")
						}
						offset = offset + 4
					}

					System.print("Updated lobby, count %(count).")
				}
			}
		}

		// Other non management events can be handled by the player logic
		if (event == App.netEvReceive) {
			for (i in _players) {
				i.value.netcode(server, packet)
			}
		}
	}
}

class Main {
	static init() { __state = State.new() }
	static update(dt) { __state.update(dt) }
	static render() { __state.render() }
	static netcode(server, client, event, peer, channel, packet) { __state.netcode(server, client, event, peer, channel, packet) }
}