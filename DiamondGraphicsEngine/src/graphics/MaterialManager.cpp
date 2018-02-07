#include "Precompiled.h"
#include "graphics/MaterialManager.h"
#include "framework/Debug.h"
#include "graphics/Materials.h"
#include "graphics/GraphicsEngine.h"
#include "graphics/TextureManager.h"
#include "graphics/Texture.h"

namespace Graphics
{
    void MaterialManager::LoadMaterials(std::string const& fileName, std::shared_ptr<GraphicsEngine> g)
    {
        std::stringstream strstr;
        strstr << ASSET_PATH << "materials/" << fileName;
        //TODO(Assignemtn 1): Load all materials in "basic.mtl" and store them in m_materials
        //The material file name is the file name under assets\materials\ directory
        //once you load them in, make sure you call function empalceMaterial after a material
        //is loaded and setup, so it will be stored in the container.


        //TODO(Assignemtn 1): Remove everything below, in this function. Since material will be loaded, not hardcoded
        ////////////////////////////////////////////////////////////////////////////
        //      Texture
        //sun
        std::shared_ptr<Material> sunMat = std::shared_ptr<Material>(new Material);
        sunMat->SetAmbientColor(Color(0.5f, 0.1f, 0.1f));
        sunMat->SetIlluminationModel(1);
        sunMat->SetIfReceiveLight(false);
        emplaceMaterial("Sun", sunMat);

#if SAMPLE_IMPLEMENTATION
        //sun
        sunMat->SetDiffuseColor(Color(1, 1, 1));
        sunMat->SetEmissiverColor(Color(1, 1, 1));


        //skybox
        //textures.at(skyboxTex)->Build();
        std::shared_ptr<Material> skyboxMat = std::shared_ptr<Material>(new Material);
        skyboxMat->SetDiffuseColor(Color(1, 1, 1));
        skyboxMat->SetIfReceiveLight(false);
        skyboxMat->SetAmbientColor(Color(0.1f, 0.1f, 0.1f));
        skyboxMat->SetSpecularColor(Color(0,0,0));
        emplaceMaterial("Skydome", skyboxMat);

        ////////////////////////////////////////////////////////////////////////////
        //      Hardcode materials
        //earth
        std::shared_ptr<Material> earthMat = std::shared_ptr<Material>(new Material);
        earthMat->SetAmbientColor(Color(0.1f, 0.1f, 0.1f));
        earthMat->SetDiffuseColor(Color(1, 1, 1));
        emplaceMaterial("Earth", earthMat);


        //moon
        std::shared_ptr<Material> moonMat = std::make_shared<Material>(*earthMat);
        emplaceMaterial("Moon", moonMat);

        //mars
        std::shared_ptr<Material> marsMat = std::make_shared<Material>(*earthMat);
        emplaceMaterial("Mars", marsMat);

        //jupiter
        std::shared_ptr<Material> jupiterMat = std::make_shared<Material>(*earthMat);
        emplaceMaterial("Jupiter", jupiterMat);

        //mercury
        std::shared_ptr<Material> mercuryMat = std::make_shared<Material>(*earthMat);
        emplaceMaterial("Mercury", mercuryMat);

        //neptune
        std::shared_ptr<Material> neptuneMat = std::make_shared<Material>(*earthMat);
        emplaceMaterial("Neptune", neptuneMat);

        //saturn
        std::shared_ptr<Material> saturnMat = std::make_shared<Material>(*earthMat);
        emplaceMaterial("Saturn", saturnMat);

        //uranus
        std::shared_ptr<Material> uranusMat = std::make_shared<Material>(*earthMat);
        emplaceMaterial("Uranus", uranusMat);

        //venus
        std::shared_ptr<Material> venusMat = std::make_shared<Material>(*earthMat);
        emplaceMaterial("Venus", venusMat);

        //plane
        std::shared_ptr<Material> planeMesh = std::make_shared<Material>(*earthMat);
        emplaceMaterial("Plane", planeMesh);


        std::shared_ptr<TextureManager> textureManager = g->GetTextureManager();

        std::string skyboxTex = "SolarSystem/Warehouse-with-lights.jpg";
        std::string earthTexWithClouds = "SolarSystem/8k/earth_daymap_with_clouds.jpg";
        std::string earthNormalMap = "SolarSystem/8k/earth_normal_map.jpg";
        std::string earthClouds = "SolarSystem/2k_earth_clouds.jpg";
        std::string moonTex = "SolarSystem/8k/moon.jpg";
        std::string marsTex = "SolarSystem/2k_mars.jpg";
        std::string jupiterTex = "SolarSystem/2k_jupiter.jpg";
        std::string mercuryTex = "SolarSystem/2k_mercury.jpg";
        std::string neptuneTex = "SolarSystem/2k_neptune.jpg";
        std::string saturnTex = "SolarSystem/2k_saturn.jpg";
        std::string uranusTex = "SolarSystem/2k_uranus.jpg";
        std::string venusTex = "SolarSystem/2k_venus.jpg";
        std::string sunTex = "SolarSystem/2k_sun.jpg";

        std::map<std::string, std::shared_ptr<Texture> > & textures = textureManager->GetTextureMap();
        textureManager->LoadTextureMultiThreadRealTime(
        {
            { skyboxTex             },
            { earthTexWithClouds    },
            { earthNormalMap        },
            { earthClouds           },
            { moonTex               },
            { marsTex               },
            { jupiterTex            },
            { mercuryTex            },
            { neptuneTex            },
            { saturnTex             },
            { uranusTex             },
            { venusTex              },
            { sunTex                },
        });

        sunMat->AssignDiffuseTexture(TextureType::DiffuseTexture_0, textures.at(sunTex));
        skyboxMat->AssignDiffuseTexture(TextureType::DiffuseTexture_0, textures.at(skyboxTex));
        earthMat->AssignDiffuseTexture(TextureType::DiffuseTexture_0, textures.at(earthTexWithClouds));
        moonMat->AssignDiffuseTexture(TextureType::DiffuseTexture_0, textures.at(moonTex));
        marsMat->AssignDiffuseTexture(TextureType::DiffuseTexture_0, textures.at(marsTex));
        jupiterMat->AssignDiffuseTexture(TextureType::DiffuseTexture_0, textures.at(jupiterTex));
        mercuryMat->AssignDiffuseTexture(TextureType::DiffuseTexture_0, textures.at(mercuryTex));
        neptuneMat->AssignDiffuseTexture(TextureType::DiffuseTexture_0, textures.at(neptuneTex));
        saturnMat->AssignDiffuseTexture(TextureType::DiffuseTexture_0, textures.at(saturnTex));
        uranusMat->AssignDiffuseTexture(TextureType::DiffuseTexture_0, textures.at(uranusTex));
        venusMat->AssignDiffuseTexture(TextureType::DiffuseTexture_0, textures.at(venusTex));

#endif // 0
    }

    std::shared_ptr<Material> MaterialManager::AddMaterial(std::shared_ptr<Material> material)
    {
        m_materials[material->GetMaterialName()] = material;
        return material;
    }

    std::shared_ptr<Material> MaterialManager::GetMaterial(std::string const& matName)
    {
#ifdef _DEBUG
        if (m_materials.find(matName) == m_materials.end())
        {
            Warning("Material not found with name \"%s\".", matName.c_str());
            return nullptr;
        }
#endif // DEBUG
        return m_materials[matName];
    }


    std::vector<std::shared_ptr<Material>> MaterialManager::GetAllMaterials() const
    {
        std::vector<std::shared_ptr<Material>> materials;
        for (auto& i : m_materials)
        {
            materials.push_back(i.second);
        }
        return materials;
    }

    void MaterialManager::emplaceMaterial(std::string const& matName, std::shared_ptr<Material> material)
    {
        material->SetMaterialName(matName);
        m_materials.emplace(matName, material);
    }
}
