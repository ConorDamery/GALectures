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
        } else if (b is Rotor) {
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

class Rotor {
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
            return Rotor.new(r * b, i * b)
        } else if (b is Rotor) {
            return Rotor.new(r * b.r - i * b.i, r * b.i + i * b.r)
        } else if (b is Point) {
            return Point.new(r * b.x + i * b.y, r * b.y + i * b.x)
        } else {
            Fiber.abort("Product not supported for %(b.type.name)")
        }
    }

    ~ { Rotor.new(r, -i) }

    exp { Rotor.new(i.cos, i.sin) * r.exp }

    glDraw(color) {
        App.glBegin(true, true, 10, 2)
        App.glVertex(-r, -i, 0, 0x00000000)
        App.glVertex( r,  i, 0, color)
        App.glEnd(App.glPoints | App.glLines)
    }

    toString { "[%(r), %(i)]" }
}

class Motor {
    construct new(s, e12, e01, e02) {
        _s = s
        _e12 = e12
        _e01 = e01
        _e02 = e02
    }

    s { _s }
    e12 { _e12 }
    e01 { _e01 }
    e02 { _e02 }

    s=(v) { _s = v }
    e12=(v) { _e12 = v }
    e01=(v) { _e01 = v }
    e02=(v) { _e02 = v }

    glUniform(name) {
        App.glUniform(name)
        App.glVec4f(s, e12, e01, e02)
    }

    toString { "[%(s), %(e12), %(e01), %(e02)]" }
}