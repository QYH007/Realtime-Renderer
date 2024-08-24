#include "Shape.h"
#include "ShaderProgram.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace cgCourse
{
    Shape::Shape()
		:  positions(0), colors(0), normals(0), faces(0), faceNormals(0), tangents(0)
	{
		// Model matrix : an identity matrix (model will be at the origin)
		this->modelMatrix = glm::mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
	}
	Shape::~Shape()
	{
		glDeleteBuffers(1, &posBufferID);
		glDeleteBuffers(1, &colorBufferID);
		glDeleteBuffers(1, &normalBufferID);
		glDeleteBuffers(1, &tangentBufferID);
		glDeleteBuffers(1, &texCoordsBufferID);
		glDeleteBuffers(1, &indexBufferID);
		glDeleteVertexArrays(1, &vaoID);
	}
   //===================drawable============================
	bool Shape::createVertexArray(GLuint posAttribLoc, GLuint colAttribLoc, GLuint normAttribLoc, GLuint texAttribLoc, GLuint tangentsAttribLoc)
	{
		// check if all buffer locations are somehow defined
		if( (posAttribLoc == GLuint(-1)) ||
			(colAttribLoc == GLuint(-1)) ||
			(normAttribLoc == GLuint(-1)) ||
			(texAttribLoc == GLuint(-1)) ||
			(tangentsAttribLoc == GLuint(-1)))
		{
			return false;
		}

        const std::vector< glm::vec3 >& positions = getPositions();
        const std::vector< glm::vec3 >& colors = getColors();
        const std::vector< glm::vec3 >& normals = getNormals();
        const std::vector< glm::vec2 >& texCoords = getTexCoords();
        const std::vector< glm::vec3 >& tangents = getTangents();

		// Initialize Vertex Array Object
		glGenVertexArrays(1, &vaoID);
		glBindVertexArray(vaoID);

		// Initialize buffer objects with geometry data
		// for positions
		glGenBuffers(1, &posBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, posBufferID);
		glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(posAttribLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(posAttribLoc);

		// for colors
        if (colors.size()>0){
            glGenBuffers(1, &colorBufferID);
            glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
            glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3),
                         colors.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(colAttribLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(colAttribLoc);
        }

		// for normals
		if(normals.size() != 0)
		{
			glGenBuffers(1, &normalBufferID);
			glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
			glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(normAttribLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(normAttribLoc);
		}

		if(texCoords.size() != 0)
		{
			glGenBuffers(1, &texCoordsBufferID);
			glBindBuffer(GL_ARRAY_BUFFER, texCoordsBufferID);
			glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(glm::vec2), texCoords.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(texAttribLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(texAttribLoc);
		}

		// for texture coordinates
		if(tangents.size() != 0)
		{
			glGenBuffers(1, &tangentBufferID);
			glBindBuffer(GL_ARRAY_BUFFER, tangentBufferID);
			glBufferData(GL_ARRAY_BUFFER, tangents.size() * sizeof(glm::vec3), tangents.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(tangentsAttribLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(tangentsAttribLoc);
		}

		glGenBuffers(1, &indexBufferID);

		switch (primitiveType) {
            case point:
                this->initIndexBufferPoint();
                break;
            case line:
                this->initIndexBufferLine();
                break;
            case triangle:
                this->initIndexBuffer();
                break;
        }

		// Reset state
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		return true;
	}

    void Shape::initIndexBuffer() {
        // Initialize buffer objects with index data
        const std::vector< glm::uvec3>& faces = getFaces();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(glm::uvec3),
                     faces.data(), GL_STATIC_DRAW);
    }

	void Shape::initIndexBufferPoint() {
        // Initialize buffer objects with index data
        const std::vector< glm::vec3 >& positions = getPositions();
        std::vector<unsigned int> indices;
        for (int i = 0;i<positions.size();i++){
            indices.push_back(i);
        }
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                     indices.data(), GL_STATIC_DRAW);
    }

    void Shape::initIndexBufferLine(){
        const std::vector< glm::uvec2>& lineIndices = getLineIndices();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, getIndexBufferId());
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, lineIndices.size() * sizeof(glm::uvec2),
                     lineIndices.data(), GL_STATIC_DRAW);
    }

	void Shape::draw() const
	{
		glBindVertexArray(vaoID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
		glDrawElements(GL_TRIANGLES, 3 * faces.size(), GL_UNSIGNED_INT, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void Shape::draw(
		const glm::mat4 &_projectionMatrix, 
		const glm::mat4 &_viewMatrix,
		std::shared_ptr<ShaderProgram> _shaderProgram, 
		bool _updateVMP, 
		std::shared_ptr<Material> _overrideMaterial 
	) const
	{
        //std::cout<<"shape::draw(list)"<<std::endl;
		// prepare parameters
		const std::vector< glm::vec3 >& colors = getColors();
		std::shared_ptr<Material>  material;
		// to see if overrideMaterial exist
        bool useAlbedoMap = 0;
        bool useMetalnessMap = 0;
        bool useRoughnessMap = 0;
        bool useNormalMap = 0;
        material = getMaterial();
        if (material != nullptr){
            if (material->diffuseTexture && material->diffuseTexture->getTexHandle() != GLuint(0)){
                _shaderProgram->addTexture("diffuseTexture", material->diffuseTexture->getTexHandle());
                useAlbedoMap = 1;
            }
            if (material->normalTexture && material->normalTexture->getTexHandle() != GLuint(0)){
                _shaderProgram->addTexture("normalTexture", material->normalTexture->getTexHandle());
                useNormalMap = 1;
            }
            if (material->specTexture && material->specTexture->getTexHandle() != GLuint(0)){
                _shaderProgram->addTexture("specTexture", material->specTexture->getTexHandle());
            }
            if (material->metalnessTexture && material->metalnessTexture->getTexHandle() != GLuint(0)){
                _shaderProgram->addTexture("metalnessTexture", material->metalnessTexture->getTexHandle());
                useMetalnessMap = 1;
            }
            if (material->roughnessTexture && material->roughnessTexture->getTexHandle() != GLuint(0)){
                _shaderProgram->addTexture("roughnessTexture", material->roughnessTexture->getTexHandle());
                useRoughnessMap = 1;
            }
        }

        if (_overrideMaterial != nullptr){
            if (_overrideMaterial->diffuseTexture && _overrideMaterial->diffuseTexture->getTexHandle() != GLuint(0)){
                _shaderProgram->addTexture("diffuseTexture", _overrideMaterial->diffuseTexture->getTexHandle());
                useAlbedoMap = 1;
            }
            if (_overrideMaterial->normalTexture &&  _overrideMaterial->normalTexture->getTexHandle() != GLuint(0)){
                _shaderProgram->addTexture("normalTexture", _overrideMaterial->normalTexture->getTexHandle());
                useNormalMap = 1;
            }
            if (_overrideMaterial->specTexture && _overrideMaterial->specTexture->getTexHandle() != GLuint(0)){
                _shaderProgram->addTexture("specTexture", _overrideMaterial->specTexture->getTexHandle());
            }
            if (_overrideMaterial->metalnessTexture && _overrideMaterial->metalnessTexture->getTexHandle() != GLuint(0)){
                _shaderProgram->addTexture("metalnessTexture", _overrideMaterial->metalnessTexture->getTexHandle());
                useMetalnessMap = 1;
            }
            if (_overrideMaterial->roughnessTexture && _overrideMaterial->roughnessTexture->getTexHandle() != GLuint(0)){
                _shaderProgram->addTexture("roughnessTexture", _overrideMaterial->roughnessTexture->getTexHandle());
                useRoughnessMap = 1;
            }
        }
        _shaderProgram->setUniformi("useAlbedoMap", useAlbedoMap);
        _shaderProgram->setUniformi("useMetalnessMap", useMetalnessMap);
        _shaderProgram->setUniformi("useRoughnessMap", useRoughnessMap);
        _shaderProgram->setUniformi("useNormalMap", useNormalMap);
        
        
		// 
		_shaderProgram->bind();

		// if (_updateVMP){
        //     auto mvpMatrix = _projectionMatrix * _viewMatrix * this->getModelMatrix();
        //     glUniformMatrix4fv(_shaderProgram->getUniformLocation("modelMatrix"), 1, GL_FALSE, &this->getModelMatrix()[0][0]);
        //     glUniformMatrix4fv(_shaderProgram->getUniformLocation("viewMatrix"), 1, GL_FALSE, &_viewMatrix[0][0]);
        //     glUniformMatrix4fv(_shaderProgram->getUniformLocation("mvpMatrix"), 1, GL_FALSE, &mvpMatrix[0][0]);
        // }
        
		// binding vao
		glBindVertexArray(vaoID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
        switch (primitiveType) {
            case point:
                glDrawElements(GL_POINTS, this->getDrawElemCount(), GL_UNSIGNED_INT, 0);
                break;
            case line:
                glDrawElements(GL_LINES, this->getDrawElemCount(), GL_UNSIGNED_INT, 0);
                break;
            case triangle:
                glDrawElements(GL_TRIANGLES, this->getDrawElemCount(), GL_UNSIGNED_INT, 0);
                break;
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		_shaderProgram->unbind();

	}

	GLuint Shape::getIndexBufferId() const{
        return indexBufferID;
    }
    GLuint Shape::getVertexArrayId() const{
        return vaoID;
    }
	int Shape::getDrawElemCount() const
    {
        switch (primitiveType) {
            case point:
                return getPositions().size();
                break;
            case line:
                return getLineIndices().size()*2;
                break;
            case triangle:
                return 3 * getFaces().size();
                break;
        }
    }

   //======================shape=================  
	void Shape::setPosition(glm::vec3 _pos)
	{
		objectPosition = _pos;
		translationMatrix = glm::translate(glm::mat4(1), _pos);
		calculateModelMatrix();
	}

	void Shape::setRotation(float _angle, glm::vec3 _rot)
	{
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(_angle), _rot);
		calculateModelMatrix();
	}

	void Shape::setScaling(glm::vec3 _scale)
	{
		scalingMatrix = glm::scale(glm::mat4(1), _scale);
		calculateModelMatrix();
	}

	void Shape::calculateModelMatrix()
	{
		modelMatrix = translationMatrix * rotationMatrix * scalingMatrix;
	}

    const std::vector< glm::vec3 >& Shape::getPositions() const
    {
        return this->positions;
    }

	const std::vector< glm::vec3 >& Shape::getNormals() const
    {
        return this->normals;
    }    
	
	const std::vector< glm::uvec3 >& Shape::getFaces() const
    {
        return this->faces;
    }

    const std::vector< glm::vec3 >& Shape::getFaceNormals() const
    {
        return this->faceNormals;
    }
    
    const std::vector< glm::vec3 >& Shape::getTangents() const
    {
        return this->tangents;
    }

	const glm::mat4 & Shape::getModelMatrix() const
	{
		return modelMatrix;
	}

	const glm::vec3 & Shape::getPosition() const
	{
		return objectPosition;
	}    
	
	const std::vector< glm::vec3>& Shape::getColors() const{
        return colors;
    }
    const std::vector< glm::vec2>& Shape::getTexCoords() const{
        return texCoords;
    }
    const std::shared_ptr<Material>& Shape::getMaterial() const{
        return material;
    }

    void Shape::setMaterial(std::shared_ptr<Material> _mat){
        material = _mat;
    }
    void Shape::addVertex(const glm::vec3 &_ver){
        positions.push_back(_ver);
    }
    void Shape::addFace(const glm::uvec3 &_face){
        faces.push_back(_face);
    }
    void Shape::addNormal(const glm::vec3 &_norm){
        normals.push_back(_norm);
    }
    void Shape::addColor(const glm::vec3 &_color){
        colors.push_back(_color);
    }
    void Shape::addTexCoord(const glm::vec3 &_texCoord){
        texCoords.push_back(_texCoord);
    }
    void Shape::addTangent(const glm::vec3 &_tangent){
        tangents.push_back(_tangent);
    }
    void Shape::addLine(const glm::uvec2 &_line){
        lineIndices.push_back(_line);
    }
    const std::vector< glm::uvec2>& Shape::getLineIndices() const{
        return lineIndices;
    }
}

