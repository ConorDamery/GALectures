import "app" for App

class Point2 {
    construct new(e02, e01) {
        _e02 = e02
        _e01 = e01
    }

    e02 { _e02 }
    e01 { _e01 }
    x { _e02 }
    y { _e01 }

    e02=(v) { _e02 = v }
    e01=(v) { _e01 = v }
    x=(v) { _e02 = v }
    y=(v) { _e01 = v }

    *(b) {
        if (b is Num) {
            return Point2.new(e02 * b, e01 * b)
        } else {
            Fiber.abort("Product not supported for %(b.type.name)")
        }
    }

    glDraw(color) {
        App.glBegin(true, true, 10, 2)
        App.glVertex(e02, e01, 0, color)
        App.glEnd(App.glPoints)
    }

    toString { "[e12 + %(e02)e02 + %(e01)e01]" }
}

class Rotor2 {
    construct new(s, e12) {
        _s = s
        _e12 = e12
    }

    s { _s }
    e12 { _e12 }
    r { _s }
    i { _e12 }

    s=(v) { _s = v }
    e12=(v) { _e12 = v }
    r=(v) { _s = v }
    i=(v) { _e12 = v }

    *(b) {
        if (b is Num) {
            return Rotor2.new(s * b, e12 * b)
        } else if (b is Rotor2) {
            return Rotor2.new(s * b.s - e12 * b.e12, s * b.e12 + e12 * b.s)
        } else {
            Fiber.abort("Product not supported for %(b.type.name)")
        }
    }

    ~ { Rotor2.new(s, -e12) }

    exp { Rotor2.new(e12.cos, e12.sin) * s.exp }

    glDraw(color) {
        App.glBegin(true, true, 10, 2)
        App.glVertex(-s, -e12, 0, 0x00000000)
        App.glVertex( s,  e12, 0, color)
        App.glEnd(App.glPoints | App.glLines)
    }

    toString { "[%(s), %(e12)]" }
}

class Motor2 {
    construct new(s, e12, e02, e01) {
        _s = s
        _e12 = e12
        _e02 = e02
        _e01 = e01
    }

    s { _s }
    e12 { _e12 }
    e02 { _e02 }
    e01 { _e01 }

    s=(v) { _s = v }
    e12=(v) { _e12 = v }
    e02=(v) { _e02 = v }
    e01=(v) { _e01 = v }

    // Geometric product
    *(b) {
        if (b is Num) {
            return Motor2.new(s * b, e12 * b, e02 * b, e01 * b)
        } else if (b is Point2) {
            return Motor2.new(
                -e12,
                s,
                s * b.e02 - e12 * b.e01 + e01,
                s * b.e01 + e12 * b.e02 - e02
        )
        } else if (b is Motor2) {
            return Motor2.new(
                s*b.s - e12*b.e12,
                s*b.e12 + e12*b.s,
                s*b.e02 + e01*b.e12 + e02*b.s - e12*b.e01,
                s*b.e01 + e01*b.s - e02*b.e12 + e12*b.e02
            )
        } else {
            Fiber.abort("Product not supported for %(b.type.name)")
        }
    }

    // Inner product
	// Outer product
	// Sandwich product
    >>(b) {
		if (b is Point2) {
            return Point2.new(
                s*s*b.e02 + s*e12*b.e01 + s*e01 + e12*e02 + s*e01 - s*e12*b.e01 - e12*e12*b.e02 + e12*e02,
                s*s*b.e01 + s*e12*b.e02 - s*e02 + e12*e01 - s*e02 + s*e12*b.e02 + e12*e12*b.e01 + e12*e01
            )
		} else {
			Fiber.abort("Sandwich product not supported for %(b.type.name)")
		}
	}

	// Left contraction
	// Regressive product
	// Reverse operator
    ~ { Motor2.new(s, -e12, -e02, -e01) }

	// Dual operator
	// Grade selection
	// Normalization
	// Exponentiation
	// Logarithm

    glUniform(name) {
        App.glUniform(name)
        App.glVec4f(s, e12, e02, e01)
    }

    toString { "[%(s) + %(e12)e12 + %(e02)e02 + %(e01)e01]" }
}