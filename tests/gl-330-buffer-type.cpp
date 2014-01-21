//**********************************
// OpenGL Buffer Type
// 10/05/2010
//**********************************
// Christophe Riccio
// ogl-samples@g-truc.net
//**********************************
// G-Truc Creation
// www.g-truc.net
//**********************************

#include <glf/glf.hpp>

namespace
{
	glf::window Window("gl-330-buffer-type");
	char const * VERT_SHADER_SOURCE("gl-330/flat-color.vert");
	char const * FRAG_SHADER_SOURCE("gl-330/flat-color.frag");

	GLsizei const VertexCount(6);

	GLsizeiptr const PositionSizeF32 = VertexCount * sizeof(glm::vec2);
	glm::vec2 const PositionDataF32[VertexCount] =
	{
		glm::vec2(-1.0f,-1.0f),
		glm::vec2( 1.0f,-1.0f),
		glm::vec2( 1.0f, 1.0f),
		glm::vec2( 1.0f, 1.0f),
		glm::vec2(-1.0f, 1.0f),
		glm::vec2(-1.0f,-1.0f)
	};

	GLsizeiptr const PositionSizeI8 = VertexCount * sizeof(glm::i8vec2);
	glm::i8vec2 const PositionDataI8[VertexCount] =
	{
		glm::i8vec2(-1,-1),
		glm::i8vec2( 1,-1),
		glm::i8vec2( 1, 1),
		glm::i8vec2( 1, 1),
		glm::i8vec2(-1, 1),
		glm::i8vec2(-1,-1)
	};

	GLsizeiptr const PositionSizeRGB10A2 = VertexCount * sizeof(glm::uint32);

	glm::uint32 const PositionDataRGB10A2[VertexCount] =
	{
		glm::packSnorm3x10_1x2(glm::vec4(-1.0f,-1.0f, 0.0f, 1.0f)),
		glm::packSnorm3x10_1x2(glm::vec4( 1.0f,-1.0f, 0.0f, 1.0f)),
		glm::packSnorm3x10_1x2(glm::vec4( 1.0f, 1.0f, 0.0f, 1.0f)),
		glm::packSnorm3x10_1x2(glm::vec4( 1.0f, 1.0f, 0.0f, 1.0f)),
		glm::packSnorm3x10_1x2(glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f)),
		glm::packSnorm3x10_1x2(glm::vec4(-1.0f,-1.0f, 0.0f, 1.0f)),
	};

	GLsizeiptr const PositionSizeI32 = VertexCount * sizeof(glm::i32vec2);
	glm::i32vec2 const PositionDataI32[VertexCount] =
	{
		glm::i32vec2(-1,-1),
		glm::i32vec2( 1,-1),
		glm::i32vec2( 1, 1),
		glm::i32vec2( 1, 1),
		glm::i32vec2(-1, 1),
		glm::i32vec2(-1,-1)
	};

	namespace buffer
	{
		enum type
		{
			RGB10A2,
			//F16,
			F32,
			I8,
			I32,
			MAX
		};
	}

	namespace shader
	{
		enum type
		{
			VERT,
			FRAG,
			MAX
		};
	}//namespace shader

	std::vector<GLuint> ShaderName(shader::MAX);
	GLuint ProgramName(0);
	GLint UniformMVP(0);
	GLint UniformDiffuse(0);
	std::vector<GLuint> BufferName(buffer::MAX);
	std::vector<GLuint> VertexArrayName(buffer::MAX);
	std::vector<glm::ivec4> Viewport(buffer::MAX);

}//namespace

bool initProgram()
{
	bool Validated = true;
	
	if(Validated)
	{
		glf::compiler Compiler;
		ShaderName[shader::VERT] = Compiler.create(GL_VERTEX_SHADER, glf::DATA_DIRECTORY + VERT_SHADER_SOURCE, "--version 330 --profile core");
		ShaderName[shader::FRAG] = Compiler.create(GL_FRAGMENT_SHADER, glf::DATA_DIRECTORY + FRAG_SHADER_SOURCE, "--version 330 --profile core");
		Validated = Validated && Compiler.check();

		ProgramName = glCreateProgram();
		glAttachShader(ProgramName, ShaderName[shader::VERT]);
		glAttachShader(ProgramName, ShaderName[shader::FRAG]);

		glLinkProgram(ProgramName);
		Validated = Validated && glf::checkProgram(ProgramName);
	}

	// Get variables locations
	if(Validated)
	{
		UniformMVP = glGetUniformLocation(ProgramName, "MVP");
		UniformDiffuse = glGetUniformLocation(ProgramName, "Diffuse");
	}

	return Validated && glf::checkError("initProgram");
}

