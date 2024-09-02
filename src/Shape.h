#ifndef SHAPE_H
#define SHAPE_H

#include "GLIncludes.h"
#include <vector>
#include <glm/mat4x4.hpp>
#include <memory>
#include <glm/glm.hpp>

namespace cgCourse
{   
    class Texture;
    class ShaderProgram;

    struct Material {
        glm::vec3 ka = {1, 1, 1};
        glm::vec3 kd = {1, 1, 1};
        glm::vec3 ks = {1, 1, 1};
        float ns = 1;
        std::shared_ptr<Texture> diffuseTexture = nullptr;
        std::shared_ptr<Texture> normalTexture = nullptr;
        std::shared_ptr<Texture> specTexture = nullptr;
        std::shared_ptr<Texture> metalnessTexture = nullptr;
        std::shared_ptr<Texture> roughnessTexture = nullptr;
        std::shared_ptr<Texture> rampTexture = nullptr;
        bool hasObjectColor = false;
        glm::vec3 color = {0.5, 0.5, 0.5};
        float illumination = 0;
    };

    class Shape
    {
    public:
        enum PrimitveType { point = 0x1, line = 0x2, triangle = 0x4 } primitiveType;

        Shape();
        virtual ~Shape();

        // ------------------------------- Drawable part --------------------------------------
        bool createVertexArray(
            GLuint posAttribLoc,
            GLuint colAttribLoc,
            GLuint normAttribLoc,
            GLuint texAttribLoc,
            GLuint tangentsAttribLoc
        );

        virtual void draw() const;
        virtual void draw(
            const glm::mat4 &_projectionMatrix, 
            const glm::mat4 &_viewMatrix,
            std::shared_ptr<ShaderProgram> _shaderProgram, 
            bool _updateVMP, 
            std::shared_ptr<Material> _overrideMaterial 
        ) const;

		GLuint vaoID = 0;
        GLuint posBufferID = 0;
        GLuint colorBufferID = 0;
        GLuint normalBufferID = 0;
        GLuint texCoordsBufferID = 0;
        GLuint tangentBufferID = 0;
        GLuint indexBufferID = 0;

        // ------------------------------- Shape part --------------------------------------
        // Gets
        const glm::mat4 &getModelMatrix() const;
        const glm::vec3 &getPosition() const;
        const std::vector<glm::vec3> &getPositions() const;
        const std::vector<glm::vec3> &getNormals() const;
        const std::vector<glm::uvec3> &getFaces() const;
        const std::vector<glm::vec3> &getFaceNormals() const;
        const std::vector<glm::vec3> &getTangents() const;
        const std::vector<glm::vec3> &getColors() const;
        const std::vector<glm::vec2> &getTexCoords() const;
        const std::shared_ptr<Material> &getMaterial() const;
        const std::vector<glm::uvec2> &getLineIndices() const;

        // Sets
        void setPosition(glm::vec3);
        void setRotation(float _angle, glm::vec3 _rot);
        void setScaling(glm::vec3 _scale);
        void setMaterial(std::shared_ptr<Material> _mat);

        // Adds
        void addVertex(const glm::vec3 &_ver);
        void addFace(const glm::uvec3 &_face);
        void addNormal(const glm::vec3 &_norm);
        void addColor(const glm::vec3 &_color);
        void addTexCoord(const glm::vec3 &_texCoord);
        void addTangent(const glm::vec3 &_tangent);
        void addLine(const glm::uvec2 &_line);

        void calculateModelMatrix();
        
        glm::vec3 objectPosition;

    protected:
        // ------------------------------- Drawable part --------------------------------------
        GLuint getIndexBufferId() const;
        GLuint getVertexArrayId() const;
        virtual GLsizei getDrawElemCount() const = 0;

        // ------------------------------- Shape part --------------------------------------
        glm::mat4 rotationMatrix = glm::mat4(1);
        glm::mat4 translationMatrix = glm::mat4(1);
        glm::mat4 scalingMatrix = glm::mat4(1);
        glm::mat4 modelMatrix = glm::mat4(1); // Identity matrix

        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> colors;
        std::vector<glm::vec3> normals;
        std::vector<glm::uvec3> faces;
        std::vector<glm::vec3> faceNormals;
        std::vector<glm::vec2> texCoords;
        std::vector<glm::vec3> tangents;
        std::vector<glm::uvec2> lineIndices;

        std::shared_ptr<Material> material = nullptr;
        

    private:
        virtual void initIndexBuffer();
        virtual void initIndexBufferPoint();
        virtual void initIndexBufferLine();  // Corrected function name
    };
}

#endif // SHAPE_H