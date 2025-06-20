import "app" for App

// Special thanks to enki and his GAmphetamine codegen!
// See: https://enki.ws/ganja.js/examples/coffeeshop.html

// The Line class represents the vector: Ae₀ + Be₁ + Ce₂
class Line2 {
    construct new(e0, e1, e2) {
        _e0 = e0
        _e1 = e1
        _e2 = e2
    }

    // GA notation
    e0 { _e0 }
    e1 { _e1 }
    e2 { _e2 }
    e0=(v) { _e0 = v }
    e1=(v) { _e1 = v }
    e2=(v) { _e2 = v }

    // Classic notation
    a { _e0 }
    b { _e1 }
    c { _e2 }
    a=(v) { _e0 = v }
    b=(v) { _e1 = v }
    c=(v) { _e2 = v }

    // Addition
    +(b) {
        if (b is Num) {
            return MVec2.new(b, e0, e1, e2, 0, 0, 0, 0).reduce

        } else if (b is Line2) {
            return Line2.new(e0+b.e0, e1+b.e1, e2+b.e2)

        } else if (b is Point2) {
            return MVec2.new(0, e0, e1, e2, b.e01, -b.e20, b.e12, 0).reduce

        } else if (b is Rotor2) {
            return MVec2.new(b.s, e0, e1, e2, 0, 0, b.e12, 0).reduce

        } else if (b is Trans2) {
            return MVec2.new(1, e0, e1, e2, b.e01, b.e02, 0, 0).reduce

        } else if (b is Motor2) {
            return MVec2.new(b.s, e0, e1, e2, b.e01, b.e02, b.e12, 0).reduce

        // TODO: PScalar2

        } else if (b is MVec2) {
            return MVec2.new(b.s, e0+b.e0, e1+b.e1, e2+b.e2, b.e01, b.e02, b.e12, b.e012).reduce
        }

        Fiber.abort("Addition not supported for: %(type.name) * %(b.type.name)")
    }

    // Subtraction
    -(b) {
        if (b is Num) {
            return MVec2.new(-b, e0, e1, e2, 0, 0, 0, 0).reduce

        } else if (b is Line2) {
            return Line2.new(e0-b.e0, e1-b.e1, e2-b.e2)

        } else if (b is Point2) {
            return MVec2.new(0, e0, e1, e2, -b.e01, b.e20, -b.e12, 0).reduce

        } else if (b is Rotor2) {
            return MVec2.new(-b.s, e0, e1, e2, 0, 0, -b.e12, 0).reduce

        } else if (b is Trans2) {
            return MVec2.new(1, e0, e1, e2, -b.e01, -b.e02, 0, 0).reduce

        } else if (b is Motor2) {
            return MVec2.new(-b.s, e0, e1, e2, -b.e01, -b.e02, -b.e12, 0).reduce

        // TODO: PScalar2

        } else if (b is MVec2) {
            return MVec2.new(-b.s, e0-b.e0, e1-b.e1, e2-b.e2, -b.e01, -b.e02, -b.e12, -b.e012).reduce
        }

        Fiber.abort("Subtraction not supported for: %(type.name) * %(b.type.name)")
    }

    // Geometric product
    *(b) {
        if (b is Num) {
            return Line2.new(
                e0 * b, e1 * b, e2 * b
            )

        } else if (b is Line2) {
            return Motor2.new(
                e1*b.e1 + e2*b.e2,
                e0*b.e1 - e1*b.e0,
                e0*b.e2 - e2*b.e0,
                e1*b.e2 - e2*b.e1
            ).reduce

        } else if (b is Point2) {
            return MVec2.new(
                0,
                b.e20*e2 - b.e01*e1,
                -b.e12*e2,
                b.e12*e1,
                0, 0, 0,
                b.e01*e2 + b.e12*e0 + b.e20*e1
            ).reduce

        } else if (b is Rotor2) {
            return MVec2.new(
                0,
                b.s*e0,
                b.s*e1 - b.e12*e2,
                b.e12*e1 + b.s*e2,
                0, 0, 0,
                b.e12*e0
            ).reduce

        } else if (b is Trans2) {
            return MVec2.new(
                0,
                -b.e01*e1 - b.e02*e2,
                -e2, e1,
                0, 0, 0,
                b.e01*e2 + e0 - b.e02*e1
            ).reduce

        // TODO: PScalar2

        } else if (b is Motor2) {
            return MVec2.new(
                0,
                b.s*e0 - b.e01*e1 - b.e02*e2,
                b.s*e1 - b.e12*e2,
                b.e12*e1 + b.s*e2,
                0, 0, 0,
                b.e01*e2 + b.e12*e0 - b.e02*e1
            ).reduce
        }

        Fiber.abort("Geometric product not supported for: %(type.name) * %(b.type.name)")
    }

    // Inner product
    |(b) {
        if (b is Num) {
            return this * b

        } else if (b is Line2) {
            return b.e1*e1 + b.e2*e2

        } else if (b is Point2) {
            return Line2.new(
                b.e20*e2 - b.e01*e1,
                -b.e12*e2,
                b.e12*e1
            )

        } else if (b is Rotor2) {
            return Line2.new(
                b.s*e0,
                b.s*e1-b.e12*e2,
                b.e12*e1+b.s*e2
            )

        } else if (b is Trans2) {
            return Line2.new(
                e0-b.e01*e1-b.e02*e2,
                e1, e2
            )
            
        } else if (b is Motor2) {
            return Line2.new(
                b.s*e0-b.e01*e1-b.e02*e2,
                b.s*e1-b.e12*e2,
                b.e12*e1+b.s*e2
            )

        // TODO: PScalar2

        } else if (b is MVec2) {
            return MVec2.new(
                b.e1*e1+b.e2*e2,
                b.s*e0-b.e01*e1-b.e02*e2,
                b.s*e1-b.e12*e2,
                b.e12*e1+b.s*e2,
                b.e012*e2,
                -b.e012*e1,
                0, 0
            ).reduce
        }

        Fiber.abort("Inner product not supported for: %(type.name) * %(b.type.name)")
    }

	// Outer product
	^(b) {
        if (b is Num) {
            return this * b

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
            ).reduce

        } else if (b is Rotor2) {
            return MVec2.new(
                0, b.s*e0, b.s*e1, b.s*e2,
                0, 0, 0, b.e12*e0
            ).reduce

        } else if (b is Trans2) {
            return MVec2.new(
                0, e0, e1, e2,
                0, 0, 0, b.e01*e2-b.e02*e1
            ).reduce

        } else if (b is Motor2) {
            return MVec2.new(
                0, b.s*e0, b.s*e1, b.s*e2,
                0, 0, 0, b.e01*e2+b.e12*e0-b.e02*e1
            ).reduce

        // TODO: PScalar2

        } else if (b is MVec2) {
            return MVec2.new(
                0, b.s*e0, b.s*e1, b.s*e2,
                b.e1*e0-b.e0*e1,
                b.e2*e0-b.e0*e2,
                b.e2*e1-b.e1*e2,
                b.e01*e2+b.e12*e0-b.e02*e1
            ).reduce
        }

