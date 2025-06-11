import "app" for App

class NumExt {
    static sinh(x) { (x.exp - (-x).exp) / 2 }
    static cosh(x) { (x.exp + (-x).exp) / 2 }
    static tanh(x) {
        var epx = x.exp
        var enx = (-x).exp
        return (epx - enx) / (epx + enx)
    }
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

    +(b) {
        if (b is Num) {
            return Complex.new(r + b, i)
        } else if (b is Complex) {
            return Complex.new(r + b.r, i + b.i)
        }
        Fiber.abort("Addition not supported for %(b.type.name)")
    }

    -(b) {
        if (b is Num) {
            return Complex.new(r - b, i)
        } else if (b is Complex) {
            return Complex.new(r - b.r, i - b.i)
        }
        Fiber.abort("Subtraction not supported for %(b.type.name)")
    }

    *(b) {
        if (b is Num) {
            return Complex.new(r * b, i * b)
        } else if (b is Complex) {
            return Complex.new(r * b.r - i * b.i, r * b.i + i * b.r)
        }
        Fiber.abort("Product not supported for %(b.type.name)")
    }

    /(b) {
        if (b is Num) {
            var d = b != 0 ? 1 / b : 0
            return Complex.new(r * d, i * d)
        } else if (b is Complex) {
            var d = b.norm
            d = d != 0 ? 1 / d : 0
            return Complex.new(r * b.r + i * b.i, i * b.r - r * b.i) * d
        }
        Fiber.abort("Division not supported for %(b.type.name)")
    }

    ~ { Complex.new(r, -i) }
    norm { r * r + i * i }
    magnitude { norm.sqrt }
    arg { i.atan(r) }

    exp { Complex.new(i.cos, i.sin) * r.exp }
    log { Complex.new(magnitude.log, arg) }

    pow(b) {
        b = b is Num ? Complex.new(b, 0) : b
        if (b is Complex) {
            return (this.log * b).exp
        }
        Fiber.abort("Power not supported for %(b.type.name)")
    }

    sqrt {
        var mag = magnitude
        if (mag == 0) {
            return Complex.new(0, 0)
        }
        var a = ((mag + r) / 2).sqrt
        var b = ((mag - r) / 2).sqrt
        b = i < 0 ? -b : b
        return Complex.new(a, b)
    }

    sin { Complex.new(r.sin * NumExt.cosh(i), r.cos * NumExt.sinh(i)) }
    cos { Complex.new(r.cos * NumExt.cosh(i), -r.sin * NumExt.sinh(i)) }
    tan { sin / cos }

    sinh { Complex.new(NumExt.sinh(r) * i.cos, NumExt.cosh(r) * i.sin) }
    cosh { Complex.new(NumExt.cosh(r) * i.cos, NumExt.sinh(r) * i.sin) }
    tanh { sinh / cosh }

    glDraw(color) {
        App.glBegin(true, true, 10, 2)
        App.glAddVertex(r,  i, 0, color)
        App.glEnd(App.glPoints)
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