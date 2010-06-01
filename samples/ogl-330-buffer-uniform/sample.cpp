//**********************************
// OpenGL uniform buffer
// 06/04/2010
//**********************************
// Christophe Riccio
// g.truc.creation@gmail.com
//**********************************
// G-Truc Creation
// www.g-truc.net
//**********************************

#include "sample.hpp"

namespace
{
	std::string const SAMPLE_NAME = "OpenGL uniform buffer";
	GLint const SAMPLE_MAJOR_VERSION = 3;
	GLint const SAMPLE_MINOR_VERSION = 3;
	std::string const VERTEX_SHADER_SOURCE(glf::DATA_DIRECTORY + "330/uniform-buffer.vert");
	std::string const FRAGMENT_SHADER_SOURCE(glf::DATA_DIRECTORY + "330/uniform-buffer.frag");

	GLsizei const VertexCount = 4;
	GLsizeiptr const PositionSize = VertexCount * sizeof(glm::vec2);
	glm::vec2 const PositionData[VertexCount] =
	{
		glm::vec2(-1.0f,-1.0f),
		glm::vec2( 1.0f,-1.0f),
		glm::vec2( 1.0f, 1.0f),
		glm::vec2(-1.0f, 1.0f)
	};

	GLsizei const ElementCount = 6;
	GLsizeiptr const ElementSize = ElementCount * sizeof(GLushort);
	GLushort const ElementData[ElementCount] =
	{
		0, 1, 2, 
		2, 3, 0
	};

}//namespace

sample::sample
(
	std::string const & Name, 
	glm::ivec2 const & WindowSize,
	glm::uint32 VersionMajor,
	glm::uint32 VersionMinor
) :
	window(Name, WindowSize, VersionMajor, VersionMinor),
	ProgramName(0)
{}

sample::~sample()
{}

bool sample::check() const
{
	GLint MajorVersion = 0;
	GLint MinorVersion = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &MajorVersion);
	glGetIntegerv(GL_MINOR_VERSION, &MinorVersion);
	bool Version = (MajorVersion * 10 + MinorVersion) >= (SAMPLE_MAJOR_VERSION * 10 + SAMPLE_MINOR_VERSION);
	return Version && glf::checkError("sample::check");
}

bool sample::begin(glm::ivec2 const & WindowSize)
{
	this->WindowSize = WindowSize;

	bool Validated = true;
	if(Validated)
		Validated = this->initProgram();
	if(Validated)
		Validated = this->initArrayBuffer();
	if(Validated)
		Validated = this->initVertexArray();
	if(Validated)
		Validated = this->initUniformBuffer();

	return Validated && glf::checkError("sample::begin");
}

bool sample::end()
{
	glDeleteVertexArrays(1, &this->VertexArrayName);
	glDeleteBuffers(1, &this->ArrayBufferName);
	glDeleteBuffers(1, &this->ElementBufferName);
	glDeleteBuffers(1, &this->TransformBufferName);
	glDeleteBuffers(1, &this->MaterialBufferName);
	glDeleteProgram(this->ProgramName);

	return glf::checkError("sample::end");
}

void sample::render()
{
	// Compute the MVP (Model View Projection matrix)
	glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -TranlationCurrent.y));
	glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, RotationCurrent.y, glm::vec3(-1.f, 0.f, 0.f));
	glm::mat4 View = glm::rotate(ViewRotateX, RotationCurrent.x, glm::vec3(0.f, 1.f, 0.f));
	glm::mat4 Model = glm::mat4(1.0f);
	glm::mat4 MVP = Projection * View * Model;

	glBindBuffer(GL_UNIFORM_BUFFER, this->TransformBufferName);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(MVP), &MVP[0][0]);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Set the display viewport
	glViewport(0, 0, this->WindowSize.x, this->WindowSize.y);

	// Clear color buffer with black
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Bind program
	glUseProgram(this->ProgramName);

	// Bind vertex array & draw 
	glBindVertexArray(this->VertexArrayName);
		glDrawElements(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);

	// Unbind program
	glUseProgram(0);

	glf::checkError("sample::render");
}

