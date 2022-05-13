//
// Created by William Ma on 3/12/22.
//

#include <cmath>
#include <memory>
#include "PLApp.h"
#include <nanogui/window.h>
#include <cpplocate/cpplocate.h>
#include <nanogui/formhelper.h>
#include "RTUtil/output.hpp"
#include "RTUtil/Sky.hpp"
#include "GLWrap/Framebuffer.hpp"
#include "MulUtil.hpp"
#include "Tessendorf.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

PLApp::PLApp(
        const std::shared_ptr<Scene> &scene,
        const std::shared_ptr<OceanScene> &oceanScene,
        int height,
        const PLAppConfig &config
) : nanogui::Screen(
        nanogui::Vector2i((int) round(scene->camera->getAspectRatio() * (float) height), height),
        "Pipeline App",
        true),
    backgroundColor(0.4f, 0.4f, 0.7f, 1.0f),
    scene(scene),
    oceanScene(oceanScene),
    shadingMode(ShadingMode_Deferred),
    config(config),
    oceanBuffers({
         tessendorf::array2d<std::complex<float>>(oceanScene->gridSize.x, oceanScene->gridSize.y),
         tessendorf::array2d<std::complex<float>>(oceanScene->gridSize.x, oceanScene->gridSize.y),
         tessendorf::array2d<std::complex<float>>(oceanScene->gridSize.x, oceanScene->gridSize.y),
         tessendorf::array2d<std::complex<float>>(oceanScene->gridSize.x, oceanScene->gridSize.y),
         tessendorf::array2d<float>(oceanScene->gridSize.x, oceanScene->gridSize.y), 0, 0,
         tessendorf::array2d<float>(oceanScene->gridSize.x, oceanScene->gridSize.y), 0, 0,
         tessendorf::array2d<float>(oceanScene->gridSize.x, oceanScene->gridSize.y), 0, 0,
    }) {

    resetFramebuffers();
    setUpPrograms();
    setUpCamera();
    setUpMeshes();
    setUpNanoguiControls();
    setUpTextures();
	//load_texture("../resources/scenes/bsptD.jpg");
    if (config.birds) {
        add_birds(this->scene->root);
    }

    set_visible(true);
}

void PLApp::add_birds(std::shared_ptr<Node> curr_node) {
    if (Bird::is_bird(curr_node->name)) {
        this->birds.push_back(Bird(curr_node));
    }
    for (std::shared_ptr<Node> child : curr_node->children) {
        add_birds(child);
    }
};

void PLApp::resetFramebuffers() {
    geomBuffer = std::make_shared<GLWrap::Framebuffer>(getViewportSize(), 3);

    std::vector<std::shared_ptr<GLWrap::Framebuffer> *> colorBuffers = {
            &accBuffer, &temp1, &temp2
    };

    for (auto &colorBufferPtr: colorBuffers) {
        auto colorBuffer = std::shared_ptr<GLWrap::Framebuffer>(new GLWrap::Framebuffer(
                getViewportSize(),
                {{GL_RGBA32F, GL_RGBA}}
        ));
        colorBuffer->colorTexture().generateMipmap();

        switch (config.textureFilteringMode) {
            case TextureFilteringMode_Linear:
                colorBuffer->colorTexture().parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                break;
            case TextureFilteringMode_Nearest:
                colorBuffer->colorTexture().parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                break;
        }

        *colorBufferPtr = colorBuffer;
    }

    shadowMap = std::shared_ptr<GLWrap::Framebuffer>(new GLWrap::Framebuffer(
            config.shadowMapResolution,
            0
    ));
}

