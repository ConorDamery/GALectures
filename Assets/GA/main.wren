import "app" for App
import "ga" for GA

var PGA2D = GA.create({ "E0":0, "E1":1, "E2":1 })
var S = PGA2D[0]
var E0 = PGA2D[1]
var E1 = PGA2D[2]
var E2 = PGA2D[3]

class GATest {
	static test() {
		var a = S*1 + E0*1 + E1*1 + E2*1
		System.print(a)
	}
}

class Main {
	static init() { GATest.test() }
	static update(dt) {}
	static render() {}
}