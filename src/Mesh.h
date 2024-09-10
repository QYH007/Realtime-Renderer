//
//  DrawableShape.hpp
//  CGL
//
//  Created by Alireza Amiraghdam on 27.05.19.
//

#ifndef MESH_hpp
#define MESH_hpp

#include <stdio.h>
#include "Shape.h"


namespace cgCourse
{
    class Mesh: public Shape{
    public:
        const std::vector< glm::vec3 >& getPositions() const ;
        const std::vector< glm::vec3 >& getNormals() const ;
        const std::vector< glm::uvec3 >& getFaces() const ;
        const std::vector< glm::vec3 >& getFaceNormals() const ;
        const std::vector< glm::vec3 >& getTangents() const ;
        const std::vector< glm::vec3>& getColors() const ;
        const std::vector< glm::vec2>& getTexCoords() const ;
        const std::shared_ptr<Material>& getMaterial() const ;
        const glm::mat4& getModelMatrix() const ;
        virtual GLsizei getDrawElemCount() const ;
        const std::vector< glm::uvec2>& getLineIndices() const ;
    };
    
    
    
}
#endif /* MESH_hpp */