        Fiber.abort("Outer product not supported for: %(type.name) * %(b.type.name)")
    }

	// Sandwich product
    >>(b) {
        if (b is Num) {
            return b

        } else if (b is Line2) {
            var e1e1 = e1*e1
            var e2e2 = e2*e2
            var e1e2_2 = e1*e2*2
            var e0_2 = e0*2
            return Line2.new(
                e0_2*(b.e1*e1 + b.e2*e2) - b.e0*(e1e1 + e2e2),
                b.e1*e1e1 + b.e2*e1e2_2 - b.e1*e2e2,
                b.e1*e1e2_2 + b.e2*e2e2 - b.e2*e1e1
            )

        } else if (b is Point2) {
            // Huh, codegen broke for this? Had to flip the sign of e01 instead of e20
            return Point2.new(
                b.e20*e2*e2-2*b.e01*e1*e2-2*b.e12*e0*e1-b.e20*e1*e1,
                -(b.e01*e2*e2+2*b.e12*e0*e2+2*b.e20*e1*e2-b.e01*e1*e1),
                -b.e12*e1*e1-b.e12*e2*e2
            )

        } else if (b is Rotor2) {
            return Motor2.new(
                b.s*e1*e1+b.s*e2*e2,
                2*b.e12*e0*e2,
                -2*b.e12*e0*e1,
                -b.e12*e1*e1-b.e12*e2*e2
            ).reduce

        } else if (b is Trans2) {
            return Trans2.new(
                b.e01*e2*e2-b.e01*e1*e1-2*b.e02*e1*e2,
                b.e02*e1*e1-2*b.e01*e1*e2-b.e02*e2*e2,
                e1*e1+e2*e2
            )

        } else if (b is Motor2) {
            return Motor2.new(
                b.s*e1*e1+b.s*e2*e2,
                b.e01*e2*e2+2*b.e12*e0*e2-b.e01*e1*e1-2*b.e02*e1*e2,
                b.e02*e1*e1-2*b.e01*e1*e2-b.e02*e2*e2-2*b.e12*e0*e1,
                -b.e12*e1*e1-b.e12*e2*e2
            ).reduce

        // TODO: PScalar2

        } else if (b is MVec2) {
            return MVec2.new(
                b.s*e1*e1+b.s*e2*e2,
                2*b.e1*e0*e1+2*b.e2*e0*e2-b.e0*e1*e1-b.e0*e2*e2,
                b.e1*e1*e1+2*b.e2*e1*e2-b.e1*e2*e2,
                2*b.e1*e1*e2+b.e2*e2*e2-b.e2*e1*e1,
                b.e01*e2*e2+2*b.e12*e0*e2-b.e01*e1*e1-2*b.e02*e1*e2,
                b.e02*e1*e1-2*b.e01*e1*e2-b.e02*e2*e2-2*b.e12*e0*e1,
                -b.e12*e1*e1-b.e12*e2*e2,
                b.e012*e1*e1+b.e012*e2*e2
            ).reduce
        }

        Fiber.abort("Sandwich product not supported for: %(type.name) * %(b.type.name)")
    }

	// Left contraction
    <<(b) {
        if (b is Num) {
            return 0

        } else if (b is Line2) {
            return b.e1*e1+b.e2*e2

        } else if (b is Point2) {
            return Line2.new(
                b.e20*e2-b.e01*e1,
                -b.e12*e2,
                b.e12*e1
            )

        } else if (b is Rotor2) {
            return Line2.new(
                0,
                -b.e12*e2,
                b.e12*e1
            )

        } else if (b is Trans2) {
            return Line2.new(
                -b.e01*e1-b.e02*e2,
                0, 0
            )

        } else if (b is Motor2) {
            return Line2.new(
                -b.e01*e1-b.e02*e2,
                -b.e12*e2,
                b.e12*e1
            )

        // TODO: PScalar2

        } else if (b is MVec2) {
            return MVec2.new(
                b.e1*e1+b.e2*e2,
                -b.e01*e1-b.e02*e2,
                -b.e12*e2,
                b.e12*e1,
                b.e012*e2,
                -b.e012*e1,
                0, 0
            ).reduce
        }

        Fiber.abort("Left contraction not supported for: %(type.name) * %(b.type.name)")
    }

	// Regressive product
    &(b) { !(!this ^ !b) }

	// Reverse operator
    ~ { Line2.new(e0, e1, e2) }

    // Negate operator
    - { Line2.new(-e0, -e1, -e2) }

	// Dual operator
    ! { Point2.new(e1, e2, e0) }

	// Grade selection
    grade(i) { mvec.grade(i) }

	// Normalization
    norm_sq { e1*e1 + e2*e2 }
    normalized {
        var n = this.norm_sq
        if (n == 0) {
            return Line2.new(0, 0, 0)
        } else {
            n = 1 / n.sqrt
            return this * n
        }
    }

    // Projection
    project(b) { (this | b) | b }

    // Utility
    copy { Line2.new(e0, e1, e2) }
    mvec { MVec2.new(0, e0, e1, e2, 0, 0, 0, 0) }

    conjugate { -this }
    involute { -this }

    // Equality
    ==(b) { mvec == b }
    !=(b) { !(this == b) }

    glSetUniform(name) {
        App.glSetUniform(name)
        App.glSetVec3f(e0, e1, e2)
    }

    // Debug
    glDraw(color) {
        App.glBegin(true, true, 1, 2)
        var n = 1e3 // Large number simulate infinity
        var d = (this ^ Line2.new(1, 0, 0)) * n
        var p = Point2.new(0, 0, 1).project(this).normalized
        App.glAddVertex(p.x - d.x, p.y - d.y, 0, color)
        App.glAddVertex(p.x + d.x, p.y + d.y, 0, color)
        App.glEnd(App.glLines)
    }

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

    toString { "[%(e0)e0 + %(e1)e1 + %(e2)e2]" }
}

// The point class represents the vector: Ae₀₂ + Be₀₁ + Ce₁₂
class Point2 {
    construct new(e20, e01, e12) {
        _e20 = e20
        _e01 = e01
        _e12 = e12
    }

    // GA notation
    e20 { _e20 }
    e01 { _e01 }
    e12 { _e12 }
    e20=(v) { _e20 = v }
    e01=(v) { _e01 = v }
    e12=(v) { _e12 = v }

    // Classic notation
    x { _e20 }
    y { _e01 }
    w { _e12 }
    x=(v) { _e20 = v }
    y=(v) { _e01 = v }
    w=(v) { _e12 = v }

    // Addition
    +(b) {
        if (b is Num) {
            return Motor2.new(b, e01, -e20, e12).reduce

        } else if (b is Point2) {
            return Point2.new(e20 + b.e20, e01 + b.e01, e12 + b.e12)

        } else if (b is Line2) {
            return MVec2.new(
                0, b.e0, b.e1, b.e2,
                e01, -e20, e12, 0
            ).reduce

        } else if (b is Rotor2) {
            return Motor2.new(
                b.s, e01, -e20, e12+b.e12
            ).reduce

        } else if (b is Trans2) {
            return Motor2.new(
                1, e01+b.e01, -e20+b.e02, e12
            ).reduce

        } else if (b is Motor2) {
            return Motor2.new(
                1, e01+b.e01, -e20+b.e02, e12+b.e12
            ).reduce

        // TODO: PScalar2

        } else if (b is MVec2) {
            return MVec2.new(
                b.s, b.e0, b.e1, b.e2, e01+b.e01, -e02+b.e20, e12+b.e12, b.e012
            ).reduce
        }
    }

    // Subtraction
    -(b) {
        if (b is Num) {
            return Motor2.new(-b, e01, -e20, e12).reduce

        } else if (b is Point2) {
            return Point2.new(e20 - b.e20, e01 - b.e01, e12 - b.e12)

        } else if (b is Line2) {
            return MVec2.new(
                0, -b.e0, -b.e1, -b.e2,
                e01, -e20, e12, 0
            ).reduce

        } else if (b is Rotor2) {
            return Motor2.new(
                -b.s, e01, -e20, e12-b.e12
            ).reduce

        } else if (b is Trans2) {
            return Motor2.new(
                1, e01-b.e01, -e20-b.e02, e12
            ).reduce

        } else if (b is Motor2) {
            return Motor2.new(
                1, e01-b.e01, -e20-b.e02, e12-b.e12
            ).reduce

        // TODO: PScalar2

        } else if (b is MVec2) {
            return MVec2.new(
                -b.s, -b.e0, -b.e1, -b.e2, e01-b.e01, -e20-b.e02, e12-b.e12, -b.e012
            ).reduce
        }
    }

    // Geometric product
    *(b) {
        if (b is Num) {
            return Point2.new(
                e20 * b, e01 * b, e12 * b
            )

        } else if (b is Point2) {
            return Motor2.new(
                -b.e12*e12,
                b.e12*e20-b.e20*e12,
                b.e12*e01-b.e01*e12,
                0
            ).reduce

        } else if (b is Line2) {
            return MVec2.new(
                0,
                b.e1*e01 - b.e2*e20,
                b.e2*e12,
                -b.e1*e12,
                0, 0, 0,
                b.e0*e12 + b.e1*e20 + b.e2*e01
            ).reduce

        } else if (b is Rotor2) {
            return Motor2.new(
                -b.e12*e12,
                b.e12*e20 + b.s*e01,
                b.e12*e01 - b.s*e20,
                b.s*e12
            ).reduce

        } else if (b is Trans2) {
            return Motor2.new(
                -e12,
                b.e02*e12 + e20,
                e01 - b.e01*e12,
                0
            ).reduce

        } else if (b is Motor2) {
            return Motor2.new(
                -b.e12*e12,
                b.e02*e12+b.e12*e20+b.s*e01,
                b.e12*e01-b.e01*e12-b.s*e20,
                b.s*e12
            ).reduce

        // TODO: PScalar2

        } else if (b is MVec2) {
            return MVec2.new(
                -b.e12*e12,
                b.e1*e01-b.e012*e12-b.e2*e20,
                b.e2*e12,
                -b.e1*e12,
                b.e02*e12+b.e12*e20+b.s*e01,
                b.e12*e01-b.e01*e12-b.s*e20,
                b.s*e12,
                b.e0*e12+b.e1*e20+b.e2*e01
            ).reduce
        }

        Fiber.abort("Geometric product not supported for: %(type.name) * %(b.type.name)")
    }

    // Inner product
    |(b) {
        if (b is Num) {
            return this * b

        } else if (b is Point2) {
            return -b.e12*e12

        } else if (b is Line2) {
            return Line2.new(
                b.e1*e01 - b.e2*e20,
                b.e2*e12,
                -b.e1*e12
            )

        } else if (b is Rotor2) {
            return Motor2.new(
                -b.e12*e12,
                b.s*e01,
                -b.s*e20,
                b.s*e12
            ).reduce

        } else if (b is Trans2) {
            return this.copy

        } else if (b is Motor2) {
            return Motor2.new(
                -b.e12*e12,
                b.s*e01,
                -b.s*e20,
                b.s*e12
            ).reduce

        // TODO: PScalar2

        } else if (b is MVec2) {
            return Motor2.new(
                -b.e12*e12,
                b.e1*e01-b.e012*e12-b.e2*e20,
                b.e2*e12,
                -b.e1*e12,
                b.s*e01,
                -b.s*e20,
                b.s*e12,
                0
            ).reduce
        }
        
        Fiber.abort("Inner product not supported for: %(type.name) * %(b.type.name)")
    }
	
    // Outer product
    ^(b) {
        if (b is Num) {
            return this * b

        } else if (b is Point2) {
            return 0

        } else if (b is Line2) {
            return MVec2.new(
                0, 0, 0, 0, 0, 0, 0,
                b.e0*e12 + b.e1*e20 + b.e2*e01
            ).reduce

        } else if (b is Rotor2) {
            return this * b.s

        } else if (b is Trans2) {
            return this.copy

        } else if (b is Motor2) {
            return this * b.s

        // TODO: PScalar2

        } else if (b is MVec2) {
            return MVec2.new(
                0, 0, 0, 0,
                b.s*e01,
                -b.s*e20,
                b.s*e12,
                b.e0*e12+b.e1*e20+b.e2*e01
            ).reduce
        }
        
        Fiber.abort("Inner product not supported for: %(type.name) * %(b.type.name)")
    }