void PLApp::setUpPrograms() {
    const std::string resourcePath =
            cpplocate::locatePath("resources", "", nullptr) + "resources/";

    programFlat = std::shared_ptr<GLWrap::Program>(new GLWrap::Program("flat", {
            {GL_VERTEX_SHADER,   resourcePath + "shaders/min.vs"},
            {GL_GEOMETRY_SHADER, resourcePath + "shaders/flat.gs"},
            {GL_FRAGMENT_SHADER, resourcePath + "shaders/lambert.fs"}
    }));

    programForward = std::shared_ptr<GLWrap::Program>(new GLWrap::Program("forward", {
            {GL_VERTEX_SHADER,   resourcePath + "shaders/deferred.vs"},
            {GL_FRAGMENT_SHADER, resourcePath + "shaders/microfacet.fs"},
            {GL_FRAGMENT_SHADER, resourcePath + "shaders/forward.fs"}
    }));

    programDeferredGeom = std::shared_ptr<GLWrap::Program>(new GLWrap::Program("deferred geometry pass", {
            {GL_VERTEX_SHADER,   resourcePath + "shaders/deferred.vs"},
            {GL_FRAGMENT_SHADER, resourcePath + "shaders/deferred_geom.fs"}
    }));

    programDeferredShadow = std::shared_ptr<GLWrap::Program>(new GLWrap::Program("deferred shadow pass", {
            {GL_VERTEX_SHADER,   resourcePath + "shaders/deferred.vs"},
            {GL_FRAGMENT_SHADER, resourcePath + "shaders/deferred_shadow.fs"}
    }));

    programDeferredPoint = std::shared_ptr<GLWrap::Program>(new GLWrap::Program("deferred point light pass", {
            {GL_VERTEX_SHADER,   resourcePath + "shaders/fsq.vs"},
            {GL_FRAGMENT_SHADER, resourcePath + "shaders/microfacet.fs"},
            {GL_FRAGMENT_SHADER, resourcePath + "shaders/deferred_shader_inputs.fs"},
            {GL_FRAGMENT_SHADER, resourcePath + "shaders/deferred_point.fs"}
    }));

    programDeferredAmbient = std::shared_ptr<GLWrap::Program>(new GLWrap::Program("deferred ambient light pass", {
            {GL_VERTEX_SHADER,   resourcePath + "shaders/fsq.vs"},
            {GL_FRAGMENT_SHADER, resourcePath + "shaders/deferred_shader_inputs.fs"},
            {GL_FRAGMENT_SHADER, resourcePath + "shaders/deferred_ambient.fs"}
    }));

    programDeferredSky = std::shared_ptr<GLWrap::Program>(new GLWrap::Program("deferred sky pass", {
            {GL_VERTEX_SHADER,   resourcePath + "shaders/fsq.vs"},
            {GL_FRAGMENT_SHADER, resourcePath + "shaders/deferred_sky.fs"},
            {GL_FRAGMENT_SHADER, resourcePath + "shaders/sunsky.fs"}
    }));

    programDeferredBlur = std::shared_ptr<GLWrap::Program>(new GLWrap::Program("deferred blur pass", {
            {GL_VERTEX_SHADER,   resourcePath + "shaders/fsq.vs"},
            {GL_FRAGMENT_SHADER, resourcePath + "shaders/blur.fs"},
    }));

    programDeferredMerge = std::shared_ptr<GLWrap::Program>(new GLWrap::Program("deferred merge pass", {
            {GL_VERTEX_SHADER,   resourcePath + "shaders/fsq.vs"},
            {GL_FRAGMENT_SHADER, resourcePath + "shaders/deferred_merge.fs"},
    }));

    programOceanForward = std::shared_ptr<GLWrap::Program>(new GLWrap::Program("ocean forward", {
            {GL_VERTEX_SHADER,   resourcePath + "shaders/ocean.vs"},
            {GL_FRAGMENT_SHADER, resourcePath + "shaders/microfacet.fs"},
            {GL_FRAGMENT_SHADER, resourcePath + "shaders/forward.fs"}
    }));

    programOceanDeferredGeom = std::shared_ptr<GLWrap::Program>(new GLWrap::Program("ocean deferred geometry pass", {
            {GL_VERTEX_SHADER,   resourcePath + "shaders/ocean.vs"},
            {GL_FRAGMENT_SHADER, resourcePath + "shaders/deferred_geom.fs"}
    }));

    programOceanDeferredShadow = std::shared_ptr<GLWrap::Program>(new GLWrap::Program("ocean deferred shadow pass", {
            {GL_VERTEX_SHADER,   resourcePath + "shaders/ocean.vs"},
            {GL_FRAGMENT_SHADER, resourcePath + "shaders/deferred_shadow.fs"}
    }));

	programTextureDeferred = std::shared_ptr<GLWrap::Program>(new GLWrap::Program("deferred Texture pass", {
			{GL_VERTEX_SHADER,   resourcePath + "shaders/deferred_texture.vs"},
			{GL_FRAGMENT_SHADER, resourcePath + "shaders/deferred_geom_texture.fs"}
	}));

    programSrgb = std::shared_ptr<GLWrap::Program>(new GLWrap::Program("srgb", {
            {GL_VERTEX_SHADER,   resourcePath + "shaders/fsq.vs"},
            {GL_FRAGMENT_SHADER, resourcePath + "shaders/srgb.fs"}
    }));
	programTextureSrgb = std::shared_ptr<GLWrap::Program>(new GLWrap::Program("texture_srgb", {
		{GL_VERTEX_SHADER,   resourcePath + "shaders/texturefsq.vs"},
		{GL_FRAGMENT_SHADER, resourcePath + "shaders/texturesrgb.fs"}
		}));
}

void PLApp::setUpCamera() {
    cam = scene->camera;
    cc = std::make_unique<RTUtil::DefaultCC>(cam);
}

void PLApp::setUpMeshes() {
    for (const Mesh &mesh: scene->meshes) {
        std::unique_ptr<GLWrap::Mesh> glWrapMesh = std::make_unique<GLWrap::Mesh>();

        glWrapMesh->setAttribute(0, mesh.vertices);
        glWrapMesh->setAttribute(1, mesh.normals);
        glWrapMesh->setAttribute(2, mesh.boneIndices);
        glWrapMesh->setAttribute(3, mesh.boneWeights);
		glWrapMesh->setAttribute(4, mesh.uvcoordinates);
        glWrapMesh->setIndices(mesh.indices, GL_TRIANGLES);

        meshes.push_back(std::move(glWrapMesh));
    }

    {   // Add FSQ Mesh
        std::vector<glm::vec3> positions = {
                glm::vec3(-1.0f, -1.0f, 0.0f),
                glm::vec3(1.0f, -1.0f, 0.0f),
                glm::vec3(1.0f, 1.0f, 0.0f),
                glm::vec3(-1.0f, 1.0f, 0.0f),
        };

        std::vector<glm::vec2> texCoords = {
                glm::vec2(0.0f, 0.0f),
                glm::vec2(1.0f, 0.0f),
                glm::vec2(1.0f, 1.0f),
                glm::vec2(0.0f, 1.0f),
        };

        fsqMesh = std::make_shared<GLWrap::Mesh>();
        fsqMesh->setAttribute(0, positions);
        fsqMesh->setAttribute(1, texCoords);
    }

    {   // Add Ocean Mesh
        oceanMesh = std::make_shared<GLWrap::Mesh>();
        oceanMesh->setAttribute(0, oceanScene->mesh.vertices);
        oceanMesh->setAttribute(1, oceanScene->mesh.texCoords);
        oceanMesh->setIndices(oceanScene->mesh.indices, GL_TRIANGLES);
    }
}

