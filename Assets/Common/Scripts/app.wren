class App {
    // Window
    foreign static winWidth
	foreign static winHeight

	foreign static winMouseX
	foreign static winMouseY
	foreign static winButton(b)
	foreign static winKey(k)
	foreign static winPadCount()
	foreign static winPadButton(i, b)
	foreign static winPadAxis(i, a)

	static winButton1 { 0 }
	static winButton2 { 1 }
	static winButton3 { 2 }
	static winButton4 { 3 }
	static winButton5 { 4 }
	static winButton6 { 5 }
	static winButton7 { 6 }
	static winButton8 { 7 }
	static winButtonLast { 7 }
	static winButtonLeft { winButton1 }
	static winButtonRight { winButton2 }
	static winButtonMiddle { winButton3 }

	static winKeySpace { 32 }
	static winKeyApostrophe { 39 }
	static winKeyComma { 44 }
	static winKeyMinus { 45 }
	static winKeyPeriod { 46 }
	static winKeySlash { 47 }
	static winKey0 { 48 }
	static winKey1 { 49 }
	static winKey2 { 50 }
	static winKey3 { 51 }
	static winKey4 { 52 }
	static winKey5 { 53 }
	static winKey6 { 54 }
	static winKey7 { 55 }
	static winKey8 { 56 }
	static winKey9 { 57 }
	static winKeySemicolon { 59 }
	static winKeyEqual { 61 }
	static winKeyA { 65 }
	static winKeyB { 66 }
	static winKeyC { 67 }
	static winKeyD { 68 }
	static winKeyE { 69 }
	static winKeyF { 70 }
	static winKeyG { 71 }
	static winKeyH { 72 }
	static winKeyI { 73 }
	static winKeyJ { 74 }
	static winKeyK { 75 }
	static winKeyL { 76 }
	static winKeyM { 77 }
	static winKeyN { 78 }
	static winKeyO { 79 }
	static winKeyP { 80 }
	static winKeyQ { 81 }
	static winKeyR { 82 }
	static winKeyS { 83 }
	static winKeyT { 84 }
	static winKeyU { 85 }
	static winKeyV { 86 }
	static winKeyW { 87 }
	static winKeyX { 88 }
	static winKeyY { 89 }
	static winKeyZ { 90 }
	static winKeyLeftBracket { 91 }
	static winKeyBackslash { 92 }
	static winKeyRightBracket { 93 }
	static winKeyGraveAccent { 96 }
	static winKeyWorld1 { 161 }
	static winKeyWorld2 { 162 }

	static winKeyEscape { 256 }
	static winKeyEnter { 257 }
	static winKeyTab { 258 }
	static winKeyBackspace { 259 }
	static winKeyInsert { 260 }
	static winKeyDelete { 261 }
	static winKeyRight { 262 }
	static winKeyLeft { 263 }
	static winKeyDown { 264 }
	static winKeyUp { 265 }
	static winKeyPageUp { 266 }
	static winKeyPageDown { 267 }
	static winKeyHome { 268 }
	static winKeyEnd { 269 }
	static winKeyCapsLock { 280 }
	static winKeyScrollLock { 281 }
	static winKeyNumLock { 282 }
	static winKeyPrintScreen { 283 }
	static winKeyPause { 284 }
	static winKeyF1 { 290 }
	static winKeyF2 { 291 }
	static winKeyF3 { 292 }
	static winKeyF4 { 293 }
	static winKeyF5 { 294 }
	static winKeyF6 { 295 }
	static winKeyF7 { 296 }
	static winKeyF8 { 297 }
	static winKeyF9 { 298 }
	static winKeyF10 { 299 }
	static winKeyF11 { 300 }
	static winKeyF12 { 301 }
	static winKeyF13 { 302 }
	static winKeyF14 { 303 }
	static winKeyF15 { 304 }
	static winKeyF16 { 305 }
	static winKeyF17 { 306 }
	static winKeyF18 { 307 }
	static winKeyF19 { 308 }
	static winKeyF20 { 309 }
	static winKeyF21 { 310 }
	static winKeyF22 { 311 }
	static winKeyF23 { 312 }
	static winKeyF24 { 313 }
	static winKeyF25 { 314 }
	static winKeyKp0 { 320 }
	static winKeyKp1 { 321 }
	static winKeyKp2 { 322 }
	static winKeyKp3 { 323 }
	static winKeyKp4 { 324 }
	static winKeyKp5 { 325 }
	static winKeyKp6 { 326 }
	static winKeyKp7 { 327 }
	static winKeyKp8 { 328 }
	static winKeyKp9 { 329 }
	static winKeyKpDecimal { 330 }
	static winKeyKpDivide { 331 }
	static winKeyKpMultiply { 332 }
	static winKeyKpSubtract { 333 }
	static winKeyKpAdd { 334 }
	static winKeyKpEnter { 335 }
	static winKeyKpEqual { 336 }
	static winKeyLeftShift { 340 }
	static winKeyLeftControl { 341 }
	static winKeyLeftAlt { 342 }
	static winKeyLeftSuper { 343 }
	static winKeyRightShift { 344 }
	static winKeyRightControl { 345 }
	static winKeyRightAlt { 346 }
	static winKeyRightSuper { 347 }
	static winKeyMenu { 348 }

	static winPadA { 0 }
	static winPadB { 1 }
	static winPadX { 2 }
	static winPadY { 3 }
	static winPadLeftBumper { 4 }
	static winPadRightBumper { 5 }
	static winPadBack { 6 }
	static winPadStart { 7 }
	static winPadGuide { 8 }
	static winPadLeftThumb { 9 }
	static winPadRightThumb { 10 }
	static winPadDpadUp { 11 }
	static winPadDpadRight { 12 }
	static winPadDpadDown { 13 }
	static winPadDpadLeft { 14 }
	static winPadLast { 14 }
	static winPadCross { 0 }
	static winPadCircle { 1 }
	static winPadSquare { 2 }
	static winPadTriangle { 3 }

	static winPadLeftX { 0 }
	static winPadLeftY { 1 }
	static winPadRightX { 2 }
	static winPadRightY { 3 }
	static winPadLeftTrigger { 4 }
	static winPadRightTrigger { 5 }

	foreign static winClose()

    // Graphics
	static glRed{ 0xFF0000FF }
	static glGreen{ 0xFF00FF00 }
	static glBlue{ 0xFFFF0000 }
	static glGray{ 0xFF505050 }

	static glPoints { 1 }
	static glLines { 2 }
	static glLineLoop { 4 }
	static glLineStrip { 8 }
    static glTriangles { 16 }
    static glTriangleStrip { 32 }
    static glTriangleFan { 64 }

	foreign static glCreateShader(path)
	foreign static glDestroyShader(shader)
	foreign static glSetShader(shader)
	
	foreign static glBegin(alpha, ztest, pointSize, lineWidth)
	foreign static glEnd(mode)

	foreign static glViewport(x, y, w, h)
	foreign static glClear(r, g, b, a, d, s, flags)

	foreign static glUniform(name)
	foreign static glFloat(x)
	foreign static glVec2f(x, y)
	foreign static glVec3f(x, y, z)
	foreign static glVec4f(x, y, z, w)
	foreign static glMat2x2f(m00, m01, m10, m11)
	foreign static glMat2x3f(m00, m01, m02, m10, m11, m12)
	foreign static glMat2x4f(m00, m01, m02, m03, m10, m11, m12, m13)
	foreign static glMat3x2f(m00, m01, m10, m11, m20, m21)
	foreign static glMat3x3f(m00, m01, m02, m10, m11, m12, m20, m21, m22)
	foreign static glMat3x4f(m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23)
	foreign static glMat4x2f(m00, m01, m10, m11, m20, m21, m30, m31)
	foreign static glMat4x3f(m00, m01, m02, m10, m11, m12, m20, m21, m22, m30, m31, m32)
	foreign static glMat4x4f(m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33)

	foreign static glVertex(x, y, z, c)

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
}