	// Sandwich product
    >>(b) {
        if (b is Num) {
            return b*e12*e12

        } else if (b is Point2) {
            return Point2.new(
                -(b.e20*e12*e12-2*b.e12*e12*e20),
                2*b.e12*e01*e12-b.e01*e12*e12,
                b.e12*e12*e12
            )

        } else if (b is Line2) {
            return Line2.new(
                b.e0*e12*e12+2*b.e1*e12*e20+2*b.e2*e01*e12,
                -b.e1*e12*e12,
                -b.e2*e12*e12
            )

        } else if (b is Rotor2) {
            return Motor2.new(
                b.s*e12*e12,
                2*b.e12*e01*e12,
                -2*b.e12*e12*e20,
                b.e12*e12*e12
            ).reduce

        } else if (b is Trans2) {
            return Trans2.new(
                -b.e01*e12*e12,
                -b.e02*e12*e12,
                e12*e12
            )

        } else if (b is Motor2) {
            return Motor2.new(
                b.s*e12*e12,
                2*b.e12*e01*e12-b.e01*e12*e12,
                -b.e02*e12*e12-2*b.e12*e12*e20,
                b.e12*e12*e12
            ).reduce

        // TODO: PScalar2

        } else if (b is MVec2) {
            return MVec2.new(
                b.s*e12*e12,
                b.e0*e12*e12+2*b.e1*e12*e20+2*b.e2*e01*e12,
                -b.e1*e12*e12,
                -b.e2*e12*e12,
                2*b.e12*e01*e12-b.e01*e12*e12,
                -b.e02*e12*e12-2*b.e12*e12*e20,
                b.e12*e12*e12,
                b.e012*e12*e12
            ).reduce
        }

        Fiber.abort("Sandwitch product not supported for: %(type.name) * %(b.type.name)")
    }

	// Left contraction
    <<(b) {
        if (b is Num) {
            return 0

        } else if (Point2) {
            return -b.e12*e12

        } else if (Line2) {
            return -b.e12*e12

        } else if (Rotor2) {
            return -b.e12*e12
            
        } else if (Trans2) {
            return 0
            
        } else if (Motor2) {
            return -b.e12*e12

        // TODO: PScalar2
            
        } else if (MVec2) {
            return MVec2.new(-b.e12*e12, -b.e012*e12, 0, 0, 0, 0, 0, 0)
        }

        Fiber.abort("Left contraction not supported for: %(type.name) * %(b.type.name)")
    }

	// Regressive product
    &(b) { !(!this ^ !b) }

	// Reverse operator
    ~ { Point2.new(-e20, -e01, -e12) }

    // Negate operator
    - { Point2.new(-e20, -e01, -e12) }

	// Dual operator
    ! { Line2.new(e12, e20, e01) }

	// Grade selection
	grade(i) { mvec.grade(i) }

    // Normalization
    norm_sq { e12 == 0 ? e20*e20 + e01*e01 : e12*e12 }
    normalized {
        var n = this.norm_sq
        if (n == 0) {
            return Point2.new(0, 0, 0)
        } else {
            n = 1 / n.sqrt
            return this * n
        }
    }

	// Exponential
    exp(b) {
        var a = b * 0.5
        if (e12 != 0) {
            var s = a.sin
            return Motor2.new(a.cos, s * e01, s * -e20, s * e12).reduce
        }
        return Trans2.new(a * e01, a * -e20)
    }

    // Projection
    project(b) { (this | b) ^ b }

    // Utility
    copy { Point2.new(e20, e01, e12) }
    mvec { MVec2.new(0, 0, 0, 0, e01, -e20, e12, 0) }

    conjugate { -this }
    involute { this.copy }

    // Equality
    ==(b) { mvec == b }
    !=(b) { !(this == b) }

    glSetUniform(name) {
        App.glSetUniform(name)
        App.glSetVec3f(e20, e01, e12)
    }

    // Debug
    glDraw(color) {
        //var n = e12 != 0 ? 1 / e12 : 1
        //App.guiAbsText(toString, 100, 100, 0xFFFFFFFF)

        App.glBegin(true, true, 10, 2)
        App.glAddVertex(e20, e01, 0, e12, color, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
        App.glEnd(App.glPoints)
    }

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

    toString { "[%(e20)e20 + %(e01)e01 + %(e12)e12]" }
}

// The rotor class represents the vector: A + Be₁₂
class Rotor2 {
    construct new(s, e12) {
        _s = s
        _e12 = e12
    }

    // GA notation
    s { _s }
    e12 { _e12 }
    s=(v) { _s = v }
    e12=(v) { _e12 = v }

    // Classic notation
    w { _s }
    z { _e12 }
    w=(v) { _s = v }
    z=(v) { _e12 = v }

    // Addition
    +(b) {
        if (b is Num) {
            return Rotor2.new(s+b, e12)
            
        } else if (b is Rotor2) {
            return Rotor2.new(s+b.s, e12+b.e12)
            
        } else if (b is Line2) {
            return MVec2.new(s, b.e0, b.e1, b.e2, 0, 0, e12, 0).reduce
            
        } else if (b is Point2) {
            return Motor2.new(s, b.e01, -b.e20, e12+b.e12).reduce
            
        } else if (b is Trans2) {
            return Motor2.new(s+1, b.e01, b.e02, e12).reduce
            
        } else if (b is Motor2) {
            return Motor2.new(s+b.s, b.e01, b.e02, e12+b.e12).reduce

        // TODO: PScalar2
            
        } else if (b is MVec2) {
            return MVec2.new(s+b.s, b.e0, b.e1, b.e2, b.e01, b.e02, e12+b.e12, b.e012).reduce
        }

        Fiber.abort("Addition not supported for: %(type.name) * %(b.type.name)")
    }

    // Subtraction
    -(b) {
        if (b is Num) {
            return Rotor2.new(s-b, e12)
            
        } else if (b is Rotor2) {
            return Rotor2.new(s-b.s, e12-b.e12)
            
        } else if (b is Line2) {
            return MVec2.new(s, -b.e0, -b.e1, -b.e2, 0, 0, e12, 0).reduce
            
        } else if (b is Point2) {
            return Motor2.new(s, -b.e01, b.e20, e12-b.e12).reduce
            
        } else if (b is Trans2) {
            return Motor2.new(s-1, -b.e01, -b.e02, e12).reduce
            
        } else if (b is Motor2) {
            return Motor2.new(s-b.s, -b.e01, -b.e02, e12-b.e12).reduce

        // TODO: PScalar2
            
        } else if (b is MVec2) {
            return MVec2.new(s-b.s, -b.e0, -b.e1, -b.e2, -b.e01, -b.e02, e12-b.e12, -b.e012).reduce
        }

        Fiber.abort("Subtraction not supported for: %(type.name) * %(b.type.name)")
    }

    // Geometric product
    *(b) {
        if (b is Num) {
            return Rotor2.new(
                s * b, e12 * b
            )

        } else if (b is Rotor2) {
            return Rotor2.new(
                b.s*s-b.e12*e12,
                b.e12*s+b.s*e12
            )

        } else if (b is Line2) {
            return MVec2.new(
                0,
                b.e0*s,
                b.e1*s+b.e2*e12,
                b.e2*s-b.e1*e12,
                0, 0, 0,
                b.e0*e12
            ).reduce

        } else if (b is Point2) {
            return Motor2.new(
                -b.e12*e12,
                b.e01*s-b.e20*e12,
                -b.e01*e12-b.e20*s,
                b.e12*s
            ).reduce

        } else if (b is Trans2) {
            return Motor2.new(
                s,
                b.e01*s+b.e02*e12,
                b.e02*s-b.e01*e12,
                e12
            ).reduce

        } else if (b is Motor2) {
            return Motor2.new(
                b.s*s-b.e12*e12,
                b.e01*s+b.e02*e12,
                b.e02*s-b.e01*e12,
                b.e12*s+b.s*e12
            ).reduce

        // TODO: PScalar2

        } else if (b is MVec2) {
            return MVec2.new(
                b.s*s-b.e12*e12,
                b.e0*s-b.e012*e12,
                b.e1*s+b.e2*e12,
                b.e2*s-b.e1*e12,
                b.e01*s+b.e02*e12,
                b.e02*s-b.e01*e12,
                b.e12*s+b.s*e12,
                b.e0*e12+b.e012*s
            ).reduce
        }

        Fiber.abort("Geometric product not supported for: %(type.name) * %(b.type.name)")
    }

    // Inner product
    |(b) {
        if (b is Num) {
            return this * b

        } else if (b is Rotor2) {
            return Rotor2.new(
                b.s*s-b.e12*e12,
                b.e12*s+b.s*e12
            )

        } else if (b is Line2) {
            return Line2.new(
                b.e0*s,
                b.e1*s+b.e2*e12,
                b.e2*s-b.e1*e12
            )

        } else if (b is Point2) {
            return Motor2.new(
                -b.e12*e12,
                b.e01*s,
                -b.e20*s,
                b.e12*s
            ).reduce

        } else if (b is Trans2) {
            return Motor2.new(
                s,
                b.e01*s,
                b.e02*s,
                e12
            ).reduce

        } else if (b is Motor2) {
            return Motor2.new(
                b.s*s-b.e12*e12,
                b.e01*s,
                b.e02*s,
                b.e12*s+b.s*e12
            ).reduce

        // TODO: PScalar2

        } else if (b is MVec2) {
            return MVec2.new(
                b.s*s-b.e12*e12,
                b.e0*s-b.e012*e12,
                b.e1*s+b.e2*e12,
                b.e2*s-b.e1*e12,
                b.e01*s,
                b.e02*s,
                b.e12*s+b.s*e12,
                b.e012*s
            ).reduce
        }

        Fiber.abort("Inner product not supported for: %(type.name) * %(b.type.name)")
    }

