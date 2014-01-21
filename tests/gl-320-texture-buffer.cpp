//**********************************
// OpenGL Buffer Texture
// 04/06/2010 - 20/03/2012
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
	glf::window Window("gl-320-texture-buffer");

	char const * VERT_SHADER_SOURCE("gl-320/texture-buffer.vert");
	char const * FRAG_SHADER_SOURCE("gl-320/texture-buffer.frag");

	GLsizei const VertexCount(6);
	GLsizeiptr const PositionSize = VertexCount * sizeof(glm::vec2);
	glm::vec2 const PositionData[VertexCount] = 
	{
		glm::vec2(-1.0f,-1.0f),
		glm::vec2( 1.0f,-1.0f),
		glm::vec2( 1.0f, 1.0f),
		glm::vec2( 1.0f, 1.0f),
		glm::vec2(-1.0f, 1.0f),
		glm::vec2(-1.0f,-1.0f)
	};

	namespace buffer
	{
		enum type
		{
			VERTEX,
			DISPLACEMENT,
			DIFFUSE,
			MAX
		};
	}//namespace buffer
	
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
	GLuint VertexArrayName = 0;
	GLuint ProgramName = 0;
	GLuint BufferName[buffer::MAX] = {0, 0, 0};
	GLuint DisplacementTextureName = 0;
	GLuint DiffuseTextureName = 0;
	GLint UniformMVP = 0;
	GLint UniformDiffuse = 0;
	GLint UniformDisplacement = 0;
}//namespace

bool initTest()
{
	bool Validated = true;
	glEnable(GL_DEPTH_TEST);

	return Validated && glf::checkError("initTest");
}

bool initProgram()
{
	bool Validated = true;
	
	// Create program
	if(Validated)
	{
		glf::compiler Compiler;
		ShaderName[shader::VERT] = Compiler.create(GL_VERTEX_SHADER, glf::DATA_DIRECTORY + VERT_SHADER_SOURCE, "--version 150 --profile core");
		ShaderName[shader::FRAG] = Compiler.create(GL_FRAGMENT_SHADER, glf::DATA_DIRECTORY + FRAG_SHADER_SOURCE, "--version 150 --profile core");
		Validated = Validated && Compiler.check();

		ProgramName = glCreateProgram();
		glAttachShader(ProgramName, ShaderName[shader::VERT]);
		glAttachShader(ProgramName, ShaderName[shader::FRAG]);

		glBindAttribLocation(ProgramName, glf::semantic::attr::POSITION, "Position");
		glBindFragDataLocation(ProgramName, glf::semantic::frag::COLOR, "Color");
		glLinkProgram(ProgramName);
		Validated = Validated && glf::checkProgram(ProgramName);
	}

	// Get variables locations
	if(Validated)
	{
		UniformMVP = glGetUniformLocation(ProgramName, "MVP");
		UniformDiffuse = glGetUniformLocation(ProgramName, "Diffuse");
		UniformDisplacement = glGetUniformLocation(ProgramName, "Displacement");
	}

	return Validated && glf::checkError("initProgram");
}

bool initBuffer()
{
	glGenBuffers(buffer::MAX, BufferName);

	glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
	glBufferData(GL_ARRAY_BUFFER, PositionSize, PositionData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glm::vec4 Position[5] = 
	{
		glm::vec4( 0.1f, 0.3f,-1.0f, 1.0f), 
		glm::vec4(-0.5f, 0.0f,-0.5f, 1.0f),
		glm::vec4(-0.2f,-0.2f, 0.0f, 1.0f),
		glm::vec4( 0.3f, 0.2f, 0.5f, 1.0f),
		glm::vec4( 0.1f,-0.3f, 1.0f, 1.0f)
	};

	glBindBuffer(GL_TEXTURE_BUFFER, BufferName[buffer::DISPLACEMENT]);
	glBufferData(GL_TEXTURE_BUFFER, sizeof(Position), Position, GL_STATIC_DRAW);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);

	glm::u8vec4 Diffuse[5] = 
	{
		glm::u8vec4(255,   0,   0, 255), 
		glm::u8vec4(255, 127,   0, 255),
		glm::u8vec4(255, 255,   0, 255),
		glm::u8vec4(  0, 255,   0, 255),
		glm::u8vec4(  0,   0, 255, 255)
	};	

	GLint MaxTextureBufferSize(0);
	glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &MaxTextureBufferSize);

	glBindBuffer(GL_TEXTURE_BUFFER, BufferName[buffer::DIFFUSE]);
	glBufferData(GL_TEXTURE_BUFFER, 500000, NULL, GL_STATIC_DRAW);
	//glBufferData(GL_TEXTURE_BUFFER, sizeof(Diffuse), Diffuse, GL_STATIC_DRAW);
	glBufferSubData(GL_TEXTURE_BUFFER, 0, sizeof(Diffuse), Diffuse);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);

	return glf::checkError("initBuffer");
}

