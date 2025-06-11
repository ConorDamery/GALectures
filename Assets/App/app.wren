class App {

	// ==============================
    // Application
    // ==============================

	// Waits for the specified amount of time.
	//
	// @param ms (u32) The number of milliseconds to wait.
	foreign static wait(ms)

	// Checks if the application is running in headless mode.
	//
	// @return (bool) `true` if running headless, `false` otherwise.
	foreign static isHeadless

	// ==============================
    // Window
    // ==============================

	// eWinMode : i32 - Window Mode Constants

	static eWinModeWindowed { 0 }
	static eWinModeBorderless { 0 }
	static eWinModeFullscreen { 0 }

	// Sets the window display mode.
	//
	// @param mode (eWinMode) The desired window mode (e.g., windowed, fullscreen, borderless).
	foreign static winMode(mode)

	// eWinCursor : i32 - Window Cursor Mode Constants

    static eWinCursorNormal { 0x00034001 } // Cursor is visible and can move freely (default behavior).
    static eWinCursorHidden { 0x00034002 } // Cursor is hidden when inside the window but still active.
    static eWinCursorDisabled { 0x00034003 } // Cursor is disabled and locked to the window (useful for FPS controls).
	static eWinCursorCaptured { 0x00034004 } // ???

	// Sets the mouse cursor mode.
	//
	// @param cursor (eWinCursor) The desired cursor mode.
	foreign static winCursor(cursor)

	// Enables or disables the window always-on-top behavior.
	//
	// @param enabled (bool) `true` to keep the window on top, `false` otherwise.
	foreign static winAlwaysOnTop(enabled)

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

    // eWinButton : i32 - Mouse Button Constants

	static eWinButton1 { 0 }
	static eWinButton2 { 1 }
	static eWinButton3 { 2 }
	static eWinButton4 { 3 }
	static eWinButton5 { 4 }
	static eWinButton6 { 5 }
	static eWinButton7 { 6 }
	static eWinButton8 { 7 }
	static eWinButtonLast { 7 }
	static eWinButtonLeft { eWinButton1 }
	static eWinButtonRight { eWinButton2 }
	static eWinButtonMiddle { eWinButton3 }

    // eWinKey : i32 - Keyboard Key Constants

	static eWinKeySpace { 32 }
	static eWinKeyApostrophe { 39 }
	static eWinKeyComma { 44 }
	static eWinKeyMinus { 45 }
	static eWinKeyPeriod { 46 }
	static eWinKeySlash { 47 }
	static eWinKey0 { 48 }
	static eWinKey1 { 49 }
	static eWinKey2 { 50 }
	static eWinKey3 { 51 }
	static eWinKey4 { 52 }
	static eWinKey5 { 53 }
	static eWinKey6 { 54 }
	static eWinKey7 { 55 }
	static eWinKey8 { 56 }
	static eWinKey9 { 57 }
	static eWinKeySemicolon { 59 }
	static eWinKeyEqual { 61 }
	static eWinKeyA { 65 }
	static eWinKeyB { 66 }
	static eWinKeyC { 67 }
	static eWinKeyD { 68 }
	static eWinKeyE { 69 }
	static eWinKeyF { 70 }
	static eWinKeyG { 71 }
	static eWinKeyH { 72 }
	static eWinKeyI { 73 }
	static eWinKeyJ { 74 }
	static eWinKeyK { 75 }
	static eWinKeyL { 76 }
	static eWinKeyM { 77 }
	static eWinKeyN { 78 }
	static eWinKeyO { 79 }
	static eWinKeyP { 80 }
	static eWinKeyQ { 81 }
	static eWinKeyR { 82 }
	static eWinKeyS { 83 }
	static eWinKeyT { 84 }
	static eWinKeyU { 85 }
	static eWinKeyV { 86 }
	static eWinKeyW { 87 }
	static eWinKeyX { 88 }
	static eWinKeyY { 89 }
	static eWinKeyZ { 90 }
	static eWinKeyLeftBracket { 91 }
	static eWinKeyBackslash { 92 }
	static eWinKeyRightBracket { 93 }
	static eWinKeyGraveAccent { 96 }
	static eWinKeyWorld1 { 161 }
	static eWinKeyWorld2 { 162 }

	static eWinKeyEscape { 256 }
	static eWinKeyEnter { 257 }
	static eWinKeyTab { 258 }
	static eWinKeyBackspace { 259 }
	static eWinKeyInsert { 260 }
	static eWinKeyDelete { 261 }
	static eWinKeyRight { 262 }
	static eWinKeyLeft { 263 }
	static eWinKeyDown { 264 }
	static eWinKeyUp { 265 }
	static eWinKeyPageUp { 266 }
	static eWinKeyPageDown { 267 }
	static eWinKeyHome { 268 }
	static eWinKeyEnd { 269 }
	static eWinKeyCapsLock { 280 }
	static eWinKeyScrollLock { 281 }
	static eWinKeyNumLock { 282 }
	static eWinKeyPrintScreen { 283 }
	static eWinKeyPause { 284 }
	static eWinKeyF1 { 290 }
	static eWinKeyF2 { 291 }
	static eWinKeyF3 { 292 }
	static eWinKeyF4 { 293 }
	static eWinKeyF5 { 294 }
	static eWinKeyF6 { 295 }
	static eWinKeyF7 { 296 }
	static eWinKeyF8 { 297 }
	static eWinKeyF9 { 298 }
	static eWinKeyF10 { 299 }
	static eWinKeyF11 { 300 }
	static eWinKeyF12 { 301 }
	static eWinKeyF13 { 302 }
	static eWinKeyF14 { 303 }
	static eWinKeyF15 { 304 }
	static eWinKeyF16 { 305 }
	static eWinKeyF17 { 306 }
	static eWinKeyF18 { 307 }
	static eWinKeyF19 { 308 }
	static eWinKeyF20 { 309 }
	static eWinKeyF21 { 310 }
	static eWinKeyF22 { 311 }
	static eWinKeyF23 { 312 }
	static eWinKeyF24 { 313 }
	static eWinKeyF25 { 314 }
	static eWinKeyKp0 { 320 }
	static eWinKeyKp1 { 321 }
	static eWinKeyKp2 { 322 }
	static eWinKeyKp3 { 323 }
	static eWinKeyKp4 { 324 }
	static eWinKeyKp5 { 325 }
	static eWinKeyKp6 { 326 }
	static eWinKeyKp7 { 327 }
	static eWinKeyKp8 { 328 }
	static eWinKeyKp9 { 329 }
	static eWinKeyKpDecimal { 330 }
	static eWinKeyKpDivide { 331 }
	static eWinKeyKpMultiply { 332 }
	static eWinKeyKpSubtract { 333 }
	static eWinKeyKpAdd { 334 }
	static eWinKeyKpEnter { 335 }
	static eWinKeyKpEqual { 336 }
	static eWinKeyLeftShift { 340 }
	static eWinKeyLeftControl { 341 }
	static eWinKeyLeftAlt { 342 }
	static eWinKeyLeftSuper { 343 }
	static eWinKeyRightShift { 344 }
	static eWinKeyRightControl { 345 }
	static eWinKeyRightAlt { 346 }
	static eWinKeyRightSuper { 347 }
	static eWinKeyMenu { 348 }

    // eWinPad : i32 - Gamepad Button & Axis Constants

	static eWinPadA { 0 }
	static eWinPadB { 1 }
	static eWinPadX { 2 }
	static eWinPadY { 3 }
	static eWinPadLeftBumper { 4 }
	static eWinPadRightBumper { 5 }
	static eWinPadBack { 6 }
	static eWinPadStart { 7 }
	static eWinPadGuide { 8 }
	static eWinPadLeftThumb { 9 }
	static eWinPadRightThumb { 10 }
	static eWinPadDpadUp { 11 }
	static eWinPadDpadRight { 12 }
	static eWinPadDpadDown { 13 }
	static eWinPadDpadLeft { 14 }
	static eWinPadLast { 14 }
	static eWinPadCross { 0 }
	static eWinPadCircle { 1 }
	static eWinPadSquare { 2 }
	static eWinPadTriangle { 3 }

	static eWinPadLeftX { 0 }
	static eWinPadLeftY { 1 }
	static eWinPadRightX { 2 }
	static eWinPadRightY { 3 }
	static eWinPadLeftTrigger { 4 }
	static eWinPadRightTrigger { 5 }

    // ==============================
    // Graphics
    // ==============================

	// Color Constants

	static glRed{ 0xFF0000FF }
	static glGreen{ 0xFF00FF00 }
	static glBlue{ 0xFFFF0000 }
	static glGray{ 0xFF505050 }

	// Sets the viewport dimensions.
    //
    // @param x (i32) The X position.
    // @param y (i32) The Y position.
    // @param w (u32) The width.
    // @param h (u32) The height.
    foreign static glViewport(x, y, w, h)

	// TODO
	foreign static glScissor(x, y, w, h)

	static glColor { 1 }
	static glDepth { 2 }
	static glStencil { 4 }
	static glClearAll { glColor | glDepth | glStencil }

    // Clears the screen with a specified color.
    //
    // @param r (f32) Red component (0 to 1).
    // @param g (f32) Green component (0 to 1).
    // @param b (f32) Blue component (0 to 1).
    // @param a (f32) Alpha component (0 to 1).
    // @param d (f64) Depth buffer clear value.
    // @param s (i32) Stencil buffer clear value.
    // @param flags (u32) Bitmask specifying which buffers to clear.
    foreign static glClear(r, g, b, a, d, s, flags)

    // Loads a shader from a file path.
    //
    // @param path (string) The file path to the shader.
    // @return (u32) The shader handle.
    foreign static glLoadShader(path)

	// TODO
	foreign static glCreateShader(source)

    // Destroys a previously created shader.
    //
    // @param shader (u32) The shader handle to be deleted.
    foreign static glDestroyShader(shader)

	// Sets the active shader for rendering.
	//
    // @param shader (u32) The shader handle.
	foreign static glSetShader(shader)

	// TODO
	foreign static glLoadImage(path, flipY)

	// TODO
	foreign static glDestroyImage(image)
	
	// TODO
	foreign static glImageWidth(image)

	// TODO
	foreign static glImageHeight(image)

	// TODO
	foreign static glImageChannels(image)

	// TODO
	foreign static glLoadModel(path)

	// TODO
	foreign static glDestroyModel(model)

	// Texture Format Constants

	static glTexFmtR8 { 0 }
	static glTexFmtRG8 { 1 }
	static glTexFmtRGB8 { 2 }
	static glTexFmtRGBA8 { 3 }

	// Texture Filter Constants

	static glTexFltNearest { 0 }
	static glTexFltLinear { 1 }

	// Texture Wrap Constants

	static glTexWrpRepeat { 0 }
	static glTexWrpClampEdge { 1 }

	// TODO
	foreign static glCreateTexture(image, format, minFilter, magFilter, wrapS, wrapT, genMipmaps)

	// TODO
	foreign static glDestroyTexture(texture)

	// Buffer Config Constants

	static glBuffVertex { 0 }
	static glBuffIndex { 1 }
	static glBuffUniform { 2 }
	static glBuffStorage { 3 }

	static glBuffUseImmutable { 0 }
	static glBuffUseDefault { 1 }
	static glBuffUseDynamic { 2 }

	static glBuffAccessNone { 0 }
	static glBuffAccessRead { 1 }
	static glBuffAccessWrite { 2 }

	// TODO
	foreign static glCreateBuffer(byteStride, type, usage, access)
	
	// TODO
	foreign static glDestroyBuffer(buffer)
	
	// TODO
	foreign static glBindBuffer(buffer)
	
	// TODO
	foreign static glSubmitBuffer(buffer)

	// Specifies a vertex position and color (utility from having to fill the entire vertex format).
    //
    // @param x (f32) X position.
    // @param y (f32) Y position.
    // @param z (f32) Z position.
    // @param c (u32) Color encoded as RGBA.
    static glAddVertex(x, y, z, c) {
		glAddVertex(x, y, z, 1, c, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
	}

	// TODO
	static glAddVertex(x, y, z, c, u, v) {
		glAddVertex(x, y, z, 1, c, 0, 0, 0, u, v, 0, 0, 0, 0, 0, 0)
	}

	// TODO
	foreign static glAddVertex(x, y, z, w, c0, c1, i0, i1, v0, v1, v2, v3, v4, v5, v6, v7)

    // Begins drawing with optional settings.
    //
	// @param alpha (bool) Enable alpha blending.
    // @param ztest (bool) Enable depth testing.
    // @param pointSize (f32) The size of points.
    // @param lineWidth (f32) The width of lines.
	foreign static glBegin(alpha, ztest, pointSize, lineWidth)

	// Primitive Constants

	static glPoints { 1 }
	static glLines { 2 }
	static glLineLoop { 4 }
	static glLineStrip { 8 }
    static glTriangles { 16 }
    static glTriangleStrip { 32 }
    static glTriangleFan { 64 }

	// TODO
	static glEnd(mode) {
		glEnd(false, mode, -1)
	}

    // Ends drawing and submits the primitives.
    //
    // @param indexed (bool) Whether we should use an index buffer or not.
	// @param mode (u32) The primitive type (e.g., `glTriangles`).
	// @param count (u32) The number of primitives to draw.
    foreign static glEnd(indexed, mode, count)

    // Specifies the name of the uniform variable to be set.
    // This must be called before setting any uniform value.
    //
    // @param name (string) The name of the uniform variable in the shader.
	foreign static glSetUniform(name)

    // Shader Uniforms
	// Requires calling `glUniform(name)` first.
	
	foreign static glSetTex2D(i, texture)
	foreign static glSetFloat(x)
	foreign static glSetVec2f(x, y)
	foreign static glSetVec3f(x, y, z)
	foreign static glSetVec4f(x, y, z, w)
	foreign static glSetMat2x2f(m00, m01, m10, m11)
	foreign static glSetMat2x3f(m00, m01, m02, m10, m11, m12)
	foreign static glSetMat2x4f(m00, m01, m02, m03, m10, m11, m12, m13)
	foreign static glSetMat3x2f(m00, m01, m10, m11, m20, m21)
	foreign static glSetMat3x3f(m00, m01, m02, m10, m11, m12, m20, m21, m22)
	foreign static glSetMat3x4f(m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23)
	foreign static glSetMat4x2f(m00, m01, m10, m11, m20, m21, m30, m31)
	foreign static glSetMat4x3f(m00, m01, m02, m10, m11, m12, m20, m21, m22, m30, m31, m32)
	foreign static glSetMat4x4f(m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33)

    // ==============================
    // GUI
    // ==============================

	// TODO
	foreign static guiPushItemWidth(w)
	
	// TODO
	foreign static guiPopItemWidth()
	
	// TODO
	foreign static guiText(text)

	// TODO
	foreign static guiAbsText(text, x, y, c)

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

	// TODO
	foreign static guiFloat(label, v, min, max)

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

	// TODO
	foreign static guiContentAvailWidth()
	
	// TODO
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

	// ==============================
    // Audio
    // ==============================

	// TODO
	foreign static sfxBindCallback()

	// TODO
	foreign static sfxUnbindCallback()

	// TODO
	foreign static sfxIsCallbackBound

	// TODO
	foreign static sfxLoadAudio(filepath)
	
	// TODO
	foreign static sfxDestroyAudio(audio)

	// TODO
	foreign static sfxCreateChannel(volume)
	
	// TODO
	foreign static sfxDestroyChannel(channel)
	
	// TODO
	foreign static sfxSetChannelVolume(channel, volume)
	
	// TODO
	foreign static sfxPlay(audio, channel, loop)
	
	// TODO
	foreign static sfxStop(audio, channel)

	// ==============================
    // Network
    // ==============================

	// Network Events

	static netEvConnect { 0 }
	static netEvReceive { 1 }
	static netEvDisconnect { 2 }
	static netEvTimeout { 3 }

	// Relay Modes

	static netPktReliable { 1 }
	static netPktUnsequenced { 2 }
	static netPktUnreliable { 8 }

	// TODO
	foreign static netStartServer(ip, port, peerCount, channelLimit)
	
	// TODO
	foreign static netStopServer()
	
	// TODO
	foreign static netConnectClient(ip, port, peerCount, channelLimit)
	
	// TODO
	foreign static netDisconnectClient(client)

	// TODO
	foreign static netMakeUuid()

	// TODO
	foreign static netIsServer()

	// TODO
	foreign static netIsClient(client)

	// TODO
	foreign static netCreatePacket(id, size)
	
	// TODO
	foreign static netPacketId(packet)

	// TODO
	foreign static netBroadcast(packet, mode)

	// TODO
	foreign static netSend(client, packet, mode)

	// TODO
	foreign static netGetBool(packet, offset)
	foreign static netGetUInt(packet, offset)
	foreign static netGetInt(packet, offset)
	foreign static netGetFloat(packet, offset)
	foreign static netGetDouble(packet, offset)
	foreign static netGetString(packet, offset)

	// TODO
	foreign static netSetBool(packet, offset, v)
	foreign static netSetUInt(packet, offset, v)
	foreign static netSetInt(packet, offset, v)
	foreign static netSetFloat(packet, offset, v)
	foreign static netSetDouble(packet, offset, v)
	foreign static netSetString(packet, offset, v)
}