    // Outer product
    ^(b) {
        if (b is Num) {
            return this * b

        } else if (b is Rotor2) {
            return Rotor2.new(
                b.s*s,
                b.e12*s+b.s*e12
            )

        } else if (b is Point2) {
            return Point2.new(
                b.e20*s,
                b.e01*s,
                b.e12*s
            )

        } else if (b is Trans2) {
            return Motor2.new(
                s,
                b.e01*s,
                b.e02*s,
                e12
            ).reduce

        } else if (b is Motor2) {
            return Motor2.new(
                b.s*s,
                b.e01*s,
                b.e02*s,
                b.e12*s+b.s*e12
            ).reduce

        // TODO: PScalar2

        } else if (b is MVec2) {
            return MVec2.new(
                b.s*s,
                b.e0*s,
                b.e1*s,
                b.e2*s,
                b.e01*s,
                b.e02*s,
                b.e12*s+b.s*e12,
                b.e0*e12+b.e012*s
            ).reduce
        }

        Fiber.abort("Outer product not supported for: %(type.name) * %(b.type.name)")
    }

    // Sandwich product
    >>(b) {
        if (b is Num) {
            return b.s*e12*e12+b.s*s*s

        } else if (b is Rotor2) {
            return Rotor2.new(
                b.s*e12*e12+b.s*s*s,
                b.e12*e12*e12+b.e12*s*s
            )

        } else if (b is Line2) {
            return Line2.new(
                b.e0*e12*e12+b.e0*s*s,
                b.e1*s*s+2*b.e2*e12*s-b.e1*e12*e12,
                b.e2*s*s-2*b.e1*e12*s-b.e2*e12*e12
            )

		} else if (b is Point2) {
            return Point2.new(
                -(b.e20*e12*e12-2*b.e01*e12*s-b.e20*s*s),
                b.e01*s*s-b.e01*e12*e12-2*b.e20*e12*s,
                b.e12*e12*e12+b.e12*s*s
            )

		} else if (b is Trans2) {
            return Trans2.new(
                b.e01*s*s+2*b.e02*e12*s-b.e01*e12*e12,
                b.e02*s*s-2*b.e01*e12*s-b.e02*e12*e12,
                e12*e12+s*s
            )

        } else if (b is Motor2) {
            return Motor2.new(
                b.s*e12*e12+b.s*s*s,
                b.e01*s*s+2*b.e02*e12*s-b.e01*e12*e12,
                b.e02*s*s-2*b.e01*e12*s-b.e02*e12*e12,
                b.e12*e12*e12+b.e12*s*s
            ).reduce

        // TODO: PScalar2

        } else if (b is MVec2) {
            return MVec2.new(
                b.s*e12*e12+b.s*s*s,
                b.e0*e12*e12+b.e0*s*s,
                b.e1*s*s+2*b.e2*e12*s-b.e1*e12*e12,
                b.e2*s*s-2*b.e1*e12*s-b.e2*e12*e12,
                b.e01*s*s+2*b.e02*e12*s-b.e01*e12*e12,
                b.e02*s*s-2*b.e01*e12*s-b.e02*e12*e12,
                b.e12*e12*e12+b.e12*s*s,
                b.e012*e12*e12+b.e012*s*s
            ).reduce
        }

        Fiber.abort("Sandwich product not supported for: %(type.name) * %(b.type.name)")
	}

    // Left contraction
    <<(b) {
        if (b is Num) {
            return b.s*s

        } else if (b is Rotor2) {
            return Rotor2.new(
                b.s*s-b.e12*e12,
                b.e12*s
            )

        } else if (b is Line2) {
            return b * s

		} else if (b is Point2) {
            return Motor2.new(
                -b.e12*e12,
                b.e01*s,
                -b.e20*s,
                b.e12*s
            ).reduce

		} else if (b is Trans2) {
            return Trans2.new(
                b.e01*s,
                b.e02*s,
                s
            )

        } else if (b is Motor2) {
            return Motor2.new(
                b.s*s-b.e12*e12,
                b.e01*s,
                b.e02*s,
                b.e12*s
            ).reduce

        // TODO: PScalar2

        } else if (b is MVec2) {
            return MVec2.new(
                b.s*s-b.e12*e12,
                b.e0*s-b.e012*e12,
                b.e1*s,
                b.e2*s,
                b.e01*s,
                b.e02*s,
                b.e12*s,
                b.e012*s
            ).reduce
        }

        Fiber.abort("Left contraction not supported for: %(type.name) * %(b.type.name)")
    }

    // Regressive product
    &(b) { !(!this ^ !b) }

    // Reverse operator
    ~ { Rotor2.new(s, -e12) }

    // Negate operator
    - { Rotor2.new(-s, -e12) }

    // Dual operator
    ! { MVec2.new(0, e12, 0, 0, 0, 0, 0, s) }

    // Grade selection
	grade(i) { mvec.grade(i) }

    // Normalization
    norm_sq { s*s + e12*e12 }
    normalized {
        var n = this.norm_sq
        if (n == 0) {
            return Rotor2.new(0, 0)
        } else {
            n = 1 / n.sqrt
            return this * n
        }
    }

    // Exponential
    exp { Rotor2.new(e12.cos, e12.sin) * s.exp }

    // Logarithm
    log {} // TOOD

    // Utility
    copy { Rotor2.new(s, e12) }
    mvec { MVec2.new(s, 0, 0, 0, 0, 0, e12, 0) }

    conjugate { ~this }
    involute { this.copy }

    // Equality
    ==(b) { mvec == b }
    !=(b) { !(this == b) }

    glSetUniform(name) {
        App.glSetUniform(name)
        App.glSetVec2f(s, e12)
    }

    // Debug
    glDraw(color) {
        Line2.new(0, 1, 0).glDraw(color)
        Line2.new(0, s, e12).glDraw(color)
    }

    guiInspect(name) {
        App.guiPushItemWidth(App.guiContentAvailWidth() / 3)
        App.guiText("%(name): ")
        App.guiSameLine()
        e20 = App.guiFloat("s##%(name)", s)
        App.guiSameLine()
        e12 = App.guiFloat("e12##%(name)", e12)
        App.guiPopItemWidth()
    }

    toString { "[%(s) + %(e12)e12]" }
}

// The trans(lator) class represents the vector: 1 + Ae₀₁ + Be₀₂
class Trans2 {
    construct new(e01, e02) {
        _e01 = e01
        _e02 = e02
    }

    construct new(e01, e02, s) {
        if (s == 0) {
            Fiber.abort("Attempting to create a translator with 0 scalar!")
        }
        s = s == 1 ? 1 : 1 / s
        _e01 = e01 * s
        _e02 = e02 * s
    }

    // GA notation
    e01 { _e01 }
    e02 { _e02 }
    e01=(v) { _e01 = v }
    e02=(v) { _e02 = v }

    // Classic notation
    dx { _e01 }
    dy { _e02 }
    dx=(v) { _e01 = v }
    dy=(v) { _e02 = v }

    // Geometric product
    *(b) {
        if (b is Num) {
            return Trans2.new(
                e01 * b, e02 * b, b
            )

        } else if (b is Trans2) {
            return Trans2.new(
                b.e01+e01,
                b.e02+e02
            )

        } else if (b is Line2) {
            return MVec2.new(
                0,
                b.e0+b.e1*e01+b.e2*e02,
                b.e1, b.e2,
                0, 0, 0,
                b.e2*e01-b.e1*e02
            ).reduce

        } else if (b is Point2) {
            return Point2.new(
                -(b.e12*e01-b.e20),
                b.e01-b.e12*e02,
                b.e12
            )

        } else if (b is Rotor2) {
            return Motor2.new(
                b.s,
                b.s*e01-b.e12*e02,
                b.e12*e01+b.s*e02,
                b.e12
            ).reduce

        } else if (b is Motor2) {
            return Motor2.new(
                b.s,
                b.e01+b.s*e01 - b.e12*e02,
                b.e02+b.e12*e01 + b.s*e02,
                b.e12
            ).reduce

        // TODO: PScalar2

        } else if (b is MVec2) {
            return MVec2.new(
                b.s,
                b.e0+b.e1*e01+b.e2*e02,
                b.e1,
                b.e2,
                b.e01+b.s*e01-b.e12*e02,
                b.e02+b.e12*e01+b.s*e02,
                b.e12,
                b.e012+b.e2*e01-b.e1*e02
            ).reduce
        }

        Fiber.abort("Geometric product not supported for: %(type.name) * %(b.type.name)")
    }

    // Inner product
    |(b) {
        if (b is Num) {
            return this * b

        } else if (b is Trans2) {
            return Trans2.new(
                b.e01+e01,
                b.e02+e02
            )

        } else if (b is Line2) {
            return Line2.new(
                b.e0+b.e1*e01+b.e2*e02,
                b.e1, b.e2
            )

        } else if (b is Point2) {
            return b.copy

        } else if (b is Rotor2) {
            return Motor2.new(
                b.s,
                b.s*e01,
                b.s*e02,
                b.e12
            ).reduce

        } else if (b is Motor2) {
            return Motor2.new(
                b.s,
                b.e01+b.s*e01,
                b.e02+b.s*e02,
                b.e12
            ).reduce

        // TODO: PScalar2

        } else if (b is MVec2) {
            return MVec2.new(
                b.s,
                b.e0+b.e1*e01+b.e2*e02,
                b.e1,
                b.e2,
                b.e01+b.s*e01,
                b.e02+b.s*e02,
                b.e12,
                b.e012
            ).reduce
        }

        Fiber.abort("Inner product not supported for: %(type.name) * %(b.type.name)")
    }

