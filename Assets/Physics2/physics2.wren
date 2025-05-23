import "app" for App
import "random" for Random

class Vec2 {
	construct new() {
		_x = 0
		_y = 0
	}
	construct new(x, y) {
		_x = x
		_y = y
	}

	x { _x }
	y { _y }
	x=(v) { _x = v }
	y=(v) { _y = v }

	set(x, y) {
		_x = x
		_y = y
	}

	- { Vec2.new(-x, -y) }
	
	+(v) { Vec2.new(x + v.x, y + v.y) }
	-(v) { Vec2.new(x - v.x, y - v.y) }
	*(a) { Vec2.new(x * a, y * a) }

	length { (x * x + y * y).sqrt }
	abs { Vec2.new(x.abs, y.abs) }

	static dot(a, b) { a.x * b.x + a.y * b.y }
	static cross(a, b) {
		if (a is Vec2 && b is Vec2) {
			return a.x * b.y - a.y * b.x
		} else if (a is Vec2 && b is Num) {
			return Vec2.new(b * a.y, -b * a.x)
		} else if (a is Num && b is Vec2) {
			return Vec2.new(-a * b.y, a * b.x)
		}
		return null
	}
}

class Mat22 {
	construct new() {
		_col1 = Vec2.new(1, 0)
		_col2 = Vec2.new(0, 1)
	}
	construct new(angle) {
		var c = angle.cos
		var s = angle.sin
		_col1 = Vec2.new(c, s)
		_col2 = Vec2.new(-s, c)
	}
	construct new(col1, col2) {
		_col1 = col1
		_col2 = col2
	}

	col1 { _col1 }
	col2 { _col2 }
	col1=(v) { _col1 = v }
	col2=(v) { _col2 = v }

	* (b) {
		if (b is Vec2) {
			return Vec2.new(col1.x * b.x + col2.x * b.y, col1.y * b.x + col2.y * b.y)
		} else if (b is Mat22) {
			return Mat22.new(this * b.col1, this * b.col2)
		}
		return null
	}

	+ (b) { Mat22.new(col1 + b.col1, col2 + b.col2) }

	transpose { Mat22.new(Vec2.new(col1.x, col2.x), Vec2.new(col1.y, col2.y)) }
	invert {
		var a = col1.x
		var b = col2.x
		var c = col1.y
		var d = col2.y
		var B = Mat22.new()
		var det = a * d - b * c
		//assert(det != 0.0f);
		det = 1 / det
		B.col1.x =  det * d
		B.col2.x = -det * b
		B.col1.y = -det * c
		B.col2.y =  det * a
		return B
	}
	abs { Mat22.new(col1.abs, col2.abs) }
}

class FeaturePair {
	construct new(value) {
		_value = value
	}

	value { _value }
	value=(v) { _value = v }

	inEdge1 { (_value >> 0) & 0xFF }
	inEdge1=(v) { _value = (_value & ~0x000000FF) | ((v & 0xFF) << 0) }

	outEdge1 { (_value >> 8) & 0xFF }
	outEdge1=(v) { _value = (_value & ~0x0000FF00) | ((v & 0xFF) << 0) }

	inEdge2 { (_value >> 16) & 0xFF }
	inEdge2=(v) { _value = (_value & ~0x00FF0000) | ((v & 0xFF) << 0) }

	outEdge2 { (_value >> 24) & 0xFF }
	outEdge2=(v) { _value = (_value & ~0xFF000000) | ((v & 0xFF) << 0) }

	flip() {
		var tmp = inEdge1
		inEdge1 = inEdge2
		inEdge2 = tmp
		tmp = outEdge1
		outEdge1 = outEdge2
		outEdge2 = tmp
	}
}

class Contact {
	construct new() {
		_position = Vec2.new()
		_normal = Vec2.new()
		_r1 = Vec2.new()
		_r2 = Vec2.new()

		_separation = 0
		_pn = 0 // accumulated normal impulse
		_pt = 0 // accumulated tangent impulse
		_pnb = 0 // accumulated normal impulse for position bias
		_massNormal = 0
		_massTangent = 0
		_bias = 0
		_feature = FeaturePair.new(0)
	}