void PLApp::setUpNanoguiControls() {
    auto *gui = new nanogui::FormHelper(this);
    gui->add_window(nanogui::Vector2i(10, 10), "Controls");

    gui->add_group("Image");
    gui->add_variable("Exposure", config.exposure)->set_spinnable(true);

    gui->add_group("Point");
    gui->add_variable("Enabled", config.pointLightsEnabled);

    gui->add_variable("Convert area to point", config.convertAreaToPoint);

    auto resolutionX = gui->add_variable("Resolution X", config.shadowMapResolution.x);
    resolutionX->set_spinnable(true);
    resolutionX->set_min_max_values(1, 10000);
    resolutionX->set_value_increment(1000);
    resolutionX->set_callback([&](int x) {
        config.shadowMapResolution.x = x;
        resetFramebuffers();
    });

    auto resolutionY = gui->add_variable("Resolution Y", config.shadowMapResolution.y);
    resolutionY->set_spinnable(true);
    resolutionY->set_min_max_values(1, 10000);
    resolutionY->set_value_increment(1000);
    resolutionY->set_callback([&](int y) {
        config.shadowMapResolution.y = y;
        resetFramebuffers();
    });

    auto bias = gui->add_variable("Bias", config.shadowBias);
    bias->set_spinnable(true);
    bias->set_min_max_values(1e-5, 1);

    auto nearr = gui->add_variable("Near", config.shadowNear);
    nearr->set_spinnable(true);
    nearr->set_min_max_values(1e-5, 1000);

    auto farr = gui->add_variable("Far", config.shadowFar);
    farr->set_spinnable(true);
    farr->set_min_max_values(1e-5, 1000);

    auto fov = gui->add_variable("Fov", config.shadowFov);
    fov->set_spinnable(true);
    fov->set_min_max_values(1e-5, glm::pi<float>());

    gui->add_variable("PCF", config.pcfEnabled);

    gui->add_group("Ambient");
    gui->add_variable("Enabled", config.ambientLightsEnabled);

    auto ssao = gui->add_variable("SSAO Samples", config.ssaoNumSamples);
    ssao->set_spinnable(true);
    ssao->set_min_max_values(0, 100);

    gui->add_group("Sunsky");
    gui->add_variable("Enabled", config.sunskyEnabled);

    auto sunTheta = gui->add_variable("Sun theta", config.thetaSun);
    sunTheta->set_spinnable(true);
    sunTheta->set_min_max_values(0, 2 * glm::pi<float>());

    auto turb = gui->add_variable("Turbidity", config.turbidity);
    turb->set_spinnable(true);
    turb->set_min_max_values(1, 10);

    gui->add_group("Bloom");
    gui->add_variable("Enabled", config.bloomFilterEnabled);

    auto filtMode = gui->add_variable("Filtering Mode", config.textureFilteringMode);
    filtMode->set_items({"Nearest", "Linear"});
    filtMode->set_callback([&](const TextureFilteringMode &m) {
        config.textureFilteringMode = m;
        resetFramebuffers();
    });

    perform_layout();
}

void PLApp::setUpTextures() {
    oceanDisplacementTexture = std::make_shared<GLWrap::Texture2D>(
            oceanScene->gridSize, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT
    );

    oceanGradXTexture = std::make_shared<GLWrap::Texture2D>(
            oceanScene->gridSize, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT
    );

    oceanGradZTexture = std::make_shared<GLWrap::Texture2D>(
            oceanScene->gridSize, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT
    );
}

bool PLApp::keyboard_event(int key, int scancode, int action, int modifiers) {
    if (Screen::keyboard_event(key, scancode, action, modifiers)) {
        return true;
    }

    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                set_visible(false);
                return true;
            case GLFW_KEY_1:
                shadingMode = ShadingMode_Flat;
                //std::cout << "Switched to flat shading" << std::endl;
                return true;
            case GLFW_KEY_2:
                shadingMode = ShadingMode_Forward;
                //std::cout << "Switched to forward shading" << std::endl;
                return true;
            case GLFW_KEY_3:
                shadingMode = ShadingMode_Deferred;
                //std::cout << "Switched to deferred shading" << std::endl;
                return true;
            case GLFW_KEY_SPACE:
                timer.setPlaying(!timer.playing());
                //std::cout << "[ ] Set playback: " << (timer.playing() ? "playing" : "paused") << std::endl;
                return true;
            case GLFW_KEY_COMMA:
                timer.offset(-0.01);
                //std::cout << "[<] Skip backward: 0.01s" << std::endl;
                return true;
            case GLFW_KEY_LEFT:
                timer.offset(-0.1);
                //std::cout << "[←] Skip backward: 0.1s" << std::endl;
                return true;
            case GLFW_KEY_PERIOD:
                timer.offset(0.01);
                //std::cout << "[>] Skip forward: 0.01s" << std::endl;
                return true;
            case GLFW_KEY_RIGHT:
                timer.offset(0.1);
                //std::cout << "[→] Skip forward: 0.1s" << std::endl;
                return true;
            case GLFW_KEY_UP:
                timer.setRate(timer.rate() + 0.25);
                //std::cout << "[↑] Set rate: " << timer.rate() << "x" << std::endl;
                return true;
            case GLFW_KEY_DOWN:
                timer.setRate(timer.rate() - 0.25);
                //std::cout << "[↓] Set rate: " << timer.rate() << "x" << std::endl;
                return true;
            default:
                break;
        }
    }

    return cc->keyboard_event(key, scancode, action, modifiers);
}

bool PLApp::mouse_button_event(const nanogui::Vector2i &p, int button, bool down, int modifiers) {
    return Screen::mouse_button_event(p, button, down, modifiers) || cc->mouse_button_event(p, button, down, modifiers);
}

