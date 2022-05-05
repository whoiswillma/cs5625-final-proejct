#ifndef CS5625_IMPORT_H
#define CS5625_IMPORT_H

#include "Scene.h"
#include <assimp/scene.h>

std::shared_ptr<Scene> importScene(const aiScene* aiScene);

#endif //CS5625_IMPORT_H
