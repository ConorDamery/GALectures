#include <App.hpp>

#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include <string>
#include <sstream>
#include <vector>

struct Image
{
	i32 w{ 0 }, h{ 0 }, c{ 0 };
	u8* data{ nullptr };
};

struct Model
{
	cgltf_data* data{ nullptr };
};

struct Vertex
{
	f32 pos[4] = { 0, 0, 0, 0 }; // Used for main position data
	u32 col[2] = { 0, 0 }; // Used for nomalized uint values (typical for color)
	u32 idx[2] = { 0, 0 }; // Used for unnormalized uint values (typical for indexing)
	f32 v[8] = { 0, 0, 0, 0, 0, 0, 0, 0 }; // Extra floats for anything else
};

struct GlData
{
	GLuint shader{ 0 };
	GLuint vao{ 0 };
	GLuint vbo{ 0 };
	std::vector<GLuint> shaders{};
	std::vector<Image> images{};
	std::vector<Model> models{};
	std::vector<GLuint> textures{};
	const char* uniformName{ nullptr };
	std::vector<Vertex> vertices{};
};

static GlData g_data{};

#if _DEBUG
static const char* opengl_source(GLenum source)
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

static const char* opengl_type(GLenum type)
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

static const char* opengl_severity(GLenum severity)
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

static void APIENTRY opengl_debug_callback(GLenum source, GLenum type, u32 id, GLenum severity, GLsizei length, const char* message, const void* userParam)
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

bool App::GlInitialize(const AppConfig& config)
{
	if (!gladLoadGLLoader((GLADloadproc)WinGetProcAddress))
	{
		LOGE("Failed to initialize GLAD!");
		return false;
	}

#if _DEBUG
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

	glGenVertexArrays(1, &g_data.vao);
	glGenBuffers(1, &g_data.vbo);

	glBindVertexArray(g_data.vao);
	glBindBuffer(GL_ARRAY_BUFFER, g_data.vbo);

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

	glDeleteProgram(g_data.shader);
	glDeleteBuffers(1, &g_data.vbo);
	glDeleteVertexArrays(1, &g_data.vao);
}

void App::GlReload()
{
	for (const auto shader : g_data.shaders)
		GlDestroyShader(shader);
	g_data.shaders.clear();

	for (const auto& image : g_data.images)
		if (image.data) stbi_image_free(image.data);
	g_data.images.clear();

	for (const auto texture : g_data.textures)
		GlDestroyTexture(texture);
	g_data.textures.clear();
}

static GLuint opengl_load_shader(const char* vsrc, const char* fsrc)
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

	g_data.shaders.emplace_back(program);
	return program;
}

static std::string shader_process_includes(const std::string& source)
{
	std::stringstream processed;
	std::istringstream input(source);
	std::string line;
	static std::string includePath;

	while (std::getline(input, line))
	{
		// Check for #include "filename"
		if (line.rfind("#include", 0) == 0)
		{
			size_t start = line.find('"');
			size_t end = line.rfind('"');
			if (start != std::string::npos && end != std::string::npos && start < end)
			{
				size_t len = end - start - 1;

				// Ensure buffer is large enough
				includePath.resize(len);
				std::memcpy(&includePath[0], line.c_str() + start + 1, len);
				includePath[len] = '\0'; // Null-terminate manually

				std::string includedSource = App::FileLoad(includePath.c_str());
				processed << shader_process_includes(includedSource) << "\n";
				continue; // Skip writing the #include line itself
			}
		}
		processed << line << "\n";
	}
	return processed.str();
}

u32 App::GlLoadShader(const char* filepath)
{
	auto src = shader_process_includes(FileLoad(filepath));
	auto vsrc = "#version 330 core\n#define VERT\n" + src;
	auto fsrc = "#version 330 core\n#define FRAG\n" + src;
	return opengl_load_shader(vsrc.c_str(), fsrc.c_str());
}

u32 App::GlCreateShader(const char* source)
{
	auto src = shader_process_includes(source);
	auto vsrc = "#version 330 core\n#define VERT\n" + src;
	auto fsrc = "#version 330 core\n#define FRAG\n" + src;
	return opengl_load_shader(vsrc.c_str(), fsrc.c_str());
}

