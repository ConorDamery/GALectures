import "app" for App

class Complex {
    construct new(r, i) {
        _r = r
        _i = i
    }

    r { _r }
    i { _i }

    r=(v) { _r = v }
    i=(v) { _i = v }

    *(b) {
        if (b is Num) {
            return Complex.new(r * b, i * b)
        } else if (b is Complex) {
            return Complex.new(r * b.r - i * b.i, r * b.i + i * b.r)
        } else if (b is Point) {
            return Point.new(r * b.x + i * b.y, r * b.y + i * b.x)
        } else {
            Fiber.abort("Product not supported for %(b.type.name)")
        }
    }

    ~ { Complex.new(r, -i) }

    exp { Complex.new(i.cos, i.sin) * r.exp }

    glDraw(color) {
        App.glBegin(true, true, 10, 2)
        App.glAddVertex(-r, -i, 0, 0x00000000)
        App.glAddVertex( r,  i, 0, color)
        App.glEnd(App.glPoints | App.glLines)
    }

    toString { "[%(r) + %(i)i]" }
}

class Hyperbolic {
    construct new(r, j) {
        _r = r
        _j = j
    }

    r { _r }
    j { _j }

    r=(v) { _r = v }
    j=(v) { _j = v }

    *(b) {
        if (b is Num) {
            return Hyperbolic.new(r * b, j * b)
        } else if (b is Complex) {
            return Hyperbolic.new(r * b.r - j * b.j, r * b.j + j * b.r)
        } else if (b is Point) {
            return Point.new(r * b.x + j * b.y, r * b.y + j * b.x)
        } else {
            Fiber.abort("Product not supported for %(b.type.name)")
        }
    }

    ~ { Hyperbolic.new(r, -j) }

    exp { Hyperbolic.new(j.acos, i.asin) * r.exp }

    glDraw(color) {
        App.glBegin(true, true, 10, 2)
        App.glAddVertex(-r, -j, 0, 0x00000000)
        App.glAddVertex( r,  j, 0, color)
        App.glEnd(App.glPoints | App.glLines)
    }

    toString { "[%(r) + %(j)j]" }
}

class Dual {
    construct new(r, e) {
        _r = r
        _e = e
    }

    r { _r }
    e { _e }

    r=(v) { _r = v }
    e=(v) { _e = v }

    *(b) {
        if (b is Num) {
            return Dual.new(r * b, j * b)
        } else if (b is Complex) {
            return Dual.new(r * b.r - j * b.j, r * b.j + j * b.r)
        } else if (b is Point) {
            return Point.new(r * b.x + j * b.y, r * b.y + j * b.x)
        } else {
            Fiber.abort("Product not supported for %(b.type.name)")
        }
    }

    ~ { Dual.new(r, -e) }

    exp { Dual.new(j.acos, i.asin) * r.exp }

    glDraw(color) {
        App.glBegin(true, true, 10, 2)
        App.glAddVertex(-r, -e, 0, 0x00000000)
        App.glAddVertex( r,  e, 0, color)
        App.glEnd(App.glPoints | App.glLines)
    }

    toString { "[%(r) + %(e)e]" }
}

class Point {
    construct new(x, y) {
        _x = x
        _y = y
    }

    x { _x }
    y { _y }

    x=(v) { _x = v }
    y=(v) { _y = v }

    *(b) {
        if (b is Num) {
            return Point.new(x * b, y * b)
        } else if (b is Complex) {
            return Point.new(x * b.r - y * b.i, x * b.i + y * b.r)
        } else {
            Fiber.abort("Product not supported for %(b.type.name)")
        }
    }

    glDraw(color) {
        App.glBegin(true, true, 10, 2)
        App.glAddVertex(x, y, 0, color)
        App.glEnd(App.glPoints)
    }

    toString { "[%(x), %(y)]" }
}