	position { _position }
	normal { _normal }
	r1 { _r1 }
	r2 { _r2 }
	separation { _separation }
	pn { _pn }
	pt { _pt }
	pnb { _pnb }
	massNormal { _massNormal }
	massTangent { _massTangent }
	bias { _bias }
	feature { _feature }

	position=(v) { _position = v }
	normal=(v) { _normal = v }
	r1=(v) { _r1 = v }
	r2=(v) { _r2 = v }
	separation=(v) { _separation = v }
	pn=(v) { _pn = v }
	pt=(v) { _pt = v }
	pnb=(v) { _pnb = v }
	massNormal=(v) { _massNormal = v }
	massTangent=(v) { _massTangent = v }
	bias=(v) { _bias = v }
	feature=(v) { _feature = v }
}

class Body {
	construct new() {
		__idCounter = __idCounter + 1
		_id = __idCounter

		_position = Vec2.new()
		_rotation = 0
		_velocity = Vec2.new()
		_angularVelocity = 0
		_force = Vec2.new()
		_torque = 0
		_friction = 0

		_width = Vec2.new(1, 1)
		_mass = Num.largest
		_invMass = 0
		_I = Num.largest
		_invI = 0
	}

	id { _id }
	position { _position }
	rotation { _rotation }
	velocity { _velocity }
	angularVelocity { _angularVelocity }
	force { _force }
	torque { _torque }
	friction { _friction }
	width { _width }
	mass { _mass }
	invMass { _invMass }
	I { _I }
	invI { _invI }

	position=(v) { _position = v }
	rotation=(v) { _rotation = v }
	velocity=(v) { _velocity = v }
	angularVelocity=(v) { _angularVelocity = v }
	force=(v) { _force = v }
	torque=(v) { _torque = v }
	friction=(v) { _friction = v }

	static init() {
		__idCounter = 0
	}

	set(w, m) {
		_position.set(0, 0)
		_rotation = 0
		_velocity.set(0, 0)
		_angularVelocity = 0
		_force.set(0, 0)
		_torque = 0
		_friction = 0.2

		_width = w
		_mass = m

		if (_mass < Num.largest) {
			_invMass = 1 / _mass
			_I = _mass * (_width.x * _width.x + _width.y * _width.y) / 12
			_invI = 1 / _I
		} else {
			_invMass = 0
			_I = Num.largest
			_invI = 0
		}
	}

	addForce(f) {
		force = force + f
	}

	glDraw(color) {
		if (width.y == 0) {
			// Draw circle
			// TODO
		} else {
			// Draw quad
			App.glBegin(true, true, 1, 1)
			var r = Mat22.new(rotation)
			var p1 = position + r * Vec2.new(-width.x, -width.y)
			var p2 = position + r * Vec2.new(-width.x,  width.y)
			var p3 = position + r * Vec2.new( width.x,  width.y)
			var p4 = position + r * Vec2.new( width.x, -width.y)
			App.glVertex(p1.x, p1.y, 0, color)
			App.glVertex(p2.x, p2.y, 0, color)
			App.glVertex(p3.x, p3.y, 0, color)
			App.glVertex(p4.x, p4.y, 0, color)
			App.glEnd(App.glLineLoop)
		}
	}
}

class Joint {
	construct new() {
		_m = Mat22.new()
		_localAnchor1 = Vec2.new()
		_localAnchor2 = Vec2.new()
		_r1 = Vec2.new()
		_r2 = Vec2.new()
		_bias = Vec2.new()
		_p = Vec2.new()
		_body1 = null
		_body2 = null
		_biasFactor = 0.2
		_softness = 0
	}

	set(b1, b2, anchor) {
		_body1 = b1
		_body2 = b2

		var rot1 = Mat22.new(_body1.rotation)
		var rot2 = Mat22.new(_body2.rotation)
		var rot1T = rot1.transpose
		var rot2T = rot2.transpose

		_localAnchor1 = rot1T * (anchor - _body1.position)
		_localAnchor2 = rot2T * (anchor - _body2.position)

		_p.set(0, 0)

		_softness = 0.0
		_biasFactor = 0.2
	}