void App::GlDestroyShader(u32 shader)
{
	glDeleteProgram(shader);
}

void App::GlSetShader(u32 shader)
{
	g_data.shader = shader;
	glUseProgram(g_data.shader);
}

static Image* gl_get_image(u32 image)
{
	if (image == 0 || image > g_data.images.size())
	{
		LOGW("Invalid image handle!");
		return nullptr;
	}

	return &g_data.images[static_cast<size_t>(image) - 1];
}

u32 App::GlLoadImage(const char* filepath, bool flipY)
{
	const char* path = FilePath(filepath);

	Image img{};
	stbi_set_flip_vertically_on_load(flipY);
	img.data = stbi_load(path, &img.w, &img.h, &img.c, 0);
	if (!img.data)
	{
		LOGW("Failed to load image: %s", path);
		return -1;
	}

	g_data.images.emplace_back(img);
	return (u32)(g_data.images.size());
}

u32 App::GlCreateImage(i32 w, i32 h, i32 c, u8* data)
{
	Image img{ w, h, c, data };

	g_data.images.emplace_back(img);
	return (u32)(g_data.images.size());
}

void App::GlDestroyImage(u32 image)
{
	auto img = gl_get_image(image);
	if (img == nullptr) return;
	stbi_image_free(img->data);
	*img = Image{};
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

static cgltf_data* gltf_load(const char* filepath)
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

static void gltf_print_hierarchy(const cgltf_node* node, int depth = 0)
{
	static std::string g_tabs;

	g_tabs.clear();
	for (int i = 0; i < depth; ++i) g_tabs += "\t";
	LOGD("%sNode: %s", g_tabs.c_str(), node->name ? node->name : "Unnamed");

	for (cgltf_size i = 0; i < node->children_count; ++i)
	{
		gltf_print_hierarchy(node->children[i], depth + 1);
	}
}

static void gltf_print_scenegraph(const cgltf_data* data)
{
	if (!data || !data->scene) return;
	LOGD("Scene: %s", data->scene->name ? data->scene->name : "Default");
	for (cgltf_size i = 0; i < data->scene->nodes_count; ++i)
	{
		gltf_print_hierarchy(data->scene->nodes[i]);
	}
}

static void gltf_print_animations(const cgltf_data* data)
{
	LOGD("Animations: %s", data->animations_count);
	for (cgltf_size i = 0; i < data->animations_count; ++i)
	{
		const cgltf_animation& anim = data->animations[i];
		LOGD("  Animation %d: %s", i, anim.name ? anim.name : "Unnamed");

		for (cgltf_size j = 0; j < anim.channels_count; ++j)
		{
			const cgltf_animation_channel& channel = anim.channels[j];
			const char* path = "Unknown";
			switch (channel.target_path) {
			case cgltf_animation_path_type_translation: path = "translation"; break;
			case cgltf_animation_path_type_rotation: path = "rotation"; break;
			case cgltf_animation_path_type_scale: path = "scale"; break;
			case cgltf_animation_path_type_weights: path = "weights"; break;
			default: break;
			}
			LOGD("    Channel to node: %s, path: %s", channel.target_node && channel.target_node->name ? channel.target_node->name : "Unnamed", path);
		}
	}
}

static void gltf_print_meshdata(const cgltf_mesh& mesh)
{
	LOGD("  Mesh: %s, primitives: %d", mesh.name ? mesh.name : "Unnamed", mesh.primitives_count);
	for (cgltf_size i = 0; i < mesh.primitives_count; ++i)
	{
		const cgltf_primitive& prim = mesh.primitives[i];
		LOGD("    Primitive %d: attributes: %d, indices: %d", i, prim.attributes_count, prim.indices ? prim.indices->count : 0);
	}
}

static void gltf_print_info(const cgltf_data* data)
{
	if (!data) return;

	LOGD("Number of meshes: %d", data->meshes_count);
	for (cgltf_size i = 0; i < data->meshes_count; ++i)
	{
		gltf_print_meshdata(data->meshes[i]);
	}

	gltf_print_scenegraph(data);
	gltf_print_animations(data);
}

u32 App::GlLoadModel(const char* filepath)
{
#if _DEBUG
	std::string pathStr = PROJECT_PATH;
	pathStr += filepath;
	const char* path = pathStr.c_str();
#else
	const char* path = filepath;
#endif

	auto data = gltf_load(path);
	if (data == nullptr)
	{
		LOGW("Failed to load model!");
		return -1;
	}
	Model model{ data };

	gltf_print_info(model.data);

	g_data.models.emplace_back(model);
	return (u32)(g_data.models.size() - 1);
}

void App::GlDestroyModel(u32 model)
{
	auto& mdl = g_data.models[model];
	gltf_free(mdl.data);
	mdl = Model{};
}

static GLenum opengl_internal_format(TextureFormat fmt)
{
	switch (fmt)
	{
	case TextureFormat::R8:    return GL_R8;
	case TextureFormat::RG8:   return GL_RG8;
	case TextureFormat::RGB8:  return GL_RGB8;
	case TextureFormat::RGBA8: return GL_RGBA8;
	default:                   return GL_RGBA8;
	}
}

GLenum opengl_format(TextureFormat fmt)
{
	switch (fmt)
	{
	case TextureFormat::R8:    return GL_RED;
	case TextureFormat::RG8:   return GL_RG;
	case TextureFormat::RGB8:  return GL_RGB;
	case TextureFormat::RGBA8: return GL_RGBA;
	default:                   return GL_RGBA;
	}
}

GLenum opengl_type(TextureFormat fmt)
{
	return GL_UNSIGNED_BYTE;
}

GLenum opengl_filter(TextureFilter filter, bool useMipmaps)
{
	switch (filter)
	{
	case TextureFilter::NEAREST:	return useMipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
	case TextureFilter::LINEAR:		return useMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
	default:						return GL_LINEAR;
	}
}

GLenum opengl_wrap(TextureWrap wrap)
{
	return wrap == TextureWrap::REPEAT ? GL_REPEAT : GL_CLAMP_TO_EDGE;
}

u32 App::GlCreateTexture(
	u32 image, TextureFormat format,
	TextureFilter minFilter, TextureFilter magFilter,
	TextureWrap wrapS, TextureWrap wrapT,
	bool genMipmaps)
{
	auto* img = gl_get_image(image);
	if (img != nullptr && (!img->data || img->w <= 0 || img->h <= 0))
	{
		// Handle error: invalid image
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

	g_data.vertices.clear();
}

void App::GlEnd(u32 mode)
{
	glBindVertexArray(g_data.vao);
	glBindBuffer(GL_ARRAY_BUFFER, g_data.vbo);

	glBufferData(GL_ARRAY_BUFFER, g_data.vertices.size() * sizeof(Vertex), g_data.vertices.data(), GL_DYNAMIC_DRAW);

	for (u32 bit = 1; bit <= (u32)GlTopology::TRIANGLE_FAN; bit <<= 1)
	{
		if (mode & bit)
		{
			u32 i = 0, b = bit;
			while (b > 1 && ++i) b >>= 1;
			glDrawArrays(GL_POINTS + i, 0, (GLsizei)g_data.vertices.size());
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void App::GlViewport(u32 x, u32 y, u32 w, u32 h)
{
	glViewport(x, y, w, h);
}

void App::GlClear(f32 r, f32 g, f32 b, f32 a, f32 d, f32 s, u32 flags)
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void App::GlUniform(const char* name)
{
	g_data.uniformName = name;
}

void App::GlTex2D(u32 i, u32 texture)
{
	glActiveTexture(GL_TEXTURE0 + i);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(glGetUniformLocation(g_data.shader, g_data.uniformName), i);
}

void App::GlFloat(f32 x)
{
	glUniform1f(glGetUniformLocation(g_data.shader, g_data.uniformName), x);
}

void App::GlVec2F(f32 x, f32 y)
{
	glUniform2f(glGetUniformLocation(g_data.shader, g_data.uniformName), x, y);
}

void App::GlVec3F(f32 x, f32 y, f32 z)
{
	glUniform3f(glGetUniformLocation(g_data.shader, g_data.uniformName), x, y, z);
}

void App::GlVec4F(f32 x, f32 y, f32 z, f32 w)
{
	glUniform4f(glGetUniformLocation(g_data.shader, g_data.uniformName), x, y, z, w);
}

void App::GlMat2x2F(
	f32 m00, f32 m01,
	f32 m10, f32 m11)
{
	const GLfloat v[4] = { m00, m01, m10, m11 };
	glUniformMatrix2fv(glGetUniformLocation(g_data.shader, g_data.uniformName), 1, GL_FALSE, v);
}

void App::GlMat2x3F(
	f32 m00, f32 m01, f32 m02,
	f32 m10, f32 m11, f32 m12)
{
	const GLfloat v[6] = { m00, m01, m02, m10, m11, m12 };
	glUniformMatrix2x3fv(glGetUniformLocation(g_data.shader, g_data.uniformName), 1, GL_FALSE, v);
}

void App::GlMat2x4F(
	f32 m00, f32 m01, f32 m02, f32 m03,
	f32 m10, f32 m11, f32 m12, f32 m13)
{
	const GLfloat v[8] = { m00, m01, m02, m03, m10, m11, m12, m13 };
	glUniformMatrix2x4fv(glGetUniformLocation(g_data.shader, g_data.uniformName), 1, GL_FALSE, v);
}

void App::GlMat3x2F(
	f32 m00, f32 m01,
	f32 m10, f32 m11,
	f32 m20, f32 m21)
{
	const GLfloat v[6] = { m00, m01, m10, m11, m20, m21 };
	glUniformMatrix3x2fv(glGetUniformLocation(g_data.shader, g_data.uniformName), 1, GL_FALSE, v);
}

void App::GlMat3x3F(
	f32 m00, f32 m01, f32 m02,
	f32 m10, f32 m11, f32 m12,
	f32 m20, f32 m21, f32 m22)
{
	const GLfloat v[9] = { m00, m01, m02, m10, m11, m12, m20, m21, m22 };
	glUniformMatrix3fv(glGetUniformLocation(g_data.shader, g_data.uniformName), 1, GL_FALSE, v);
}

void App::GlMat3x4F(
	f32 m00, f32 m01, f32 m02, f32 m03,
	f32 m10, f32 m11, f32 m12, f32 m13,
	f32 m20, f32 m21, f32 m22, f32 m23)
{
	const GLfloat v[12] = { m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23 };
	glUniformMatrix3x4fv(glGetUniformLocation(g_data.shader, g_data.uniformName), 1, GL_FALSE, v);
}

void App::GlMat4x2F(
	f32 m00, f32 m01,
	f32 m10, f32 m11,
	f32 m20, f32 m21,
	f32 m30, f32 m31)
{
	const GLfloat v[8] = { m00, m01, m10, m11, m20, m21 };
	glUniformMatrix4x2fv(glGetUniformLocation(g_data.shader, g_data.uniformName), 1, GL_FALSE, v);
}

void App::GlMat4x3F(
	f32 m00, f32 m01, f32 m02,
	f32 m10, f32 m11, f32 m12,
	f32 m20, f32 m21, f32 m22,
	f32 m30, f32 m31, f32 m32)
{
	const GLfloat v[12] = { m00, m01, m02, m10, m11, m12, m20, m21, m22, m30, m31, m32 };
	glUniformMatrix4x3fv(glGetUniformLocation(g_data.shader, g_data.uniformName), 1, GL_FALSE, v);
}

void App::GlMat4x4F(
	f32 m00, f32 m01, f32 m02, f32 m03,
	f32 m10, f32 m11, f32 m12, f32 m13,
	f32 m20, f32 m21, f32 m22, f32 m23,
	f32 m30, f32 m31, f32 m32, f32 m33)
{
	const GLfloat v[16] = { m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33 };
	glUniformMatrix4fv(glGetUniformLocation(g_data.shader, g_data.uniformName), 1, GL_FALSE, v);
}

void App::GlVertex(
	f32 x, f32 y, f32 z, f32 w,
	u32 c0, u32 c1, u32 i0, u32 i1,
	f32 v0, f32 v1, f32 v2, f32 v3,
	f32 v4, f32 v5, f32 v6, f32 v7)
{
	g_data.vertices.emplace_back(Vertex{ { x, y, z, w }, { c0, c1 }, { i0, i1 }, { v0, v1, v2, v3, v4, v5, v6, v7 } });
}