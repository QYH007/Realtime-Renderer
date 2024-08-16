#include "GLExample.h"
#include "Cube.h"
#include <imgui/imgui.h>
// UI dependencies
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw_gl3.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <iostream>
#include <random>


//mouse settings
int xLast = 0;
int yLast = 0;
bool rightButtonPressed = false;
bool firstMouse = true;
int dx = 0;
int dy = 0;
cgCourse::Camera cam;

void cursor_callback(GLFWwindow* window, double xPos, double yPos) {
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		rightButtonPressed = true;
	else
		rightButtonPressed = false;

	if (firstMouse) {
		xLast = xPos;
		yLast = yPos;
		firstMouse = false;
	}

	if (rightButtonPressed){
		cam.ProcessMouseMovement(xPos - xLast, yLast - yPos);
		dx = xPos - xLast;
		dy = yLast - yPos;
	}
	else
		firstMouse = true;

	xLast = xPos;
	yLast = yPos;
}


namespace cgCourse

{
	Assimp::Importer importer;
	GLExample::GLExample(glm::uvec2 _windowSize, std::string _title): GLApp(_windowSize, _title, false) {}

	bool GLExample::init()
	{
        /********************* UI ******************************/
        // Setup Dear ImGui context
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.Fonts->AddFontFromFileTTF((std::string(RES_DIR) + "/Cousine-Regular.ttf").c_str(), 22.0f);

		ImGui_ImplGlfwGL3_Init(window_, true);
		//setup style
		ImGui::StyleColorsDark();
		glfwSetCursorPosCallback(window_, cursor_callback);

		cam.create(	getFramebufferSize(),
					glm::vec3(3, 3, -8),
					glm::vec3(0, 1, 0),
					-30, 127
					);
		//std::cout<<cam.getPosition().x<<", "<<cam.getPosition().y<<", "<<cam.getPosition().z<<std::endl;

		programForShadows = std::make_shared<ShaderProgram>(std::string(SHADER_DIR) + "/Shadows");
		programForShape = std::make_shared<ShaderProgram>(std::string(SHADER_DIR) + "/Shape");
		programForTorusNormals = std::make_shared<ShaderProgram>(std::string(SHADER_DIR) + "/Normals");
		programForLightBox = std::make_shared<ShaderProgram>(std::string(SHADER_DIR) + "/Lightbox");
		programForPlane = std::make_shared<ShaderProgram>(std::string(SHADER_DIR) + "/Plane");
		programForPBR = std::make_shared<ShaderProgram>(std::string(SHADER_DIR) + "/PBR");
		programForSkybox = std::make_shared<ShaderProgram>(std::string(SHADER_DIR) + "/Skybox");
		programForDefer = std::make_shared<ShaderProgram>(std::string(SHADER_DIR) + "/Defer");
		programForDeferLighting = std::make_shared<ShaderProgram>(std::string(SHADER_DIR) + "/DeferLighting");
		programForSSAO = std::make_shared<ShaderProgram>(std::string(SHADER_DIR) + "/SSAO");
		programForSSAOBlur = std::make_shared<ShaderProgram>(std::string(SHADER_DIR) + "/SSAOBlur");
		
		programForSAT = std::make_shared<ComputingShaderProgram>(std::string(SHADER_DIR) + "/ComputeSAT");
		programForSAT->bind();
		programForSAT->SetUniform1i("input_image", 0);
		programForSAT->SetUniform1i("output_image", 1);

		lightInit();
		sceneInit();
		deferredInit();

		return true;
	}

	bool GLExample::update()
	{
		// torus->setRotation(1, glm::vec3(1.0f, 1.0f, 1.0f));
		gun.setRotation(0.3, glm::vec3(0, 0, 1.0f));
		fufu.setPosition(fufu.objectPosition);
		gun.setPosition(gun.objectPosition);
		cube->setPosition(cube->objectPosition);
		if(animationDir == Forward)
		{
			if(animation > 2.0)
				animationDir = Backward;
			else
				animation += 0.01;
		}
		else
		{
			if(animation < -2.0)
				animationDir = Forward;
			else
				animation -= 0.01;
		}
		lights[0].position = glm::vec3(animation, 10, -10);
		lightboxes[0]->setPosition(glm::vec3(animation, 10, -15));
		lightboxes[1]->setPosition(lights[1].position);
		gun.setPosition(gun.getPosition()+glm::vec3(0, animation*0.005, 0));
		for(int i =0;i<manyLightsPositions.size();i++){
			// manyLightsPositions[i] = manyLightsPositions[i] + manyLightPosition;
			manyLightsboxes[i]->setPosition(manyLightsPositions[i] + manyLightPosition);
		}
		return true;
	}

