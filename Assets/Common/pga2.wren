import "app" for App

class Line2 {
    construct new(e0, e1, e2) {
        _e0 = e0
        _e1 = e1
        _e2 = e2
    }

    e0 { _e0 }
    e1 { _e1 }
    e2 { _e2 }

    e0=(v) { _e0 = v }
    e1=(v) { _e1 = v }
    e2=(v) { _e2 = v }

    // Geometric & scalar product
    *(b) {
        if (b is Num) {
            return Line2.new(
                e0 * b,
                e1 * b,
                e2 * b
            )
        } else if (b is Line2) {
            return Motor2.new(
                e1*b.e1 + e2*b.e2,
                e0*b.e1 - e1*b.e0,
                e0*b.e2 - e2*b.e0,
                e1*b.e2 - e2*b.e1
            )
        } else if (b is Point2) {
            return MVec2.new(
                0,
                b.e20*e2 - b.e01*e1,
                -b.e12*e2,
                b.e12*e1,
                0, 0, 0,
                b.e01*e2 + b.e12*e0 + b.e20*e1
            )
        } else if (b is Rotor2) {
            return MVec2.new(
                0,
                b.s*e0,
                b.s*e1 - b.e12*e2,
                b.e12*e1 + b.s*e2,
                0, 0, 0,
                b.e12*e0
            )
        } else if (b is Translator2) {
            return MVec2.new(
                0,
                -b.e01*e1 - b.e02*e2,
                -e2, e1,
                0, 0, 0,
                b.e01*e2 + e0 - b.e02*e1
            )
        } else if (b is Motor2) {
            return MVec2.new(
                0,
                b.s*e0 - b.e01*e1 - b.e02*e2,
                b.s*e1 - b.e12*e2,
                b.e12*e1 + b.s*e2,
                0, 0, 0,
                b.e01*e2 + b.e12*e0 - b.e02*e1
            )
        } else {
            Fiber.abort("Geometric product not supported for: %(type.name) * %(b.type.name)")
        }
    }

    // Inner product
    |(b) {
        if (b is Num) {
            return Line2.new(
                e0 * b,
                e1 * b,
                e2 * b
            )
        } else if (b is Line2) {
            return b.e1*e1 + b.e2*e2

        } else if (b is Point2) {
            return Line2.new(
                b.e20*e2 - b.e01*e1,
                -b.e12*e2,
                b.e12*e1
            )
        } else {
            Fiber.abort("Inner product not supported for: %(type.name) * %(b.type.name)")
        }
    }

	// Outer product
	^(b) {
        if (b is Num) {
            return Line2.new(
                e0 * b,
                e1 * b,
                e2 * b
            )
        } else if (b is Line2) {
            return Point2.new(
                b.e0*e2 - b.e2*e0,
                b.e1*e0 - b.e0*e1,
                b.e2*e1 - b.e1*e2
            )
        } else if (b is Point2) {
            return MVec2.new(
                0, 0, 0, 0, 0, 0, 0,
                b.e01*e2 + b.e12*e0 + b.e20*e1
            )
        } else {
            Fiber.abort("Outer product not supported for: %(type.name) * %(b.type.name)")
        }
    }

	// Sandwich product
	// Left contraction

	// Regressive product
    &(b) { !(!this ^ !b) }

	// Reverse operator
    ~ { Line2.new(e0, e1, e2) }

	// Dual operator
    ! { Point2.new(e1, e2, e0) }

	// Grade selection
    // TODO?

	// Normalization
    normalized {
        var norm = e1*e1 + e2*e2
        if (norm == 0) {
            return Line2.new(0, 0, 0)
        } else {
            norm = 1 / norm.sqrt
            return this * norm
        }
    }

	// Exponentiation
	// Logarithm

    // Projection
    proj(b) { (this | b) | b }

    guiInspect(name) {
        App.guiPushItemWidth(App.guiContentAvailWidth() / 4)
        App.guiText("%(name): ")
        App.guiSameLine()
        e0 = App.guiFloat("e0##%(name)", e0)
        App.guiSameLine()
        e1 = App.guiFloat("e1##%(name)", e1)
        App.guiSameLine()
        e2 = App.guiFloat("e2##%(name)", e2)
        App.guiPopItemWidth()
    }