bool PLApp::mouse_motion_event(const nanogui::Vector2i &p, const nanogui::Vector2i &rel, int button, int modifiers) {
    return Screen::mouse_motion_event(p, rel, button, modifiers) || cc->mouse_motion_event(p, rel, button, modifiers);
}

bool PLApp::scroll_event(const nanogui::Vector2i &p, const nanogui::Vector2f &rel) {
    return Screen::scroll_event(p, rel) || cc->scroll_event(p, rel);
}

bool PLApp::resize_event(const nanogui::Vector2i &size) {
    resetFramebuffers();
    cam->setAspectRatio(((float) size.x()) / ((float) size.y()));
    return Screen::resize_event(size);
}

/*****************************************************************************
 * FLAT SHADING                                                              *
 *****************************************************************************/

void PLApp::draw_contents_flat() {
    glClearColor(backgroundColor.r(), backgroundColor.g(), backgroundColor.b(), backgroundColor.w());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    std::shared_ptr<GLWrap::Program> prog = programFlat;

    prog->use();
    prog->uniform("mV", cam->getViewMatrix());
    prog->uniform("mP", cam->getProjectionMatrix());
    prog->uniform("k_a", glm::vec3(0.1, 0.1, 0.1));
    prog->uniform("k_d", glm::vec3(0.9, 0.9, 0.9));
    prog->uniform("lightDir", glm::normalize(glm::vec3(1.0, 1.0, 1.0)));

    std::vector<std::shared_ptr<Node>> nodes = {scene->root};
    while (!nodes.empty()) {
        std::shared_ptr<Node> node = nodes.back();
        nodes.pop_back();

        for (const std::shared_ptr<Node> &child: node->children) {
            nodes.push_back(child);
        }

        prog->uniform("mM", node->getTransformTo(nullptr));

        for (unsigned int i: node->meshIndices) {
            meshes[i]->drawElements();
        }
    }

    prog->unuse();
}

/*****************************************************************************
 * FORWARD SHADING                                                           *
 *****************************************************************************/

void PLApp::draw_contents_forward() {
    glClearColor(backgroundColor.r(), backgroundColor.g(), backgroundColor.b(), backgroundColor.w());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    PointLight light;
    if (scene->pointLights.empty()) {
        light.nodeToWorld = glm::identity<glm::mat4>();
        light.position = glm::vec3(3, 4, 5);
        light.power = glm::vec3(1000, 1000, 1000);
    } else {
        light = scene->pointLights.front();
    }

    {
        std::shared_ptr<GLWrap::Program> prog = programForward;
        prog->use();

        prog->uniform("mV", cam->getViewMatrix());
        prog->uniform("mP", cam->getProjectionMatrix());

        prog->uniform("lightPower", light.power);
        prog->uniform("vLightPos", MulUtil::mulh(
                cam->getViewMatrix() * light.nodeToWorld,
                light.position,
                1
        ));

        std::vector<std::shared_ptr<Node>> nodes = {scene->root};
        while (!nodes.empty()) {
            std::shared_ptr<Node> node = nodes.back();
            nodes.pop_back();

            for (const std::shared_ptr<Node> &child: node->children) {
                nodes.push_back(child);
            }

            prog->uniform("mM", node->getTransformTo(nullptr));

            for (unsigned int i: node->meshIndices) {
                Mesh mesh = scene->meshes[i];
                Material material = scene->materials[mesh.materialIndex];
                prog->uniform("alpha", material.roughnessFactor);
                prog->uniform("eta", 1.5f);
                prog->uniform("diffuseReflectance", material.color);

                prog->uniform("useBones", !mesh.bones.empty());
                if (!mesh.bones.empty()) {
                    for (int j = 0; j < mesh.bones.size(); j++) {
                        auto bone = mesh.bones[j];
                        std::shared_ptr<Node> boneNode = scene->nameToNode[bone.first];
                        glm::mat4 boneTransform = boneNode->getTransformTo(nullptr) * bone.second;

                        prog->uniform(
                                "boneTransforms[" + std::to_string(j) + "]",
                                boneTransform
                        );
                    }
                }

                meshes[i]->drawElements();
            }
        }

        prog->unuse();
    }

    if (config.ocean) {
        std::shared_ptr<GLWrap::Program> prog = programOceanForward;
        prog->use();

        prog->uniform("mV", cam->getViewMatrix());
        prog->uniform("mP", cam->getProjectionMatrix());
        prog->uniform("mM", oceanScene->transform);

        prog->uniform("lightPower", light.power);
        prog->uniform("vLightPos", MulUtil::mulh(
                cam->getViewMatrix() * light.nodeToWorld,
                light.position,
                1
        ));

        prog->uniform("alpha", 0.5f);
        prog->uniform("eta", 1.5f);
        prog->uniform("diffuseReflectance", glm::vec3(0.2, 0.3, 0.5));

        update_ocean_textures(timer.time());

        oceanDisplacementTexture->bindToTextureUnit(0);
        prog->uniform("displacementMap", 0);
        prog->uniform("displacementA", oceanBuffers.displacementA);
        prog->uniform("displacementB", oceanBuffers.displacementB);

        oceanGradXTexture->bindToTextureUnit(1);
        prog->uniform("gradXMap", 1);
        prog->uniform("gradXA", oceanBuffers.gradXA);
        prog->uniform("gradXB", oceanBuffers.gradXB);

        oceanGradZTexture->bindToTextureUnit(2);
        prog->uniform("gradZMap", 2);
        prog->uniform("gradZA", oceanBuffers.gradZA);
        prog->uniform("gradZB", oceanBuffers.gradZB);

        oceanMesh->drawElements();

        prog->unuse();
    }
}

