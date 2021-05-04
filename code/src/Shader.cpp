#include "Shader.h"

extern GLuint compileShader(const char* shaderStr, GLenum shaderType, const char* name);
extern void linkProgram(GLuint program);

Shader::Shader() {}

Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath)
{
    // 1. retrieve the vertex/geomtry/fragment source code from filePath
    std::string vertexCode;
    std::string geometryCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream gShaderFile;
    std::ifstream fShaderFile;

    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        // open files
        vShaderFile.open(vertexPath);
        gShaderFile.open(geometryPath);
        fShaderFile.open(fragmentPath);

        std::stringstream vShaderStream, gShaderStream,fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        gShaderStream << gShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // close file handlers
        vShaderFile.close();
        gShaderFile.close();
        fShaderFile.close();
        // convert stream into string
        vertexCode = vShaderStream.str();
        geometryCode = gShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    
    const char* vShaderCode = vertexCode.c_str();
    const char* gShaderCode = geometryCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    // 2. compile shaders
    unsigned int vertex, geometry, fragment;
    int success;
    char infoLog[512];
    
    // compile vertex, geometry, fargment
    vertex = compileShader(vShaderCode, GL_VERTEX_SHADER, "Vertex shader");
    geometry = compileShader(gShaderCode, GL_GEOMETRY_SHADER, "Geometry shader");
    fragment = compileShader(fShaderCode, GL_FRAGMENT_SHADER, "Fragment shader");
    
    // Shader Program
    program = glCreateProgram();
    glAttachShader(program, vertex);   
    glAttachShader(program, geometry);
    glAttachShader(program, fragment);
    
    glBindAttribLocation(program, 0, "in_Position");
    glBindAttribLocation(program, 1, "in_Normal");
    glBindAttribLocation(program, 2, "in_TexCoord");
    
    linkProgram(program);

    // Delete the shaders as they're linked into our program now and no longer necessary
    //glDeleteShader(vertex);
    //glDeleteShader(geometry);
    //glDeleteShader(fragment);
}

Shader::~Shader()
{

}

void Shader::Use()
{
    glUseProgram(program);
}

void Shader::SetMatrix(const std::string& name, int size, bool transpose, const float* value)
{
    glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 1, transpose, value);
}

void Shader::SetFloat(const std::string& name, float val1, float val2, float val3, float val4) const
{
    glUniform4f(glGetUniformLocation(program, name.c_str()), val1, val2, val3, val4);
}
void Shader::SetFloat(const std::string& name, float val1, float val2, float val3) const
{
    glUniform3f(glGetUniformLocation(program, name.c_str()), val1, val2, val3);
}
void Shader::SetFloat(const std::string& name, float val1) const
{
    glUniform1f(glGetUniformLocation(program, name.c_str()), val1);
}
void Shader::SetInt(const std::string& name, int val1) const
{
    glUniform1i(glGetUniformLocation(program, name.c_str()), val1);
}