#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <regex>

#include "Import.h"
#include "PLApp.h"

int main(int argc, char **argv) {
    if (argc <= 1) {
        std::cerr << "Usage: " << argv[0] << " [scene]" << std::endl;
        return 1;
    }

    std::string filename = std::string(argv[1]);

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

    std::shared_ptr<Scene> scene = importScene(aiScene);

    PLAppConfig config;
    for (int i = 2; i < argc; i++) {
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

        printf("Unable to parse argument: \"%s\"", argv[i]);
        exit(1);
    }

    nanogui::init();

    nanogui::ref<PLApp> app = new PLApp(scene, 700, config);
    nanogui::mainloop(16);
}
