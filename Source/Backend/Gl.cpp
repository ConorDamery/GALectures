#include <App.hpp>

#include <glad/glad.h>
#include <backends/imgui_impl_opengl3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include <sstream>
#include <cstring>
#include <stdexcept>

namespace GASandbox
{
	struct sGlImage
	{
		i32 w{ 0 }, h{ 0 }, c{ 0 };
		u8* data{ nullptr };

		sGlImage() = default;
		sGlImage(i32 w, i32 h, i32 c, u8* data)
			: w(w), h(h), c(c), data(data)
		{}
	};

	struct sGlAnim
	{
		string name{};
		u64 translations{ 0 };
		u64 rotations{ 0 };
		u64 scales{ 0 };
	};

	enum struct sGlMeshAttr : u32
	{
		POSITION,
		NORMAL,
		TEXCOORD,
		TANGENT,
		COLOR,
		COUNT
	};

	struct sGlMesh
	{
		u64 attributes[(u32)sGlMeshAttr::COUNT]{ 0 };
		u64 indices{ 0 };
	};

	struct sGlNode
	{
		string name{};
		u64 children{ 0 };
		u32 mesh{ 0 };
		u32 anim{ 0 };
		u32 transform{ 0 };
	};

	struct sGlVertex
	{
		array<f32,4> pos = {0,0,0,0};
		array<u32,2> col = {0,0};
		array<u32,2> idx = {0,0};
		array<f32,8> v = {0,0,0,0,0,0,0,0};

		sGlVertex() = default;
		sGlVertex(const array<f32,4>& pos,
				 const array<u32,2>& col,
				 const array<u32,2>& idx,
				 const array<f32,8>& v)
			: pos(pos), col(col), idx(idx), v(v)
		{}
	};

	struct sGlGlobal
	{
		GLuint shader{ 0 };
		GLuint vao{ 0 };
		GLuint vbo{ 0 };

		cstring uniformName{ nullptr };

		list<GLuint> shaders{};
		list<sGlImage> images{};
		list<f32> g_modelData{}; // Float buffer for all model data
		list<sGlMesh> g_meshes{};
		list<sGlAnim> g_anims{};
		list<sGlNode> g_nodes{};
		list<u32> g_models{};
		list<GLuint> textures{};
		list<sGlVertex> vertices{};
	};
	static sGlGlobal g{};

	static u32 gl_extract_index(u64 encoded) { return (u32)(encoded >> 32); }
	static u32 gl_extract_count(u64 encoded) { return (u32)(encoded & 0xFFFFFFFF); }
	static u64 gl_encode_range(u32 index, u32 count) { return ((u64)index << 32) | count; }

	template <typename T>
	T& gl_get(list<T>& vec, u32 handle)
	{
		assert(handle > 0 && handle <= vec.size()); // Debug safety
		return vec[handle - 1];
	}

	template <typename T>
	const T& gl_get(const list<T>& vec, u32 handle)
	{
		assert(handle > 0 && handle <= vec.size());
		return vec[handle - 1];
	}

	#ifdef _DEBUG
	static cstring opengl_source(GLenum source)
	{
		switch (source)
		{
		case GL_DEBUG_SOURCE_API:               return "API";
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:     return "Window System";
		case GL_DEBUG_SOURCE_SHADER_COMPILER:   return "Shader Compiler";
		case GL_DEBUG_SOURCE_THIRD_PARTY:       return "Third Party";
		case GL_DEBUG_SOURCE_APPLICATION:       return "Application";
		case GL_DEBUG_SOURCE_OTHER:             return "Other";
		}

		return "Unknown";
	}

	static cstring opengl_type(GLenum type)
	{
		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR:               return "Error";
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "Deprecated Behaviour";
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "Undefined Behaviour";
		case GL_DEBUG_TYPE_PORTABILITY:         return "Portability";
		case GL_DEBUG_TYPE_PERFORMANCE:         return "Performance";
		case GL_DEBUG_TYPE_MARKER:              return "Marker";
		case GL_DEBUG_TYPE_PUSH_GROUP:          return "Push Group";
		case GL_DEBUG_TYPE_POP_GROUP:           return "Pop Group";
		case GL_DEBUG_TYPE_OTHER:               return "Other";
		}

