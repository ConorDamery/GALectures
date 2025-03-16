class App {
	// Utils
	foreign static debugBool(label, v)
	foreign static debugInt(label, i)
	foreign static debugInt(label, i, min, max)
	foreign static debugFloat(label, v)
	foreign static debugSeparator(label)
	foreign static debugButton(label)

    // Window
    foreign static width
    foreign static height

	foreign static mouseX
	foreign static mouseY
	foreign static getButton(b)
	foreign static getKey(k)

	foreign static close()

    // Graphics
	static points { 0 }
	static lines { 1 }
    static triangles { 2 }

	foreign static createShader(path)
	foreign static destroyShader(shader)
	foreign static setShader(shader)
	
	foreign static begin(alpha, ztest, pointSize, lineWidth)
	foreign static end(mode)

	foreign static viewport(x, y, w, h)
	foreign static clear(r, g, b, a, d, s, flags)

	foreign static uniform(name)

	foreign static float(x)
    
    foreign static vec2f(x, y)
    foreign static vec3f(x, y, z)
    foreign static vec4f(x, y, z, w)

    foreign static mat2x2f(m00, m01, m10, m11)
    foreign static mat3x2f(m00, m01, m02, m10, m11, m12)
    foreign static mat2x3f(m00, m01, m10, m11, m20, m21)
    foreign static mat3x3f(m00, m01, m02, m10, m11, m12, m20, m21, m22)

    foreign static mat4x3f(m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23)
    foreign static mat3x4f(m00, m01, m02, m10, m11, m12, m20, m21, m22, m30, m31, m32)
    
	foreign static mat4x4f(m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33)

	foreign static vertex(x, y, z, c)
}