	void GLExample::deferredInit(){
		// configure g-buffer framebuffer
		// ------------------------------
		glGenFramebuffers(1, &gBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

		// position color buffer
		glGenTextures(1, &gPosition);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _windowSize.x, _windowSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

		// normal color buffer
		glGenTextures(1, &gNormal);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _windowSize.x, _windowSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

		// color + specular color buffer
		glGenTextures(1, &gAlbedoSpec);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _windowSize.x, _windowSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

		glGenTextures(1, &gRough);
		glBindTexture(GL_TEXTURE_2D, gRough);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _windowSize.x, _windowSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gRough, 0);

		// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
		unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glDrawBuffers(4, attachments);

		// create and attach depth buffer (renderbuffer)
		unsigned int rboDepth;
		glGenRenderbuffers(1, &rboDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _windowSize.x, _windowSize.y);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

		// finally check if framebuffer is complete
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Also create framebuffer to hold SSAO processing stage 
		glGenFramebuffers(1, &ssaoFBO);  glGenFramebuffers(1, &ssaoBlurFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
		// - SSAO color buffer
		glGenTextures(1, &ssaoColorBuffer);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, _windowSize.x, _windowSize.y, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "SSAO Framebuffer not complete!" << std::endl;
		// - and blur stage
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
		glGenTextures(1, &ssaoColorBufferBlur);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, _windowSize.x, _windowSize.y, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		// Sample kernel
		std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
		std::default_random_engine generator;
		for (GLuint i = 0; i < 64; ++i)
		{
			glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
			sample = glm::normalize(sample);
			sample *= randomFloats(generator);
			GLfloat scale = GLfloat(i) / 64.0;

			// Scale samples s.t. they're more aligned to center of kernel
			scale = 0.1f + scale * scale * (1.0f - 0.1f);
			sample *= scale;
			ssaoKernel.push_back(sample);
		}
		// Noise texture
		for (GLuint i = 0; i < 16; i++)
		{
			glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
			ssaoNoise.push_back(noise);
		}
		glGenTextures(1, &noiseTexture);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	}

	void GLExample::deferredRender(){

			// 1. geometry pass: render scene's geometry/color data into gbuffer
			// -----------------------------------------------------------------
			glEnable(GL_DEPTH_TEST);
			glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		
			glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
			glViewport(0, 0, getFramebufferSize().x, getFramebufferSize().y) ;
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);

			programForDefer->setUniformi("useDiffuseTexture", 1);
			programForDefer->setUniformf("near", cam.NearPlane);
			programForDefer->setUniformf("far", cam.FarPlane);
			programForDefer->setUniformi("useDiffuseTexture", 1);
			programForDefer->setUniformMat4fv("viewMatrix", cam.getViewMatrix());
			fufu.draw(cam.getProjectionMatrix(), cam.getViewMatrix(), programForDefer);
			gun.draw(cam.getProjectionMatrix(), cam.getViewMatrix(), programForDefer);
			renderGround(programForDefer);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			// 2.SSAO
			glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
            glClear(GL_COLOR_BUFFER_BIT);
			programForSSAO->addTexture("gPosition", gPosition);
			programForSSAO->addTexture("gNormal", gNormal);
			programForSSAO->addTexture("texNoise", noiseTexture);

            // // Send kernel + rotation 
            for (GLuint i = 0; i < 64; ++i)
				programForSSAO->setUniform3fv(("samples[" + std::to_string(i) + "]").c_str(), ssaoKernel[i]);
			programForSSAO->setUniformMat4fv("projection", cam.getProjectionMatrix());
			programForSSAO->setUniformMat4fv("viewMatrix", cam.getViewMatrix());
			programForSSAO->setUniformf("SSAOradius", SSAOradius);
            programForSSAO->bind();
			renderQuad();
			programForSSAO->unbind();

        	glBindFramebuffer(GL_FRAMEBUFFER, 0);

			// 3.SSAO blurring
			glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
            glClear(GL_COLOR_BUFFER_BIT);
            programForSSAOBlur->addTexture("ssaoMap", ssaoColorBuffer);
			programForSSAOBlur->setUniformi("blurSize", SSAOblurSize);
			programForSSAOBlur->bind();
            renderQuad();
			programForSSAOBlur->unbind();
        	glBindFramebuffer(GL_FRAMEBUFFER, 0);

			// 4. lighting pass: calculate lighting by iterating over a screen filled quad pixel-by-pixel using the gbuffer's content.
			// -----------------------------------------------------------------------------------------------------------------------
			glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
			programForDeferLighting->addTexture("gPosition", gPosition);
			programForDeferLighting->addTexture("gNormal", gNormal);
			programForDeferLighting->addTexture("gAlbedoSpec", gAlbedoSpec);
			programForDeferLighting->addTexture("gRough", gRough);
			programForDeferLighting->addTexture("ssao", ssaoColorBufferBlur);

			// send light relevant uniforms
			programForDeferLighting->setUniformf("manyLightIntensity", manyLightIntensity);
			for (unsigned int i = 0; i < manyLightsPositions.size(); i++)
			{
				const float constant = 1.0f; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
				const float linear = 0.7f;
				const float quadratic = 1.8f;
				programForDeferLighting->setUniform3fv("lights[" + std::to_string(i) + "].Position", manyLightsPositions[i] + manyLightPosition);
				programForDeferLighting->setUniform3fv("lights[" + std::to_string(i) + "].Color", manyLightsColors[i]);
				// update attenuation parameters and calculate radius
				programForDeferLighting->setUniformf("lights[" + std::to_string(i) + "].Linear", linear);
				programForDeferLighting->setUniformf("lights[" + std::to_string(i) + "].Quadratic", quadratic);
				
				// then calculate radius of light volume/sphere
				const float maxBrightness = std::fmaxf(std::fmaxf(manyLightsColors[i].r, manyLightsColors[i].g), manyLightsColors[i].b);
				float radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * maxBrightness))) / (2.0f * quadratic);
				programForDeferLighting->setUniformf("lights[" + std::to_string(i) + "].Radius", radius);
			}
			
			programForDeferLighting->setUniform3fv("viewPos", cam.getPosition());
			programForDeferLighting->setUniformMat4fv("viewMatrix", cam.getViewMatrix());
			programForDeferLighting->setUniformi("useDiffuseTexture", 1);
			programForDeferLighting->setUniformi("showNormal", showNormal);
			programForDeferLighting->setUniformi("showDiffuseTerm", showDiffuseTerm);
			programForDeferLighting->setUniformi("showSpecular", showSpecular);
			programForDeferLighting->setUniformi("showColor", showColor);
			programForDeferLighting->setUniformi("outputMood", deferRenderOutput);
			// finally render quad
			programForDeferLighting->bind();
			renderQuad();
			programForDeferLighting->unbind();
			glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
			// blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
			// the internal formats are implementation defined. This works on all of my systems, but if it doesn't on yours you'll likely have to write to the 		
			// depth buffer in another shader stage (or somehow see to match the default framebuffer's internal format with the FBO's internal format).
			glBlitFramebuffer(0, 0, _windowSize.x, _windowSize.y, 0, 0,_windowSize.x, _windowSize.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			renderLightBox(true);

			renderPlane(ssaoColorBufferBlur);
			// renderPlane(gRough);
			// renderPlane(gAlbedoSpec);
	}

	
	void GLExample::renderQuad(){
		if (quadVAO == 0)
		{
			float quadVertices[] = {
				// positions        // texture Coords
				-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
				1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
				1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			};
			// setup plane VAO
			glGenVertexArrays(1, &quadVAO);
			glGenBuffers(1, &quadVBO);
			glBindVertexArray(quadVAO);
			glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		}
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
	}

	void GLExample::forwardRender(){
		//***********----------------First Pass rendering from light view space-----------------**********************//
		
		glEnable(GL_DEPTH_TEST);
		std::vector<glm::mat4> lightSpaceMatrixes;
		glm::mat4 lightSpaceMatrix;
		float near_plane =0.0f, far_plane = 150.0f;
		glm::mat4 lightProjection = glm::ortho(-40.0f, 40.0f, -40.0f, 40.0f, near_plane, far_plane);
		// TODO: compute lightSpaceMatrix
		for(int i = 0; i<lights.size(); ++i){
			glm::mat4 lightView = glm::lookAt(lights[i].position,
                                  fufu.objectPosition, 
                                  glm::vec3( 0.0f, 1.0f,  0.0f)); 
			lightSpaceMatrix = lightProjection * lightView; 
			lightSpaceMatrixes.push_back(lightSpaceMatrix);
			shadow_mapping(shadows[i], lightSpaceMatrix);
			computeSAT(shadows[i]);
		}

		//***********----------------Second Pass Rendering from camera view space-----------------**********************//
		// 状态重置
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glEnable(GL_DEPTH_TEST);
		glViewport(0,0,getFramebufferSize().x,getFramebufferSize().y) ;
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		renderLightBox(false);
		if(disPlay_shadowMap)
			renderPlane(shadows[0].depthMap);
		if(isPBR){
			addMultipleLightVariables(programForPBR);
			addShadowVariables(programForPBR, lightSpaceMatrixes);
		}

		renderCubes(lightSpaceMatrixes);
		renderGround(programForPBR);

		programForPBR->setUniformi("useDiffuseTexture", 1);
		programForPBR->setUniformi("showNormal", showNormal);
		programForPBR->setUniformi("showDiffuseTerm", showDiffuseTerm);
		programForPBR->setUniformi("showSpecular", showSpecular);
		programForPBR->setUniformi("showColor", showColor);

		gun.draw(cam.getProjectionMatrix(), cam.getViewMatrix(), programForPBR);
		fufu.draw(cam.getProjectionMatrix(), cam.getViewMatrix(), programForPBR);
	}

	bool GLExample::render()
	{
		//***********----------------Control and frame-----------------**********************//
		currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		update();
		cam.updateCameraData();

		if(isDefer)
			deferredRender();
		else
			forwardRender();
		

		renderSkybox();
		processInput(window_);
		updateGUI();
		
		return true;
	}

	bool GLExample::sceneInit(){
		
		skybox = std::make_shared<Skybox>();
		if(!skybox->createVertexArray(0))
			return false;
		
		std::string skyboxname = "/ruin";
		std::vector<std::string> faces
		{
			std::string(SKY_DIR) + skyboxname + "/right.png",
			std::string(SKY_DIR) + skyboxname + "/left.png",
			std::string(SKY_DIR) + skyboxname + "/top.png",
			std::string(SKY_DIR) + skyboxname + "/bottom.png",
			std::string(SKY_DIR) + skyboxname + "/front.png",
			std::string(SKY_DIR) + skyboxname + "/back.png",
		};

		skyBosxtex = std::make_shared<Texture>();
		skyBosxtex->loadCubemap(faces);


		ground = std::make_shared<Cube>();
		if(!ground->createVertexArray(0, 1, 2, 3, 4))
			return false;
		ground->setPosition(glm::vec3(0, -10, 0));
		ground->setScaling(glm::vec3(30, 0.01, 30));


		cube = std::make_shared<Cube>();
		if(!cube->createVertexArray(0, 1, 2, 3, 4))
			return false;

		cube->setPosition(glm::vec3(-4, 1, 7));
		cube->setScaling(glm::vec3(1, 1, 1));
		// Create Material
        cube->setMaterial(std::make_shared<Material>());
        cube->getMaterial()->diffuseTexture = std::make_shared<Texture>();
        cube->getMaterial()->diffuseTexture->loadFromFile(std::string(RES_DIR) + "/textures/rusty/rust-coated-basecolor.png");
        //cube->getMaterial()->diffuseTexture->loadFromFile(std::string(RES_DIR) + "/FBX/Cerberus_A.tga", true);
		cube->getMaterial()->metalnessTexture = std::make_shared<Texture>();
        cube->getMaterial()->metalnessTexture->loadFromFile(std::string(RES_DIR) + "/textures/rusty/rust-coated-metal.png");
        //cube->getMaterial()->metalnessTexture->loadFromFile(std::string(RES_DIR) + "/FBX/cerberus_M.png");
        cube->getMaterial()->normalTexture = std::make_shared<Texture>();
        //cube->getMaterial()->normalTexture->loadFromFile(std::string(RES_DIR) + "/FBX/cerberus_N.png");
        cube->getMaterial()->normalTexture->loadFromFile(std::string(RES_DIR) + "/textures/rusty/rust-coated-normal.png");

		cubetex = std::make_shared<Texture>();
		cubetex->loadFromFile(std::string(RES_DIR) + "/container.png");

		cubetexSpec = std::make_shared<Texture>();
		cubetexSpec->loadFromFile(std::string(RES_DIR) + "/container_specular.png");

		cubetexNormal = std::make_shared<Texture>();
		cubetexNormal->loadFromFile(std::string(RES_DIR) + "/container_normal.jpg");

		// ==================gun=================
		gun.load(std::string(RES_DIR) + "/FBX/", "cerberus.fbx", false, false, false);
        gun.setScaling(glm::vec3(0.03, 0.03, 0.03));
        gun.setPosition(glm::vec3(-8, 2, -3));
        gun.setRotation(-90, glm::vec3(1, 0, 0));
        gun.setRotation(-100, glm::vec3(0, 0, 1));

        // Create Material
        gun.setMaterial(std::make_shared<Material>());
        // gun.getMaterial()->diffuseTexture = std::make_shared<Texture>();
        // gun.getMaterial()->diffuseTexture->loadFromFile(std::string(RES_DIR) + "/FBX/Textures/Cerberus_A.tga", true);
        gun.getMaterial()->metalnessTexture = std::make_shared<Texture>();
        gun.getMaterial()->metalnessTexture->loadFromFile(std::string(RES_DIR) + "/FBX/Textures/cerberus_M.png");
        gun.getMaterial()->normalTexture = std::make_shared<Texture>();
        gun.getMaterial()->normalTexture->loadFromFile(std::string(RES_DIR) + "/FBX/Textures/cerberus_N.png");
        gun.getMaterial()->roughnessTexture = std::make_shared<Texture>();
        gun.getMaterial()->roughnessTexture->loadFromFile(std::string(RES_DIR) + "/FBX/Textures/cerberus_R.png");
		

		// ===============fufu=====================
		fufu.load(std::string(RES_DIR) + "/fufu/", "fufu.pmx", false, false, false);
		fufu.setScaling(glm::vec3(0.9, 0.9, 0.9));
        fufu.setPosition(glm::vec3(-1, -10, 1));
		fufu.setRotation(180, glm::vec3(0, 1, 0));

		float aspect_ratio = float(_windowSize.y) / _windowSize.x;
		screenPlane = std::make_shared<ScreenPlane>(1- 0.5*aspect_ratio, 0.5f, 1.0f ,1.0f);

		if(!screenPlane->createVertexArray(0, 1))
			return false;
	}

	void GLExample::lightInit(){
		
		light.ambientTerm = {1, 1, 1};
		light.diffuseTerm = {1, 1, 1};
		light.specularTerm = {1, 1, 1};

		lights.push_back(LightInfo());
        lights.back().radiance = glm::vec3(0.7, 0.7, 0.7);
        lights.back().position = glm::vec3(0.0, 3.0, -2.0);

		lightboxes.push_back(std::make_shared<Cube>());
		lightboxes.back()->createVertexArray(0,1,2,3,4);
		lightboxes.back()->setPosition(lights.back().position);
		lightboxes.back()->setScaling(glm::vec3(0.05, 0.05, 0.05));

		lights.push_back(LightInfo());
        lights.back().radiance = glm::vec3(1.7, 1.7, 1.7);
        lights.back().position = glm::vec3(15.0, 4.0, -8.0);

		lightboxes.push_back(std::make_shared<Cube>());
		lightboxes.back()->createVertexArray(0,1,2,3,4);
		lightboxes.back()->setPosition(lights.back().position);
		lightboxes.back()->setScaling(glm::vec3(0.05, 0.05, 0.05));

		for(int i=0;i<lights.size();i++){
			
			ShadowMapping shadow;
			// TODO: shadow mapping buffers initialization, use struct ShadowMapping object: shadows

			glGenFramebuffers(1, &shadow.depthMapFBO); 
			glGenTextures(1, &shadow.depthMap);
			glBindTexture(GL_TEXTURE_2D, shadow.depthMap);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, 
					shadow.width, shadow.height, 0, GL_RG, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			
			glGenRenderbuffers(1, &shadow.depthBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, shadow.depthBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, shadow.width, shadow.height);

			glBindFramebuffer(GL_FRAMEBUFFER, shadow.depthMapFBO);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadow.depthMap, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, shadow.depthBuffer);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
				std::cerr << "Framebuffer not complete!" << std::endl;
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glBindTexture(GL_TEXTURE_2D, 0);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);

			//frame buffer for compute variance
			glGenFramebuffers(2, shadow.varianceFBO);
			glGenTextures(2, shadow.varianceTexture);

			for (int i = 0; i < 2; ++i) {
				glBindFramebuffer(GL_FRAMEBUFFER, shadow.varianceFBO[i]);
				glBindTexture(GL_TEXTURE_2D, shadow.varianceTexture[i]);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, shadow.width, shadow.height, 0, GL_RG, GL_FLOAT, nullptr);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadow.varianceTexture[i], 0);
			}
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
				std::cerr << "Framebuffer not complete!" << std::endl;
			}

			shadows.push_back(shadow);
		}

		// manylight
		for (unsigned int i = 0; i < MANY_LIGHT_NUM; i++)
			{
				// calculate slightly random offsets
				float xPos = static_cast<float>(((rand() % 100) / 100.0) * manyLightRadius - 3.0);
				float yPos = static_cast<float>(((rand() % 100) / 100.0) * manyLightRadius - 4.0);
				float zPos = static_cast<float>(((rand() % 100) / 100.0) * manyLightRadius - 3.0);
				manyLightsPositions.push_back(glm::vec3(xPos, yPos, zPos));

				manyLightsboxes.push_back(std::make_shared<Cube>());
				manyLightsboxes.back()->createVertexArray(0,1,2,3,4);
				manyLightsboxes.back()->setPosition(manyLightsPositions.back());
				manyLightsboxes.back()->setScaling(glm::vec3(0.01, 0.01, 0.01));
				
				// also calculate random color
				float rColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.)
				float gColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.)
				float bColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.)
				manyLightsColors.push_back(glm::vec3(rColor, gColor, bColor));
			}
	}

	void GLExample::updateGUI(){
		//**********---------------------- UI settings ----------------------**********// 
		ImGui_ImplGlfwGL3_NewFrame();
		{
			ImGui::Begin("Center control");
				
			ImGui::Checkbox("Deferred Rendering", &isDefer);
			ImGui::End();
		}
		{
			ImGui::Begin("Furina Controll");
			ImGui::Text("Position:");
			ImGui::SliderFloat("X", &fufu.objectPosition.x, -10.0f, 10.0f, "%.2f");
			ImGui::SliderFloat("Y", &fufu.objectPosition.y, -15.0f, 10.0f, "%.2f");
			ImGui::SliderFloat("Z", &fufu.objectPosition.z, -10.0f, 10.0f, "%.2f");

			ImGui::End();
		}

		{
			ImGui::Begin("Gun Controll");
			ImGui::Text("Position:");
			ImGui::SliderFloat("X", &gun.objectPosition.x, -10.0f, 10.0f, "%.2f");
			ImGui::SliderFloat("Y", &gun.objectPosition.y, -10.0f, 10.0f, "%.2f");
			ImGui::SliderFloat("Z", &gun.objectPosition.z, -10.0f, 10.0f, "%.2f");

			ImGui::End();
		}

		{
			ImGui::Begin("Box Controll");
			ImGui::Text("Position:");
			ImGui::SliderFloat("X", &cube->objectPosition.x, -10.0f, 10.0f, "%.2f");
			ImGui::SliderFloat("Y", &cube->objectPosition.y, -10.0f, 10.0f, "%.2f");
			ImGui::SliderFloat("Z", &cube->objectPosition.z, -10.0f, 10.0f, "%.2f");

			ImGui::End();
		}

		if(isDefer){
			{
				ImGui::Begin("Many Light Controll");
				ImGui::Text("Position:");
				ImGui::SliderFloat("X", &manyLightPosition.x, -15.0f, 15.0f, "%.2f");
				ImGui::SliderFloat("Y", &manyLightPosition.y, -15.0f, 15.0f, "%.2f");
				ImGui::SliderFloat("Z", &manyLightPosition.z, -15.0f, 15.0f, "%.2f");
				ImGui::Text("Intensity:");
				ImGui::SliderFloat("", &manyLightIntensity, 0.0, 10.0f, "%.2f");
				ImGui::Text("SSAORadius:");
				ImGui::SliderFloat("SSAORadius", &SSAOradius, 0.1f, 3.0f, "%.2f");
				ImGui::End();
			}
			{
				ImGui::Begin("Rendering Controll");
				ImGui::Text("Output");
				ImGui::RadioButton("All", &deferRenderOutput, 0);
				ImGui::RadioButton("posotion", &deferRenderOutput, 1);ImGui::SameLine();
				ImGui::RadioButton("depth", &deferRenderOutput, 6);ImGui::SameLine();
				ImGui::RadioButton("albedo", &deferRenderOutput, 2);
				ImGui::RadioButton("normal", &deferRenderOutput, 3);
				ImGui::RadioButton("metalness", &deferRenderOutput, 4);ImGui::SameLine();
				ImGui::RadioButton("roughness", &deferRenderOutput, 5);
				ImGui::RadioButton("ssao", &deferRenderOutput, 7);

				ImGui::Text("SSAORadius:");
				ImGui::SliderFloat("SSAORadius", &SSAOradius, 0.1f, 3.0f, "%.2f");
				ImGui::Text("SSAOblurSize:");
				ImGui::SliderInt("SSAOblurSize", &SSAOblurSize, 0, 6);
				
				ImGui::End();
			}
		}
		else{

		{
			ImGui::Begin("Shadow Render");
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::Text("Shadow Type: ");
			ImGui::RadioButton("No Shadow", (int *)&shadowType, 0); ImGui::SameLine();
            ImGui::RadioButton("basic shadow", (int *)&shadowType, 1); ImGui::SameLine();
			ImGui::RadioButton("PCF", (int *)&shadowType, 2); ImGui::SameLine();
			
			ImGui::RadioButton("PCSS with VSSM", (int *)&shadowType, 3); 
			if(shadowType == 2)
				ImGui::SliderInt("PCF Sample Numbers",  (int *)&PCF_samples, 0, 10);
			if(shadowType == 3)
				ImGui::SliderFloat("Light Size", &lightSize, 0.1f, 100.0f);
			ImGui::Checkbox("Display shadow map", &disPlay_shadowMap);
			ImGui::End();
		}



		{
			ImGui::Begin("Static Light Controll");
			ImGui::Text("Position:");
			ImGui::SliderFloat("X", &lights[1].position.x, -15.0f, 15.0f, "%.2f");
			ImGui::SliderFloat("Y", &lights[1].position.y, -15.0f, 15.0f, "%.2f");
			ImGui::SliderFloat("Z", &lights[1].position.z, -15.0f, 15.0f, "%.2f");
			ImGui::Text("Radiance:");
			ImGui::InputFloat3("RBG", &lights[1].radiance.x, 2);
			ImGui::End();
		}

		
		

		{
			ImGui::Begin("Rendering Controll");
			ImGui::Text("model: ");
			ImGui::Checkbox("Model Normal", &showNormal);
			ImGui::Text("shading: ");
			ImGui::Checkbox("Diffuse Term", &showDiffuseTerm);
			ImGui::Checkbox("SpecularTerm Term", &showSpecular);
			ImGui::Checkbox("Colormap", &showColor);
			ImGui::Text("Ambient factor(Enviorment)");
			ImGui::SliderFloat(" ", &ambientFactor, 0, 1.0f, "%.1f");
			ImGui::End();
		}

		}
		

		// {
		// 	ImGui::Begin("PBR Debuger");
				
		// 	ImGui::Checkbox("PBR", &isPBR);
		// 	// 	ImGui::SliderFloat("Yaw", &cam.Yaw, -180.0f, 180.0f);
		// 	ImGui::End();
		// }


		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

		/* Poll for and process events */
		glfwPollEvents();

	}

	void GLExample::processInput(GLFWwindow* window) {
		if (glfwGetKey(window, GLFW_KEY_W)){
			cam.ProcessKeyboard(FORWORD, deltaTime);
		}
		if (glfwGetKey(window, GLFW_KEY_S))
			cam.ProcessKeyboard(BACKWORD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A))
			cam.ProcessKeyboard(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D))
			cam.ProcessKeyboard(RIGHT, deltaTime);

		if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, 1);
		}
	}

	void GLExample::shadow_mapping(const ShadowMapping & shadow, const glm::mat4 & lightSpaceMatrix)
	{
		// TODO: render to the depth buffer
		glBindFramebuffer(GL_FRAMEBUFFER, shadow.depthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadow.depthMap, 0);
		// glDrawBuffer(GL_NONE);
		// glReadBuffer(GL_NONE);
		
		glViewport(0, 0, shadow.width, shadow.height);
		
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		programForShadows->bind();
		
		//programForShadows->setUniformMat4fv("mvpMatrix", lightSpaceMatrix);
		glUniformMatrix4fv(programForShadows->getUniformLocation("mvpMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);
		//programForShadows->setUniformMat4fv("modelMatrix", cube->getModelMatrix());
		glUniformMatrix4fv(programForShadows->getUniformLocation("modelMatrix"), 1, GL_FALSE, &cube->getModelMatrix()[0][0]);
		cube -> draw();
		//programForShadows->setUniformMat4fv("modelMatrix", torus->getModelMatrix());
		// glUniformMatrix4fv(programForShadows->getUniformLocation("modelMatrix"), 1, GL_FALSE, &torus->getModelMatrix()[0][0]);
		// torus -> draw();
		glUniformMatrix4fv(programForShadows->getUniformLocation("modelMatrix"), 1, GL_FALSE, &ground->getModelMatrix()[0][0]);
		ground -> draw();
		glUniformMatrix4fv(programForShadows->getUniformLocation("modelMatrix"), 1, GL_FALSE, &gun.getModelMatrix()[0][0]);
		gun.draw();
		glUniformMatrix4fv(programForShadows->getUniformLocation("modelMatrix"), 1, GL_FALSE, &fufu.getModelMatrix()[0][0]);
		fufu.draw();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
	}

	void GLExample::computeSAT(const ShadowMapping & shadow){
		programForSAT->bind();
		//下面这一句疯狂报错，segmentation fault
		glBindImageTexture(0, shadow.depthMap, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
		glBindImageTexture(1, shadow.varianceTexture[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
		glDispatchCompute(shadow.width, 1, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		glBindImageTexture(0, shadow.varianceTexture[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
		glBindImageTexture(1, shadow.varianceTexture[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
		glDispatchCompute(shadow.width, 1, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}

	void GLExample::renderSkybox(){
		
		glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
		glm::mat4 view = glm::mat4(glm::mat3(cam.getViewMatrix())); // remove translation from the view matrix
		glm::mat4 projection = cam.getProjectionMatrix(); // remove translation from the view matrix

		programForSkybox->setUniformMat4fv("view", view);
		programForSkybox->setUniformMat4fv("projection", projection);

        programForSkybox->bind();
		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyBosxtex->getTexHandle());
        // skybox cube
		skybox->draw();
 		glDepthFunc(GL_LESS); // set depth function back to default
		programForSkybox->unbind();
	}

	void GLExample::renderPlane(const GLuint id){
		programForPlane->bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D,  id);
		//glBindTexture(GL_TEXTURE_2D, id);
		//glBindTexture(GL_TEXTURE_2D, shadows[0].varianceTexture[1]);
		glUniform1i(programForPlane->getUniformLocation("screenTexture"), 0);
		screenPlane->draw();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		programForPlane->unbind();
	}

	void GLExample::addMultipleLightVariables(const std::shared_ptr<ShaderProgram> & program)
	{
		
		program->setUniform3fv("camPos", cam.getPosition());
		program->setUniformf("ambientFactor", ambientFactor);

		int c = 0;
		
        for (auto tlight: lights) {
            program->setUniform3fv("lights[" + std::to_string(c) + "].position", tlight.position);
            program->setUniform3fv("lights[" + std::to_string(c) + "].radiance", tlight.radiance);
			program->setUniformf("light.lightSize", lightSize);
            c++;
        }
	}

	void GLExample::addShadowVariables(const std::shared_ptr<ShaderProgram> & program, const std::vector<glm::mat4> & lightSpaceMatrixes)
	{
		program->setUniformi("shadowtype", shadowType);
		program->setUniformi("PCF_samples", PCF_samples);
		program->setUniformf("u_TextureSize", shadows[0].width);
					// glUniform1f(programForPBR->getUniformLocation("u_TextureSize"), shadows[0].width);
		

		for(int i =0;i<lights.size();i++){
				program->addTexture("u_DepthMap[" + std::to_string(i) + "]", shadows[i].depthMap);
				program->addTexture("u_DepthSAT[" + std::to_string(i) + "]", shadows[i].varianceTexture[1]);
				program->setUniformMat4fv("lightSpaceMatrixes[" + std::to_string(i) + "]", lightSpaceMatrixes[i]);
		}
	}

	void GLExample::renderGround(const std::shared_ptr<ShaderProgram> & program){

			mvpMatrix = cam.getViewProjectionMatrix() * ground->getModelMatrix();

			program->setUniformMat4fv("mvpMatrix", mvpMatrix);
       		program->setUniformMat4fv("modelMatrix", ground->getModelMatrix());
			program->setUniformi("useDiffuseTexture", 0);

			ground->draw(cam.getProjectionMatrix(), cam.getViewMatrix(), program, false, nullptr);
	}

	void GLExample::renderCubes(const std::vector<glm::mat4> & lightSpaceMatrixes)
	{
		if(isPBR){
			
			mvpMatrix = cam.getViewProjectionMatrix() * cube->getModelMatrix();
			programForPBR->setUniformMat4fv("mvpMatrix", mvpMatrix);
       		programForPBR->setUniformMat4fv("modelMatrix", cube->getModelMatrix());
			programForPBR->setUniformi("useDiffuseTexture", 1);

			cube->draw(cam.getProjectionMatrix(), cam.getViewMatrix(), programForPBR, false, nullptr);
		}
	}

	void GLExample::renderLightBox(bool isMany)
	{
		programForLightBox->bind();
		if(!isMany){
			for(int i = 0; i<lights.size();i++){
				mvpMatrix = cam.getViewProjectionMatrix() * lightboxes[i]->getModelMatrix();
				glUniform3fv(programForLightBox->getUniformLocation("objectColor"), 1,  &lights[i].radiance[0]);
				glUniformMatrix4fv(programForLightBox->getUniformLocation("mvpMatrix"), 1, GL_FALSE, &mvpMatrix[0][0]);
				lightboxes[i]->draw();
			}

		}else{
			for(int i = 0; i<manyLightsboxes.size(); i++){
				mvpMatrix = cam.getViewProjectionMatrix() * manyLightsboxes[i]->getModelMatrix();
				glUniform3fv(programForLightBox->getUniformLocation("objectColor"), 1,  &manyLightsColors[i][0]);
				glUniformMatrix4fv(programForLightBox->getUniformLocation("mvpMatrix"), 1, GL_FALSE, &mvpMatrix[0][0]);
				manyLightsboxes[i]->draw();
			}
		}
		
		programForLightBox->unbind();
	}

	bool GLExample::end()
	{
		programForShape->deleteShaderProgramFromGPU();
		programForTorusNormals->deleteShaderProgramFromGPU();
		return true;
	}
}

