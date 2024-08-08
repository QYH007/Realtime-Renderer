//
//  DrawableShape.hpp
//  CGL
//
//  Created by Alireza Amiraghdam on 27.05.19.
//

#ifndef MODEL_hpp
#define MODEL_hpp

#include <stdio.h>
#include <unordered_map>
#include "Shape.h"
#include "Mesh.h"
#include <assimp/scene.h>


namespace cgCourse
{
    class Model: public Mesh{
    public:
        void draw(const glm::mat4 &_projectionMatrix, const glm::mat4 &_viewMatrix,
                  std::shared_ptr<ShaderProgram> _shaderProgram, bool _updateVMP = true,
                  std::shared_ptr<Material> _overrideMaterial = nullptr) const override;

        bool load(std::string _path, std::string _fileName, bool _flipNormals, bool _swapYZ, bool _reverseWinding);
        void draw()const override;
        std::string fileMapping(aiString _path);

    private:
        const aiScene *scene = NULL;
        std::vector<std::shared_ptr<Mesh>> elements;
    };
    
    
    
}
#endif /* MODEL_hpp */
