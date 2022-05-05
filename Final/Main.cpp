#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <regex>

#include "Import.h"
#include "PLApp.h"

void importOcean(const std::shared_ptr<Scene>& scene) {
    std::shared_ptr<Node> oceanNode = std::make_shared<Node>(
            glm::identity<glm::mat4>(),
            std::vector<unsigned int>({MESH_IDX_OCEAN}),
            std::vector<std::shared_ptr<Node>>(),
            scene->root
    );
    scene->root->children.push_back(oceanNode);
}

void importFile(const std::shared_ptr<Scene>& scene, const std::string& filename) {
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
        printf("error: %s", importer.GetErrorString());
        exit(1);
    }

    importScene(scene, aiScene);
}

int main(int argc, char **argv) {
    std::shared_ptr<Scene> scene(new Scene());

    PLAppConfig config;
    for (int i = 1; i < argc; i++) {
        if (strcmp("--ocean", argv[i]) == 0) {
            importOcean(scene);
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

        std::string arg(argv[i]);
        std::smatch match;

        if (std::regex_match(arg, match, std::regex("^--scene=(.+)$"))) {
            importFile(scene, match[1]);
            continue;
        }

        printf("Unable to parse argument: \"%s\"", argv[i]);
        exit(1);
    }

    nanogui::init();

    nanogui::ref<PLApp> app = new PLApp(scene, 700, config);
    nanogui::mainloop(16);
}
