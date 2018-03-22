#include "Precompiled.h"
#include "graphics/ShaderManager.h"
#include "graphics/Shader.h"
#include "graphics/ShaderProgram.h"

namespace
{
  // Kept as a file-scope global so that it can be returned as a const ref.
  std::shared_ptr<Graphics::ShaderProgram> const NullProgram = nullptr;
  std::shared_ptr<Graphics::Shader> const NullShader = nullptr;
}

namespace Graphics
{
  ShaderManager::ShaderManager()
  {
  }

  ShaderManager::~ShaderManager()
  {
  }
  
    std::shared_ptr<Shader> ShaderManager::LoadShader(ShaderType shaderType, std::string const& vertexSourceFile,
        std::string const& fragmentSourceFile)
    {
        std::shared_ptr<ShaderProgram> program = ShaderProgram::LoadShaderProgram(vertexSourceFile, fragmentSourceFile);
        program->Build();
        std::vector<std::shared_ptr<ShaderProgram> > shaders;
        shaders.emplace_back(program);

        std::shared_ptr<Shader> spshader = std::make_shared<Shader>(shaders, false, shaderType);

        m_shaders.emplace(shaderType, spshader);

        return spshader;
    }

    
    std::shared_ptr<Shader> ShaderManager::LoadShader(ShaderType shaderType,
        std::vector<std::tuple<std::string, std::string, ShaderUsage>> const& shaderFiles)
    {
        std::vector<std::shared_ptr<ShaderProgram> > shaders;
        for (auto& i : shaderFiles)
        {
            if (std::get<2>(i) == ShaderUsage::ComputeShader)
            {
                std::shared_ptr<ShaderProgram> program = ShaderProgram::LoadOtherShaderProgram(std::get<0>(i), ShaderUsage::ComputeShader);
                program->SetUsage(ShaderUsage::ComputeShader);
                program->Build(ShaderUsage::ComputeShader);
                shaders.emplace_back(program);
            }
            else
            {
                std::shared_ptr<ShaderProgram> program = ShaderProgram::LoadShaderProgram(std::get<0>(i), std::get<1>(i));
                program->SetUsage(std::get<2>(i));
                program->Build();
                shaders.emplace_back(program);
            }
        }

        Shader* pshader = new Shader(shaders, true, shaderType);
        std::shared_ptr<Shader> spshader = std::shared_ptr<Shader>(pshader);

        m_shaders.emplace(shaderType, spshader);

        return spshader;
    }

    //std::shared_ptr<ShaderProgram> const &ShaderManager::GetShader(
  //  ShaderType type) const
  //{
  //  // find a shader given the specified type; if it doesn't exist, return null
  //  // instead
  //  auto find = m_shaderPrograms.find(type);
  //  return (find != m_shaderPrograms.end()) ? find->second : NullProgram;
  //}
  std::shared_ptr<Shader> const &ShaderManager::GetShader(
      ShaderType type) const
  {
      // find a shader given the specified type; if it doesn't exist, return null
      // instead
      auto find = m_shaders.find(type);
      return (find != m_shaders.end()) ? find->second : NullShader;
  }
  void ShaderManager::ClearAllShaders()
  {
    m_shaders.clear(); // delete all shader program instances registered
  }
}
