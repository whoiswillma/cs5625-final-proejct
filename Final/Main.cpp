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
    );
    if (aiScene == nullptr) {
        std::cerr << "error: " << importer.GetErrorString() << std::endl;
        exit(1);
    }

    return importScene(aiScene);
}

int main(int argc, char **argv) {
    std::shared_ptr<Scene> scene(new Scene());
    std::shared_ptr<OceanScene> ocean(new OceanScene());

    PLAppConfig config;
    for (int i = 1; i < argc; i++) {
        if (strcmp("--ocean", argv[i]) == 0) {
            config.ocean = true;
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
            PointLight light;
            light.nodeToWorld = glm::identity<glm::mat4>();
            light.position = glm::vec3(3, 4, 5);
            light.power = glm::vec3(1000, 1000, 1000);
            scene->pointLights.push_back(light);
            continue;
        }

        std::string arg(argv[i]);
        std::smatch match;

        if (std::regex_match(arg, match, std::regex("^--scene=(.+)$"))) {
            scene = importFile(match[1]);
            continue;
        }

        std::cout << "Unable to parse argument: \"" << argv[i] << "\"" << std::endl;
        exit(1);
    }

    if (scene->root == nullptr) {
        scene->root = std::make_shared<Node>();
    }

    if (scene->camera == nullptr) {
        scene->camera = std::make_shared<RTUtil::PerspectiveCamera>(
                glm::vec3(3, 4, 5),
                glm::vec3(0, 0, 0),
                glm::vec3(0, 1, 0),
                1.0f,
                0.1f,
                1000.0f,
                glm::pi<float>() / 6
        );
    }

    nanogui::init();

    nanogui::ref<PLApp> app = new PLApp(scene, 700, config);
    nanogui::mainloop(16);
}
