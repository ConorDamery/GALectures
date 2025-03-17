class App {
	// Gui
	foreign static guiBool(label, v)
	foreign static guiInt(label, i)
	foreign static guiInt(label, i, min, max)
	foreign static guiFloat(label, v)
	foreign static guiSeparator(label)
	foreign static guiButton(label)
	foreign static guiSameLine()
	foreign static guiBeginChild(name, px, py)
	foreign static guiEndChild()

    // Window
    foreign static winWidth
	foreign static winHeight

	foreign static winMouseX
	foreign static winMouseY
	foreign static winButton(b)
	foreign static winKey(k)

	foreign static winClose()

    // Graphics
	static glPoints { 0 }
	static glLines { 1 }
	static glLineLoop { 2 }
	static glLineStrip { 3 }
    static glTriangles { 4 }
    static glTriangleStrip { 5 }
    static glTriangleFan { 6 }

	foreign static glCreateShader(path)
	foreign static glDestroyShader(shader)
	foreign static glSetShader(shader)
	
	foreign static glBegin(alpha, ztest, pointSize, lineWidth)
	foreign static glEnd(mode)

	foreign static glViewport(x, y, w, h)
	foreign static glClear(r, g, b, a, d, s, flags)

	// Shader uniforms
	foreign static glUniform(name)      // Set the current uniform name
	foreign static glFloat(x)
	foreign static glVec2f(x, y)
	foreign static glVec3f(x, y, z)
	foreign static glVec4f(x, y, z, w)
	foreign static glMat2x2f(m00, m01, m10, m11)
	foreign static glMat3x2f(m00, m01, m02, m10, m11, m12)
	foreign static glMat2x3f(m00, m01, m10, m11, m20, m21)
	foreign static glMat2x4f(m00, m01, m10, m11, m20, m21, m30, m31)
	foreign static glMat3x3f(m00, m01, m02, m10, m11, m12, m20, m21, m22)
	foreign static glMat4x3f(m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23)
	foreign static glMat3x4f(m00, m01, m02, m10, m11, m12, m20, m21, m22, m30, m31, m32)
	foreign static glMat4x4f(m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33)

	foreign static glVertex(x, y, z, c)
}