/*****************************************************************************
 * DEFERRED SHADING                                                          *
 *****************************************************************************/

void PLApp::deferred_texture_pass() {
	std::shared_ptr<GLWrap::Program> prog = programTextureDeferred;
	prog->use();

	prog->uniform("mV", cam->getViewMatrix());
	prog->uniform("mP", cam->getProjectionMatrix());

	//stbi_image_free(textureData);
	std::string path = "../resources/scenes/bsptD.jpg";
	texturemap = std::make_shared<GLWrap::Texture2D> (path, true,true);
	//texturemap->generateMipmap();
	texturemap->bindToTextureUnit(1);
	// Perform a depth-first traversal of the scene graph and draw all the nodes.
	std::vector<std::shared_ptr<Node>> nodes = { scene->root };
	while (!nodes.empty()) {
		std::shared_ptr<Node> node = nodes.back();
		nodes.pop_back();

		for (const std::shared_ptr<Node> &child : node->children) {
			nodes.push_back(child);
		}

		prog->uniform("mM", node->getTransformTo(nullptr));

		for (unsigned int i : node->meshIndices) {
			Mesh mesh = scene->meshes[i];
			Material material = scene->materials[mesh.materialIndex];
			prog->uniform("alpha", material.roughnessFactor);
			prog->uniform("eta", 1.5f);
			prog->uniform("diffuseReflectance", material.color);
			prog->uniform("image", 1);
			prog->uniform("useBones", !mesh.bones.empty());
			if (!mesh.bones.empty()) {
				for (int j = 0; j < mesh.bones.size(); j++) {
					auto bone = mesh.bones[j];
					std::shared_ptr<Node> boneNode = scene->nameToNode[bone.first];
					glm::mat4 boneTransform = boneNode->getTransformTo(nullptr) * bone.second;

					prog->uniform(
						"boneTransforms[" + std::to_string(j) + "]",
						boneTransform
					);
				}
			}

			meshes[i]->drawElements();
		}
	}
	prog->unuse();
}

void PLApp::deferred_geometry_pass() {
    std::shared_ptr<GLWrap::Program> prog = programDeferredGeom;
    prog->use();

    prog->uniform("mV", cam->getViewMatrix());
    prog->uniform("mP", cam->getProjectionMatrix());

    // Perform a depth-first traversal of the scene graph and draw all the nodes.
    std::vector<std::shared_ptr<Node>> nodes = {scene->root};
    while (!nodes.empty()) {
        std::shared_ptr<Node> node = nodes.back();
        nodes.pop_back();

        for (const std::shared_ptr<Node> &child: node->children) {
            nodes.push_back(child);
        }

        prog->uniform("mM", node->getTransformTo(nullptr));

        for (unsigned int i: node->meshIndices) {
            Mesh mesh = scene->meshes[i];
            Material material = scene->materials[mesh.materialIndex];
            prog->uniform("alpha", material.roughnessFactor);
            prog->uniform("eta", 1.5f);
            prog->uniform("diffuseReflectance", material.color);

            prog->uniform("useBones", !mesh.bones.empty());
            if (!mesh.bones.empty()) {
                for (int j = 0; j < mesh.bones.size(); j++) {
                    auto bone = mesh.bones[j];
                    std::shared_ptr<Node> boneNode = scene->nameToNode[bone.first];
                    glm::mat4 boneTransform = boneNode->getTransformTo(nullptr) * bone.second;

                    prog->uniform(
                            "boneTransforms[" + std::to_string(j) + "]",
                            boneTransform
                    );
                }
            }

            meshes[i]->drawElements();
        }
    }

    prog->unuse();
}

void texture_normalize_and_store(
        const std::shared_ptr<GLWrap::Texture2D>& texture,
        float &scale,
        float &offset,
        tessendorf::array2d<float> buffer
) {
    float min = buffer.min();
    float max = buffer.max();
    buffer.plus(-min);
    buffer.times(1 / (max - min));

    scale = max - min;
    offset = min;

    glBindTexture(GL_TEXTURE_2D, texture->id());
    glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_DEPTH_COMPONENT32F,
            buffer.size_x,
            buffer.size_y,
            0,
            GL_DEPTH_COMPONENT,
            GL_FLOAT,
            buffer.data.get()
    );
}

void PLApp::update_ocean_textures(double time) {
    tessendorf::fourier_amplitudes(oceanBuffers.fourierAmplitudes, oceanScene->tessendorfIv, (float) time, oceanScene->config);
    tessendorf::ifft(oceanBuffers.displacementMap, oceanBuffers.fourierAmplitudes, oceanBuffers.buffer, true);
    texture_normalize_and_store(oceanDisplacementTexture, oceanBuffers.displacementA, oceanBuffers.displacementB, oceanBuffers.displacementMap);

    tessendorf::gradient_amplitudes(oceanBuffers.gradientXAmplitudes, oceanBuffers.gradientZAmplitudes, oceanBuffers.fourierAmplitudes, oceanScene->config);

    tessendorf::ifft(oceanBuffers.gradXMap, oceanBuffers.gradientXAmplitudes, oceanBuffers.buffer, false);
    texture_normalize_and_store(oceanGradXTexture, oceanBuffers.gradXA, oceanBuffers.gradXB, oceanBuffers.gradXMap);

    tessendorf::ifft(oceanBuffers.gradZMap, oceanBuffers.gradientZAmplitudes, oceanBuffers.buffer, false);
    texture_normalize_and_store(oceanGradZTexture, oceanBuffers.gradZA, oceanBuffers.gradZB, oceanBuffers.gradZMap);
}

