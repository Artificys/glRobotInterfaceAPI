#ifndef URDF_H
#define URDF_H

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "rapidxml.hpp"

struct URDF_Link {
    std::string name;
    std::string filename;
    std::string rgba;
};

struct URDF_Joint {
    std::string name;
    std::string type;
    std::string parent;
    std::string child;
    std::string xyz;
    std::string rpy;
    std::string axis;
    std::string lower;
    std::string upper;
    std::string velocity;
};

class XMLIterator {
public:
    rapidxml::xml_node<>* first;
    const char* childName;

    class Iterator {
    public:
        explicit Iterator(rapidxml::xml_node<>* _n = nullptr)
            : ptr(_n)
        {
        }
        rapidxml::xml_node<>& operator*() const { return *ptr; }
        rapidxml::xml_node<>* operator->() const { return ptr; }

        Iterator& operator++()
        {
            if (ptr)
                ptr = ptr->next_sibling(ptr->name());

            return *this;
        }

        bool operator!=(const Iterator& other) const { return ptr != other.ptr; }

    private:
        rapidxml::xml_node<>* ptr;
    };

    XMLIterator(rapidxml::xml_node<>* parent, const char* child_name)
        : first(parent ? parent->first_node(child_name) : nullptr)
        , childName(child_name)
    {
    }

    Iterator begin() { return Iterator(first); }
    Iterator end() { return Iterator(nullptr); }
};

std::pair<std::vector<URDF_Link>, std::vector<URDF_Joint>> parseURDF(const std::string& filePath);

// Helper functions for model.h to parse strings into glm vectors
glm::vec3 parseVec3(const std::string& str);
glm::vec4 parseVec4(const std::string& str);

#endif // URDF_H