	preStep(inv_dt) {
		// Pre-compute anchors, mass matrix, and bias.
		var rot1 = Mat22.new(_body1.rotation)
		var rot2 = Mat22.new(_body2.rotation)

		_r1 = rot1 * _localAnchor1
		_r2 = rot2 * _localAnchor2

		// deltaV = deltaV0 + K * impulse
		// invM = [(1/m1 + 1/m2) * eye(2) - skew(r1) * invI1 * skew(r1) - skew(r2) * invI2 * skew(r2)]
		//      = [1/m1+1/m2     0    ] + invI1 * [r1.y*r1.y -r1.x*r1.y] + invI2 * [r1.y*r1.y -r1.x*r1.y]
		//        [    0     1/m1+1/m2]           [-r1.x*r1.y r1.x*r1.x]           [-r1.x*r1.y r1.x*r1.x]
		var k1 = Mat22.new()
		k1.col1.x = _body1.invMass + _body2.invMass
		k1.col2.x = 0.0
		k1.col1.y = 0.0
		k1.col2.y = _body1.invMass + _body2.invMass

		var k2 = Mat22.new()
		k2.col1.x =  _body1.invI * _r1.y * _r1.y
		k2.col2.x = -_body1.invI * _r1.x * _r1.y
		k2.col1.y = -_body1.invI * _r1.x * _r1.y
		k2.col2.y =  _body1.invI * _r1.x * _r1.x

		var k3 = Mat22.new()
		k3.col1.x =  _body2.invI * _r2.y * _r2.y
		k3.col2.x = -_body2.invI * _r2.x * _r2.y
		k3.col1.y = -_body2.invI * _r2.x * _r2.y
		k3.col2.y =  _body2.invI * _r2.x * _r2.x

		var k = k1 + k2 + k3
		k.col1.x = k.col1.x + softness
		k.col2.y = k.col2.y + softness

		_m = k.invert

		var p1 = _body1.position + r1
		var p2 = _body2.position + r2
		var dp = p2 - p1

		if (World.positionCorrection) {
			_bias = -_biasFactor * inv_dt * dp
		} else {
			_bias.set(0, 0)
		}

		if (World.warmStarting) {
			// Apply accumulated impulse.
			_body1.velocity = _body1.velocity - _body1.invMass * _p
			_body1.angularVelocity = _body1.angularVelocity - _body1.invI * Vec2.cross(_r1, _p)

			_body2.velocity = _body2.velocity + _body2.invMass * _p
			_body2.angularVelocity = _body2.angularVelocity + _body2.invI * Vec2.cross(_r2, _p)
		} else {
			_p.set(0, 0)
		}
	}

	applyImpulse() {
		var dv = _body2.velocity + Vec2.cross(_body2.angularVelocity, _r2) - _body1.velocity - Vec2.cross(_body1.angularVelocity, _r1)

		var impulse = Vec2.new()
		impulse = _m * (_bias - dv - _softness * _p)

		_body1.velocity = _body1.velocity - _body1.invMass * impulse
		_body1.angularVelocity = _body1.angularVelocity - _body1.invI * Vec2.cross(_r1, impulse)

		_body2.velocity = _body2.velocity + _body2.invMass * impulse
		_body2.angularVelocity = _body2.angularVelocity + _body2.invI * Vec2.cross(_r2, impulse)

		_p = _p + impulse
	}
}

class Arbiter {
	construct new(b1, b2) {
		if (b1.id < b2.id){
			_b1 = b1
			_b2 = b2
		} else {
			_b1 = b2
			_b2 = b1
		}

		_contacts = Collision.collide(_b1, _b2)
		_friction = (_b1.friction * _b2.friction).sqrt
	}

	b1 { _b1 }
	b2 { _b2 }
	contacts { _contacts }
	friction { _friction }

	key { (b1.id << 16) | (b2.id & 0xFFFF) }

	update(contacts) {
		var mergedContacts = [null, null]

		for (cNew in contacts.count) {
			var k = -1
			for (cOld in contacts) {
				if (cNew.feature.value == cOld.feature.value) {
					k = cOld
				}
			}

			if (k > -1) {
				var cOld = k
			}
		}
	}

