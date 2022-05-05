#ifndef CS5625_IMPORT_H
#define CS5625_IMPORT_H

#include "Scene.h"
#include <assimp/scene.h>

void importScene(const std::shared_ptr<Scene>& scene, const aiScene* aiScene);

#endif //CS5625_IMPORT_H
