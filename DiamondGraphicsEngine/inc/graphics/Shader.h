#pragma once
#include "graphics/ShaderManager.h"

namespace Graphics
{
    class Shader
    {
    public:
        Shader() = default;
        Shader(std::vector<std::shared_ptr<ShaderProgram>> const& shaderPrograms
               , bool isDeferred, ShaderType shaderType);

        virtual ~Shader();

        virtual std::shared_ptr<ShaderProgram> GetShaderProgram(ShaderStage stage = ShaderStage::ForwardRendering);
        virtual std::vector<std::shared_ptr<ShaderProgram> >& GetAllShadersRef() { return m_shaderPrograms; }

        constexpr bool IsDeferred() const { return m_isDeferred; }
        constexpr ShaderType GetShaderType() const { return m_shaderType; }

    protected:
        std::vector<std::shared_ptr<ShaderProgram> > m_shaderPrograms;
        const bool m_isDeferred;
        const ShaderType m_shaderType;
    };
}