		return "Unknown";
	}

	static cstring opengl_severity(GLenum severity)
	{
		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:            return "High";
		case GL_DEBUG_SEVERITY_MEDIUM:          return "Medium";
		case GL_DEBUG_SEVERITY_LOW:             return "Low";
		case GL_DEBUG_SEVERITY_NOTIFICATION:    return "Notification";
		}

		return "Unknown";
	}

	static void APIENTRY opengl_debug_callback(GLenum source, GLenum type, u32 id, GLenum severity, GLsizei length, cstring message, const void* userParam)
	{
		// Ignore non-significant error/warning codes
		if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
			return;

		if (type == GL_DEBUG_TYPE_ERROR)
			LOGE("GL %s - %s (%s) [%d]: %s", opengl_source(source), opengl_type(type), opengl_severity(severity), id, message);
		else if (type == GL_DEBUG_TYPE_OTHER)
			LOGI("GL %s - %s (%s) [%d]: %s", opengl_source(source), opengl_type(type), opengl_severity(severity), id, message);
		else
			LOGW("GL %s - %s (%s) [%d]: %s", opengl_source(source), opengl_type(type), opengl_severity(severity), id, message);
	}
	#endif

	bool App::GlInitialize(const sAppConfig& config)
	{
		if (!gladLoadGLLoader((GLADloadproc)WinGetProcAddress))
		{
			LOGE("Failed to initialize GLAD!");
			return false;
		}

	#ifdef _DEBUG
		i32 flags;
		glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
		if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
		{
			// Initialize debug output
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(opengl_debug_callback, nullptr);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		}
		else
		{
			LOGW("Failed to initialize OpenGL debug output.");
		}
	#endif

		glGenVertexArrays(1, &g.vao);
		glGenBuffers(1, &g.vbo);

		glBindVertexArray(g.vao);
		glBindBuffer(GL_ARRAY_BUFFER, g.vbo);

		const GLsizei stride = 64;
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, stride, (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (GLvoid*)(16));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (GLvoid*)(20));
		glEnableVertexAttribArray(3);
		glVertexAttribIPointer(3, 4, GL_UNSIGNED_BYTE, stride, (GLvoid*)(24));
		glEnableVertexAttribArray(4);
		glVertexAttribIPointer(4, 4, GL_UNSIGNED_BYTE, stride, (GLvoid*)(28));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(32));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(48));

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		return true;
	}

	void App::GlShutdown()
	{
		GlReload();

		glDeleteProgram(g.shader);
		glDeleteBuffers(1, &g.vbo);
		glDeleteVertexArrays(1, &g.vao);
	}

	void App::GlReload()
	{
		for (const auto shader : g.shaders)
			GlDestroyShader(shader);
		g.shaders.clear();

		for (const auto& image : g.images)
			if (image.data) stbi_image_free(image.data);
		g.images.clear();

		for (const auto texture : g.textures)
			GlDestroyTexture(texture);
		g.textures.clear();

		// Graphics API
		#define GET_SLOT_FUNC(t) CodeGetSlot##t
		#define SCRIPT_ARGS_ARR(l, s, n, p, t) \
  				p l[n]; \
  				for (u8 i = 0; i < n; ++i) l[i] = GET_SLOT_FUNC(t)(vm, s + i + 1);
	
		CodeBindMethod("app", "App", true, "glViewport(_,_,_,_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 4);
				SCRIPT_ARGS_ARR(vi, 0, 2, i32, Int);
				SCRIPT_ARGS_ARR(vu, 3, 2, u32, UInt);
				GlViewport(vi[0], vi[1], vu[0], vu[1]);
			});

		CodeBindMethod("app", "App", true, "glScissor(_,_,_,_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 4);
				SCRIPT_ARGS_ARR(vi, 0, 2, i32, Int);
				SCRIPT_ARGS_ARR(vu, 3, 2, u32, UInt);
				GlScissor(vi[0], vi[1], vu[0], vu[1]);
			});

		CodeBindMethod("app", "App", true, "glClear(_,_,_,_,_,_,_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 7);
				SCRIPT_ARGS_ARR(v, 0, 4, f32, Float);
				GlClear(v[0], v[1], v[2], v[3], CodeGetSlotDouble(vm, 7), CodeGetSlotInt(vm, 7), (eGlClearFlags)CodeGetSlotUInt(vm, 7));
			});

		CodeBindMethod("app", "App", true, "glLoadShader(_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 1);
				auto shader = GlLoadShader(CodeGetSlotString(vm, 1));
				CodeSetSlotUInt(vm, 0, shader);
			});

		CodeBindMethod("app", "App", true, "glCreateShader(_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 1);
				auto shader = GlCreateShader(CodeGetSlotString(vm, 1));
				CodeSetSlotUInt(vm, 0, shader);
			});

		CodeBindMethod("app", "App", true, "glDestroyShader(_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 1);
				auto shader = CodeGetSlotUInt(vm, 1);
				GlDestroyShader(shader);
			});

		CodeBindMethod("app", "App", true, "glSetShader(_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 1);
				auto shader = CodeGetSlotUInt(vm, 1);
				GlSetShader(shader);
			});

		CodeBindMethod("app", "App", true, "glLoadImage(_,_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 2);
				auto image = GlLoadImage(CodeGetSlotString(vm, 1), CodeGetSlotBool(vm, 2));
				CodeSetSlotUInt(vm, 0, image);
			});

		//WrenBindMethod("app", "App", true, "glCreateImage(_,_)",
		//	[](sScriptVM* vm)
		//	{
		//		WrenEnsureSlots(vm, 2);
		//		auto image = GlCreateImage(WrenGetSlotString(vm, 1), WrenGetSlotBool(vm, 2));
		//		WrenSetSlotUInt(vm, 0, image);
		//	});

		CodeBindMethod("app", "App", true, "glDestroyImage(_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 1);
				auto image = CodeGetSlotUInt(vm, 1);
				GlDestroyImage(image);
			});

		CodeBindMethod("app", "App", true, "glImageWidth(_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 1);
				auto image = CodeGetSlotUInt(vm, 1);
				CodeSetSlotInt(vm, 0, GlImageWidth(image));
			});

		CodeBindMethod("app", "App", true, "glImageHeight(_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 1);
				auto image = CodeGetSlotUInt(vm, 1);
				CodeSetSlotInt(vm, 0, GlImageHeight(image));
			});

		CodeBindMethod("app", "App", true, "glImageChannels(_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 1);
				auto image = CodeGetSlotUInt(vm, 1);
				CodeSetSlotInt(vm, 0, GlImageChannels(image));
			});

		CodeBindMethod("app", "App", true, "glLoadModel(_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 1);
				auto model = GlLoadModel(CodeGetSlotString(vm, 1));
				CodeSetSlotUInt(vm, 0, model);
			});

		CodeBindMethod("app", "App", true, "glDestroyModel(_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 1);
				auto model = CodeGetSlotUInt(vm, 1);
				GlDestroyModel(model);
			});

		CodeBindMethod("app", "App", true, "glCreateBuffer(_,_,_,_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 4);
				SCRIPT_ARGS_ARR(a, 0, 4, u32, UInt);
				GlCreateBuffer(a[0], (eGlBufferType)a[1], (eGlBufferUsage)a[2], (eGlBufferAccess)a[3]);
			});

		CodeBindMethod("app", "App", true, "glDestroyBuffer(_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 1);
				GlDestroyBuffer(CodeGetSlotUInt(vm, 1));
			});

		CodeBindMethod("app", "App", true, "glBindBuffer(_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 1);
				GlBindBuffer(CodeGetSlotUInt(vm, 1));
			});

		CodeBindMethod("app", "App", true, "glSubmitBuffer(_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 1);
				GlSubmitBuffer(CodeGetSlotUInt(vm, 1));
			});

		CodeBindMethod("app", "App", true, "glAddVertex(_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 5);
				SCRIPT_ARGS_ARR(p, 0, 4, f32, Float);
				SCRIPT_ARGS_ARR(c, 4, 4, u32, UInt);
				SCRIPT_ARGS_ARR(v, 8, 8, f32, Float);
				GlAddVertex(
					p[0], p[1], p[2], p[3], c[0], c[1], c[2], c[3],
					v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
			});

		CodeBindMethod("app", "App", true, "glCreateTexture(_,_,_,_,_,_,_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 7);
				SCRIPT_ARGS_ARR(v, 0, 6, u32, UInt);
				auto texture = GlCreateTexture(v[0], (eGlTextureFormat)v[1], (eGlTextureFilter)v[2], (eGlTextureFilter)v[3], (eGlTextureWrap)v[4], (eGlTextureWrap)v[5], CodeGetSlotBool(vm, 7));
				CodeSetSlotUInt(vm, 0, texture);
			});

		CodeBindMethod("app", "App", true, "glDestroyTexture(_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 1);
				auto texture = CodeGetSlotUInt(vm, 1);
				GlDestroyTexture(texture);
			});

		CodeBindMethod("app", "App", true, "glBegin(_,_,_,_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 4);
				GlBegin(CodeGetSlotBool(vm, 1), CodeGetSlotBool(vm, 2), CodeGetSlotFloat(vm, 3), CodeGetSlotFloat(vm, 4));
			});

		CodeBindMethod("app", "App", true, "glEnd(_,_,_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 3);
				GlEnd(CodeGetSlotBool(vm, 1), CodeGetSlotUInt(vm, 2), CodeGetSlotUInt(vm, 3));
			});

		CodeBindMethod("app", "App", true, "glSetUniform(_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 1);
				cstring value = CodeGetSlotString(vm, 1);
				GlSetUniform(value);
			});

		CodeBindMethod("app", "App", true, "glSetTex2D(_,_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 2);
				GlSetTex2D(CodeGetSlotUInt(vm, 1), CodeGetSlotUInt(vm, 2));
			});

		CodeBindMethod("app", "App", true, "glSetFloat(_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 1);
				f32 value = CodeGetSlotFloat(vm, 1);
				GlSetFloat(value);
			});

		CodeBindMethod("app", "App", true, "glSetVec2f(_,_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 2);
				SCRIPT_ARGS_ARR(v, 0, 2, f32, Float);
				GlSetVec2F(v[0], v[1]);
			});

		CodeBindMethod("app", "App", true, "glSetVec3f(_,_,_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 3);
				SCRIPT_ARGS_ARR(v, 0, 3, f32, Float);
				GlSetVec3F(v[0], v[1], v[2]);
			});

		CodeBindMethod("app", "App", true, "glSetVec4f(_,_,_,_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 4);
				SCRIPT_ARGS_ARR(v, 0, 4, f32, Float);
				GlSetVec4F(v[0], v[1], v[2], v[3]);
			});

		CodeBindMethod("app", "App", true, "glSetMat2x2f(_,_,_,_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 4);
				SCRIPT_ARGS_ARR(v, 0, 4, f32, Float);
				GlSetMat2x2F(v[0], v[1], v[2], v[3]);
			});

		CodeBindMethod("app", "App", true, "glSetMat2x3f(_,_,_,_,_,_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 6);
				SCRIPT_ARGS_ARR(v, 0, 6, f32, Float);
				GlSetMat2x3F(v[0], v[1], v[2], v[3], v[4], v[5]);
			});

		CodeBindMethod("app", "App", true, "glSetMat2x4f(_,_,_,_,_,_,_,_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 8);
				SCRIPT_ARGS_ARR(v, 0, 8, f32, Float);
				GlSetMat2x4F(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
			});

		CodeBindMethod("app", "App", true, "glSetMat3x2f(_,_,_,_,_,_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 6);
				SCRIPT_ARGS_ARR(v, 0, 6, f32, Float);
				GlSetMat3x2F(v[0], v[1], v[2], v[3], v[4], v[5]);
			});

		CodeBindMethod("app", "App", true, "glSetMat3x3f(_,_,_,_,_,_,_,_,_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 9);
				SCRIPT_ARGS_ARR(v, 0, 9, f32, Float);
				GlSetMat3x3F(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8]);
			});

		CodeBindMethod("app", "App", true, "glSetMat3x4f(_,_,_,_,_,_,_,_,_,_,_,_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 12);
				SCRIPT_ARGS_ARR(v, 0, 12, f32, Float);
				GlSetMat3x4F(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8], v[9], v[10], v[11]);
			});

		CodeBindMethod("app", "App", true, "glSetMat4x2f(_,_,_,_,_,_,_,_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 8);
				SCRIPT_ARGS_ARR(v, 0, 8, f32, Float);
				GlSetMat4x2F(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
			});

		CodeBindMethod("app", "App", true, "glSetMat4x3f(_,_,_,_,_,_,_,_,_,_,_,_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 12);
				SCRIPT_ARGS_ARR(v, 0, 12, f32, Float);
				GlSetMat4x3F(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8], v[9], v[10], v[11]);
			});

		CodeBindMethod("app", "App", true, "glSetMat4x4f(_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_)",
			[](sCodeVM* vm)
			{
				CodeEnsureSlots(vm, 16);
				SCRIPT_ARGS_ARR(v, 0, 16, f32, Float);
				GlSetMat4x4F(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]);
			});
	}

	bool App::GuiGlInitialize()
	{
		return ImGui_ImplOpenGL3_Init("#version 330 core\n");
	}

	void App::GuiGlShutdown()
	{
		ImGui_ImplOpenGL3_Shutdown();
	}

	void App::GuiGlNewFrame()
	{
		ImGui_ImplOpenGL3_NewFrame();
	}

	void App::GuiGlRender()
	{
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR)
		{
			LOGE("OpenGL Error: 0x%X", err);
		}
	}

	static GLuint opengl_load_shader(cstring vsrc, cstring fsrc)
	{
		// Compile vertex shader
		GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vshader, 1, &vsrc, NULL);
		glCompileShader(vshader);

		i32 success = 0;
		char log[1024];
		glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vshader, 1024, NULL, log);
			LOGE("Failed to compile vertex shader: %s", log);
		}

		// Compile fragment shader
		GLuint fshader(glCreateShader(GL_FRAGMENT_SHADER));
		glShaderSource(fshader, 1, &fsrc, NULL);
		glCompileShader(fshader);

		glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fshader, 1024, NULL, log);
			LOGE("Failed to compile fragment shader: %s", log);
		}

		// Link vertex and fragment shader together
		GLuint program = glCreateProgram();
		glAttachShader(program, vshader);
		glAttachShader(program, fshader);
		glLinkProgram(program);

		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(program, 1024, NULL, log);
			LOGE("Failed to link shader program: %s", log);
		}

		glValidateProgram(program);
		glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(program, 1024, NULL, log);
			LOGE("Shader validation error: %s", log);
		}

		// Delete shaders objects
		glDeleteShader(vshader);
		glDeleteShader(fshader);

		if (!success)
		{
			glDeleteProgram(program);
			return 0;
		}

		g.shaders.emplace_back(program);
		return program;
	}

	static string shader_process_includes(const string& source)
	{
		std::stringstream processed;
		std::istringstream input(source);
		string line;
		static string includePath;

		while (std::getline(input, line))
		{
			// Check for #include "filename"
			if (line.rfind("#include", 0) == 0)
			{
				size_type start = line.find('"');
				size_type end = line.rfind('"');
				if (start != string::npos && end != string::npos && start < end)
				{
					size_type len = end - start - 1;

					// Ensure buffer is large enough
					includePath.resize(len);
					std::memcpy(&includePath[0], line.c_str() + start + 1, len);
					includePath[len] = '\0'; // Null-terminate manually

					string includedSource = App::FileLoad(includePath.c_str());
					processed << shader_process_includes(includedSource) << "\n";
					continue; // Skip writing the #include line itself
				}
			}
			processed << line << "\n";
		}
		return processed.str();
	}

	u32 App::GlLoadShader(cstring filepath)
	{
		auto src = shader_process_includes(FileLoad(filepath));
		auto vsrc = "#version 330 core\n#define VERT\n" + src;
		auto fsrc = "#version 330 core\n#define FRAG\n" + src;
		auto shader = opengl_load_shader(vsrc.c_str(), fsrc.c_str());
		if (shader == 0)
			LOGW("Failed to load shader from file: %s", filepath);
		return shader;
	}

	u32 App::GlCreateShader(cstring source)
	{
		auto src = shader_process_includes(source);
		auto vsrc = "#version 330 core\n#define VERT\n" + src;
		auto fsrc = "#version 330 core\n#define FRAG\n" + src;
		auto shader = opengl_load_shader(vsrc.c_str(), fsrc.c_str());
		if (shader == 0)
			LOGW("Failed to create shader from source.");
		return shader;
	}

	void App::GlDestroyShader(u32 shader)
	{
		glDeleteProgram(shader);
	}

	void App::GlSetShader(u32 shader)
	{
		g.shader = shader;
		glUseProgram(g.shader);
	}

	static sGlImage* gl_get_image(u32 image)
	{
		if (image == 0 || image > g.images.size())
		{
			LOGW("Invalid image handle!");
			return nullptr;
		}

		return &g.images[static_cast<size_type>(image) - 1];
	}

	u32 App::GlLoadImage(cstring filepath, bool flipY)
	{
		cstring path = FilePath(filepath);

		sGlImage img{};
		stbi_set_flip_vertically_on_load(flipY);
		img.data = stbi_load(path, &img.w, &img.h, &img.c, 0);
		if (!img.data)
		{
			LOGW("Failed to load image: %s", path);
			return -1;
		}

		g.images.emplace_back(img);
		return (u32)(g.images.size());
	}

	u32 App::GlCreateImage(i32 w, i32 h, i32 c, u8* data)
	{
		sGlImage img{ w, h, c, data };

		g.images.emplace_back(img);
		return (u32)(g.images.size());
	}

	void App::GlDestroyImage(u32 image)
	{
		auto img = gl_get_image(image);
		if (img == nullptr) return;
		stbi_image_free(img->data);
		*img = sGlImage{};
	}

	i32 App::GlImageWidth(u32 image)
	{
		auto img = gl_get_image(image);
		if (img == nullptr) return 0;
		return img->w;
	}

	i32 App::GlImageHeight(u32 image)
	{
		auto img = gl_get_image(image);
		if (img == nullptr) return 0;
		return img->h;
	}

	i32 App::GlImageChannels(u32 image)
	{
		auto img = gl_get_image(image);
		if (img == nullptr) return 0;
		return img->c;
	}

	u8* App::GlImageData(u32 image)
	{
		auto img = gl_get_image(image);
		if (img == nullptr) return nullptr;
		return img->data;
	}

	static void gltf_free(cgltf_data* data)
	{
		if (data != nullptr)
			cgltf_free(data);
	}

	static cgltf_data* gltf_load(cstring filepath)
	{
		cgltf_options options = {};
		cgltf_data* data{ nullptr };

		cgltf_result result = cgltf_parse_file(&options, filepath, &data);
		if (result != cgltf_result_success)
		{
			LOGW("Failed to parse glTF file: %s", filepath);
			return nullptr;
		}

		result = cgltf_load_buffers(&options, data, filepath);
		if (result != cgltf_result_success)
		{
			LOGW("Failed to load buffers for glTF file: %s", filepath);
			gltf_free(data);
			return nullptr;
		}

		result = cgltf_validate(data);
		if (result != cgltf_result_success)
		{
			LOGW("Invalid glTF file: %s", filepath);
			gltf_free(data);
			return nullptr;
		}

		return data;
	}

	static void gltf_extract_mesh_data(const cgltf_mesh& mesh)
	{
		sGlMesh glMesh;

		for (cgltf_size i = 0; i < mesh.primitives_count; ++i)
		{
			const cgltf_primitive& prim = mesh.primitives[i];

			// Extracting attributes
			for (cgltf_size j = 0; j < prim.attributes_count; ++j)
			{
				const cgltf_attribute& attribute = prim.attributes[j];
				u64& attributeHandle = glMesh.attributes[(u32)attribute.type];

				// Allocate space in global model data and link the attribute buffer
				if (attributeHandle == 0) {
					attributeHandle = gl_encode_range(g.g_modelData.size(), attribute.data->count);
					// Add attribute data to the global buffer here (e.g., position, normal, etc.)
					for (cgltf_size k = 0; k < attribute.data->count; ++k) {
						const cgltf_accessor& accessor = *attribute.data;
						// Convert data based on type and store in g.g_modelData
						// Example: copying float data for POSITION
						if (accessor.type == cgltf_type_vec3) {
							const float* position = (const float*)accessor.buffer_view->buffer->data + accessor.offset;
							g.g_modelData.insert(g.g_modelData.end(), position, position + 3);
						}
					}
				}
			}

			// Extracting indices
			if (prim.indices)
			{
				u64& indicesHandle = glMesh.indices;
				if (indicesHandle == 0) {
					indicesHandle = gl_encode_range(g.g_modelData.size(), prim.indices->count);
					// Allocate space in the global model data buffer for indices and copy them
					const u32* indices = (const u32*)prim.indices->buffer_view->buffer->data + prim.indices->offset;
					g.g_modelData.insert(g.g_modelData.end(), indices, indices + prim.indices->count);
				}
			}
		}

		g.g_meshes.push_back(glMesh);
	}

	static void gltf_extract_node_data(const cgltf_node& node, u64& childrenRange)
	{
		sGlNode glNode;

		glNode.name = node.name ? node.name : "Unnamed";

		// Extracting mesh and animation references
		//if (node.mesh)
		//	glNode.mesh = g.g_meshes.size() + 1;  // Store mesh handle
		//if (node.animation)
		//	glNode.anim = g.g_anims.size() + 1; // Store animation handle

		// Extracting children (stored as a range of indices)
		u32 childStart = g.g_nodes.size();
		for (cgltf_size i = 0; i < node.children_count; ++i)
		{
			gltf_extract_node_data(*node.children[i], childrenRange);
		}
		u32 childEnd = g.g_nodes.size();
		glNode.children = gl_encode_range(childStart, childEnd - childStart);

		g.g_nodes.push_back(glNode);
	}

	static void gltf_extract_animation_data(const cgltf_animation& animation)
	{
		sGlAnim glAnim;
		glAnim.name = animation.name ? animation.name : "Unnamed";

		for (cgltf_size i = 0; i < animation.channels_count; ++i)
		{
			const cgltf_animation_channel& channel = animation.channels[i];
			switch (channel.target_path) {
			case cgltf_animation_path_type_translation:
				// Store translation data (timestamp + vec3 translation)
				break;
			case cgltf_animation_path_type_rotation:
				// Store rotation data (timestamp + quat rotation)
				break;
			case cgltf_animation_path_type_scale:
				// Store scale data (timestamp + vec3 scale)
				break;
			default:
				break;
		}
	}

		g.g_anims.push_back(glAnim);
	}

	u32 App::GlLoadModel(cstring filepath)
	{
	#ifdef _DEBUG
		string pathStr = PROJECT_PATH;
		pathStr += filepath;
		cstring path = pathStr.c_str();
	#else
		cstring path = filepath;
	#endif

		auto data = gltf_load(path);
		if (data == nullptr)
		{
			throw std::runtime_error("Failed to load model!");
			return 0;
		}

		// Extract meshes
		for (cgltf_size i = 0; i < data->meshes_count; ++i)
		{
			gltf_extract_mesh_data(data->meshes[i]);
		}

		// Extract animations
		for (cgltf_size i = 0; i < data->animations_count; ++i)
		{
			gltf_extract_animation_data(data->animations[i]);
		}

		// Extract nodes (scenegraph)
		u64 childrenRange = 0;
		for (cgltf_size i = 0; i < data->scene->nodes_count; ++i)
		{
			gltf_extract_node_data(*data->scene->nodes[i], childrenRange);
		}

		// Store the model handle (root node)
		g.g_models.push_back(g.g_nodes.size());

		gltf_free(data); // Done with glTF data

		return (u32)g.g_models.size(); // Return the model handle
	}

	void App::GlDestroyModel(u32 model)
	{
		// This will not free the global data, just mark the model as destroyed if necessary
		// Currently, we just reset the root node (but children and other data are still intact)
		g.g_models[model] = 0;  // Reset root node handle to 0 (indicating 'destroyed' state)
	}

	u32 App::GlCreateBuffer(u32 byteStride, eGlBufferType type, eGlBufferUsage usage, eGlBufferAccess access)
	{
		return 0;
	}

	void App::GlDestroyBuffer(u32 buffer)
	{
	}

	void App::GlBindBuffer(u32 buffer)
	{
	}

	void App::GlSubmitBuffer(u32 buffer)
	{
	}

	static GLenum opengl_internal_format(eGlTextureFormat fmt)
	{
		switch (fmt)
		{
		case eGlTextureFormat::R8:    return GL_R8;
		case eGlTextureFormat::RG8:   return GL_RG8;
		case eGlTextureFormat::RGB8:  return GL_RGB8;
		case eGlTextureFormat::RGBA8: return GL_RGBA8;
		default:                   return GL_RGBA8;
		}
	}

	GLenum opengl_format(eGlTextureFormat fmt)
	{
		switch (fmt)
		{
		case eGlTextureFormat::R8:    return GL_RED;
		case eGlTextureFormat::RG8:   return GL_RG;
		case eGlTextureFormat::RGB8:  return GL_RGB;
		case eGlTextureFormat::RGBA8: return GL_RGBA;
		default:                   return GL_RGBA;
		}
	}

	GLenum opengl_type(eGlTextureFormat fmt)
	{
		return GL_UNSIGNED_BYTE;
	}

	GLenum opengl_filter(eGlTextureFilter filter, bool useMipmaps)
	{
		switch (filter)
		{
		case eGlTextureFilter::NEAREST:	return useMipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
		case eGlTextureFilter::LINEAR:	return useMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
		default:						return GL_LINEAR;
		}
	}

	GLenum opengl_wrap(eGlTextureWrap wrap)
	{
		return wrap == eGlTextureWrap::REPEAT ? GL_REPEAT : GL_CLAMP_TO_EDGE;
	}

	u32 App::GlCreateTexture(
		u32 image, eGlTextureFormat format,
		eGlTextureFilter minFilter, eGlTextureFilter magFilter,
		eGlTextureWrap wrapS, eGlTextureWrap wrapT,
		bool genMipmaps)
	{
		auto* img = gl_get_image(image);
		if (img == nullptr)
			return 0;

		if (!img->data || img->w <= 0 || img->h <= 0)
		{
			LOGW("Attempted to create texture from invalid image data.");
			return 0;
		}

		GLenum glInternalFormat = opengl_internal_format(format);
		GLenum glFormat = opengl_format(format);
		GLenum glType = opengl_type(format);

		u32 texture = 0;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		// Set texture parameters: filtering and wrapping
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, opengl_filter(minFilter, genMipmaps));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, opengl_filter(magFilter, false));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, opengl_wrap(wrapS));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, opengl_wrap(wrapT));

		// Upload the texture data
		glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, img->w, img->h, 0, glFormat, glType, img->data);

		if (genMipmaps)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		// Unbind the texture
		glBindTexture(GL_TEXTURE_2D, 0);
		return texture;
	}

	void App::GlDestroyTexture(u32 texture)
	{
		if (texture != 0)
			glDeleteTextures(1, &texture);
	}

	void App::GlBegin(bool alpha, bool ztest, f32 pointSize, f32 lineWidth)
	{
		if (alpha)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glBlendEquation(GL_FUNC_ADD);
		}
		else
		{
			glDisable(GL_BLEND);
		}

		if (ztest)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);

		glPointSize(pointSize);
		glLineWidth(lineWidth);

		g.vertices.clear();
	}

	void App::GlEnd(bool indexed, u32 mode, u32 count)
	{
		glBindVertexArray(g.vao);
		glBindBuffer(GL_ARRAY_BUFFER, g.vbo);

		glBufferData(GL_ARRAY_BUFFER, g.vertices.size() * sizeof(sGlVertex), g.vertices.data(), GL_DYNAMIC_DRAW);

		for (u32 bit = 1; bit <= (u32)eGlTopology::TRIANGLE_FAN; bit <<= 1)
		{
			if (mode & bit)
			{
				u32 i = 0, b = bit;
				while (b > 1 && ++i) b >>= 1;
				glDrawArrays(GL_POINTS + i, 0, (GLsizei)g.vertices.size());
			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void App::GlSetRenderTarget(u32 target, u32 depthStencil)
	{
	}

	void App::GlReadPixels(u32 x, u32 y, u32 w, u32 h, void* pixelData, u32 target)
	{
	}

	void App::GlViewport(i32 x, i32 y, u32 w, u32 h)
	{
		glViewport(x, y, w, h);
	}

	void App::GlScissor(i32 x, i32 y, u32 w, u32 h)
	{
		glScissor(x, y, w, h);
	}

	void App::GlClear(f32 r, f32 g, f32 b, f32 a, f64 d, i32 s, eGlClearFlags flags)
	{
		GLbitfield clear = 0;

		if (((u32)flags & (u32)eGlClearFlags::COLOR) != 0)
		{
			clear |= GL_COLOR_BUFFER_BIT;
			glClearColor(r, g, b, a);
		}

		if (((u32)flags & (u32)eGlClearFlags::DEPTH) != 0)
		{
			clear |= GL_DEPTH_BUFFER_BIT;
			glClearDepth(d);
		}

		if (((u32)flags & (u32)eGlClearFlags::STENCIL) != 0)
		{
			clear |= GL_STENCIL_BUFFER_BIT;
			glClearStencil(s);
		}

		glClear(clear);
	}

	void App::GlSetUniform(cstring name)
	{
		g.uniformName = name;
	}

	void App::GlSetTex2D(u32 i, u32 texture)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform1i(glGetUniformLocation(g.shader, g.uniformName), i);
	}

	void App::GlSetFloat(f32 x)
	{
		glUniform1f(glGetUniformLocation(g.shader, g.uniformName), x);
	}

	void App::GlSetVec2F(f32 x, f32 y)
	{
		glUniform2f(glGetUniformLocation(g.shader, g.uniformName), x, y);
	}

	void App::GlSetVec3F(f32 x, f32 y, f32 z)
	{
		glUniform3f(glGetUniformLocation(g.shader, g.uniformName), x, y, z);
	}

	void App::GlSetVec4F(f32 x, f32 y, f32 z, f32 w)
	{
		glUniform4f(glGetUniformLocation(g.shader, g.uniformName), x, y, z, w);
	}

	void App::GlSetMat2x2F(
		f32 m00, f32 m01,
		f32 m10, f32 m11)
	{
		const GLfloat v[4] = { m00, m01, m10, m11 };
		glUniformMatrix2fv(glGetUniformLocation(g.shader, g.uniformName), 1, GL_FALSE, v);
	}

	void App::GlSetMat2x3F(
		f32 m00, f32 m01, f32 m02,
		f32 m10, f32 m11, f32 m12)
	{
		const GLfloat v[6] = { m00, m01, m02, m10, m11, m12 };
		glUniformMatrix2x3fv(glGetUniformLocation(g.shader, g.uniformName), 1, GL_FALSE, v);
	}

	void App::GlSetMat2x4F(
		f32 m00, f32 m01, f32 m02, f32 m03,
		f32 m10, f32 m11, f32 m12, f32 m13)
	{
		const GLfloat v[8] = { m00, m01, m02, m03, m10, m11, m12, m13 };
		glUniformMatrix2x4fv(glGetUniformLocation(g.shader, g.uniformName), 1, GL_FALSE, v);
	}

	void App::GlSetMat3x2F(
		f32 m00, f32 m01,
		f32 m10, f32 m11,
		f32 m20, f32 m21)
	{
		const GLfloat v[6] = { m00, m01, m10, m11, m20, m21 };
		glUniformMatrix3x2fv(glGetUniformLocation(g.shader, g.uniformName), 1, GL_FALSE, v);
	}

	void App::GlSetMat3x3F(
		f32 m00, f32 m01, f32 m02,
		f32 m10, f32 m11, f32 m12,
		f32 m20, f32 m21, f32 m22)
	{
		const GLfloat v[9] = { m00, m01, m02, m10, m11, m12, m20, m21, m22 };
		glUniformMatrix3fv(glGetUniformLocation(g.shader, g.uniformName), 1, GL_FALSE, v);
	}

	void App::GlSetMat3x4F(
		f32 m00, f32 m01, f32 m02, f32 m03,
		f32 m10, f32 m11, f32 m12, f32 m13,
		f32 m20, f32 m21, f32 m22, f32 m23)
	{
		const GLfloat v[12] = { m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23 };
		glUniformMatrix3x4fv(glGetUniformLocation(g.shader, g.uniformName), 1, GL_FALSE, v);
	}

	void App::GlSetMat4x2F(
		f32 m00, f32 m01,
		f32 m10, f32 m11,
		f32 m20, f32 m21,
		f32 m30, f32 m31)
	{
		const GLfloat v[8] = { m00, m01, m10, m11, m20, m21 };
		glUniformMatrix4x2fv(glGetUniformLocation(g.shader, g.uniformName), 1, GL_FALSE, v);
	}

	void App::GlSetMat4x3F(
		f32 m00, f32 m01, f32 m02,
		f32 m10, f32 m11, f32 m12,
		f32 m20, f32 m21, f32 m22,
		f32 m30, f32 m31, f32 m32)
	{
		const GLfloat v[12] = { m00, m01, m02, m10, m11, m12, m20, m21, m22, m30, m31, m32 };
		glUniformMatrix4x3fv(glGetUniformLocation(g.shader, g.uniformName), 1, GL_FALSE, v);
	}

	void App::GlSetMat4x4F(
		f32 m00, f32 m01, f32 m02, f32 m03,
		f32 m10, f32 m11, f32 m12, f32 m13,
		f32 m20, f32 m21, f32 m22, f32 m23,
		f32 m30, f32 m31, f32 m32, f32 m33)
	{
		const GLfloat v[16] = { m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33 };
		glUniformMatrix4fv(glGetUniformLocation(g.shader, g.uniformName), 1, GL_FALSE, v);
	}

	void App::GlAddVertex(
		f32 x, f32 y, f32 z, f32 w,
		u32 c0, u32 c1, u32 i0, u32 i1,
		f32 v0, f32 v1, f32 v2, f32 v3,
		f32 v4, f32 v5, f32 v6, f32 v7)
	{
		g.vertices.emplace_back(sGlVertex{ { x, y, z, w }, { c0, c1 }, { i0, i1 }, { v0, v1, v2, v3, v4, v5, v6, v7 } });
	}
}