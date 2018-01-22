#include "Precompiled.h"
#include "graphics/Shader.h"
#include "framework/Debug.h"

namespace Graphics
{
    Shader::Shader(std::vector<std::shared_ptr<ShaderProgram> > const& shaderPrograms, bool isDeferred,
                   ShaderType shaderType): m_shaderPrograms(shaderPrograms), m_isDeferred(isDeferred),
                                           m_shaderType(shaderType)
    {
        m_shaderPrograms.reserve(static_cast<size_t>(ShaderStage::Count));
    }

    Shader::~Shader()
    {
    }

    std::shared_ptr<ShaderProgram> Shader::GetShaderProgram(ShaderStage stage)
    {
        Assert(static_cast<unsigned char>(stage) < m_shaderPrograms.size(), "Invalid index to retrieve shader program.");
        return m_shaderPrograms[static_cast<unsigned char>(stage)];
    }
}