	preStep(inv_dt) {
		var k_allowedPenetration = 0.01
		var k_biasFactor = World.positionCorrection ? 0.2 : 0.0

		for (c in _contacts) {
			var r1 = c.position - b1.position
			var r2 = c.position - b2.position

			// Precompute normal mass, tangent mass, and bias.
			var rn1 = Vec2.dot(r1, c.normal)
			var rn2 = Vec2.dot(r2, c.normal)
			var kNormal = b1.invMass + b2.invMass
			kNormal = kNormal + (b1.invI * (Vec2.dot(r1, r1) - rn1 * rn1) + b2.invI * (Vec2.dot(r2, r2) - rn2 * rn2))
			c.massNormal = 1.0 / kNormal

			var tangent = Vec2.cross(c.normal, 1.0)
			var rt1 = Vec2.dot(r1, tangent)
			var rt2 = Vec2.dot(r2, tangent)
			var kTangent = b1.invMass + b2.invMass
			kTangent = kTangent + (b1.invI * (Vec2.dot(r1, r1) - rt1 * rt1) + b2.invI * (Vec2.dot(r2, r2) - rt2 * rt2))
			c.massTangent = 1.0 /  kTangent

			c.bias = -k_biasFactor * inv_dt * (c.separation + k_allowedPenetration).min(0)

			if (World.accumulateImpulses) {
				// Apply normal + friction impulse
				var P = c.Pn * c.normal + c.Pt * tangent

				b1.velocity = b1.velocity - b1.invMass * P
				b1.angularVelocity = b1.angularVelocity - b1.invI * Vec2.cross(r1, P)

				b2.velocity = b2.velocity + b2.invMass * P
				b2.angularVelocity = b2.angularVelocity + b2.invI * Vec2.cross(r2, P)
			}
		}
	}

	applyImpulse() {
		for (c in contacts) {
			c.r1 = c.position - b1.position
			c.r2 = c.position - b2.position

			// Relative velocity at contact
			var dv = b2.velocity + Vec2.cross(b2.angularVelocity, c.r2) - b1.velocity - Vec2.cross(b1.angularVelocity, c.r1)

			// Compute normal impulse
			var vn = Vec2.dot(dv, c.normal)
			var dPn = c.massNormal * (-vn + c.bias)

			if (World.accumulateImpulses) {
				// Clamp the accumulated impulse
				var Pn0 = c.Pn
				c.Pn = (Pn0 + dPn).max(0)
				dPn = c.Pn - Pn0
			} else {
				dPn = dPn.max(0)
			}

			// Apply contact impulse
			var Pn = dPn * c.normal

			b1.velocity = b1.velocity - Pn * b1.invMass
			b1.angularVelocity = b1.angularVelocity - Vec2.cross(c.r1, Pn) * b1.invI

			b2.velocity = b2.velocity + Pn * b2.invMass
			b2.angularVelocity = b2.angularVelocity + Vec2.cross(c.r2, Pn) * b2.invI

			// Relative velocity at contact
			dv = b2.velocity + Vec2.cross(b2.angularVelocity, c.r2) - b1.velocity - Vec2.cross(b1.angularVelocity, c.r1)

			var tangent = Vec2.cross(c.normal, 1.0)
			var vt = Vec2.dot(dv, tangent)
			var dPt = c.massTangent * (-vt)

			if (World.accumulateImpulses) {
				// Compute friction impulse
				var maxPt = friction * c.Pn

				// Clamp friction
				var oldTangentImpulse = c.Pt
				c.Pt = (oldTangentImpulse + dPt).clamp(-maxPt, maxPt)
				dPt = c.Pt - oldTangentImpulse
			} else {
				var maxPt = friction * dPn
				dPt = dPt.clamp(-maxPt, maxPt)
			}

			// Apply contact impulse
			var Pt = dPt * tangent

			b1.velocity = b1.velocity - Pt * b1.invMass
			b1.angularVelocity = b1.angularVelocity - Vec2.cross(c.r1, Pt) * b1.invI

			b2.velocity = b2.velocity + Pt * b2.invMass
			b2.angularVelocity = b2.angularVelocity + Vec2.cross(c.r2, Pt) * b2.invI
		}
	}

	glDraw(color) {
		App.glBegin(true, true, 10, 1)
		for (c in _contacts) {
			App.glVertex(c.position.x, c.position.y, 0, color)
		}
		App.glEnd(App.glPoints)
	}
}