    glDraw(color) {
        App.glBegin(true, true, 1, 2)
        var n = 1e3 // Large number simulate infinity
        var d = (this ^ Line2.new(1, 0, 0)) * n
        var p = Point2.new(0, 0, 1).proj(this).normalized
        App.glAddVertex(p.x - d.x, p.y - d.y, 0, color)
        App.glAddVertex(p.x + d.x, p.y + d.y, 0, color)
        App.glEnd(App.glLines)
    }

    toString { "[%(e0)e0 + %(e1)e1 + %(e2)e2]" }
}

class Point2 {
    construct new(e20, e01, e12) {
        _e20 = e20
        _e01 = e01
        _e12 = e12
    }

    e20 { _e20 }
    e01 { _e01 }
    e12 { _e12 }
    x { _e20 }
    y { _e01 }
    w { _e12 }

    e20=(v) { _e20 = v }
    e01=(v) { _e01 = v }
    e12=(v) { _e12 = v }
    x=(v) { _e20 = v }
    y=(v) { _e01 = v }
    w=(v) { _e12 = v }

    +(b) { Point2.new(e20 + b.e20, e01 + b.e01, e12 + b.e12) }

    // Geometric product
    *(b) {
        if (b is Num) {
            return Point2.new(
                e20 * b,
                e01 * b,
                e12 * b
            )
        } else if (b is Point2) {
            return Motor2.new(
                -b.e12*e12,
                b.e12*e20 - b.e20*e12,
                b.e12*e01 - b.e01*e12,
                0
            )
        } else if (b is Line2) {
            return MVec2.new(
                0,
                b.e1*e01 - b.e2*e20,
                b.e2*e12,
                -b.e1*e12,
                0, 0, 0,
                b.e0*e12 + b.e1*e20 + b.e2*e01
            )
        } else if (b is Rotor2) {
            return Motor2.new(
                -b.e12*e12,
                b.e12*e20 + b.s*e01,
                b.e12*e01 - b.s*e20,
                b.s*e12
            )
        } else if (b is Translator2) {
            return Motor2.new(
                -e12,
                b.e02*e12 + e20,
                e01 - b.e01*e12,
                0
            )
        } else if (b is Motor2) {
            return Motor2.new(
                -b.e12*e12,
                b.e02*e12 + b.e12*e20 + b.s*e01,
                b.e12*e01 - b.e01*e12 - b.s*e20,
                b.s*e12
            )
        } else {
            Fiber.abort("Geometric product not supported for: %(type.name) * %(b.type.name)")
        }
    }

    // Inner product
    |(b) {
        if (b is Num) {
            return Point2.new(
                e20 * b,
                e01 * b,
                e12 * b
            )
        } else if (b is Point2) {
            return -b.e12*e12

        } else if (b is Line2) {
            return Line2.new(
                b.e1*e01 - b.e2*e20,
                b.e2*e12,
                -b.e1*e12
            )
        } else {
            Fiber.abort("Inner product not supported for: %(type.name) * %(b.type.name)")
        }
    }
	
    // Outer product
    ^(b) {
        if (b is Num) {
            return Point2.new(
                e20 * b,
                e01 * b,
                e12 * b
            )
        } else if (b is Point2) {
            return 0

        } else if (b is Line2) {
            return MVec2.new(
                0, 0, 0, 0, 0, 0, 0,
                b.e0*e12 + b.e1*e20 + b.e2*e01
            )
        } else {
            Fiber.abort("Inner product not supported for: %(type.name) * %(b.type.name)")
        }
    }

	// Sandwich product
	// Left contraction

	// Regressive product
    &(b) { 
        //!(!this ^ !b)
        return Line2.new(
            b.e01*e20 - b.e20*e01,
            b.e12*e01 - b.e01*e12,
            b.e20*e12 - b.e12*e20
        )
    }

	// Reverse operator
    ~ { Point2.new(-e20, -e01, -e12) }

	// Dual operator
    ! { Line2.new(e12, e20, e01) }

	// Grade selection
	// TODO?