    // Outer product
    ^(b) {
        if (b is Num) {
            return this * b

        } else if (b is Trans2) {
            return Trans2.new(
                b.e01+e01,
                b.e02+e02
            )

        } else if (b is Line2) {
            return MVec2.new(
                0,
                b.e0, b.e1, b.e2,
                0, 0, 0,
                b.e2*e01-b.e1*e02
            ).reduce

        } else if (b is Point2) {
            return b.copy

        } else if (b is Rotor2) {
            return Motor2.new(
                b.s,
                b.s*e01,
                b.s*e02,
                b.e12
            ).reduce

        } else if (b is Motor2) {
            return Motor2.new(
                b.s,
                b.e01+b.s*e01,
                b.e02+b.s*e02,
                b.e12
            ).reduce

        // TODO: PScalar2

        } else if (b is MVec2) {
            return MVec2.new(
                b.s, b.e0, b.e1, b.e2,
                b.e01+b.s*e01,
                b.e02+b.s*e02,
                b.e12,
                b.e012+b.e2*e01-b.e1*e02
            ).reduce
        }

        Fiber.abort("Outer product not supported for: %(type.name) * %(b.type.name)")
    }

    // Sandwich product
    >>(b) {
        if (b is Num) {
            return b

        } else if (b is Trans2) {
            return b.copy

        } else if (b is Line2) {
            return Line2.new(
                b.e0+2*b.e1*e01+2*b.e2*e02,
                b.e1, b.e2
            )

        } else if (b is Point2) {
            return Point2.new(
                -(2*b.e12*e01-b.e20),
                b.e01-2*b.e12*e02,
                b.e12
            )

        } else if (b is Rotor2) {
            return Motor2.new(
                b.s,
                -2*b.e12*e02,
                2*b.e12*e01,
                b.e12
            ).reduce

        } else if (b is Motor2) {
            return Motor2.new(
                b.s,
                b.e01-2*b.e12*e02,
                b.e02+2*b.e12*e01,
                b.e12
            ).reduce

        // TODO: PScalar2

        } else if (b is MVec2) {
            return MVec2.new(
                b.s,
                b.e0+2*b.e1*e01+2*b.e2*e02,
                b.e1, b.e2,
                b.e01-2*b.e12*e02,
                b.e02+2*b.e12*e01,
                b.e12,
                b.e012
            ).reduce
        }

        Fiber.abort("Sandwich not supported for: %(type.name) * %(b.type.name)")
	}

    // Regressive product
    &(b) { !(!this ^ !b) }

    // Reverse operator
    ~ { Trans2.new(-e01, -e02) }

    // Negate operator
    - { Trans2.new(-e01, -e02) }

    // Dual operator
    ! { MVec2.new(0, 0, -e02, e01, 0, 0, 0, 1) }

    // Grade selection
	grade(i) { mvec.grade(i) }

    // Normalization (since we're not encoding the scalar our translator will always be normalized)
    /*norm_sq {}
    normalized {
        var n = this.norm_sq
        if (n == 0) {
            return Trans2.new(0, 0)
        } else {
            n = 1 / n.sqrt
            return this * n
        }
    }*/

    // Exponential
    exp {} // TOOD

    // Logarithm
    log {} // TOOD

    // Utility
    copy { Trans2.new(e01, e02) }
    mvec { MVec2.new(0, 0, 0, 0, e01, e02, 0, 0) }

    conjugate { -this }
    involute { this.copy }

    // Equality
    ==(b) { mvec == b }
    !=(b) { !(this == b) }

    glSetUniform(name) {
        App.glSetUniform(name)
        App.glSetVec2f(e01, e02)
    }

    // Debug
    glDraw(color) {
        Line2.new(0, 1, 0).glDraw(color)
        Line2.new(0, 0, 1).glDraw(color)
    }

    guiInspect(name) {
        App.guiPushItemWidth(App.guiContentAvailWidth() / 3)
        App.guiText("%(name): ")
        App.guiSameLine()
        e20 = App.guiFloat("e01##%(name)", e01)
        App.guiSameLine()
        e12 = App.guiFloat("e02##%(name)", e02)
        App.guiPopItemWidth()
    }

    toString { "[1 + %(e01)e01 + %(e02)e02]" }
}

// The motor class represents the vector: A + Be₀₁ + Ce₀₂ + De₁₂
class Motor2 {
    construct new(s, e01, e02, e12) {
        _s = s
        _e01 = e01
        _e02 = e02
        _e12 = e12
    }

    // GA notation
    s { _s }
    e01 { _e01 }
    e02 { _e02 }
    e12 { _e12 }
    s=(v) { _s = v }
    e01=(v) { _e01 = v }
    e02=(v) { _e02 = v }
    e12=(v) { _e12 = v }

    // Classic notation
    w { _s }
    dx { _e01 }
    dy { _e02 }
    z { _e12 }
    w=(v) { _s = v }
    dx=(v) { _e01 = v }
    dy=(v) { _e02 = v }
    z=(v) { _e12 = v }

    // Geometric product
    *(b) {
        if (b is Num) {
            return Motor2.new(
                s * b, e12 * b,
                e20 * b, e01 * b
            )

        } else if (b is Motor2) {
            // 12 mul 8 add
            return Motor2.new(
                b.s*s-b.e12*e12,
                b.e01*s+b.e02*e12+b.s*e01-b.e12*e02,
                b.e02*s+b.e12*e01+b.s*e02-b.e01*e12,
                b.e12*s+b.s*e12
            ).reduce

        } else if (b is Line2) {
            return MVec2.new(
                0,
                b.e0*s+b.e1*e01+b.e2*e02,
                b.e1*s+b.e2*e12,
                b.e2*s-b.e1*e12,
                0, 0, 0,
                b.e0*e12+b.e2*e01-b.e1*e02
            ).reduce

        } else if (b is Point2) {
            return Motor2.new(
                -b.e12*e12,
                b.e01*s-b.e12*e02-b.e20*e12,
                b.e12*e01-b.e01*e12-b.e20*s,
                b.e12*s
            ).reduce

        } else if (b is Rotor2) {
            return Motor2.new(
                b.s*s-b.e12*e12,
                b.s*e01-b.e12*e02,
                b.e12*e01+b.s*e02,
                b.e12*s+b.s*e12
            ).reduce

        } else if (b is Trans2) {
            return Motor2.new(
                s,
                b.e01*s+b.e02*e12+e01,
                b.e02*s+e02-b.e01*e12,
                e12
            ).reduce

        // TODO: PScalar2

        } else if (b is MVec2) {
            return MVec2.new(
                b.s*s-b.e12*e12,
                b.e0*s+b.e1*e01+b.e2*e02-b.e012*e12,
                b.e1*s+b.e2*e12,
                b.e2*s-b.e1*e12,
                b.e01*s+b.e02*e12+b.s*e01-b.e12*e02,
                b.e02*s+b.e12*e01+b.s*e02-b.e01*e12,
                b.e12*s+b.s*e12,
                b.e0*e12+b.e012*s+b.e2*e01-b.e1*e02
            ).reduce
        }

        Fiber.abort("Geometric product not supported for: %(type.name) * %(b.type.name)")
    }

    // Inner product
    |(b) {
        if (b is Num) {
            return this * b

        } else if (b is Motor2) {
            return Motor2.new(
                b.s*s-b.e12*e12,
                b.e01*s+b.s*e01,
                b.e02*s+b.s*e02,
                b.e12*s+b.s*e12
            ).reduce

        } else if (b is Line2) {
            return Line2.new(
                b.e0*s+b.e1*e01+b.e2*e02,
                b.e1*s+b.e2*e12,
                b.e2*s-b.e1*e12
            )

        } else if (b is Point2) {
            return Motor2.new(
                -b.e12*e12,
                b.e01*s,
                -b.e20*s,
                b.e12*s
            ).reduce

        } else if (b is Rotor2) {
            return Motor2.new(
                b.s*s-b.e12*e12,
                b.s*e01,
                b.s*e02,
                b.e12*s+b.s*e12
            ).reduce

        } else if (b is Trans2) {
            return Motor2.new(
                s,
                b.e01*s+e01,
                b.e02*s+e02,
                e12
            ).reduce

        // TODO: PScalar2

        } else if (b is MVec2) {
            return MVec2.new(
                b.s*s-b.e12*e12,
                b.e0*s+b.e1*e01+b.e2*e02-b.e012*e12,
                b.e1*s+b.e2*e12,
                b.e2*s-b.e1*e12,
                b.e01*s+b.s*e01,
                b.e02*s+b.s*e02,
                b.e12*s+b.s*e12,
                b.e012*s
            ).reduce
        }

        Fiber.abort("Inner product not supported for: %(type.name) * %(b.type.name)")
    }
	
