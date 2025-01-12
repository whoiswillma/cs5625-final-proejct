#ifndef CS5625_PLAPP_H
#define CS5625_PLAPP_H

#include <map>

#include "Scene.h"
#include "OceanScene.h"

#include <nanogui/screen.h>

#include <GLWrap/Program.hpp>
#include <GLWrap/Mesh.hpp>
#include <RTUtil/Camera.hpp>
#include <RTUtil/CameraController.hpp>
#include <GLWrap/Framebuffer.hpp>
#include "Animators.h"
#include "Tessendorf.h"
#include "Timer.h"
#include "Bird.hpp"

enum ShadingMode {
    ShadingMode_Flat,
    ShadingMode_Forward,
    ShadingMode_Deferred
};

enum TextureFilteringMode {
    TextureFilteringMode_Nearest = 0,
    TextureFilteringMode_Linear = 1
};

enum OceanShadingMode {
    OceanShadingMode_Plastic,
    OceanShadingMode_Tessendorf,
    OceanShadingMode_Toon
};

struct PLAppConfig {
    glm::ivec2 shadowMapResolution = {1024, 1024};
    float shadowBias = 1e-2;
    float shadowNear = 1e-5;
    float shadowFar = 100;
    float shadowFov = 1;
    float exposure = 1;
    float thetaSun = glm::pi<float>() / 6;
    float turbidity = 4;
    int ssaoNumSamples = 15;
    int depthLineWidth = 2;
    float depthLineThreshold = 0.001f;
    int normalLineWidth = 2;
    float normalLineThreshold = 2.0f;
    float specularThreshold = 0.95f;
    float specularIntensity = 1.0f;
    float specularSmoothness = 0.5f;
    float edgeThreshold = 0.6f;
    float edgeIntensity = 3.0f;
    float strokeThreshold = 0.5f;
    float AAThreshold = 0.25f;
    float AAIntensity = 2.0f;
    bool fxaaEnabled = true;
    bool rampEnabled = false;
    bool strokeEnabled = true;
    bool multipleLightsEnabled = false;
    bool pcfEnabled = true;
    bool pointLightsEnabled = true;
    bool convertAreaToPoint = true;
    bool ambientLightsEnabled = true;
    bool sunskyEnabled = true;
    bool bloomFilterEnabled = false;
    bool toonEnabled = false;
    TextureFilteringMode textureFilteringMode = TextureFilteringMode_Linear;

    bool ocean = false;
    OceanShadingMode oceanShadingMode = OceanShadingMode_Tessendorf;
    float renderDistance = 100;

    bool birds = false;
};

class PLApp : nanogui::Screen {
public:
    PLApp(
            const std::shared_ptr<Scene>& scene,
            const std::shared_ptr<OceanScene>& oceanScene,
            int height,
            const std::string& rampFileName,
            const PLAppConfig& config = PLAppConfig()
    );

    virtual bool keyboard_event(int key, int scancode, int action, int modifiers) override;
    virtual bool mouse_button_event(const nanogui::Vector2i &p, int button, bool down, int modifiers) override;
    virtual bool mouse_motion_event(const nanogui::Vector2i &p, const nanogui::Vector2i &rel, int button, int modifiers) override;
    virtual bool scroll_event(const nanogui::Vector2i &p, const nanogui::Vector2f &rel) override;
    virtual bool resize_event(const nanogui::Vector2i &size) override;

    virtual void draw_contents() override;

private:
    struct NanoguiWindows {
        nanogui::Window* deferred;
        nanogui::Window* ocean;
        nanogui::Window* toon;
    } nanoguiWindows;

    void setUpNanoguiControls();
    void setUpMeshes();
    void setUpCamera();
    void setUpPrograms();
    void setUpTextures();
    void resetFramebuffers();
    void loadTextures();

    std::shared_ptr<Scene> scene;
    std::shared_ptr<OceanScene> oceanScene;
    Animators animators;
    std::string rampFileName;

