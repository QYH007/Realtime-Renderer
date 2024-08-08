//
//  Mesh.cpp
//  CGL
//
//  Created by Alireza Amiraghdam on 17.05.19.
//
#include <fstream>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "Model.h"
#include "ShaderProgram.h"

#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>


namespace cgCourse{
    extern Assimp::Importer importer;
    
    bool Model::load(std::string _path,std::string _file, bool _flipNormals, bool _swapYZ, bool _reverseWinding){
        // Check if file exists
        std::ifstream fin((_path+_file).c_str());
        if(!fin.fail())
        {
            fin.close();
        }
        else
        {
            std::cout<<( importer.GetErrorString())<<std::endl;
            return false;
        }
        
        scene = importer.ReadFile( _path+_file, aiProcessPreset_TargetRealtime_Quality);
        
        // If the import failed, report it
        if( !scene)
        {
            return false;
        }
        
        // Now we can access the file's contents.
        scene->mMeshes;
        std::cout<<scene->mRootNode->mNumMeshes<<" "<<scene->mRootNode->mNumChildren<<std::endl;
        // for(int i =0; i < scene->mNumMeshes; i++){
        //      std::cout<<"no."<<i<<" mesh, name: "<<scene->mMeshes[i]->mName.C_Str() << "   使用的材质为 "<<scene->mMeshes[i]->mMaterialIndex<<std::endl;
        // }


        std::vector<std::string> path2textures;
        for(int i =0; i < scene->mNumMaterials; i++){
            aiString path2texture;
            //std::cout<<"material No."<<i<<"has"<<scene->mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE)<<"diffuse, "<<scene->mMaterials[i]->GetTextureCount(aiTextureType_UNKNOWN)<<"metal, "<<scene->mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS)<<"normal,"<<std::endl;
            for(int j =0; j < scene->mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE); j++){
                scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, j, &path2texture);
                std::string s =  path2texture.data;
                if(_file == "fufu.pmx"){
                    if(i == 0){
                        path2textures.push_back("tex/face.png");
                    }
                    else if(i == 1){
                        path2textures.push_back("tex/hair.png");
                    }
                    else if(i == 2){
                        path2textures.push_back("tex/hair2.png");
                    }
                    else if(i == 3){
                        path2textures.push_back("tex/body.png");
                    }
                    else 
                        path2textures.push_back(s);
                }else{
                    path2textures.push_back(s);
                }
                
            }
            //std::cout<<"no."<<i<<" material, name: "<<scene->mMaterials[i]->GetName().C_Str()<<"  路径为？"<<path2textures[i]<<std::endl;
        }
        
        
        for (int i = 0; i<scene->mNumMeshes;i++){
            const aiMesh* mesh = scene->mMeshes[i];
            std::shared_ptr<Mesh> element=std::make_shared<Mesh>();
            switch (mesh->mPrimitiveTypes) {
                case aiPrimitiveType_POINT:
                    element->primitiveType=point;
                    break;
                case aiPrimitiveType_LINE:
                    element->primitiveType = line;
                    break;
                case aiPrimitiveType_TRIANGLE:
                    element->primitiveType = triangle;
                    break;
            }
            for (int j = 0; j<mesh->mNumVertices;j++){
                const aiVector3D &tempVec=mesh->mVertices[j];
                element->addVertex(glm::vec3(tempVec.x,_swapYZ?tempVec.z:tempVec.y,_swapYZ?tempVec.y:tempVec.z));
            }
            float normalCorrection = _flipNormals?-1:1;
            if (mesh->mNormals != NULL){
                for (int j = 0; j<mesh->mNumVertices;j++){
                    const aiVector3D &tempVec=mesh->mNormals[j];
                    element->addNormal(normalCorrection*glm::vec3(tempVec.x,_swapYZ?tempVec.z:tempVec.y,_swapYZ?tempVec.y:tempVec.z));
                }
            }
            if (mesh->HasVertexColors(0)){
                for (int j = 0; j<mesh->mNumVertices;j++){
                    const aiColor4D &tempVec=mesh->mColors[0][j];
                    element->addColor(glm::vec3(tempVec.r,tempVec.g,tempVec.b));
                }
            }else{
                for (int j = 0; j<mesh->mNumVertices;j++){
                    element->addColor(glm::vec3(0.9f , 0.9f, 0.9f));
                }
            }
            if (mesh->HasTextureCoords(0)){
                for (int j = 0; j<mesh->mNumVertices;j++){
                    const aiVector3D &tempVec=mesh->mTextureCoords[0][j];

                    // TODO: Check if it is correct on all machines now
                    element->addTexCoord(glm::vec3(tempVec.x,tempVec.y,tempVec.z));
                }
            }
            if (mesh->mTangents !=NULL){
                for (int j = 0; j<mesh->mNumVertices;j++){
                    const aiVector3D &tempVec=mesh->mTangents[j];
                    element->addTangent(glm::vec3(tempVec.x,tempVec.y,tempVec.z));
                }
            }
            for (int j = 0; j<mesh->mNumFaces;j++){
                const aiFace &tempFace=mesh->mFaces[j];
                if (tempFace.mNumIndices==3){
                    element->addFace(glm::uvec3(tempFace.mIndices[0],_reverseWinding? tempFace.mIndices[2]:tempFace.mIndices[1],_reverseWinding? tempFace.mIndices[1]:tempFace.mIndices[2]));
                }else if (tempFace.mNumIndices==2){
                    element->addLine(glm::uvec2(tempFace.mIndices[0],tempFace.mIndices[1]));
                    std::cout<<"WARNING: non-triagle face (numIndices="<<tempFace.mNumIndices<<") SKIPPED"<<std::endl;
                }
            }
            element->createVertexArray(0, 1, 2, 3, 4);
            // handle material and texture.
            element->setMaterial(std::make_shared<Material>());
            element->getMaterial()->diffuseTexture = std::make_shared<Texture>();
            //std::cout<<"index: "<<  mesh->mName.C_Str() <<" should use:"<<path2textures[mesh->mMaterialIndex]<< std::endl;
            if(_file == "fufu.pmx")
                element->getMaterial()->diffuseTexture->loadFromFile( _path + path2textures[mesh->mMaterialIndex], true);
            else {
                aiString subpath;
                scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &subpath);
                element->getMaterial()->diffuseTexture->loadFromFile( _path + subpath.C_Str(), true);
            }
            element->getMaterial()->metalnessTexture = std::make_shared<Texture>();
		    element->getMaterial()->roughnessTexture = std::make_shared<Texture>();
            element->getMaterial()->normalTexture = std::make_shared<Texture>();

            elements.push_back(element);
        }
        return true;
    }
    void Model::draw() const{
        
            for (auto &e:elements){
                e->draw();
            }
    }
    
    void Model::draw(const glm::mat4 &_projectionMatrix, const glm::mat4 &_viewMatrix,std::shared_ptr<ShaderProgram> _shaderProgram, bool _updateVMP, std::shared_ptr<Material> _overrideMaterial ) const{
        auto mvpMatrix = _projectionMatrix * _viewMatrix * this->getModelMatrix();
        
            _shaderProgram->setUniformMat4fv("mvpMatrix", mvpMatrix);
            // _shaderProgram->setUniformMat4fv("viewMatrix", _viewMatrix);
            _shaderProgram->setUniformMat4fv("modelMatrix", getModelMatrix());
            for (auto &e:elements){
                e->draw(_projectionMatrix, _viewMatrix, _shaderProgram, false, getMaterial());
            }
        }
    
}
