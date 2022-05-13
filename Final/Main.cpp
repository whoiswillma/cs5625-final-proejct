#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <regex>

#include "Import.h"
#include "PLApp.h"
#include "OceanScene.h"

std::shared_ptr<Scene> importFile(const std::string& filename) {
    Assimp::Importer importer;
    const aiScene* aiScene = importer.ReadFile(
            filename,
            aiProcess_CalcTangentSpace
            | aiProcess_Triangulate
            | aiProcess_JoinIdenticalVertices
            | aiProcess_SortByPType
            | aiProcess_GenNormals
            | aiProcess_LimitBoneWeights
    );
    if (aiScene == nullptr) {
        std::cerr << "error: " << importer.GetErrorString() << std::endl;
        exit(1);
    }

    return importScene(aiScene);
}

const std::regex SCENE_ARG_REGEX("^--scene=(.+)$");

int main(int argc, char **argv) {
    std::shared_ptr<Scene> scene = std::make_shared<Scene>();
    scene->root = std::make_shared<Node>();
    scene->camera = std::make_shared<RTUtil::PerspectiveCamera>(
            glm::vec3(3, 4, 5),
            glm::vec3(0, 0, 0),
            glm::vec3(0, 1, 0),
            1.0f,
            0.1f,
            1000.0f,
            glm::pi<float>() / 6
    );

    std::shared_ptr<OceanScene> ocean = std::make_shared<OceanScene>(
            glm::vec2(32, 32),
            glm::vec2(256, 256)
    );

    PLAppConfig config;
    for (int i = 1; i < argc; i++) {
        if (strcmp("--ocean", argv[i]) == 0) {
            config.ocean = true;
            continue;
        }

        if (strcmp("--birds", argv[i]) == 0) {
            config.birds = true;
            continue;
        }

        if (strcmp("--no-point", argv[i]) == 0) {
            config.pointLightsEnabled = false;
            continue;
        }

        if (strcmp("--no-ambient", argv[i]) == 0) {
            config.ambientLightsEnabled = false;
            continue;
        }

        if (strcmp("--no-sunsky", argv[i]) == 0) {
            config.sunskyEnabled = false;
            continue;
        }

        if (strcmp("--no-bloom", argv[i]) == 0) {
            config.bloomFilterEnabled = false;
            continue;
        }

        if (strcmp("--add-default-light", argv[i]) == 0) {
            {
                PointLight light;
                light.nodeToWorld = glm::identity<glm::mat4>();
                light.position = {3, 4, 5};
                light.power = {1000, 1000, 1000};
                scene->pointLights.push_back(light);
            }

            {
                AmbientLight light;
                light.radiance = {0.5, 0.5, 0.5};
                light.distance = 0.2;
                scene->ambientLights.push_back(light);
            }
            continue;
        }

        std::string arg(argv[i]);
        std::smatch match;

        if (std::regex_match(arg, match, SCENE_ARG_REGEX)) {
            if (i != 1) {
                std::cerr << "--scene must be the first argument" << std::endl;
                exit(1);
            }

            scene = importFile(match[1]);
            continue;
        }

        std::cerr << "Unable to parse argument: \"" << argv[i] << "\"" << std::endl;
        exit(1);
    }

    nanogui::init();

    nanogui::ref<PLApp> app = new PLApp(scene, ocean, 700, config);
    nanogui::mainloop(16);
}