bool initTexture()
{
	glGenTextures(1, &DisplacementTextureName);
	glBindTexture(GL_TEXTURE_BUFFER, DisplacementTextureName);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, BufferName[buffer::DISPLACEMENT]);
	glBindTexture(GL_TEXTURE_BUFFER, 0);

	glGenTextures(1, &DiffuseTextureName);
	glBindTexture(GL_TEXTURE_BUFFER, DiffuseTextureName);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA8, BufferName[buffer::DIFFUSE]);
	glBindTexture(GL_TEXTURE_BUFFER, 0);	

	return glf::checkError("initTexture");
}

bool initVertexArray()
{
	glGenVertexArrays(1, &VertexArrayName);
	glBindVertexArray(VertexArrayName);
		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
			glVertexAttribPointer(glf::semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnableVertexAttribArray(glf::semantic::attr::POSITION);
	glBindVertexArray(0);

	return glf::checkError("initVertexArray");
}

bool begin()
{
	bool Validated = true;

	if(Validated)
		Validated = initTest();
	if(Validated)
		Validated = initProgram();
	if(Validated)
		Validated = initBuffer();
	if(Validated)
		Validated = initTexture();
	if(Validated)
		Validated = initVertexArray();

	return Validated && glf::checkError("begin");
}

bool end()
{
	for(std::size_t i = 0; 0 < shader::MAX; ++i)
		glDeleteShader(ShaderName[i]);
	glDeleteTextures(1, &DiffuseTextureName);
	glDeleteTextures(1, &DisplacementTextureName);
	glDeleteBuffers(buffer::MAX, BufferName);
	glDeleteProgram(ProgramName);
	glDeleteVertexArrays(1, &VertexArrayName);

	return glf::checkError("end");
}

void display()
{
	// Compute the MVP (Model View Projection matrix)
	glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, 4.0f / 3.0f, 0.1f, 100.0f);
	glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Window.TranlationCurrent.y));
	glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, Window.RotationCurrent.y, glm::vec3(1.f, 0.f, 0.f));
	glm::mat4 View = glm::rotate(ViewRotateX, Window.RotationCurrent.x, glm::vec3(0.f, 1.f, 0.f));
	glm::mat4 Model = glm::mat4(1.0f);
	glm::mat4 MVP = Projection * View * Model;

	// Set the display viewport
	glViewport(0, 0, Window.Size.x, Window.Size.y);

	// Clear color buffer with black
	float Depth(1.0f);
	glClearBufferfv(GL_DEPTH, 0, &Depth);
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f)[0]);

	// Bind program
	glUseProgram(ProgramName);
	glUniformMatrix4fv(UniformMVP, 1, GL_FALSE, &MVP[0][0]);
	glUniform1i(UniformDisplacement, 0);
	glUniform1i(UniformDiffuse, 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_BUFFER, DisplacementTextureName);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_BUFFER, DiffuseTextureName);

	glBindVertexArray(VertexArrayName);
	glDrawArraysInstanced(GL_TRIANGLES, 0, VertexCount, 5);

	glf::checkError("display");

}

int main(int argc, char* argv[])
{
	return glf::run(argc, argv, glf::CORE, 3, 2);
}