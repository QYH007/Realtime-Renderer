//
//  DrawableShape.cpp
//  CGL
//
//  Created by Alireza Amiraghdam on 27.05.19.
//

#include "Mesh.h"
namespace cgCourse
{
    
const std::vector< glm::vec3>& Mesh::getPositions() const{
        return Shape::getPositions();
    }
    const std::vector< glm::vec3>& Mesh::getNormals() const{
        return Shape::getNormals();
    }
    const std::vector< glm::uvec3>& Mesh::getFaces() const{
        return Shape::getFaces();
    }
    const std::vector< glm::vec3>& Mesh::getFaceNormals() const{
        return Shape::getFaceNormals();
    }
    const std::vector< glm::vec3>& Mesh::getTangents() const{
        return Shape::getTangents();
    }
    const std::vector< glm::vec3>& Mesh::getColors() const{
        return Shape::getColors();
    }
    const std::vector< glm::vec2 >& Mesh::getTexCoords() const{
        return Shape::getTexCoords();
    }
    const glm::mat4& Mesh::getModelMatrix() const{
        return Shape::getModelMatrix();
    }
    const std::shared_ptr<Material>& Mesh::getMaterial() const{
        return Shape::getMaterial();
    }
    GLsizei Mesh::getDrawElemCount() const{
        return Shape::getDrawElemCount();
    }
    const std::vector< glm::uvec2>& Mesh::getLineIndices() const{
        return Shape::getLineIndices();
    }

    
}