    // Outer product
    ^(b) {
        if (b is Num) {
            return this * b

        } else if (b is Motor2) {
            return Motor2.new(
                b.s*s,
                b.e01*s+b.s*e01,
                b.e02*s+b.s*e02,
                b.e12*s+b.s*e12
            ).reduce

        } else if (b is Line2) {
            return MVec2.new(
                0,
                b.e0*s, b.e1*s, b.e2*s,
                0, 0, 0,
                b.e0*e12+b.e2*e01-b.e1*e02
            ).reduce

        } else if (b is Point2) {
            return Point2.new(
                b.e20*s,
                b.e01*s,
                b.e12*s
            )

        } else if (b is Rotor2) {
            return Motor2.new(
                b.s*s,
                b.s*e01,
                b.s*e02,
                b.e12*s+b.s*e12
            ).reduce

        } else if (b is Trans2) {
            return Motor2.new(
                s,
                b.e01*s+e01,
                b.e02*s+e02,
                e12
            ).reduce

        // TODO: PScalar2

        } else if (b is MVec2) {
            return MVec2.new(
                b.s*s,
                b.e0*s,
                b.e1*s,
                b.e2*s,
                b.e01*s+b.s*e01,
                b.e02*s+b.s*e02,
                b.e12*s+b.s*e12,
                b.e0*e12+b.e012*s+b.e2*e01-b.e1*e02
            ).reduce
        }

        Fiber.abort("Outer product not supported for: %(type.name) * %(b.type.name)")
    }

	// Sandwich product
    >>(b) {
        if (b is Num) {
            return b*e12*e12+b*s*s

        } else if (b is Motor2) {
            return Motor2.new(
                b.s*e12*e12+b.s*s*s,
                b.e01*s*s+2*b.e02*e12*s+2*b.e12*e01*e12-b.e01*e12*e12-2*b.e12*e02*s,
                b.e02*s*s+2*b.e12*e01*s+2*b.e12*e02*e12-2*b.e01*e12*s-b.e02*e12*e12,
                b.e12*e12*e12+b.e12*s*s
            ).reduce

        } else if (b is Line2) {
            return Line2.new(
                b.e0*e12*e12+b.e0*s*s+2*b.e1*e01*s+2*b.e2*e01*e12+2*b.e2*e02*s-2*b.e1*e02*e12,
                b.e1*s*s+2*b.e2*e12*s-b.e1*e12*e12,
                b.e2*s*s-2*b.e1*e12*s-b.e2*e12*e12
            )

        } else if (b is Point2) {
            // Old: 30 mul 9 add
            // Op: 16 mul 8 add
            // Aff: 6 mul 4 add
            // Proj: 9 mul 6 add
            var s_sq = s*s
            var e12_sq = e12*e12
            var e12_s = e12*s
            var s_sq_n_e12_sq = s_sq-e12_sq
            return Point2.new(
                b.e20*s_sq_n_e12_sq + 2*(b.e01*e12_s - b.e12*(e01*s + e02*e12)),
                b.e01*s_sq_n_e12_sq - 2*(b.e20*e12_s + b.e12*(e02*s - e01*e12)),
                b.e12*(s_sq + e12_sq)
            )

        } else if (b is Rotor2) {
            return Motor2.new(
                b.s*e12*e12+b.s*s*s,
                2*b.e12*e01*e12-2*b.e12*e02*s,
                2*b.e12*e01*s+2*b.e12*e02*e12,
                b.e12*e12*e12+b.e12*s*s
            ).reduce

        } else if (b is Trans2) {
            return Trans2.new(
                b.e01*s*s+2*b.e02*e12*s-b.e01*e12*e12,
                b.e02*s*s-2*b.e01*e12*s-b.e02*e12*e12,
                e12*e12+s*s
            )

        // TODO: PScalar2

        } else if (b is MVec2) {
            return MVec2.new(
                b.s*e12*e12+b.s*s*s,
                b.e0*e12*e12+b.e0*s*s+2*b.e1*e01*s+2*b.e2*e01*e12+2*b.e2*e02*s-2*b.e1*e02*e12,
                b.e1*s*s+2*b.e2*e12*s-b.e1*e12*e12,
                b.e2*s*s-2*b.e1*e12*s-b.e2*e12*e12,
                b.e01*s*s+2*b.e02*e12*s+2*b.e12*e01*e12-b.e01*e12*e12-2*b.e12*e02*s,
                b.e02*s*s+2*b.e12*e01*s+2*b.e12*e02*e12-2*b.e01*e12*s-b.e02*e12*e12,
                b.e12*e12*e12+b.e12*s*s,
                b.e012*e12*e12+b.e012*s*s
            ).reduce
        }

        Fiber.abort("Sandwich not supported for: %(type.name) * %(b.type.name)")
	}

	// Left contraction
    <<(b) {
        if (b is Num) {
            return b*s

        } else if (b is Motor2) {
            return Motor2.new(
                b.s*s-b.e12*e12,
                b.e01*s,
                b.e02*s,
                b.e12*s
            ).reduce

        } else if (b is Line2) {
            return b * s

        } else if (b is Point2) {
            return Motor2.new(
                -b.e12*e12,
                b.e01*s,
                -b.e20*s,
                b.e12*s
            ).reduce

        } else if (b is Rotor2) {
            return Rotor2.new(
                b.s*s-b.e12*e12,
                b.e12*s
            )

        } else if (b is Trans2) {
            return Trans2.new(
                b.e01*s, b.e02*s, s
            )

        // TODO: PScalar2

        } else if (b is MVec2) {
            return MVec2.new(
                b.s*s-b.e12*e12,
                b.e0*s-b.e012*e12,
                b.e1*s, b.e2*s,
                b.e01*s, b.e02*s, b.e12*s,
                b.e012*s
            ).reduce
        }

        Fiber.abort("Left contraction not supported for: %(type.name) * %(b.type.name)")
    }

    // Regressive product
    &(b) { !(!this ^ !b) }

	// Reverse operator
    ~ { Motor2.new(s, -e01, -e02, -e12) }

	// Dual operator
    ! { MVec2.new(0, e12, -e02, e01, 0, 0, 0, s) }

	// Grade selection
    grade(i) { mvec.grade(i) }

	// Normalization
    norm_sq { s*s + e12*e12 }
    normalized {
        var n = this.norm_sq
        if (n == 0) {
            return Motor2.new(0, 0, 0, 0)
        } else {
            n = 1 / n.sqrt
            return this * n
        }
    }

	// Exponentiation
	// Logarithm

    // Equality
    ==(b) { mvec == b }
    !=(b) { !(this == b) }

    // Utility
    copy { Motor2.new(s, e01, e02, e12) }
    mvec { MVec2.new(s, 0, 0, 0, e01, e02, e12, 0) }
    reduce { mvec.reduce }

    glSetUniform(name) {
        App.glSetUniform(name)
        App.glSetVec4f(s, e01, e02, e12)
    }

    // Debug
    glDraw(color) {
        Line2.new(0, 1, 0).glDraw(color)
        Line2.new(0, s, e12).glDraw(color)
    }

    guiInspect(name) {
        App.guiPushItemWidth(App.guiContentAvailWidth() / 5)
        App.guiText("%(name): ")
        App.guiSameLine()
        e20 = App.guiFloat("s##%(name)", s)
        App.guiSameLine()
        e20 = App.guiFloat("e01##%(name)", e01)
        App.guiSameLine()
        e20 = App.guiFloat("e02##%(name)", e02)
        App.guiSameLine()
        e12 = App.guiFloat("e12##%(name)", e12)
        App.guiPopItemWidth()
    }

    toString { "[%(s) + %(e01)e01 + %(e02)e02 + %(e12)e12]" }
}

// The pseudo-scalar class represents the vector: Ae₀₁₂
class PScalar2 {
    construct new(e012) {
        _e012 = e012
    }

    e012 { _e012 }
    e012=(v) { _e012 = v }

    toString { "[%(e012)e012]" }
}

// The full multivector class represents: A + Be₀ + Ce₁ + De₂ + Ee₀₁ + Fe₀₂ + Ge₁₂ + He₀₁₂
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

    // Geometric product
    *(b) {
        if (b is Num) {
            return MVec2.new(
                s * b, e0 * b, e1 * b, e2 * b,
                e01 * b, e02 * b, e12 * b, e012 * b
            )

        } else if(b is MVec2) {
            return MVec2.new(
                b.e1*e1+b.e2*e2+b.s*s-b.e12*e12,
                b.e0*s+b.e1*e01+b.e2*e02+b.s*e0-b.e01*e1-b.e012*e12-b.e02*e2-b.e12*e012,
                b.e1*s+b.e2*e12+b.s*e1-b.e12*e2,
                b.e12*e1+b.e2*s+b.s*e2-b.e1*e12,
                b.e01*s+b.e012*e2+b.e02*e12+b.e1*e0+b.e2*e012+b.s*e01-b.e0*e1-b.e12*e02,
                b.e02*s+b.e12*e01+b.e2*e0+b.s*e02-b.e0*e2-b.e01*e12-b.e012*e1-b.e1*e012,
                b.e12*s+b.e2*e1+b.s*e12-b.e1*e2,
                b.e0*e12+b.e01*e2+b.e012*s+b.e12*e0+b.e2*e01+b.s*e012-b.e02*e1-b.e1*e02
            ).reduce

        } else if(b is Line2) {
            return MVec2.new(
                b.e1*e1+b.e2*e2,
                b.e0*s+b.e1*e01+b.e2*e02,
                b.e1*s+b.e2*e12,
                b.e2*s-b.e1*e12,
                b.e1*e0+b.e2*e012-b.e0*e1,
                b.e2*e0-b.e0*e2-b.e1*e012,
                b.e2*e1-b.e1*e2,
                b.e0*e12+b.e2*e01-b.e1*e02
            ).reduce

        } else if(b is Point2) {
            return MVec2.new(
                -b.e12*e12,
                -b.e01*e1-b.e02*e2-b.e12*e012,
                -b.e12*e2,
                b.e12*e1,
                b.e01*s+b.e02*e12-b.e12*e02,
                b.e02*s+b.e12*e01-b.e01*e12,
                b.e12*s,
                b.e01*e2+b.e12*e0-b.e02*e1
            ).reduce

        } else if(b is Trans2) {
            return MVec2.new(
                -b.e12*e12,
                -b.e01*e1-b.e02*e2-b.e12*e012,
                -b.e12*e2,
                b.e12*e1,
                b.e01*s+b.e02*e12-b.e12*e02,
                b.e02*s+b.e12*e01-b.e01*e12,
                b.e12*s,
                b.e01*e2+b.e12*e0-b.e02*e1
            ).reduce

        } else if(b is Rotor2) {
            return MVec2.new(
                b.s*s-b.e12*e12,
                b.s*e0-b.e12*e012,
                b.s*e1-b.e12*e2,
                b.e12*e1+b.s*e2,
                b.s*e01-b.e12*e02,
                b.e12*e01+b.s*e02,
                b.e12*s+b.s*e12,
                b.e12*e0+b.s*e012
            ).reduce

        } else if(b is Motor2) {
            return MVec2.new(
                b.s*s-b.e12*e12,
                b.s*e0-b.e01*e1-b.e02*e2-b.e12*e012,
                b.s*e1-b.e12*e2,
                b.e12*e1+b.s*e2,
                b.e01*s+b.e02*e12+b.s*e01-b.e12*e02,
                b.e02*s+b.e12*e01+b.s*e02-b.e01*e12,
                b.e12*s+b.s*e12,
                b.e01*e2+b.e12*e0+b.s*e012-b.e02*e1
            ).reduce
        }

        // TODO: PScalar2

        Fiber.abort("Geometric product not supported for: %(type.name) * %(b.type.name)")
    }