class World {
    construct new(gravity, it) {
		_gravity = gravity
		_it = it

        _bodies = []
		_joints = []
		_arbiters = []
    }

	gravity { _gravity }
	it { _it }

	gravity=(v) { _gravity = v }
	it=(v) { _it = v }

	static init() {
		Body.init()
		__accumulateImpulses = false
		__warmStarting = false
		__positionCorrection = false
	}

	static accumulateImpulses { __accumulateImpulses }
	static warmStarting { __warmStarting }
	static positionCorrection { __positionCorrection }

	static accumulateImpulses=(v) { __accumulateImpulses = v }
	static warmStarting=(v) { __warmStarting = v }
	static positionCorrection=(v) { __positionCorrection = v }

	addBody(body) {
		_bodies.add(body)
	}

	addJoint(joint) {
		_joints.add(joint)
	}

	clear() {
		_bodies.clear()
		_joints.clear()
		_arbiters.clear()
	}

	broadphase() {
		// O(n^2) broad-phase
		for (i in 0..._bodies.count) {
			var bi = _bodies[i]

			for (j in i+1..._bodies.count) {
				var bj = _bodies[j]

				if (bi.invMass == 0 && bj.invMass == 0) {
					continue
				}

				var newArb = Arbiter.new(bi, bj)
				var key = newArb.key

				if (newArb.contacts.count > 0) {
					var arb = _arbiters[key]
					if (arb == null) {
						arbiters[key] = newArb
					} else {
						arb.update(newArb.contacts)
					}
				} else {
					_arbiters.remove(key)
				}
			}
		}
	}

	step(dt) {
		var inv_dt = 0
		if (dt > 0) {
			inv_dt = 1 / dt
		}

		// Determine overlapping bodies and update contact points.
		broadphase()

		// Integrate forces.
		for (b in _bodies) {
			if (b.invMass == 0) {
				continue
			}

			b.velocity = b.velocity + (gravity + b.force * b.invMass) * dt
			b.angularVelocity = b.angularVelocity + b.torque * b.invI * dt
		}

		// Perform pre-steps.
		for (arb in _arbiters) {
			arb.value.preStep(inv_dt)
		}

		for (j in _joints) {
			j.preStep(inv_dt)
		}

		// Perform iterations
		for (i in 0...it) {
			for (arb in _arbiters) {
				arb.value.applyImpulse()
			}

			for (j in _joints) {
				j.applyImpulse()
			}
		}

		// Integrate Velocities
		for (b in _bodies) {
			b.position = b.position + b.velocity * dt
			b.rotation = b.rotation + b.angularVelocity * dt

			b.force.set(0, 0)
			b.torque = 0
		}
    }

    glDraw() {
        for (body in _bodies) {
            body.glDraw(0xFFFFFFFF)
        }
		for (alb in _arbiters) {
            alb.glDraw(0xFFFFFFFF)
        }
    }
}

class Axis {
	static FACE_A_X { 0 }
	static FACE_A_Y { 1 }
	static FACE_B_X { 2 }
	static FACE_B_Y { 3 }
}

class EdgeNumbers {
	static NO_EDGE { 0 }
	static EDGE1 { 1 }
	static EDGE2 { 2 }
	static EDGE3 { 3 }
	static EDGE4 { 4 }
}

class ClipVertex {
	construct new() {
		_v = Vec2.new()
		_fp = FeaturePair.new(0)
	}
	v { _v }
	fp { _fp }
	v=(v) { _v = v }
	fp=(v) { _fp = v }
}

class Collision {
	static clipSegmentToLine(vOut, vIn, normal, offset, clipEdge) {
		// Start with no output points
		var numOut = 0

		// Calculate the distance of end points to the line
		var distance0 = Vec2.dot(normal, vIn[0].v) - offset
		var distance1 = Vec2.dot(normal, vIn[1].v) - offset

		// If the points are behind the plane
		if (distance0 <= 0.0) {
			vOut[numOut] = vIn[0]
			numOut = numOut + 1
		}
		if (distance1 <= 0.0) {
			vOut[numOut] = vIn[1]
			numOut = numOut + 1
		}

		// If the points are on different sides of the plane
		if (distance0 * distance1 < 0.0) {
			// Find intersection point of edge and plane
			var interp = distance0 / (distance0 - distance1)
			vOut[numOut].v = vIn[0].v + interp * (vIn[1].v - vIn[0].v)
			if (distance0 > 0.0) {
				vOut[numOut].fp = vIn[0].fp
				vOut[numOut].fp.inEdge1 = clipEdge
				vOut[numOut].fp.inEdge2 = EdgeNumbers.NO_EDGE
			} else {
				vOut[numOut].fp = vIn[1].fp
				vOut[numOut].fp.outEdge1 = clipEdge
				vOut[numOut].fp.outEdge2 = EdgeNumbers.NO_EDGE
			}
			numOut = numOut + 1
		}

		return numOut
	}

