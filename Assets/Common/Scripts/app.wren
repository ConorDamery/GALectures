class App {

	// ==============================
    // Window
    // ==============================

	static winModeWindowed { 0 }
	static winModeBorderless { 0 }
	static winModeFullscreen { 0 }

	foreign static winMode(mode)

    // Cursor is visible and can move freely (default behavior).
    static winCursorNormal { 0x00034001 }

    // Cursor is hidden when inside the window but still active.
    static winCursorHidden { 0x00034002 }

    // Cursor is disabled and locked to the window (useful for FPS controls).
    static winCursorDisabled { 0x00034003 }

	// ???
	static winCursorCaptured { 0x00034004 }

	foreign static winCursor(mode)

    // The width of the application window in pixels.
    //
    // @return (i32) The window width.
    foreign static winWidth

    // The height of the application window in pixels.
    //
    // @return (i32) The window height.
    foreign static winHeight

    // The current X position of the mouse in window coordinates.
    //
    // @return (f64) The mouse X position.
    foreign static winMouseX

    // The current Y position of the mouse in window coordinates.
    //
    // @return (f64) The mouse Y position.
    foreign static winMouseY

    // Checks if a specific mouse button is pressed.
    //
    // @param b (i32) The button ID.
    // @return (bool) `true` if the button is pressed, `false` otherwise.
    foreign static winButton(b)

    // Checks if a specific key is pressed.
    //
    // @param k (i32) The key code.
    // @return (bool) `true` if the key is pressed, `false` otherwise.
    foreign static winKey(k)

    // Returns the number of connected gamepads.
    //
    // @return (i32) The number of gamepads detected.
    foreign static winPadCount()

    // Checks if a specific button on a gamepad is pressed.
    //
    // @param i (i32) The gamepad index.
    // @param b (i32) The button ID.
    // @return (bool) `true` if the button is pressed, `false` otherwise.
    foreign static winPadButton(i, b)

    // Gets the value of a gamepad axis.
    //
    // @param i (i32) The gamepad index.
    // @param a (i32) The axis ID.
    // @return (f32) The axis value (-1 to 1).
    foreign static winPadAxis(i, a)

    // Closes the application window.
    foreign static winClose()

    // Mouse Button Constants

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

    // Keyboard Key Constants

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

    // Gamepad Button Constants

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

    // Gamepad Axis Constants

	static winPadLeftX { 0 }
	static winPadLeftY { 1 }
	static winPadRightX { 2 }
	static winPadRightY { 3 }
	static winPadLeftTrigger { 4 }
	static winPadRightTrigger { 5 }

    // ==============================
    // Graphics
    // ==============================

    // Creates a shader from a file path.
    //
    // @param path (string) The file path to the shader.
    // @return (u32) The shader handle.
    foreign static glCreateShader(path)

    // Destroys a previously created shader.
    //
    // @param shader (u32) The shader handle to be deleted.
    foreign static glDestroyShader(shader)

    // Sets the active shader for rendering.
    //
    // @param shader (u32) The shader handle.
    foreign static glSetShader(shader)

    // Begins drawing with optional settings.
    //
    // @param alpha (bool) Enable alpha blending.
    // @param ztest (bool) Enable depth testing.
    // @param pointSize (f32) The size of points.
    // @param lineWidth (f32) The width of lines.
    foreign static glBegin(alpha, ztest, pointSize, lineWidth)

    // Ends drawing and submits the primitives.
    //
    // @param mode (u32) The primitive type (e.g., `glTriangles`).
    foreign static glEnd(mode)

    // Sets the viewport dimensions.
    //
    // @param x (u32) The X position.
    // @param y (u32) The Y position.
    // @param w (u32) The width.
    // @param h (u32) The height.
    foreign static glViewport(x, y, w, h)

    // Clears the screen with a specified color.
    //
    // @param r (f32) Red component (0 to 1).
    // @param g (f32) Green component (0 to 1).
    // @param b (f32) Blue component (0 to 1).
    // @param a (f32) Alpha component (0 to 1).
    // @param d (f32) Depth buffer clear value.
    // @param s (f32) Stencil buffer clear value.
    // @param flags (u32) Bitmask specifying which buffers to clear.
    foreign static glClear(r, g, b, a, d, s, flags)

    // Specifies the name of the uniform variable to be set.
    // This must be called before setting any uniform value.
    //
    // @param name (string) The name of the uniform variable in the shader.
	foreign static glUniform(name)

    // Shader Uniforms
	// Requires calling `glUniform(name)` first.

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

    // Specifies a vertex position and color.
    //
    // @param x (f32) X position.
    // @param y (f32) Y position.
    // @param z (f32) Z position.
    // @param c (u32) Color encoded as RGBA.
    foreign static glVertex(x, y, z, c)

    // Color Constants

	static glRed{ 0xFF0000FF }
	static glGreen{ 0xFF00FF00 }
	static glBlue{ 0xFFFF0000 }
	static glGray{ 0xFF505050 }

    // Primitive Constants

	static glPoints { 1 }
	static glLines { 2 }
	static glLineLoop { 4 }
	static glLineStrip { 8 }
    static glTriangles { 16 }
    static glTriangleStrip { 32 }
    static glTriangleFan { 64 }

    // ==============================
    // GUI
    // ==============================

	foreign static guiPushItemWidth(w)
	foreign static guiPopItemWidth()
	foreign static guiText(text)

    // Creates a GUI checkbox.
    //
    // @param label (string) The label of the checkbox.
    // @param v (bool) The initial value.
    // @return (bool) The new state of the checkbox.
    foreign static guiBool(label, v)

    // Creates a GUI integer input field.
    //
    // @param label (string) The label of the input field.
    // @param i (i32) The initial value.
    // @return (i32) The new value.
    foreign static guiInt(label, i)

    // Creates a GUI integer input field with min and max values.
    //
    // @param label (string) The label of the input field.
    // @param i (i32) The initial value.
    // @param min (i32) The minimum allowed value.
    // @param max (i32) The maximum allowed value.
    // @return (i32) The new value.
    foreign static guiInt(label, i, min, max)

    // Creates a GUI float input field.
    //
    // @param label (string) The label of the input field.
    // @param v (f32) The initial value.
    // @return (f32) The new value.
    foreign static guiFloat(label, v)

    // Creates a GUI separator with a label.
    //
    // @param label (string) The label of the separator.
    foreign static guiSeparator(label)

    // Creates a GUI button.
    //
    // @param label (string) The label of the button.
    // @return (bool) `true` if the button was pressed.
    foreign static guiButton(label)

    // Forces the next GUI element to appear on the same line.
    foreign static guiSameLine()

	foreign static guiContentAvailWidth()
	foreign static guiContentAvailHeight()
	
    // Begins a child GUI container.
    //
    // @param name (string) The name of the child container.
    // @param w (f32) The width of the container.
    // @param h (f32) The height of the container.
    // @return (bool) `true` if the child is open.
    foreign static guiBeginChild(name, w, h)

    // Ends a child GUI container.
	foreign static guiEndChild()
}