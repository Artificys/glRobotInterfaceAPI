#ifndef MODEL_H
#define MODEL_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/string_cast.hpp>

#include <map>
#include <string>
#include <vector>

#include "mesh.h"
#include "urdf.h"

using namespace std;

struct Link {
    string name;
    Mesh* mesh;
    vector<struct Joint*> children; // Joints with child links
    struct Joint* parentJoint = nullptr; // Joint with parent link
    glm::mat4 transform = glm::mat4(1.0f); // World transform
};

struct Joint {
    string name;
    string type; // revolute | prismatic | fixed | continuous
    glm::vec3 axis;
    glm::vec3 xyz;
    glm::vec3 rpy; // rot @ origin
    Link* parentLink;
    Link* childLink;

    float targetPosition = 0.0f;
    float currentPosition = 0.0f; // interpolated from time & target for smooth rendering
    float velocity = 5.0f;
    float lowerLimit = 0.0f;
    float upperLimit = 0.0f;
};

/**
 * @class Model
 * @brief Represents a 3D articulated model loaded from a URDF file.
 *
 * The Model class provides functionality to load, render, and update the state of a robot or articulated structure
 * defined by a URDF file. It manages the hierarchical structure of links and joints as a directed acyclic graph (DAG),
 * supports rendering with OpenGL, and allows for transformation updates and bounding box queries.
 */
class Model {
public:
    Model(const char* urdfPath) : urdfPath(urdfPath) {}
    ~Model();

    bool load();
    void render(GLuint shaderProg); // Rendering from root
    void updateTransforms(float dt); // Update world matrix calcs

    pair<glm::vec3, glm::vec3> getBoundingBox() const;
    vector<string> getJointNames() const;
    string getRootName() const { return root ? root->name : ""; }

    bool updatePosition(const string& jointName, float newPosition);

private:
    const string urdfPath;

    // Use a DAG to represent the model
    map<string, Link*> links;
    map<string, Joint*> joints;
    Link* root = nullptr;

    void render(Link* link, GLuint shaderProg);

    glm::mat4 transformOrigin(const glm::vec3& xyz, const glm::vec3& rpy);
    glm::mat4 transformJointMotion(const Joint* joint);

    void updateLinkTransforms(Link* link, const glm::mat4& parentTransform);
};

#endif // MODEL_H