    // Geometric product
    |(b) {
        if (b is Num) {
            return this * b

        } else if(b is MVec2) {
            return MVec2.new(
                b.e1*e1+b.e2*e2+b.s*s-b.e12*e12,
                b.e0*s+b.e1*e01+b.e2*e02+b.s*e0-b.e01*e1-b.e012*e12-b.e02*e2-b.e12*e012,
                b.e1*s+b.e2*e12+b.s*e1-b.e12*e2,
                b.e12*e1+b.e2*s+b.s*e2-b.e1*e12,
                b.e01*s+b.e012*e2+b.e2*e012+b.s*e01,
                b.e02*s+b.s*e02-b.e012*e1-b.e1*e012,
                b.e12*s+b.s*e12,
                b.e012*s+b.s*e012
            ).reduce

        } else if(b is Line2) {
            return MVec2.new(
                b.e1*e1+b.e2*e2,
                b.e0*s+b.e1*e01+b.e2*e02,
                b.e1*s+b.e2*e12,
                b.e2*s-b.e1*e12,
                b.e2*e012,
                -b.e1*e012,
                0, 0
            ).reduce

        } else if(b is Point2) {
            return MVec2.new(
                -b.e12*e12,
                b.e20*e2-b.e01*e1-b.e12*e012,
                -b.e12*e2, b.e12*e1,
                b.e01*s, -b.e20*s, b.e12*s,
                0
            ).reduce

        } else if(b is Trans2) {
            return MVec2.new(
                s,
                e0-b.e01*e1-b.e02*e2,
                e1, e2,
                b.e01*s+e01,
                b.e02*s+e02,
                e12, e012
            ).reduce

        } else if(b is Rotor2) {
            return MVec2.new(
                b.s*s-b.e12*e12,
                b.s*e0-b.e12*e012,
                b.s*e1-b.e12*e2,
                b.e12*e1+b.s*e2,
                b.s*e01,
                b.s*e02,
                b.e12*s+b.s*e12,
                b.s*e012
            ).reduce

        } else if(b is Motor2) {
            return MVec2.new(
                b.s*s-b.e12*e12,
                b.s*e0-b.e01*e1-b.e02*e2-b.e12*e012,
                b.s*e1-b.e12*e2,
                b.e12*e1+b.s*e2,
                b.e01*s+b.s*e01,
                b.e02*s+b.s*e02,
                b.e12*s+b.s*e12,
                b.s*e012
            ).reduce
        }

        // TODO: PScalar2

        Fiber.abort("Inner product not supported for: %(type.name) * %(b.type.name)")
    }

    // Outer product
    ^(b) {
        if (b is Num) {
            return this*b

        } else if(b is MVec2) {
            return MVec2.new(
                b.s*s,
                b.e0*s+b.s*e0,
                b.e1*s+b.s*e1,
                b.e2*s+b.s*e2,
                b.e01*s+b.e1*e0+b.s*e01-b.e0*e1,
                b.e02*s+b.e2*e0+b.s*e02-b.e0*e2,
                b.e12*s+b.e2*e1+b.s*e12-b.e1*e2,
                b.e0*e12+b.e01*e2+b.e012*s+b.e12*e0+b.e2*e01+b.s*e012-b.e02*e1-b.e1*e02
            ).reduce

        } else if(b is Line2) {
            return MVec2.new(
                0, b.e0*s,
                b.e1*s,
                b.e2*s,
                b.e1*e0-b.e0*e1,
                b.e2*e0-b.e0*e2,
                b.e2*e1-b.e1*e2,
                b.e0*e12+b.e2*e01-b.e1*e02
            ).reduce

        } else if(b is Point2) {
            return MVec2.new(
                0, 0, 0, 0,
                b.e01*s, -b.e20*s, b.e12*s,
                b.e01*e2+b.e12*e0+b.e20*e1
            ).reduce

        } else if(b is Trans2) {
            return MVec2.new(
                s, e0, e1, e2,
                b.e01*s+e01,
                b.e02*s+e02,
                e12,
                b.e01*e2+e012-b.e02*e1
            ).reduce

        } else if(b is Rotor2) {
            return MVec2.new(
                b.s*s, b.s*e0, b.s*e1, b.s*e2,
                b.s*e01, b.s*e02,
                b.e12*s+b.s*e12,
                b.e12*e0+b.s*e012
            ).reduce

        } else if(b is Motor2) {
            return MVec2.new(
                b.s*s, b.s*e0, b.s*e1, b.s*e2,
                b.e01*s+b.s*e01,
                b.e02*s+b.s*e02,
                b.e12*s+b.s*e12,
                b.e01*e2+b.e12*e0+b.s*e012-b.e02*e1
            ).reduce
        }

        // TODO: PScalar2

        Fiber.abort("Outer product not supported for: %(type.name) * %(b.type.name)")
    }