void PLApp::deferred_ocean_geometry_pass() {
    std::shared_ptr<GLWrap::Program> prog = programOceanDeferredGeom;
    prog->use();

    prog->uniform("mV", cam->getViewMatrix());
    prog->uniform("mP", cam->getProjectionMatrix());
    prog->uniform("mM", oceanScene->transform);

    prog->uniform("alpha", 0.5f);
    prog->uniform("eta", 1.5f);
    prog->uniform("diffuseReflectance", glm::vec3(0.2, 0.3, 0.5));

    oceanDisplacementTexture->bindToTextureUnit(0);
    prog->uniform("displacementMap", 0);
    prog->uniform("displacementA", oceanBuffers.displacementA);
    prog->uniform("displacementB", oceanBuffers.displacementB);

    oceanGradXTexture->bindToTextureUnit(1);
    prog->uniform("gradXMap", 1);
    prog->uniform("gradXA", oceanBuffers.gradXA);
    prog->uniform("gradXB", oceanBuffers.gradXB);

    oceanGradZTexture->bindToTextureUnit(2);
    prog->uniform("gradZMap", 2);
    prog->uniform("gradZA", oceanBuffers.gradZA);
    prog->uniform("gradZB", oceanBuffers.gradZB);

    oceanMesh->drawElements();

    prog->unuse();
}

RTUtil::PerspectiveCamera PLApp::get_light_camera(const PointLight &light) const {
    return {
            MulUtil::mulh(light.nodeToWorld, light.position, 1),
            glm::vec3(0, 0, 0),
            glm::vec3(0, 1, 0),
            1,
            config.shadowNear,
            config.shadowFar,
            config.shadowFov
    };
}

void PLApp::deferred_shadow_pass(
        const PointLight &light
) {
    std::shared_ptr<GLWrap::Program> prog = programDeferredShadow;
    prog->use();

    RTUtil::PerspectiveCamera lightCamera = get_light_camera(light);
    prog->uniform("mV", lightCamera.getViewMatrix());
    prog->uniform("mP", lightCamera.getProjectionMatrix());

    std::vector<std::shared_ptr<Node>> nodes = {scene->root};
    while (!nodes.empty()) {
        std::shared_ptr<Node> node = nodes.back();
        nodes.pop_back();

        for (const std::shared_ptr<Node> &child: node->children) {
            nodes.push_back(child);
        }

        prog->uniform("mM", node->getTransformTo(nullptr));

        for (unsigned int i: node->meshIndices) {
            Mesh mesh = scene->meshes[i];

            prog->uniform("useBones", !mesh.bones.empty());
            if (!mesh.bones.empty()) {
                for (int j = 0; j < mesh.bones.size(); j++) {
                    auto bone = mesh.bones[j];
                    std::shared_ptr<Node> boneNode = scene->nameToNode[bone.first];
                    glm::mat4 boneTransform = boneNode->getTransformTo(nullptr) * bone.second;

                    prog->uniform(
                            "boneTransforms[" + std::to_string(j) + "]",
                            boneTransform
                    );
                }
            }
            meshes[i]->drawElements();
        }
    }

    prog->unuse();
}

void PLApp::deferred_ocean_shadow_pass(
        const PointLight &light
) {
    std::shared_ptr<GLWrap::Program> prog = programOceanDeferredShadow;
    prog->use();

    RTUtil::PerspectiveCamera lightCamera = get_light_camera(light);
    prog->uniform("mV", lightCamera.getViewMatrix());
    prog->uniform("mP", lightCamera.getProjectionMatrix());
    prog->uniform("mM", oceanScene->transform);

    oceanDisplacementTexture->bindToTextureUnit(0);
    prog->uniform("displacementMap", 0);
    prog->uniform("displacementA", oceanBuffers.displacementA);
    prog->uniform("displacementB", oceanBuffers.displacementB);

    oceanGradXTexture->bindToTextureUnit(1);
    prog->uniform("gradXMap", 1);
    prog->uniform("gradXA", oceanBuffers.gradXA);
    prog->uniform("gradXB", oceanBuffers.gradXB);

    oceanGradZTexture->bindToTextureUnit(2);
    prog->uniform("gradZMap", 2);
    prog->uniform("gradZA", oceanBuffers.gradZA);
    prog->uniform("gradZB", oceanBuffers.gradZB);

    prog->unuse();
}

glm::ivec2 PLApp::getViewportSize() {
    return {
            framebuffer_size().x(),
            framebuffer_size().y()
    };
}

void PLApp::deferred_lighting_pass(
        const std::shared_ptr<GLWrap::Framebuffer> &geomBuffer,
        const GLWrap::Texture2D &shadowTexture,
        const PointLight &light
) {
    geomBuffer->colorTexture(0).bindToTextureUnit(0);
    geomBuffer->colorTexture(1).bindToTextureUnit(1);
    geomBuffer->colorTexture(2).bindToTextureUnit(2);
    geomBuffer->depthTexture().bindToTextureUnit(3);
    shadowTexture.bindToTextureUnit(4);

    std::shared_ptr<GLWrap::Program> prog = programDeferredPoint;
    prog->use();
    prog->uniform("viewportSize", getViewportSize());
    prog->uniform("mV", cam->getViewMatrix());
    prog->uniform("mP", cam->getProjectionMatrix());
    prog->uniform("shadowBias", config.shadowBias);
    prog->uniform("shadowMapRes", config.shadowMapResolution);
    prog->uniform("pcfEnabled", config.pcfEnabled);
    prog->uniform("diffuseReflectanceTex", 0);
    prog->uniform("materialTex", 1);
    prog->uniform("normalsTex", 2);
    prog->uniform("depthTex", 3);
    prog->uniform("shadowTex", 4);

    RTUtil::PerspectiveCamera lightCamera = get_light_camera(light);
    prog->uniform("mV_light", lightCamera.getViewMatrix());
    prog->uniform("mP_light", lightCamera.getProjectionMatrix());
    prog->uniform("lightPower", light.power);
    prog->uniform("vLightPos", MulUtil::mulh(
            cam->getViewMatrix() * light.nodeToWorld,
            light.position,
            1
    ));

    fsqMesh->drawArrays(GL_TRIANGLE_FAN, 0, 4);
    prog->unuse();
}