bool initBuffer()
{
	// Generate a buffer object
	glGenBuffers(buffer::MAX, &BufferName[0]);

	glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::F32]);
	glBufferData(GL_ARRAY_BUFFER, PositionSizeF32, PositionDataF32, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::I8]);
	glBufferData(GL_ARRAY_BUFFER, PositionSizeI8, PositionDataI8, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::I32]);
	glBufferData(GL_ARRAY_BUFFER, PositionSizeI32, PositionDataI32, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::RGB10A2]);
	glBufferData(GL_ARRAY_BUFFER, PositionSizeRGB10A2, PositionDataRGB10A2, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return glf::checkError("initBuffer");
}

bool initVertexArray()
{
	glGenVertexArrays(buffer::MAX, &VertexArrayName[0]);

	glBindVertexArray(VertexArrayName[buffer::F32]);
		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::F32]);
		glVertexAttribPointer(glf::semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), GLF_BUFFER_OFFSET(0));
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnableVertexAttribArray(glf::semantic::attr::POSITION);
	glBindVertexArray(0);

	glBindVertexArray(VertexArrayName[buffer::I8]);
		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::I8]);
		glVertexAttribPointer(glf::semantic::attr::POSITION, 2, GL_BYTE, GL_FALSE, sizeof(glm::u8vec2), GLF_BUFFER_OFFSET(0));
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnableVertexAttribArray(glf::semantic::attr::POSITION);
	glBindVertexArray(0);

	glBindVertexArray(VertexArrayName[buffer::I32]);
		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::I32]);
		glVertexAttribPointer(glf::semantic::attr::POSITION, 2, GL_INT, GL_FALSE, sizeof(glm::i32vec2), GLF_BUFFER_OFFSET(0));
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnableVertexAttribArray(glf::semantic::attr::POSITION);
	glBindVertexArray(0);

	glBindVertexArray(VertexArrayName[buffer::RGB10A2]);
		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::RGB10A2]);
		glVertexAttribPointer(glf::semantic::attr::POSITION, 4, GL_INT_2_10_10_10_REV, GL_TRUE, sizeof(glm::uint32), GLF_BUFFER_OFFSET(0));
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnableVertexAttribArray(glf::semantic::attr::POSITION);
	glBindVertexArray(0);

	return glf::checkError("initVertexArray");
}

bool begin()
{
	Viewport[buffer::RGB10A2] = glm::ivec4(0, 0, Window.Size >> 1);
	Viewport[buffer::F32] = glm::ivec4(Window.Size.x >> 1, 0, Window.Size >> 1);
	Viewport[buffer::I8]  = glm::ivec4(Window.Size.x >> 1, Window.Size.y >> 1, Window.Size >> 1);
	Viewport[buffer::I32] = glm::ivec4(0, Window.Size.y >> 1, Window.Size >> 1);

	bool Validated = true;

	if(Validated)
		Validated = initProgram();
	if(Validated)
		Validated = initBuffer();
	if(Validated)
		Validated = initVertexArray();

	return Validated && glf::checkError("begin");
}

bool end()
{
	for(std::size_t i = 0; 0 < shader::MAX; ++i)
		glDeleteShader(ShaderName[i]);
	glDeleteBuffers(buffer::MAX, &BufferName[0]);
	glDeleteVertexArrays(buffer::MAX, &VertexArrayName[0]);
	glDeleteProgram(ProgramName);

	return glf::checkError("end");
}

void display()
{
	// Compute the MVP (Model View Projection matrix)
	glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, 4.0f / 3.0f, 0.1f, 100.0f);
	glm::mat4 ViewTranslateZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Window.TranlationCurrent.y));
	glm::mat4 ViewRotateX = glm::rotate(ViewTranslateZ, Window.RotationCurrent.y, glm::vec3(1.f, 0.f, 0.f));
	glm::mat4 ViewRotateY = glm::rotate(ViewRotateX, Window.RotationCurrent.x, glm::vec3(0.f, 1.f, 0.f));
	glm::mat4 View = ViewRotateY;
	glm::mat4 Model = glm::mat4(1.0f);
	glm::mat4 MVP = Projection * View * Model;

	glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f)[0]);

	glUseProgram(ProgramName);
	glUniform4fv(UniformDiffuse, 1, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);
	glUniformMatrix4fv(UniformMVP, 1, GL_FALSE, &MVP[0][0]);

	for(std::size_t Index = 0; Index < buffer::MAX; ++Index)
	{
		glViewport(Viewport[Index].x, Viewport[Index].y, Viewport[Index].z, Viewport[Index].w);

		glBindVertexArray(VertexArrayName[Index]);
		glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 1);
	}

	glf::checkError("display");

}

int main(int argc, char* argv[])
{
	return glf::run(argc, argv, glf::CORE, 3, 3);
}