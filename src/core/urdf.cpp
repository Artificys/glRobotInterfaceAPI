#include "urdf.h"

std::pair<std::vector<URDF_Link>, std::vector<URDF_Joint>> parseURDF(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << "\n";
        return {};
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    std::string xmlData = buffer.str();

    rapidxml::xml_document<> doc;

    try {
        doc.parse<0>(&xmlData[0]);
    } catch (rapidxml::parse_error& e) {
        std::cerr << "XML parse error: " << e.what() << "\n";
        return {};
    }

    std::vector<URDF_Link> links;
    std::vector<URDF_Joint> joints;

    rapidxml::xml_node<>* robot = doc.first_node("robot");

    // Iterate links
    for (auto& link : XMLIterator(robot, "link")) {
        std::string name = link.first_attribute("name")->value();
        std::string filename;
        std::string rgba = "0.8 0.8 0.8 1.0";

        rapidxml::xml_node<>* visual = link.first_node("visual");
        if (visual) {
            rapidxml::xml_node<>* geometry = visual->first_node("geometry");
            if (geometry) {
                rapidxml::xml_node<>* mesh = geometry->first_node("mesh");
                if (mesh) {
                    if (rapidxml::xml_attribute<>* attr = mesh->first_attribute("filename"))
                        filename = attr->value();
                    // Remove leading "../" if present
                    while (filename.substr(0, 3) == "../") {
                        filename = filename.substr(3);
                    }
                }
            }

            rapidxml::xml_node<>* material = visual->first_node("material");
            if (material) {
                rapidxml::xml_node<>* color = material->first_node("color");
                if (color) {
                    if (rapidxml::xml_attribute<>* attr = color->first_attribute("rgba"))
                        rgba = attr->value();
                }
            }
        }

        links.push_back({ name, filename, rgba });
    }

    // Iterate joints
    for (auto& joint : XMLIterator(robot, "joint")) {
        std::string name = joint.first_attribute("name")->value();
        std::string type = joint.first_attribute("type")->value();
        std::string parent, child, xyz = "0 0 0", rpy = "0 0 0", axis = "1 0 0";
        std::string lower, upper, velocity;

        rapidxml::xml_node<>* parentNode = joint.first_node("parent");
        if (parentNode && parentNode->first_attribute("link"))
            parent = parentNode->first_attribute("link")->value();

        rapidxml::xml_node<>* childNode = joint.first_node("child");
        if (childNode && childNode->first_attribute("link"))
            child = childNode->first_attribute("link")->value();

        rapidxml::xml_node<>* origin = joint.first_node("origin");
        if (origin) {
            if (rapidxml::xml_attribute<>* attr = origin->first_attribute("xyz"))
                xyz = attr->value();
            if (rapidxml::xml_attribute<>* attr = origin->first_attribute("rpy"))
                rpy = attr->value();
        }

        rapidxml::xml_node<>* limitNode = joint.first_node("limit");
        if (limitNode) {
            if (rapidxml::xml_attribute<>* attr = limitNode->first_attribute("lower"))
                lower = attr->value();
            if (rapidxml::xml_attribute<>* attr = limitNode->first_attribute("upper"))
                upper = attr->value();
            if (rapidxml::xml_attribute<>* attr = limitNode->first_attribute("velocity"))
                velocity = attr->value();
        }

        rapidxml::xml_node<>* axisNode = joint.first_node("axis");
        if (axisNode && axisNode->first_attribute("xyz"))
            axis = axisNode->first_attribute("xyz")->value();

        joints.push_back({ name, type, parent, child, xyz, rpy, axis, lower, upper });
    }

    return { links, joints };
}

// Helper functions for model.h to parse strings into glm vectors

glm::vec3 parseVec3(const std::string& str)
{
    glm::vec3 vec;
    std::istringstream iss(str);
    iss >> vec.x >> vec.y >> vec.z;
    return vec;
}

glm::vec4 parseVec4(const std::string& str)
{
    glm::vec4 vec;
    std::istringstream iss(str);
    iss >> vec.r >> vec.g >> vec.b >> vec.a;
    return vec;
}