#ifndef GLEXAMPLE_H
#define GLEXAMPLE_H

#include "GLApp.h"
#include "Cube.h"
#include "Ground.h"
#include "Torus.h"
#include "Skybox.h"
#include "ShaderProgram.h"
#include "ComputingShaderProgram.h"
#include "Camera.h"
#include "Texture.h"
#include "ScreenPlane.h"
#include "Model.h"

#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <memory>

namespace cgCourse
{
	struct LightInfo
	{
		glm::vec3 ambientTerm;
		glm::vec3 diffuseTerm;
		glm::vec3 specularTerm;
		float light_Size;
		glm::vec3 radiance;
        glm::vec3 position;
	};

	struct ShadowMapping
	{
		unsigned int width = 2000;
		unsigned int height = 2000;

		GLuint depthMap;
		GLuint depthMapFBO;
		GLuint depthBuffer;

		unsigned int varianceFBO[2];
		unsigned int varianceTexture[2];
	};

	class GLExample : public GLApp
	{
	public:
		enum LightMotionMode
		{
			Forward = 0, Backward = 1
		};

		GLExample(glm::uvec2 _windowSize, std::string _title);

		bool init() override;
		bool update() override;
		bool render() override;
		bool testrender();
		bool end() override;
		// Camera cam;
		

		void imgui() override {};

	private:
		void lightInit();
		bool sceneInit();
		void addLightVariables(const std::shared_ptr<ShaderProgram>& _program);
		void addMultipleLightVariables(const std::shared_ptr<ShaderProgram>& _program);
		void addShadowVariables(const std::shared_ptr<ShaderProgram>& _program,const std::vector<glm::mat4> & lightSpaceMatrixes);
		void renderLightBox();
		void computeSAT(const ShadowMapping & shadow);
		void renderCubes(const std::vector<glm::mat4> & lightSpaceMatrixes);
		void renderTorus(const std::vector<glm::mat4> & lightSpaceMatrixes);
		void renderGround(const std::vector<glm::mat4> & lightSpaceMatrixes);
		void renderPlane();
		void renderSkybox();
		void updateGUI();
		void processInput(GLFWwindow* window);
		unsigned int loadCubemap(std::vector<std::string>& faces);

		void shadow_mapping(const ShadowMapping & shadow, const glm::mat4 & lightSpaceMatrix);

		std::shared_ptr<ShaderProgram> programForShadows;
		std::shared_ptr<ShaderProgram> programForShape;
		std::shared_ptr<ShaderProgram> programForPlane;
		std::shared_ptr<ShaderProgram> programForTorusNormals;
		std::shared_ptr<ShaderProgram> programForLightBox;
		std::shared_ptr<ShaderProgram> programForPBR;
		std::shared_ptr<ShaderProgram> programForSkybox;

		std::shared_ptr<ComputingShaderProgram> programForSAT;

		std::shared_ptr<Cube> cube;
		std::shared_ptr<Cube> lightbox;
		std::shared_ptr<Torus> torus;
		std::shared_ptr<Cube> ground;
		
		Model gun;
		Model fufu;

		std::shared_ptr<ScreenPlane> screenPlane;
		std::shared_ptr<Skybox> skybox;

		std::shared_ptr<Texture> cubetex;
		std::shared_ptr<Texture> cubetexSpec;
        std::shared_ptr<Texture> cubetexNormal;

		std::shared_ptr<Texture> cubeDiffuse;
		std::shared_ptr<Texture> cubeNormal;
        std::shared_ptr<Texture> cubeMetalness;
		std::shared_ptr<Texture> cubeRoughness;

		std::shared_ptr<Texture> torustex;
		std::shared_ptr<Texture> torustexSpec;
        std::shared_ptr<Texture> torustexNormal;
		std::shared_ptr<Texture> skyBosxtex;

		float animation = 0;
		LightMotionMode animationDir = Forward;
		//deltaTime
		float lastFrame = 0.0f;
		float currentFrame = 0.0f;
		float deltaTime = 0.0f;


		glm::vec3 lightboxColor;
		glm::mat4 mvpMatrix = glm::mat4(1);

		bool isPBR = true;
		bool drawTorusNormals = false;
		bool disPlay_shadowMap = true;
		unsigned int PCF_samples = 3;
		unsigned int shadowType=2;
		float lightSize = 5.5;
		float ambientFactor = 0.4;

		LightInfo light;
		std::vector<LightInfo> lights;
		std::vector<std::shared_ptr<Cube>> lightboxes;
		std::vector<ShadowMapping> shadows;
		
		bool showNormal = false;
		bool showDiffuseTerm = true;
		bool showColor = true;
		bool showSpecular = true;
	};
}

#endif // GLEXAMPLE_H