bool sample::initProgram()
{
	bool Validated = true;
	
	// Create program
	if(Validated)
	{
		this->ProgramName = glf::createProgram(VERTEX_SHADER_SOURCE, FRAGMENT_SHADER_SOURCE);
		glLinkProgram(this->ProgramName);
		Validated = glf::checkProgram(this->ProgramName);
	}

	// Get variables locations
	if(Validated)
	{
		this->UniformMaterial = glGetUniformBlockIndex(this->ProgramName, "material");
		this->UniformTransform = glGetUniformBlockIndex(this->ProgramName, "transform");
	}

	return Validated && glf::checkError("sample::initProgram");
}

bool sample::initVertexArray()
{
	// Build a vertex array object
	glGenVertexArrays(1, &this->VertexArrayName);
    glBindVertexArray(this->VertexArrayName);
		glBindBuffer(GL_ARRAY_BUFFER, this->ArrayBufferName);
		glVertexAttribPointer(glf::semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(glf::semantic::attr::POSITION);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ElementBufferName);
	glBindVertexArray(0);

	return glf::checkError("sample::initVertexArray");
}

bool sample::initArrayBuffer()
{
	// Generate a buffer object
	glGenBuffers(1, &this->ElementBufferName);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ElementBufferName);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenBuffers(1, &this->ArrayBufferName);
    glBindBuffer(GL_ARRAY_BUFFER, this->ArrayBufferName);
    glBufferData(GL_ARRAY_BUFFER, PositionSize, PositionData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return glf::checkError("sample::initArrayBuffer");
}

bool sample::initUniformBuffer()
{
	GLint UniformBlockSize = 0;

	{
		glGetActiveUniformBlockiv(
			this->ProgramName, 
			this->UniformTransform,
			GL_UNIFORM_BLOCK_DATA_SIZE,
			&UniformBlockSize);

		glGenBuffers(1, &this->TransformBufferName);
		glBindBuffer(GL_UNIFORM_BUFFER, this->TransformBufferName);
		glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, 0, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		// Attach the buffer to UBO binding point glf::semantic::uniform::TRANSFORM
		glBindBufferBase(GL_UNIFORM_BUFFER, glf::semantic::uniform::TRANSFORM, this->TransformBufferName);
		// Associate the uniform block to this binding point.
		glUniformBlockBinding(this->ProgramName, this->UniformTransform, glf::semantic::uniform::TRANSFORM);
	}

	{
		glm::vec4 Diffuse(1.0f, 0.5f, 0.0f, 1.0f);

		glGetActiveUniformBlockiv(
			this->ProgramName, 
			this->UniformMaterial,
			GL_UNIFORM_BLOCK_DATA_SIZE,
			&UniformBlockSize);

		glGenBuffers(1, &this->MaterialBufferName);
		glBindBuffer(GL_UNIFORM_BUFFER, this->MaterialBufferName);
		glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, &Diffuse[0], GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		// Attach the buffer to UBO binding point glf::semantic::uniform::MATERIAL
		glBindBufferBase(GL_UNIFORM_BUFFER, glf::semantic::uniform::MATERIAL, this->MaterialBufferName);
		// Associate the uniform block to this binding point.
		glUniformBlockBinding(this->ProgramName, this->UniformMaterial, glf::semantic::uniform::MATERIAL);
	}

	return glf::checkError("sample::initUniformBuffer");
}

int main(int argc, char* argv[])
{
	glm::ivec2 ScreenSize(640, 480);

	sample* Sample = new sample(
		SAMPLE_NAME, 
		ScreenSize, 
		SAMPLE_MAJOR_VERSION,
		SAMPLE_MINOR_VERSION);

	if(Sample->check())
	{
		Sample->begin(ScreenSize);
		Sample->run();
		Sample->end();

		delete Sample;
		return 0;
	}

	fprintf(stderr, "OpenGL Error: this sample failed to run\n");

	delete Sample;
	Sample = 0;
	return 1;
}