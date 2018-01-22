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
        emplaceMaterial("Sample Sun", sunMat);

#if SAMPLE_IMPLEMENTATION
        std::shared_ptr<TextureManager> textureManager = g->GetTextureManager();
        std::shared_ptr<Texture> sunText = textureManager->RegisterTexture("SolarSystem/2k_sun.jpg");
        sunMat->SetDiffuseColor(Color(1, 1, 1));
        sunMat->SetEmissiverColor(Color(1, 1, 1));
        sunText->Build();


        sunMat->AssignDiffuseTexture(TextureType::DiffuseTexture_0, sunText);

        std::shared_ptr<Texture> skyTex = textureManager->RegisterTexture("SolarSystem/stars_milky_way.jpg");
        //std::shared_ptr<Texture> skyTex = textureManager->RegisterTexture("skydome.jpg");
        std::shared_ptr<Texture> earthTex = textureManager->RegisterTexture("SolarSystem/2k_earth.jpg");
        std::shared_ptr<Texture> earthCloudsTex = textureManager->RegisterTexture("SolarSystem/2k_earth_clouds.jpg");
        //std::shared_ptr<Texture> moonTex = textureManager->RegisterTexture("SolarSystem/2k_moon.jpg");
        std::shared_ptr<Texture> moonTex = textureManager->RegisterTexture("SolarSystem/8k/moon.jpg");
        std::shared_ptr<Texture> marsTex = textureManager->RegisterTexture("SolarSystem/2k_mars.jpg");
        std::shared_ptr<Texture> jupiterTex = textureManager->RegisterTexture("SolarSystem/2k_jupiter.jpg");
        std::shared_ptr<Texture> mercuryTex = textureManager->RegisterTexture("SolarSystem/2k_mercury.jpg");
        std::shared_ptr<Texture> neptuneTex = textureManager->RegisterTexture("SolarSystem/2k_neptune.jpg");
        std::shared_ptr<Texture> saturnTex = textureManager->RegisterTexture("SolarSystem/2k_saturn.jpg");
        std::shared_ptr<Texture> uranusTex = textureManager->RegisterTexture("SolarSystem/2k_uranus.jpg");
        std::shared_ptr<Texture> venusTex = textureManager->RegisterTexture("SolarSystem/2k_venus.jpg");
        earthTex->Build();
        earthCloudsTex->Build();
        moonTex->Build();
        marsTex->Build();
        skyTex->Build();
        jupiterTex->Build();
        mercuryTex->Build();
        neptuneTex->Build();
        saturnTex->Build();
        uranusTex->Build();
        venusTex->Build();
    
        //skybox
        std::shared_ptr<Material> skyboxMat = std::shared_ptr<Material>(new Material);
        skyboxMat->AssignDiffuseTexture(TextureType::DiffuseTexture_0, skyTex);
        skyboxMat->SetDiffuseColor(Color(1, 1, 1));
        skyboxMat->SetIfReceiveLight(false);
        skyboxMat->SetAmbientColor(Color(0.1f, 0.1f, 0.1f));
        emplaceMaterial("Skydome", skyboxMat);

        ////////////////////////////////////////////////////////////////////////////
        //      Hardcode materials
        //earth
        std::shared_ptr<Material> earthMat = std::shared_ptr<Material>(new Material);
        earthMat->SetAmbientColor(Color(0.1f, 0.1f, 0.1f));
        earthMat->SetDiffuseColor(Color(1, 1, 1));
        earthMat->AssignDiffuseTexture(TextureType::DiffuseTexture_0, earthTex);
        emplaceMaterial("Earth", earthMat);


        //moon
        std::shared_ptr<Material> moonMat = std::make_shared<Material>(*earthMat);
        moonMat->AssignDiffuseTexture(TextureType::DiffuseTexture_0, moonTex);
        emplaceMaterial("Moon", moonMat);

        //mars
        std::shared_ptr<Material> marsMat = std::make_shared<Material>(*earthMat);
        marsMat->AssignDiffuseTexture(TextureType::DiffuseTexture_0, marsTex);
        emplaceMaterial("Mars", marsMat);
        
        //jupiter
        std::shared_ptr<Material> jupiterMat = std::make_shared<Material>(*earthMat);
        jupiterMat->AssignDiffuseTexture(TextureType::DiffuseTexture_0, jupiterTex);
        emplaceMaterial("Jupiter", jupiterMat);

        //mercury
        std::shared_ptr<Material> mercuryMat = std::make_shared<Material>(*earthMat);
        mercuryMat->AssignDiffuseTexture(TextureType::DiffuseTexture_0, mercuryTex);
        emplaceMaterial("Mercury", mercuryMat);
        
        //neptune
        std::shared_ptr<Material> neptuneMat = std::make_shared<Material>(*earthMat);
        neptuneMat->AssignDiffuseTexture(TextureType::DiffuseTexture_0, neptuneTex);
        emplaceMaterial("Neptune", neptuneMat);

        //saturn
        std::shared_ptr<Material> saturnMat = std::make_shared<Material>(*earthMat);
        saturnMat->AssignDiffuseTexture(TextureType::DiffuseTexture_0, saturnTex);
        emplaceMaterial("Saturn", saturnMat);

        //uranus
        std::shared_ptr<Material> uranusMat = std::make_shared<Material>(*earthMat);
        uranusMat->AssignDiffuseTexture(TextureType::DiffuseTexture_0, uranusTex);
        emplaceMaterial("Uranus", uranusMat);

        //venus
        std::shared_ptr<Material> venusMat = std::make_shared<Material>(*earthMat);
        venusMat->AssignDiffuseTexture(TextureType::DiffuseTexture_0, venusTex);
        emplaceMaterial("Venus", venusMat);



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
        std::vector<std::shared_ptr<Material> > materials;
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