    // Sandwich product
    >>(b) {
        if (b is Num) {
            return MVec2.new(
                b*e1*e1+b*e12*e12+b*e2*e2+b*s*s,
                2*b*e0*s+2*b*e01*e1+2*b*e012*e12+2*b*e02*e2,
                2*b*e1*s+2*b*e12*e2,
                2*b*e2*s-2*b*e1*e12,
                0, 0, 0, 0
            )

        } else if(b is MVec2) {
            return MVec2.new(
                2*b.e1*e1*s+2*b.e2*e1*e12+2*b.e2*e2*s+b.s*e1*e1+b.s*e12*e12+b.s*e2*e2+b.s*s*s-2*b.e1*e12*e2,
                b.e0*e12*e12+b.e0*s*s+2*b.e1*e0*e1+2*b.e1*e01*s+2*b.e2*e0*e2+2*b.e2*e01*e12+2*b.e2*e012*e1+2*b.e2*e02*s+2*b.s*e0*s+2*b.s*e01*e1+2*b.s*e012*e12+2*b.s*e02*e2-b.e0*e1*e1-b.e0*e2*e2-2*b.e1*e012*e2-2*b.e1*e02*e12,
                b.e1*e1*e1+b.e1*s*s+2*b.e2*e1*e2+2*b.e2*e12*s+2*b.s*e1*s+2*b.s*e12*e2-b.e1*e12*e12-b.e1*e2*e2,
                2*b.e1*e1*e2+b.e2*e2*e2+b.e2*s*s+2*b.s*e2*s-2*b.e1*e12*s-b.e2*e1*e1-b.e2*e12*e12-2*b.s*e1*e12,
                b.e01*e2*e2+b.e01*s*s+2*b.e012*e2*s+2*b.e02*e12*s+2*b.e12*e0*e2+2*b.e12*e01*e12-b.e01*e1*e1-b.e01*e12*e12-2*b.e012*e1*e12-2*b.e02*e1*e2-2*b.e12*e012*e1-2*b.e12*e02*s,
                b.e02*e1*e1+b.e02*s*s+2*b.e12*e01*s+2*b.e12*e02*e12-2*b.e01*e1*e2-2*b.e01*e12*s-2*b.e012*e1*s-2*b.e012*e12*e2-b.e02*e12*e12-b.e02*e2*e2-2*b.e12*e0*e1-2*b.e12*e012*e2,
                b.e12*e12*e12+b.e12*s*s-b.e12*e1*e1-b.e12*e2*e2,
                2*b.e01*e1*e12+2*b.e01*e2*s+b.e012*e1*e1+b.e012*e12*e12+b.e012*e2*e2+b.e012*s*s+2*b.e02*e12*e2+2*b.e12*e0*s+2*b.e12*e012*e12-2*b.e02*e1*s-2*b.e12*e01*e1-2*b.e12*e02*e2
            ).reduce

        } else if(b is Line2) {
            return MVec2.new(
                2*b.e1*e1*s+2*b.e2*e1*e12+2*b.e2*e2*s-2*b.e1*e12*e2,
                b.e0*e12*e12+b.e0*s*s+2*b.e1*e0*e1+2*b.e1*e01*s+2*b.e2*e0*e2+2*b.e2*e01*e12+2*b.e2*e012*e1+2*b.e2*e02*s-b.e0*e1*e1-b.e0*e2*e2-2*b.e1*e012*e2-2*b.e1*e02*e12,
                b.e1*e1*e1+b.e1*s*s+2*b.e2*e1*e2+2*b.e2*e12*s-b.e1*e12*e12-b.e1*e2*e2,
                2*b.e1*e1*e2+b.e2*e2*e2+b.e2*s*s-2*b.e1*e12*s-b.e2*e1*e1-b.e2*e12*e12,
                0, 0, 0, 0
            ).reduce

        } else if(b is Point2) {
            return MVec2.new(
                0, 0, 0, 0,
                b.e01*e2*e2+b.e01*s*s+2*b.e12*e0*e2+2*b.e12*e01*e12+2*b.e20*e1*e2-b.e01*e1*e1-b.e01*e12*e12-2*b.e12*e012*e1-2*b.e12*e02*s-2*b.e20*e12*s,
                2*b.e12*e01*s+2*b.e12*e02*e12+b.e20*e12*e12+b.e20*e2*e2-2*b.e01*e1*e2-2*b.e01*e12*s-2*b.e12*e0*e1-2*b.e12*e012*e2-b.e20*e1*e1-b.e20*s*s,
                b.e12*e12*e12+b.e12*s*s-b.e12*e1*e1-b.e12*e2*e2,
                2*b.e01*e1*e12+2*b.e01*e2*s+2*b.e12*e0*s+2*b.e12*e012*e12+2*b.e20*e1*s-2*b.e12*e01*e1-2*b.e12*e02*e2-2*b.e20*e12*e2
            ).reduce

        } else if(b is Trans2) {
            return MVec2.new(
                e1*e1+e12*e12+e2*e2+s*s,
                2*e0*s+2*e01*e1+2*e012*e12+2*e02*e2,
                2*e1*s+2*e12*e2,
                2*e2*s-2*e1*e12,
                b.e01*e2*e2+b.e01*s*s+2*b.e02*e12*s-b.e01*e1*e1-b.e01*e12*e12-2*b.e02*e1*e2,
                b.e02*e1*e1+b.e02*s*s-2*b.e01*e1*e2-2*b.e01*e12*s-b.e02*e12*e12-b.e02*e2*e2,
                2*b.e01*e1*e12+2*b.e01*e2*s+2*b.e02*e12*e2-2*b.e02*e1*s,
                0
            ).reduce

        } else if(b is Rotor2) {
            return MVec2.new(
                b.s*e1*e1+b.s*e12*e12+b.s*e2*e2+b.s*s*s,
                2*b.s*e0*s+2*b.s*e01*e1+2*b.s*e012*e12+2*b.s*e02*e2,
                2*b.s*e1*s+2*b.s*e12*e2,
                2*b.s*e2*s-2*b.s*e1*e12,
                2*b.e12*e0*e2+2*b.e12*e01*e12-2*b.e12*e012*e1-2*b.e12*e02*s,
                2*b.e12*e01*s+2*b.e12*e02*e12-2*b.e12*e0*e1-2*b.e12*e012*e2,
                b.e12*e12*e12+b.e12*s*s-b.e12*e1*e1-b.e12*e2*e2,
                2*b.e12*e0*s+2*b.e12*e012*e12-2*b.e12*e01*e1-2*b.e12*e02*e2
            ).reduce

        } else if(b is Motor2) {
            return MVec2.new(
                b.s*e1*e1+b.s*e12*e12+b.s*e2*e2+b.s*s*s,
                2*b.s*e0*s+2*b.s*e01*e1+2*b.s*e012*e12+2*b.s*e02*e2,
                2*b.s*e1*s+2*b.s*e12*e2,
                2*b.s*e2*s-2*b.s*e1*e12,
                b.e01*e2*e2+b.e01*s*s+2*b.e02*e12*s+2*b.e12*e0*e2+2*b.e12*e01*e12-b.e01*e1*e1-b.e01*e12*e12-2*b.e02*e1*e2-2*b.e12*e012*e1-2*b.e12*e02*s,
                b.e02*e1*e1+b.e02*s*s+2*b.e12*e01*s+2*b.e12*e02*e12-2*b.e01*e1*e2-2*b.e01*e12*s-b.e02*e12*e12-b.e02*e2*e2-2*b.e12*e0*e1-2*b.e12*e012*e2,
                b.e12*e12*e12+b.e12*s*s-b.e12*e1*e1-b.e12*e2*e2,
                2*b.e01*e1*e12+2*b.e01*e2*s+2*b.e02*e12*e2+2*b.e12*e0*s+2*b.e12*e012*e12-2*b.e02*e1*s-2*b.e12*e01*e1-2*b.e12*e02*e2
            ).reduce
        }

        // TODO: PScalar2

        Fiber.abort("Sandwich product not supported for: %(type.name) * %(b.type.name)")
    }

    // Left contraction
    <<(b) {
        if (b is Num) {
            return b*s

        } else if(b is MVec2) {
            return MVec2.new(
                b.e1*e1+b.e2*e2+b.s*s-b.e12*e12,
                b.e0*s-b.e01*e1-b.e012*e12-b.e02*e2,
                b.e1*s-b.e12*e2,
                b.e12*e1+b.e2*s,
                b.e01*s+b.e012*e2,
                b.e02*s-b.e012*e1,
                b.e12*s,
                b.e012*s
            ).reduce

        } else if(b is Line2) {
            return MVec2.new(
                b.e1*e1+b.e2*e2,
                b.e0*s, b.e1*s, b.e2*s,
                0, 0, 0, 0
            ).reduce

        } else if(b is Point2) {
            return MVec2.new(
                -b.e12*e12,
                b.e20*e2-b.e01*e1,
                -b.e12*e2, b.e12*e1,
                b.e01*s, -b.e20*s, b.e12*s,
                0
            ).reduce

        } else if(b is Trans2) {
            return MVec2.new(
                s,
                -b.e01*e1-b.e02*e2, 0, 0,
                b.e01*s, b.e02*s, 0,
                0
            ).reduce

        } else if(b is Rotor2) {
            return MVec2.new(
                b.s*s-b.e12*e12,
                0, -b.e12*e2, b.e12*e1,
                0, 0, b.e12*s,
                0
            ).reduce

        } else if(b is Motor2) {
            return MVec2.new(
                b.s*s-b.e12*e12,
                -b.e01*e1-b.e02*e2,
                -b.e12*e2, b.e12*e1,
                b.e01*s, b.e02*s, b.e12*s,
                0
            ).reduce
        }

        // TODO: PScalar2

        Fiber.abort("Left contraction not supported for: %(type.name) * %(b.type.name)")
    }

    // Regressive product
    &(b) { !(!this ^ !b) }

	// Reverse operator
    ~ { Motor2.new(s, -e01, -e02, -e12) }

	// Dual operator
    ! { MVec2.new(0, e12, -e02, e01, 0, 0, 0, s) }

	// Grade selection
    grade(i) {
        if (i == 0) {
            return s
        } else if (i == 1) {
            return Line2.new(e0, e1, e2)
        } else if (i == 2) {
            return Point2.new(e20, e01, e12)
        } else if (i == 3) {
            return PScalar2.new(e012)
        }
    }

	// Normalization
    norm_sq { 0 }
    normalized { null }

	// Exponentiation
	// Logarithm

    // Equality
    ==(b) {
        if (!(b is Num) && !(b is Line2) && !(b is Point2) && !(b is Rotor2) && !(b is Trans2) && !(b is Motor2) && !(b is PScalar2)) {
            return false
        }
        var t = b is Num ? MVec2.new(b, 0, 0, 0, 0, 0, 0, 0) : b.mvec
        return
            s == t.s &&
            e0 == t.e0 && e1 == t.e1 && e2 == t.e2 &&
            e01 == t.e01 && e02 == t.e02 && e12 == t.e12 &&
            e012 == t.e012
    }
    !=(b) { !(this == b) }

    // Utility
    copy { MVec2.new(s, e0, e1, e2, e01, e02, e12, e012) }
    mvec { this }
    reduce {
        var sbm = 0 // scalar bitmask
        sbm = sbm | (s    == 0 ? 0 : 0x01)
        sbm = sbm | (e0   == 0 ? 0 : 0x02)
        sbm = sbm | (e1   == 0 ? 0 : 0x04)
        sbm = sbm | (e2   == 0 ? 0 : 0x08)
        sbm = sbm | (e01  == 0 ? 0 : 0x10)
        sbm = sbm | (e02  == 0 ? 0 : 0x20)
        sbm = sbm | (e12  == 0 ? 0 : 0x40)
        sbm = sbm | (e012 == 0 ? 0 : 0x80)

        if (sbm & 0x0E != 0 && sbm & 0xF1 == 0) {
            return Line2.new(e0, e1, e2)
        } else if (sbm & 0x70 != 0 && sbm & 0x8F == 0) {
            return Point2.new(-e02, e01, e12)
        } else if (sbm & 0x41 != 0 && sbm & 0xBE == 0) {
            return Rotor2.new(s, e12)
        } else if (sbm & 0x31 != 0 && sbm & 0xCE == 0) {
            return Trans2.new(e01, e02, s)
        } else if (sbm & 0x71 != 0 && sbm & 0x8E == 0) {
            return Motor2.new(s, e01, e02, e12)
        } else if (sbm & 0x80 != 0 && sbm & 0x7F == 0) {
            return PScalar2.new(e012)
        }
        return this
    }

    // Debug
    toString { "[%(s) + %(e0)e0 + %(e1)e1 + %(e2)e2 + %(e01)e01 + %(e02)e02 + %(e12)e12 + %(e012)e012]" }
}

class PGA2 {
    static test() {
    }
}