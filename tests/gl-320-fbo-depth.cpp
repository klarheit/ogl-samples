//**********************************
// OpenGL Framebuffer Depth Render
// 28/01/2013 - 28/01/2013
//**********************************
// Christophe Riccio
// ogl-samples@g-truc.net
//**********************************
// G-Truc Creation
// www.g-truc.net
//**********************************

#include <glf/glf.hpp>
#include <glf/compiler.hpp>

namespace
{
	glf::window Window("gl-320-fbo-depth");

	char const * VERT_SHADER_SOURCE_TEXTURE("gl-320/texture-2d.vert");
	char const * FRAG_SHADER_SOURCE_TEXTURE("gl-320/texture-2d.frag");
	char const * VERT_SHADER_SOURCE_SPLASH("gl-320/fbo-depth.vert");
	char const * FRAG_SHADER_SOURCE_SPLASH("gl-320/fbo-depth.frag");
	char const * TEXTURE_DIFFUSE("kueken1-dxt1.dds");

	GLsizei const VertexCount(4);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glf::vertex_v2fv2f);
	glf::vertex_v2fv2f const VertexData[VertexCount] =
	{
		glf::vertex_v2fv2f(glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f,-1.0f), glm::vec2(1.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 0.0f))
	};

	GLsizei const ElementCount(6);
	GLsizeiptr const ElementSize = ElementCount * sizeof(GLushort);
	GLushort const ElementData[ElementCount] =
	{
		0, 1, 2, 
		2, 3, 0
	};

	namespace buffer
	{
		enum type
		{
			VERTEX,
			ELEMENT,
			TRANSFORM,
			MAX
		};
	}//namespace buffer

	namespace texture
	{
		enum type
		{
			DIFFUSE,
			RENDERBUFFER,
			MAX
		};
	}//namespace texture
	
	namespace program
	{
		enum type
		{
			TEXTURE,
			SPLASH,
			MAX
		};
	}//namespace program

	namespace shader
	{
		enum type
		{
			VERT_TEXTURE,
			FRAG_TEXTURE,
			VERT_SPLASH,
			FRAG_SPLASH,
			MAX
		};
	}//namespace shader

	GLuint FramebufferName(0);
	std::vector<GLuint> ShaderName(shader::MAX);
	std::vector<GLuint> ProgramName(program::MAX);
	std::vector<GLuint> VertexArrayName(program::MAX);
	std::vector<GLuint> BufferName(buffer::MAX);
	std::vector<GLuint> TextureName(texture::MAX);
	GLint UniformTransform(0);
}//namespace

bool initProgram()
{
	bool Validated(true);
	
	if(Validated)
	{
		glf::compiler Compiler;
		ShaderName[shader::VERT_TEXTURE] = Compiler.create(GL_VERTEX_SHADER, glf::DATA_DIRECTORY + VERT_SHADER_SOURCE_TEXTURE, "--version 150 --profile core");
		ShaderName[shader::FRAG_TEXTURE] = Compiler.create(GL_FRAGMENT_SHADER, glf::DATA_DIRECTORY + FRAG_SHADER_SOURCE_TEXTURE, "--version 150 --profile core");
		Validated = Validated && Compiler.check();

		ProgramName[program::TEXTURE] = glCreateProgram();
		glAttachShader(ProgramName[program::TEXTURE], ShaderName[shader::VERT_TEXTURE]);
		glAttachShader(ProgramName[program::TEXTURE], ShaderName[shader::FRAG_TEXTURE]);

		glBindAttribLocation(ProgramName[program::TEXTURE], glf::semantic::attr::POSITION, "Position");
		glBindAttribLocation(ProgramName[program::TEXTURE], glf::semantic::attr::TEXCOORD, "Texcoord");
		glBindFragDataLocation(ProgramName[program::TEXTURE], glf::semantic::frag::COLOR, "Color");
		glLinkProgram(ProgramName[program::TEXTURE]);

		Validated = Validated && glf::checkProgram(ProgramName[program::TEXTURE]);
	}

	if(Validated)
		UniformTransform = glGetUniformBlockIndex(ProgramName[program::TEXTURE], "transform");

	if(Validated)
	{
		glf::compiler Compiler;
		ShaderName[shader::VERT_SPLASH] = Compiler.create(GL_VERTEX_SHADER, glf::DATA_DIRECTORY + VERT_SHADER_SOURCE_SPLASH, "--version 150 --profile core");
		ShaderName[shader::FRAG_SPLASH] = Compiler.create(GL_FRAGMENT_SHADER, glf::DATA_DIRECTORY + FRAG_SHADER_SOURCE_SPLASH, "--version 150 --profile core");
		Validated = Validated && Compiler.check();

		ProgramName[program::SPLASH] = glCreateProgram();
		glAttachShader(ProgramName[program::SPLASH], ShaderName[shader::VERT_SPLASH]);
		glAttachShader(ProgramName[program::SPLASH], ShaderName[shader::FRAG_SPLASH]);

		glBindFragDataLocation(ProgramName[program::TEXTURE], glf::semantic::frag::COLOR, "Color");
		glLinkProgram(ProgramName[program::SPLASH]);

		Validated = Validated && glf::checkProgram(ProgramName[program::SPLASH]);
	}

	return Validated;
}