    std::shared_ptr<GLWrap::Program> programFlat;
    std::shared_ptr<GLWrap::Program> programForward;
	std::shared_ptr<GLWrap::Program> programTextureDeferred;
    std::shared_ptr<GLWrap::Program> programDeferredGeom;
    std::shared_ptr<GLWrap::Program> programToonPoint;
    std::shared_ptr<GLWrap::Program> programToonMerge;
    std::shared_ptr<GLWrap::Program> programToonOutline;
    std::shared_ptr<GLWrap::Program> programDeferredShadow;
    std::shared_ptr<GLWrap::Program> programDeferredPoint;
    std::shared_ptr<GLWrap::Program> programDeferredAmbient;
    std::shared_ptr<GLWrap::Program> programDeferredSky;
    std::shared_ptr<GLWrap::Program> programDeferredBlur;
    std::shared_ptr<GLWrap::Program> programDeferredMerge;
    std::shared_ptr<GLWrap::Program> programSrgb;
    std::shared_ptr<GLWrap::Program> programOceanForward;
    std::shared_ptr<GLWrap::Program> programOceanDeferredGeom;
    std::shared_ptr<GLWrap::Program> programOceanDeferredShadow;
    std::shared_ptr<GLWrap::Program> programOceanDeferredDirectional;

    std::vector<std::shared_ptr<GLWrap::Mesh>> meshes;
    std::shared_ptr<GLWrap::Mesh> oceanMesh;
    std::shared_ptr<GLWrap::Mesh> fsqMesh;

    std::shared_ptr<RTUtil::PerspectiveCamera> cam;
    std::unique_ptr<RTUtil::DefaultCC> cc;

    // size = viewport, color attachments = 3, depth attachment = yes
    std::shared_ptr<GLWrap::Framebuffer> geomBuffer;

    // size = viewport, color attachments = 1, depth attachment = no, with mipmaps
    std::shared_ptr<GLWrap::Framebuffer> accBuffer, temp1, temp2, toonBuffer, outlineBuffer;

    // size = shadow map resolution, color attachments = 0, depth attachment = yes
    std::shared_ptr<GLWrap::Framebuffer> shadowMap;

	// texturmap for seagulls
	std::shared_ptr<GLWrap::Texture2D> texturemap;
    std::shared_ptr<GLWrap::Texture2D> ramp;
    std::shared_ptr<GLWrap::Texture2D> hatch1;
    std::shared_ptr<GLWrap::Texture2D> hatch2;
    std::shared_ptr<GLWrap::Texture2D> hatch3;
    std::shared_ptr<GLWrap::Texture2D> hatch4;
    std::shared_ptr<GLWrap::Texture2D> hatch5;
    std::shared_ptr<GLWrap::Texture2D> hatch6;

    nanogui::Color backgroundColor;
    PLAppConfig config;

    Timer timer;

    const std::vector<std::pair<float, int>> blurLevels = {
            {6.2, 2},
            {24.9, 4},
            {81.0, 6},
            {263.0, 8}
    };

    ShadingMode shadingMode;
    void draw_contents_flat();
    void draw_contents_forward();

    RTUtil::PerspectiveCamera get_light_camera(const PointLight &light) const;
    glm::ivec2 getViewportSize();

    void deferred_geometry_pass();
	void deferred_texture_pass();
    void deferred_ocean_geometry_pass();
    void draw_contents_deferred();
    void deferred_draw_pass(const std::shared_ptr<GLWrap::Framebuffer>& accBuffer);
    void deferred_shadow_pass(const PointLight &light);
    void deferred_ocean_shadow_pass(const PointLight &light);
    void toon_lighting_pass(
            const std::shared_ptr<GLWrap::Framebuffer>& geomBuffer,
            const GLWrap::Texture2D& shadowTexture,
            const PointLight& light
    );
    void toon_merge_pass(
        const std::shared_ptr<GLWrap::Framebuffer>& geomBuffer,
        const GLWrap::Texture2D& toonTexture,
        const glm::vec3 ambient,
        const int lightNum
    );
    void toon_outline_pass(
            const std::shared_ptr<GLWrap::Framebuffer>& geomBuffer,
            const GLWrap::Texture2D& image
    );
    void deferred_lighting_pass(
            const std::shared_ptr<GLWrap::Framebuffer> &geomBuffer,
            const GLWrap::Texture2D &shadowTexture,
            const PointLight &light
    );
    void deferred_ambient_pass(
            const std::shared_ptr<GLWrap::Framebuffer> &geomBuffer,
            const AmbientLight &light
    );
    void deferred_ocean_directional_pass(const std::shared_ptr<GLWrap::Framebuffer> &geomBuffer);
    void deferred_sky_pass(const GLWrap::Texture2D &image);
    void deferred_blur_pass(
            const GLWrap::Texture2D& image,
            glm::vec2 dir,
            float stdev,
            int level
    );
    void deferred_merge_pass(const GLWrap::Texture2D &image, const GLWrap::Texture2D &blurred);
};
#endif //CS5625_PLAPP_H