	static computeIncidentEdge(c, h, pos, Rot, normal) {
		// The normal is from the reference box. Convert it
		// to the incident boxe's frame and flip sign.
		var RotT = Rot.transpose
		var n = -(RotT * normal)
		var nAbs = n.abs

		if (nAbs.x > nAbs.y) {
			if (n.x.sign > 0.0) {
				c[0].v.set(h.x, -h.y)
				c[0].fp.inEdge2 = EdgeNumbers.EDGE3
				c[0].fp.outEdge2 = EdgeNumbers.EDGE4

				c[1].v.set(h.x, h.y)
				c[1].fp.inEdge2 = EdgeNumbers.EDGE4
				c[1].fp.outEdge2 = EdgeNumbers.EDGE1
			} else {
				c[0].v.set(-h.x, h.y)
				c[0].fp.inEdge2 = EdgeNumbers.EDGE1
				c[0].fp.outEdge2 = EdgeNumbers.EDGE2

				c[1].v.set(-h.x, -h.y)
				c[1].fp.inEdge2 = EdgeNumbers.EDGE2
				c[1].fp.outEdge2 = EdgeNumbers.EDGE3
			}
		} else {
			if (n.y.sign > 0.0) {
				c[0].v.set(h.x, h.y)
				c[0].fp.inEdge2 = EdgeNumbers.EDGE4
				c[0].fp.outEdge2 = EdgeNumbers.EDGE1

				c[1].v.set(-h.x, h.y)
				c[1].fp.inEdge2 = EdgeNumbers.EDGE1
				c[1].fp.outEdge2 = EdgeNumbers.EDGE2
			} else {
				c[0].v.set(-h.x, -h.y)
				c[0].fp.inEdge2 = EdgeNumbers.EDGE2
				c[0].fp.outEdge2 = EdgeNumbers.EDGE3

				c[1].v.set(h.x, -h.y)
				c[1].fp.inEdge2 = EdgeNumbers.EDGE3
				c[1].fp.outEdge2 = EdgeNumbers.EDGE4
			}
		}

		c[0].v = pos + Rot * c[0].v
		c[1].v = pos + Rot * c[1].v
	}

