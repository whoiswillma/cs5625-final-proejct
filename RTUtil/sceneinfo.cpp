/*
 * Cornell CS5625
 * RTUtil library
 * 
 * Implementation of scene information parser.  
 * 
 * Author: srm, Spring 2020
 */

#include <fstream>

#include "sceneinfo.hpp"
#include "microfacet.hpp"
#include "json.hpp"
using json = nlohmann::json;

using std::shared_ptr;
using std::make_shared;

namespace RTUtil {

// Function to convert JSON arrays easily to GLM vectors

class json_conversion_error : public nlohmann::detail::exception {
  public:
    static json_conversion_error create(const std::string& what_arg)
    {
        std::string w = nlohmann::detail::exception::name("json_conversion_error", 502) + what_arg;
        return json_conversion_error(502, w.c_str());
    }

  private:
    json_conversion_error(int id_, const char* what_arg)
        : nlohmann::detail::exception(id_, what_arg)
    {}
};

template<class T, int n>
void from_json(const nlohmann::json &j, glm::vec<n, T>& val) {
    if (!j.is_array())
        throw json_conversion_error::create("Attempt to convert non-array JSON value to a cpp array");
    if (j.size() != n)
        throw json_conversion_error::create("Attempt to convert a JSON array to a cpp array of non-matching length");
    for (int i = 0; i < n; i++)
        val[i] = j.at(i);
}

void readLightInfo(const json &sceneInfo, SceneInfo &info) {
    bool backgroundSet = false;
    for (const auto &lightInfo : sceneInfo["lights"]) {
        std::string nodeName = lightInfo["node"];
        LightType type;
        glm::vec3 position, power;
        glm::vec3 normal, up;
        glm::vec2 size;
        glm::vec3 radiance;
        float range = std::numeric_limits<float>::infinity();
        if (lightInfo["type"] == "point") {
            type = LightType::Point;
            from_json(lightInfo["position"], position);
            from_json(lightInfo["power"], power);
        } else if (lightInfo["type"] == "area") {
            type = LightType::Area;
            from_json(lightInfo["position"], position);
            from_json(lightInfo["normal"], normal);
            from_json(lightInfo["up"], up);
            from_json(lightInfo["size"], size);
            from_json(lightInfo["power"], power);
        } else if (lightInfo["type"] == "ambient") {
            type = LightType::Ambient;
            from_json(lightInfo["radiance"], radiance);
            if (lightInfo.find("range") != lightInfo.end()) {
                range = lightInfo["range"];
            }
            if (!backgroundSet) {
                info.backgroundRadiance = radiance;
                backgroundSet = true;
            }
        } else {
            std::cerr << "warning: unkonwn light type '" << lightInfo["type"] << "' encountered; ignored." << std::endl;
            continue;
        }
        // LightInfo foo(std::initializer_list<LightInfo> { nodeName, type, power, position, normal, up, size, radiance, range });
        info.lights.push_back(make_shared<LightInfo>(
            LightInfo { nodeName, type, power, position, normal, up, size, radiance, range }
            ));
    }
}

bool readSceneInfo(const std::string infoPath, SceneInfo &info) {
    info.backgroundRadiance = glm::vec3(0.0);
    
    /*
     * New plan: lights specified in separate input, add them using existing nodes
     */
    json sceneInfo;
    try {
        std::ifstream infi(infoPath);
        if (infi.fail()) {
            std::cerr << "Unable to open scene information file " << infoPath << std::endl;
            return false;
        }
        infi >> sceneInfo;
    } catch (json::parse_error &e) {
        std::cerr << "Error parsing scene information file: " << e.what() << std::endl;
        return false;
    }

    readLightInfo(sceneInfo, info);

    return true;   
}

}