bool initBuffer()
{
	glGenBuffers(buffer::MAX, &BufferName[0]);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
	glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLint UniformBufferOffset(0);

	glGetIntegerv(
		GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT,
		&UniformBufferOffset);

	GLint UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

	glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
	glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	return true;
}

bool initTexture()
{
	bool Validated(true);

	gli::texture2D Texture(gli::load_dds((glf::DATA_DIRECTORY + TEXTURE_DIFFUSE).c_str()));
	assert(!Texture.empty());

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(texture::MAX, &TextureName[0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureName[texture::DIFFUSE]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	for(std::size_t Level = 0; Level < Texture.levels(); ++Level)
	{
		glCompressedTexImage2D(
			GL_TEXTURE_2D,
			GLint(Level),
			GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
			GLsizei(Texture[Level].dimensions().x), 
			GLsizei(Texture[Level].dimensions().y), 
			0, 
			GLsizei(Texture[Level].size()), 
			Texture[Level].data());
	}
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureName[texture::RENDERBUFFER]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, GLint(0), GL_DEPTH_COMPONENT24, GLsizei(Window.Size.x), GLsizei(Window.Size.y), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	return Validated;
}

bool initVertexArray()
{
	glGenVertexArrays(program::MAX, &VertexArrayName[0]);
	glBindVertexArray(VertexArrayName[program::TEXTURE]);
		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
		glVertexAttribPointer(glf::semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), GLF_BUFFER_OFFSET(0));
		glVertexAttribPointer(glf::semantic::attr::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), GLF_BUFFER_OFFSET(sizeof(glm::vec2)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnableVertexAttribArray(glf::semantic::attr::POSITION);
		glEnableVertexAttribArray(glf::semantic::attr::TEXCOORD);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
	glBindVertexArray(0);

	glBindVertexArray(VertexArrayName[program::SPLASH]);
	glBindVertexArray(0);

	return true;
}

bool initFramebuffer()
{
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, TextureName[texture::RENDERBUFFER], 0);

	if(glf::checkFramebuffer(FramebufferName))
		return false;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);	
	return true;
}

bool begin()
{
	bool Validated(true);

	if(Validated)
		Validated = initProgram();
	if(Validated)
		Validated = initBuffer();
	if(Validated)
		Validated = initVertexArray();
	if(Validated)
		Validated = initTexture();
	if(Validated)
		Validated = initFramebuffer();

	return Validated;
}

bool end()
{
	for(std::size_t i = 0; 0 < shader::MAX; ++i)
		glDeleteShader(ShaderName[i]);
	glDeleteFramebuffers(1, &FramebufferName);
	glDeleteProgram(ProgramName[program::SPLASH]);
	glDeleteProgram(ProgramName[program::TEXTURE]);
	glDeleteBuffers(buffer::MAX, &BufferName[0]);
	glDeleteTextures(texture::MAX, &TextureName[0]);
	glDeleteVertexArrays(program::MAX, &VertexArrayName[0]);

	return glf::checkError("end");
}

void display()
{
	// Update of the uniform buffer
	{
		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		glm::mat4* Pointer = (glm::mat4*)glMapBufferRange(
			GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4),
			GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

		glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, 4.0f / 3.0f, 2.0f, 8.0f);
		glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Window.TranlationCurrent.y));
		glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, Window.RotationCurrent.y, glm::vec3(1.f, 0.f, 0.f));
		glm::mat4 View = glm::rotate(ViewRotateX, Window.RotationCurrent.x, glm::vec3(0.f, 1.f, 0.f));
		glm::mat4 Model = glm::mat4(1.0f);
		*Pointer = Projection * View * Model;

		// Make sure the uniform buffer is uploaded
		glUnmapBuffer(GL_UNIFORM_BUFFER);
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glViewport(0, 0, Window.Size.x, Window.Size.y);

	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
	float Depth(1.0f);
	glClearBufferfv(GL_DEPTH , 0, &Depth);

	// Bind rendering objects
	glUseProgram(ProgramName[program::TEXTURE]);
	glUniformBlockBinding(ProgramName[program::TEXTURE], UniformTransform, glf::semantic::uniform::TRANSFORM0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureName[texture::DIFFUSE]);
	glBindVertexArray(VertexArrayName[program::TEXTURE]);
	glBindBufferBase(GL_UNIFORM_BUFFER, glf::semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);

	glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, 0, 1, 0);

	glDisable(GL_DEPTH_TEST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(ProgramName[program::SPLASH]);

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VertexArrayName[program::SPLASH]);
	glBindTexture(GL_TEXTURE_2D, TextureName[texture::RENDERBUFFER]);

	glDrawArraysInstanced(GL_TRIANGLES, 0, 3, 1);
}

int main(int argc, char* argv[])
{
	return glf::run(argc, argv, glf::CORE, 3, 2);
}