	// The normal points from A to B
	static collide(b1, b2) {
		// Setup
		var hA = b1.width * 0.5
		var hB = b2.width * 0.5

		var posA = b1.position
		var posB = b2.position

		var RotA = Mat22.new(b1.rotation)
		var RotB = Mat22.new(b2.rotation)

		var RotAT = RotA.transpose
		var RotBT = RotB.transpose

		var dp = posB - posA
		var dA = RotAT * dp
		var dB = RotBT * dp

		var C = RotAT * RotB
		var absC = C.abs
		var absCT = absC.transpose

		// Box A faces
		var faceA = dA.abs - hA - absC * hB
		if (faceA.x > 0.0 || faceA.y > 0.0) {
			return []
		}

		// Box B faces
		var faceB = dB.abs - absCT * hA - hB
		if (faceB.x > 0.0 || faceB.y > 0.0) {
			return []
		}

		// Find best axis
		var axis = 0
		var separation = 0
		var normal = Vec2.new()

		// Box A faces
		axis = Axis.FACE_A_X
		separation = faceA.x
		normal = dA.x > 0.0 ? RotA.col1 : -RotA.col1

		var relativeTol = 0.95
		var absoluteTol = 0.01

		if (faceA.y > relativeTol * separation + absoluteTol * hA.y) {
			axis = Axis.FACE_A_Y
			separation = faceA.y
			normal = dA.y > 0.0 ? RotA.col2 : -RotA.col2
		}

		// Box B faces
		if (faceB.x > relativeTol * separation + absoluteTol * hB.x) {
			axis = Axis.FACE_B_X
			separation = faceB.x
			normal = dB.x > 0.0 ? RotB.col1 : -RotB.col1
		}

		if (faceB.y > relativeTol * separation + absoluteTol * hB.y) {
			axis = Axis.FACE_B_Y
			separation = faceB.y
			normal = dB.y > 0.0 ? RotB.col2 : -RotB.col2
		}

		// Setup clipping plane data based on the separating axis
		var frontNormal = Vec2.new()
		var sideNormal = Vec2.new()
		var incidentEdge = [ClipVertex.new(), ClipVertex.new()]
		var front = 0
		var negSide = 0
		var posSide = 0
		var negEdge = 0
		var posEdge = 0

		// Compute the clipping lines and the line segment to be clipped.
		if (axis == Axis.FACE_A_X) {
			frontNormal = normal
			front = Vec2.dot(posA, frontNormal) + hA.x
			sideNormal = RotA.col2
			var side = Vec2.dot(posA, sideNormal)
			negSide = -side + hA.y
			posSide =  side + hA.y
			negEdge = EdgeNumbers.EDGE3
			posEdge = EdgeNumbers.EDGE1
			computeIncidentEdge(incidentEdge, hB, posB, RotB, frontNormal)
		} else if (axis == Axis.FACE_A_Y) {
			frontNormal = normal
			front = Vec2.dot(posA, frontNormal) + hA.y
			sideNormal = RotA.col1
			var side = Vec2.dot(posA, sideNormal)
			negSide = -side + hA.x
			posSide =  side + hA.x
			negEdge = EdgeNumbers.EDGE2
			posEdge = EdgeNumbers.EDGE4
			computeIncidentEdge(incidentEdge, hB, posB, RotB, frontNormal)
		} else if (axis == Axis.FACE_B_X) {
			frontNormal = -normal
			front = Vec2.dot(posB, frontNormal) + hB.x
			sideNormal = RotB.col2
			var side = Vec2.dot(posB, sideNormal)
			negSide = -side + hB.y
			posSide =  side + hB.y
			negEdge = EdgeNumbers.EDGE3
			posEdge = EdgeNumbers.EDGE1
			computeIncidentEdge(incidentEdge, hA, posA, RotA, frontNormal)
		} else if (axis == Axis.FACE_B_Y) {
			frontNormal = -normal
			front = Vec2.dot(posB, frontNormal) + hB.y
			sideNormal = RotB.col1
			var side = Vec2.dot(posB, sideNormal)
			negSide = -side + hB.x
			posSide =  side + hB.x
			negEdge = EdgeNumbers.EDGE2
			posEdge = EdgeNumbers.EDGE4
			computeIncidentEdge(incidentEdge, hA, posA, RotA, frontNormal)
		}

		// clip other face with 5 box planes (1 face plane, 4 edge planes)

		var clipPoints1 = [ClipVertex.new(), ClipVertex.new()]
		var clipPoints2 = [ClipVertex.new(), ClipVertex.new()]
		var np = 0

		// Clip to box side 1
		np = clipSegmentToLine(clipPoints1, incidentEdge, -sideNormal, negSide, negEdge)

		if (np < 2) {
			return []
		}

		// Clip to negative box side 1
		np = clipSegmentToLine(clipPoints2, clipPoints1,  sideNormal, posSide, posEdge)

		if (np < 2) {
			return []
		}

		// Now clipPoints2 contains the clipping points.
		// Due to roundoff, it is possible that clipping removes all points.

		var contacts = []
		for (i in 0...2) {
			var separation = Vec2.dot(frontNormal, clipPoints2[i].v) - front
			if (separation <= 0) {
				var c = Contact.new()
				c.separation = separation
				c.normal = normal
				// slide contact point onto reference face (easy to cull)
				c.position = clipPoints2[i].v - frontNormal * separation
				c.feature = clipPoints2[i].fp
				if (axis == Axis.FACE_B_X || axis == Axis.FACE_B_Y) {
					c.feature.flip
				}
			} else {
				System.print("As")
			}
		}

		return contacts
	}
}