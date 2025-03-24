import "app" for App

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
        App.glVertex(x, y, 0, color)
        App.glEnd(App.glPoints)
    }

    toString { "[%(x), %(y)]" }
}

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
        App.glVertex(-r, -i, 0, 0x00000000)
        App.glVertex( r,  i, 0, color)
        App.glEnd(App.glPoints | App.glLines)
    }

    toString { "[%(r), %(i)]" }
}