#include "model.h"

Model::~Model()
{
    for (auto& linkPair : links) {
        delete linkPair.second->mesh;
        delete linkPair.second;
    }
    for (auto& jointPair : joints) {
        delete jointPair.second;
    }
}

bool Model::load()
{
    std::pair<std::vector<URDF_Link>, std::vector<URDF_Joint>> urdf = parseURDF(urdfPath);

    if (urdf.first.empty() && urdf.second.empty()) {
        std::cerr << "URDF parsing failed\n";
        return false;
    }

    // Process links and joints
    for (const auto& link : urdf.first) {
        Link* newLink = new Link();

        newLink->name = link.name;
        try {
            newLink->mesh = new Mesh(link.filename);
        } catch (const std::exception& e) {
            std::cerr << "Error loading mesh for link " << link.name << ": " << e.what() << "\n";
            delete newLink;
            continue;
        }

        newLink->mesh->color = parseVec3(link.rgba);

        links[link.name] = newLink;
    }

    for (const auto& joint : urdf.second) {
        Joint* newJoint = new Joint();
        newJoint->name = joint.name;
        newJoint->type = joint.type;
        newJoint->axis = parseVec3(joint.axis);
        newJoint->xyz = parseVec3(joint.xyz);
        newJoint->rpy = parseVec3(joint.rpy);
        if (!joint.lower.empty())
            newJoint->lowerLimit = std::stof(joint.lower);
        if (!joint.upper.empty())
            newJoint->upperLimit = std::stof(joint.upper);
        if (!joint.velocity.empty())
            newJoint->velocity = std::stof(joint.velocity);

        auto parentIt = links.find(joint.parent);
        auto childIt = links.find(joint.child);
        if (parentIt != links.end() && childIt != links.end()) {
            newJoint->parentLink = parentIt->second;
            newJoint->childLink = childIt->second;
            childIt->second->parentJoint = newJoint;
            parentIt->second->children.push_back(newJoint);
            joints[joint.name] = newJoint;
        } else {
            std::cerr << "Invalid joint link references in joint: " << joint.name << "\n";
            delete newJoint;
            continue;
        }
    }

    // find root
    for (const auto& link : links) {
        if (link.second->parentJoint == nullptr) {
            root = link.second;
            break;
        }
    }

    return true;
}

void Model::updateTransforms(float dt)
{
    // Note: this is only relevant if position is interpolated
    for (auto& pair : joints) {
        Joint* joint = pair.second;

        float diff = joint->targetPosition - joint->currentPosition;
        joint->currentPosition += diff * std::min(dt * joint->velocity, 1.0f);
    }
    
    if (root) {
        updateLinkTransforms(root, root->transform);
    }
}

void Model::updateLinkTransforms(Link* link, const glm::mat4& parentTransform)
{
    if (!link)
        return;

    if (link->parentJoint) {
        Joint* parentJoint = link->parentJoint;
        glm::mat4 T_origin = transformOrigin(parentJoint->xyz, parentJoint->rpy);
        glm::mat4 T_motion = transformJointMotion(parentJoint);

        link->transform = parentTransform * T_origin * T_motion;
    } else {
        link->transform = parentTransform; // root link
    }

    // recurse to branches
    for (const auto& childJoint : link->children) {
        Link* childLink = childJoint->childLink;
        if (childLink) {
            updateLinkTransforms(childLink, link->transform);
        }
    }
}

// URDF files require Radian measurements, so while the value will be slightly off, it is required.
glm::mat4 Model::transformOrigin(const glm::vec3& xyz, const glm::vec3& rpy)
{
    glm::mat4 T = glm::mat4(1.0f);
    T = glm::translate(T, xyz); // Translate first to rotate around the local origin
    T = glm::rotate(T, rpy.z, glm::vec3(0, 0, 1));
    T = glm::rotate(T, rpy.y, glm::vec3(0, 1, 0)); // NOTE TO SELF: glm::radians CONVERTS to radians. If it is already in radians, you will mess it up...
    T = glm::rotate(T, rpy.x, glm::vec3(1, 0, 0));
    return T;
}

glm::mat4 Model::transformJointMotion(const Joint* joint)
{
    float pos = joint->currentPosition; // use currentPosition for rendering

    if (joint->type == "revolute" || joint->type == "continuous") {
        return glm::rotate(glm::mat4(1.0f), pos, joint->axis);
    } else if (joint->type == "prismatic") {
        return glm::translate(glm::mat4(1.0f), pos * joint->axis);
    }

    return glm::mat4(1.0f);
}

void Model::render(GLuint shaderProg)
{
    if (root) {
        render(root, shaderProg);
    }
}

void Model::render(Link* link, GLuint shaderProg)
{
    if (!link->mesh)
        return;

    GLint locM = glGetUniformLocation(shaderProg, "uModel");
    GLint locCol = glGetUniformLocation(shaderProg, "uColor");

    glUniformMatrix4fv(locM, 1, GL_FALSE, glm::value_ptr(link->transform));
    glUniform3fv(locCol, 1, glm::value_ptr(link->mesh->color));

    link->mesh->draw();

    // Recurse to children
    for (const auto& childJoint : link->children) {
        render(childJoint->childLink, shaderProg);
    }
}

pair<glm::vec3, glm::vec3> Model::getBoundingBox() const
{
    glm::vec3 globalBMin(numeric_limits<float>::max());
    glm::vec3 globalBMax(numeric_limits<float>::lowest());

    for (const auto& linkPair : links) {
        const Link* link = linkPair.second;
        if (link->mesh) {
            // Transform the local bounding box corners to world space
            glm::vec3 corners[8] = {
                glm::vec3(link->mesh->bmin.x, link->mesh->bmin.y, link->mesh->bmin.z),
                glm::vec3(link->mesh->bmin.x, link->mesh->bmin.y, link->mesh->bmax.z),
                glm::vec3(link->mesh->bmin.x, link->mesh->bmax.y, link->mesh->bmin.z),
                glm::vec3(link->mesh->bmin.x, link->mesh->bmax.y, link->mesh->bmax.z),
                glm::vec3(link->mesh->bmax.x, link->mesh->bmin.y, link->mesh->bmin.z),
                glm::vec3(link->mesh->bmax.x, link->mesh->bmin.y, link->mesh->bmax.z),
                glm::vec3(link->mesh->bmax.x, link->mesh->bmax.y, link->mesh->bmin.z),
                glm::vec3(link->mesh->bmax.x, link->mesh->bmax.y, link->mesh->bmax.z)
            };

            for (const auto& corner : corners) {
                glm::vec4 worldPos = link->transform * glm::vec4(corner, 1.0f);
                globalBMin = glm::min(globalBMin, glm::vec3(worldPos));
                globalBMax = glm::max(globalBMax, glm::vec3(worldPos));
            }
        }
    }

    return { globalBMin, globalBMax };
}

bool Model::updatePosition(const std::string& jointName, float position)
{
    auto it = joints.find(jointName);
    if (it != joints.end()) {
        Joint* joint = it->second;
        // Clamp position within limits if it exists
        if (joint->type == "revolute" || joint->type == "prismatic") {
            if (position < joint->lowerLimit)
                position = joint->lowerLimit;
            if (position > joint->upperLimit)
                position = joint->upperLimit;
        }
        joint->targetPosition = position;
        // std::cout << "Updated joint '" << jointName << "' to position " << glm::degrees(position) << "\n";
        return true;
    }
    return false;
}