void PLApp::deferred_draw_pass(const std::shared_ptr<GLWrap::Framebuffer> &accBuffer) {
    accBuffer->colorTexture(0).bindToTextureUnit(0);

    std::shared_ptr<GLWrap::Program> prog = programSrgb;
    prog->use();
    prog->uniform("image", 0);
    prog->uniform("exposure", config.exposure);

    fsqMesh->drawArrays(GL_TRIANGLE_FAN, 0, 4);
    prog->unuse();
}

void PLApp::deferred_ambient_pass(
        const std::shared_ptr<GLWrap::Framebuffer> &geomBuffer,
        const AmbientLight &light
) {
    geomBuffer->colorTexture(0).bindToTextureUnit(0);
    geomBuffer->colorTexture(1).bindToTextureUnit(1);
    geomBuffer->colorTexture(2).bindToTextureUnit(2);
    geomBuffer->depthTexture().bindToTextureUnit(3);

    std::shared_ptr<GLWrap::Program> prog = programDeferredAmbient;
    prog->use();

    // Set uniforms in deferred_shader_inputs.fs
    prog->uniform("mP", cam->getProjectionMatrix());
    prog->uniform("viewportSize", getViewportSize());
    prog->uniform("diffuseReflectanceTex", 0);
    prog->uniform("materialTex", 1);
    prog->uniform("normalsTex", 2);
    prog->uniform("depthTex", 3);

    // Set uniforms in deferred_ambient.fs
    prog->uniform("ambientRadiance", light.radiance);
    prog->uniform("ambientOcclusionRange", light.distance);
    prog->uniform("numSamples", config.ssaoNumSamples);

    fsqMesh->drawArrays(GL_TRIANGLE_FAN, 0, 4);
    prog->unuse();
}

void PLApp::deferred_sky_pass(
        const GLWrap::Texture2D &image
) {
    image.bindToTextureUnit(0);

    std::shared_ptr<GLWrap::Program> prog = programDeferredSky;
    prog->use();

    // Bind uniforms in sunsky.fs
    RTUtil::Sky sky(config.thetaSun, config.turbidity);
    sky.setUniforms(*prog);

    // Bind uniforms in deferred_sky.fs
    prog->uniform("image", 0);
    prog->uniform("mP", cam->getProjectionMatrix());
    prog->uniform("mV", cam->getViewMatrix());

    fsqMesh->drawArrays(GL_TRIANGLE_FAN, 0, 4);
    prog->unuse();
}

void PLApp::deferred_blur_pass(
        const GLWrap::Texture2D &image,
        glm::vec2 dir,
        float stdev,
        int level
) {
    image.bindToTextureUnit(0);

    std::shared_ptr<GLWrap::Program> prog = programDeferredBlur;
    prog->use();

    float stdevAtLevel = stdev / (1 << level);

    // Bind uniforms in blur.fs
    prog->uniform("image", 0);
    prog->uniform("dir", dir);
    prog->uniform("stdev", stdevAtLevel);
    prog->uniform("radius", (int) (3 * stdevAtLevel + 1));
    prog->uniform("level", level);

    fsqMesh->drawArrays(GL_TRIANGLE_FAN, 0, 4);
    prog->unuse();
}

void PLApp::deferred_merge_pass(
        const GLWrap::Texture2D &image,
        const GLWrap::Texture2D &blurred
) {
    image.bindToTextureUnit(0);
    blurred.bindToTextureUnit(1);

    std::shared_ptr<GLWrap::Program> prog = programDeferredMerge;
    prog->use();

    prog->uniform("image", 0);
    prog->uniform("blurred", 1);

    fsqMesh->drawArrays(GL_TRIANGLE_FAN, 0, 4);
    prog->unuse();
}

