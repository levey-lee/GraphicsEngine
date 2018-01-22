#pragma once
#include "core/ComponentBase.h"
#include "graphics/LightManager.h"

namespace Graphics
{
    struct LightAttribute;
}

namespace Component
{
    class Light
        : public ComponentBase<Light, UNSHADED>
        , public Graphics::LightBase
    {
    public:
        explicit Light(bool defaultEnable = true) : ComponentBase(defaultEnable) {}
        void Start() override;
        void Update(float dt) override;

        REGISTER_EDITOR_COMPONENT(Light)
		void Reflect(TwBar* editor, std::string const& barName, std::string const& groupName, Graphics::GraphicsEngine* graphics) override;

    protected:
        void OnEnable() override;
        void OnDisable() override;
    };
}