    // Normalization
    normalized {
        var norm = e12*e12
        if (norm == 0) {
            return Point2.new(0, 0, 0)
        } else {
            norm = 1 / norm.sqrt
            return this * norm
        }
    }

	// Exponentiation
	// Logarithm

    proj(b) { (this | b) ^ b }

    guiInspect(name) {
        App.guiPushItemWidth(App.guiContentAvailWidth() / 4)
        App.guiText("%(name): ")
        App.guiSameLine()
        e20 = App.guiFloat("e20##%(name)", e20)
        App.guiSameLine()
        e01 = App.guiFloat("e01##%(name)", e01)
        App.guiSameLine()
        e12 = App.guiFloat("e12##%(name)", e12)
        App.guiPopItemWidth()
    }

    glDraw(color) {
        App.glBegin(true, true, 10, 2)
        App.glAddVertex(e20, e01, 0, color)
        App.glEnd(App.glPoints)
    }

    toString { "[%(e20)e20 + %(e01)e01 + %(e12)e12]" }
}

class Rotor2 {
    construct new(s, e12) {
        _s = s
        _e12 = e12
    }

    s { _s }
    e12 { _e12 }

    s=(v) { _s = v }
    e12=(v) { _e12 = v }

    // Geometric & scalar product
    *(b) {
        if (b is Num) {
            return Rotor2.new(
                s * b,
                e12 * b
            )
        } else if (b is Rotor2) {
            return Rotor2.new(
                s*b.s - e12*b.e12,
                e12*b.s + s*b.e12
            )
        } else if (b is Point2) {
            return Motor2.new(
                -e12,
                b.e01*s + b.e20*e12,
                b.e20*s - b.e01*e12,
                s
            )
        } else if (b is Translator2) {
            return Motor2.new(
                s,
                e12*b.e02 + s*b.e01,
                s*b.e02 - e12*b.e01,
                e12
            )
        } else if (b is Motor2) {
            return Motor2.new(
                b.s*s - b.e12*e12,
                b.e01*s + b.e02*e12,
                b.e02*s - b.e01*e12,
                b.e12*s + b.s*e12
            )
        } else {
            Fiber.abort("Geometric product not supported for: %(type.name) * %(b.type.name)")
        }
    }

    // Sandwich product
    >>(b) {
		if (b is Point2) {
            return Point2.new(
                2*b.e01*e12*s + 2*b.e20*e12*e12 - b.e20,
                2*b.e20*e12*s - 2*b.e01*e12*e12 + b.e01
            )
		} else {
			Fiber.abort("Sandwich not supported for: %(type.name) * %(b.type.name)")
		}
	}

    ~ { Rotor2.new(s, -e12) }

    exp { Rotor2.new(e12.cos, e12.sin) * s.exp }

    glDraw(color) {
        App.glBegin(true, true, 10, 2)
        App.glAddVertex(-s, -e12, 0, 0x00000000)
        App.glAddVertex( s,  e12, 0, color)
        App.glEnd(App.glPoints | App.glLines)
    }

    toString { "[%(s) + %(e12)e12]" }
}

class Translator2 {
    construct new(e01, e02) {
        _e01 = e01
        _e02 = e02
    }

    e01 { _e01 }
    e02 { _e02 }

    e01=(v) { _e01 = v }
    e02=(v) { _e02 = v }

    // Geometric & scalar product
    *(b) {
        if (b is Num) {
            return Translator2.new(
                e01 * b,
                e02 * b
            )
        } else if (b is Translator2) {
            return Translator2.new(
                e01 + b.e01,
                e02 + b.e02
            )
        } else if (b is Point2) {
            return Motor2.new(
                0,
                e01 + b.e01,
                e02 - b.e20,
                1
            )
        } else if (b is Rotor2) {
            return Motor2.new(
                b.s,
                b.s*e01 - b.e12*e02,
                b.e12*e01 + b.s*e02,
                b.e12
            )
        } else if (b is Motor2) {
            return Motor2.new(
                b.s,
                b.e01+b.s*e01 - b.e12*e02,
                b.e02+b.e12*e01 + b.s*e02,
                b.e12
            )
        } else {
            Fiber.abort("Geometric product not supported for: %(type.name) * %(b.type.name)")
        }
    }

