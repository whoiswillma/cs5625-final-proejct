#include <regex>

#include "conversions.hpp"

namespace RTUtil {

bool parseAreaLight(const std::string &name, float &width, float &height) {
    std::regex area_regex("^\\w+_w([+-]?([0-9]*[.])?[0-9]+)_h([+-]?([0-9]*[.])?[0-9]+)_.*$");
    std::smatch match;
    if (std::regex_match(name, match, area_regex)) {
        width = std::stof(match[1]);
        height = std::stof(match[3]);
        return true;
    }
    return false;
}


bool parseAmbientLight(const std::string &name, float &range) {
    std::regex ambient_regex("^[Aa]mbient_r([+-]?([0-9]*[.])?[0-9]+)_.*$");
    std::smatch match;
    if (std::regex_match(name, match, ambient_regex)) {
        range = std::stof(match[1]);
        return true;
    }
    return false;
}


} // namespae RTUtil