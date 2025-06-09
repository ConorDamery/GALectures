import "app" for App
import "ga" for GA

var pga2d = GA.create({ "E0":0, "E1":1, "E2":1 })
var S = pga2d["S"]
var E0 = pga2d["E0"]
var E1 = pga2d["E1"]
var E2 = pga2d["E2"]

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