void PLApp::draw_contents_deferred() {
    if (config.ocean) {
        update_ocean_textures(timer.time());
    }

    geomBuffer->bind();
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    const GLenum buffers[] = {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2,
    };
    glDrawBuffers(3, buffers);
    deferred_texture_pass();
    if (config.ocean) {
        deferred_ocean_geometry_pass();
    }
    geomBuffer->unbind();

    accBuffer->bind();
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    accBuffer->unbind();

    if (config.pointLightsEnabled) {
        std::vector<PointLight> lights;
        for (const PointLight &light: scene->pointLights) {
            lights.push_back(light);
        }

        if (config.convertAreaToPoint) {
            for (const AreaLight &light: scene->areaLights) {
                PointLight p;
                p.position = light.center;
                p.nodeToWorld = light.nodeToWorld;
                p.power = light.power;
                lights.push_back(p);
            }
        }

        for (const PointLight &light: lights) {
            shadowMap->bind();
            glClear(GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
            glViewport(0, 0, config.shadowMapResolution.x, config.shadowMapResolution.y);
            deferred_shadow_pass(light);
            if (config.ocean) {
                deferred_ocean_shadow_pass(light);
            }
            shadowMap->unbind();

            accBuffer->bind();
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            glViewport(0, 0, getViewportSize().x, getViewportSize().y);
            deferred_lighting_pass(geomBuffer, shadowMap->depthTexture(), light);
            glDisable(GL_BLEND);
            accBuffer->unbind();
        }
    }

    if (config.ambientLightsEnabled) {
        for (const AmbientLight &light: scene->ambientLights) {
            accBuffer->bind();
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            glViewport(0, 0, getViewportSize().x, getViewportSize().y);
            deferred_ambient_pass(geomBuffer, light);
            glDisable(GL_BLEND);
            accBuffer->unbind();
        }
    }

    if (config.sunskyEnabled) {
        // Use temp1 to hold the result of the sky pass

        temp1->bind();
        glViewport(0, 0, getViewportSize().x, getViewportSize().y);
        deferred_sky_pass(accBuffer->colorTexture());
        temp1->unbind();

        // Swap temp1 and accBuffer
        std::swap(accBuffer, temp1);
    }

    if (config.bloomFilterEnabled) {
        // Use temp1 to hold blur in the x direction
        // Use temp2 to hold blur in both directions

        accBuffer->colorTexture().generateMipmap();

        for (const std::pair<float, int> &blurLevel: blurLevels) {
            float stdev = blurLevel.first;
            int level = blurLevel.second;

            glViewport(0, 0, getViewportSize().x / (1 << level), getViewportSize().y / (1 << level));

            temp1->bind(level);
            glClear(GL_COLOR_BUFFER_BIT);
            deferred_blur_pass(accBuffer->colorTexture(), {1.0, 0.0}, stdev, level);
            temp1->unbind();

            temp2->bind(level);
            glClear(GL_COLOR_BUFFER_BIT);
            deferred_blur_pass(temp1->colorTexture(), {0.0, 1.0}, stdev, level);
            temp2->unbind();
        }

        // Use temp1 to hold the temporary result of merging the accBuffer and the full blur (temp2)
        temp1->bind();
        glViewport(0, 0, getViewportSize().x, getViewportSize().y);
        glClear(GL_COLOR_BUFFER_BIT);
        deferred_merge_pass(accBuffer->colorTexture(), temp2->colorTexture());
        temp1->unbind();

        // Swap temp1 and accBuffer
        std::swap(accBuffer, temp1);
    }

    glViewport(0, 0, getViewportSize().x, getViewportSize().y);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    deferred_draw_pass(accBuffer);
}

void PLApp::animate_birds() {
    /* returns the vector we need to add to the position of the current boid to move it
     * 1% of the way to the center of mass of its neighbors
     */
    const auto center_of_mass = [&](const size_t currBoid) -> glm::vec3 {
        glm::vec3 avgPosition(0);
        for (size_t currNeighbor = 0; currNeighbor < this->birds.size(); currNeighbor++) {
            if (currBoid != currNeighbor) avgPosition += this->birds[currNeighbor].position;
        }
        avgPosition /= (this->birds.size() - 1);
        return (avgPosition - this->birds[currBoid].position) / 5000.f;
    };

    /* returns the vector we need to add to the position of the current boid to prevent collision with
     * other boids
     */
    const auto course_correction = [&](const size_t currBoid) -> glm::vec3 {
        glm::vec3 correctionAmt(0);
        for (size_t currNeighbor = 0; currNeighbor < this->birds.size(); currNeighbor++) {
            if (currBoid != currNeighbor) {
                float distance = glm::distance(this->birds[currBoid].position, this->birds[currNeighbor].position);
                if (distance <= 0.25f) {
                    correctionAmt -=
                            (this->birds[currNeighbor].position - this->birds[currBoid].position);
                }
            }
        }

        // if a bird is going to go out of bounds, make it turn hard
        if (glm::dot(Bird::walls[0].normal, this->birds[currBoid].position - Bird::walls[0].point) <= 1.0)
            correctionAmt.x -= 0.002;
        if (glm::dot(Bird::walls[1].normal, this->birds[currBoid].position - Bird::walls[1].point) <= 1.0)
            correctionAmt.x += 0.002;
        if (glm::dot(Bird::walls[2].normal, this->birds[currBoid].position - Bird::walls[2].point) <= 1.0)
            correctionAmt.z -= 0.002;
        if (glm::dot(Bird::walls[3].normal, this->birds[currBoid].position - Bird::walls[3].point) <= 1.0)
            correctionAmt.z += 0.002;

        return correctionAmt;
    };

    /* returns the vector that we need to add to the velocity of the current boid to get the new velocity */
    const auto center_of_velocity = [&](const size_t currBoid) -> glm::vec3 {
        glm::vec3 avgVelocity(0);
        for (size_t currNeighbor = 0; currNeighbor < this->birds.size(); currNeighbor++) {
            if (currBoid != currNeighbor) avgVelocity += this->birds[currNeighbor].velocity;
        }
        avgVelocity /= (this->birds.size() - 1);
        return (avgVelocity - this->birds[currBoid].velocity) / 800.f;
    };

    for (size_t currBoid = 0; currBoid < this->birds.size(); currBoid++) {
        glm::vec3 deltaV = center_of_mass(currBoid) +
                           course_correction(currBoid)  +
                           center_of_velocity(currBoid);
        this->birds[currBoid].velocity += deltaV;
        this->birds[currBoid].position += this->birds[currBoid].velocity;
        this->birds[currBoid].update_self(deltaV);
    }
}

void PLApp::draw_contents() {
    GLWrap::checkGLError("drawContents start");

    scene->animate(timer.time());
    if (config.birds) {
        animate_birds();
    }

    switch (shadingMode) {
        case ShadingMode_Flat:
            draw_contents_flat();
            break;
        case ShadingMode_Forward:
            draw_contents_forward();
            break;
        case ShadingMode_Deferred:
            draw_contents_deferred();
            break;
    }

    GLWrap::checkGLError("drawContents end");
}
