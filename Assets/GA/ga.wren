import "meta" for Meta

class GA {
	static create(elems) {
		if (elems.type != Map) Fiber.abort("Elements must be a map.")
		if (elems.isEmpty) Fiber.abort("A GA must have at least one element.")

		var elemCount = 1 << elems.count
		var si = "["
		for (i in 0...elemCount) {
			si = si + (i == elemCount - 1 ? "0]" : "0,")
		}

		var s =
		"class MVec_ {\n
			construct new() {\n
				_s = %(si)\n
			}\n
			construct new(s) {\n
				_s = s\n
			}\n
			s { _s }\n
			s=(v) { _s = v }\n
			+(b) {\n
				var m = MVec_.new()\n
				for (i in 0...s.count) {\n
					m.s[i] = s[i] + b.s[i]\n
				}\n
				return m\n
			}\n
			toString { s.toString }\n
		}\n
		class Blade_ {\n
			construct new(t, s) {\n
				_t = t\n
				_s = s\n
			}\n
			t { t }\n
			s { s }\n
			s=(v) { s = v }\n
		}\n"

		s = s + "class S_ {\n
			static i { 0 }\n
			static *(b) { MVec_.new([b,0,0,0,0,0,0,0]) }\n
		}\n"

		var idx = 1
		var ret = "return {\"S\":S_"
		for (i in elems) {
			var sj = "["
			for (j in 0...elemCount) {
				var sym = idx == j ? "b" : "0"
				sj = sj + (j == elemCount - 1 ? "%(sym)]" : "%(sym),")
			}
			idx = idx + 1

			ret = "%(ret), \"%(i.key)\":%(i.key)_"
			s = s +
			"class %(i.key)_ {\n
				static i { 0 }\n
				static *(b) { MVec_.new(%(sj)) }\n
			}\n"
		}
		ret = ret + "}"
		s = s + ret

		/*var s = "class %(name) {\n"
		for (i in 0...members.count) {
			var m = members[i]
			s = s + "  static %(m) { %(i + startsFrom) }\n"
		}
		var mems = members.map { |m| "\"%(m)\"" }.join(", ")
		s = s + "  static startsFrom { %(startsFrom) }\n"
		s = s + "  static members { [%(mems)] }\n}\n"
		s = s + "return %(name)"*/

		//System.print(s)
		return Meta.compile(s).call()
		//return s
	}
}