    // Sandwich product
    >>(b) {
		if (b is Point2) {
            return Point2.new(
                -b.e20 - 2*e01,
                b.e01 - 2*e02
            )
		} else {
			Fiber.abort("Sandwich not supported for: %(type.name) * %(b.type.name)")
		}
	}

    glDraw(color) {
        App.glBegin(true, true, 10, 2)
        App.glAddVertex(e01, e02, 0, color)
        App.glEnd(App.glPoints)
    }

    toString { "[1 + %(e01)e01 + %(e02)e02]" }
}

class Motor2 {
    construct new(s, e01, e02, e12) {
        _s = s
        _e01 = e01
        _e02 = e02
        _e12 = e12
    }

    s { _s }
    e01 { _e01 }
    e02 { _e02 }
    e12 { _e12 }

    s=(v) { _s = v }
    e01=(v) { _e01 = v }
    e02=(v) { _e02 = v }
    e12=(v) { _e12 = v }

    // Geometric & scalar product
    *(b) {
        if (b is Num) {
            return Motor2.new(
                s * b,
                e12 * b,
                e20 * b,
                e01 * b
            )
        } else if (b is Motor2) {
            return Motor2.new(
                s*b.s - e12*b.e12,
                e01*b.s + e12*b.e02 + s*b.e01 - e02*b.e12,
                e01*b.e12 + e02*b.s + s*b.e02 - e12*b.e01,
                e12*b.s + s*b.e12
            )
        } else if (b is Point2) {
            return Motor2.new(
                -e12,
                b.e20*e12 + s*b.e01 - e02,
                b.e20*s - e12*b.e01 + e01,
                s
            )
        } else if (b is Rotor2) {
            return Motor2.new(
                s*b.s - e12*b.e12,
                e01*b.s - e02*b.e12,
                e01*b.e12 + e02*b.s,
                e12*b.s + s*b.e12
            )
        } else if (b is Translator2) {
            return Motor2.new(
                s,
                e01+e12*b.e02 + s*b.e01,
                e02+s*b.e02 - e12*b.e01,
                e12
            )
        } else {
            Fiber.abort("Geometric product not supported for: %(type.name) * %(b.type.name)")
        }
    }

    // Inner product
	// Outer product
	// Sandwich product
    >>(b) {
		if (b is Point2) {
            return Point2.new(
                2*e12*s*b.e01 - 2*e01*s - 2*e02*e12 - 2*e12*e12*b.e20 + b.e20,
                2*e12*s*b.e20 - 2*e02*s + 2*e01*e12 - 2*e12*e12*b.e01 + b.e01,
                1
            )
		} else {
			Fiber.abort("Sandwich not supported for: %(type.name) * %(b.type.name)")
		}
	}

	// Left contraction
	// Regressive product
	// Reverse operator
    ~ { Motor2.new(s, -e01, -e02, -e12) }

	// Dual operator
	// Grade selection
	// Normalization
	// Exponentiation
	// Logarithm

    glSetUniform(name) {
        App.glSetUniform(name)
        App.glSetVec4f(s, e01, e02, e12)
    }

    toString { "[%(s) + %(e01)e01 + %(e02)e02 + %(e12)e12]" }
}

class MVec2 {
    construct new(s, e0, e1, e2, e01, e02, e12, e012) {
        _s = s
        _e0 = e0
        _e1 = e1
        _e2 = e2
        _e01 = e01
        _e02 = e02
        _e12 = e12
        _e012 = e012
    }

    s { _s }
    e0 { _e0 }
    e1 { _e1 }
    e2 { _e2 }
    e01 { _e01 }
    e02 { _e02 }
    e12 { _e12 }
    e012 { _e012 }

    s=(v) { _s = v }
    e0=(v) { _e0 = v }
    e1=(v) { _e1 = v }
    e2=(v) { _e2 = v }
    e01=(v) { _e01 = v }
    e02=(v) { _e02 = v }
    e12=(v) { _e12 = v }
    e012=(v) { _e012 = v }

    toString { "[%(s) + %(e0)e0 + %(e1)e1 + %(e2)e2 + %(e01)e01 + %(e02)e02 + %(e12)e12 + %(e012)e012]" }
}