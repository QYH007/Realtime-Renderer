#include "GLExample.h"
#include "Cube.h"
#include <imgui/imgui.h>
// UI dependencies
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw_gl3.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <iostream>


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
		
		programForSAT = std::make_shared<ComputingShaderProgram>(std::string(SHADER_DIR) + "/ComputeSAT");
		programForSAT->bind();
		programForSAT->SetUniform1i("input_image", 0);
		programForSAT->SetUniform1i("output_image", 1);

		lightInit();
		sceneInit();

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
		
		return true;
	}

	bool GLExample::render()
	{
		//***********----------------Control and frame-----------------**********************//
		currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		update();
		cam.updateCameraData();
		//***********----------------First Pass rendering from light view space-----------------**********************//
		
		glEnable(GL_DEPTH_TEST);
		std::vector<glm::mat4> lightSpaceMatrixes;
		glm::mat4 lightSpaceMatrix;
		float near_plane =0.0f, far_plane = 150.0f;
		glm::mat4 lightProjection = glm::ortho(-40.0f, 40.0f, -40.0f, 40.0f, near_plane, far_plane);
		// TODO: compute lightSpaceMatrix
		// for(int i = 0; i<lights.size(); i++){
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

		renderLightBox();
		if(disPlay_shadowMap)
			renderPlane();
		if(isPBR){
			addMultipleLightVariables(programForPBR);
			addShadowVariables(programForPBR, lightSpaceMatrixes);
		}

		renderCubes(lightSpaceMatrixes);
		renderGround(lightSpaceMatrixes);

		programForPBR->setUniformi("useDiffuseTexture", 1);
		programForPBR->setUniformi("showNormal", showNormal);
		programForPBR->setUniformi("showDiffuseTerm", showDiffuseTerm);
		programForPBR->setUniformi("showSpecular", showSpecular);
		programForPBR->setUniformi("showColor", showColor);

		gun.draw(cam.getProjectionMatrix(), cam.getViewMatrix(), programForPBR);
		fufu.draw(cam.getProjectionMatrix(), cam.getViewMatrix(), programForPBR);
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
        lights.back().radiance = glm::vec3(0.7, 0.7, 0.7);
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

	}
	void GLExample::updateGUI(){
		//**********---------------------- UI settings ----------------------**********// 
		ImGui_ImplGlfwGL3_NewFrame();
        // ImGui_ImplGlfw_NewFrame();
		{
			// ImGui::Begin("Lamp");
			//lamp control
			// ImGui::Text("Lamp control: ");
			// ImGui::SliderFloat3("Light position", &pointLight.Position.x, -5.0f, 5.0f);
			// ImGui::SliderFloat("Light width", &lightWidth, 2.0f, 250.0f);
			// ImGui::ColorEdit3("Light color", &pointLight.Color.x);
		}
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
			ImGui::Begin("Furina Controll");
			ImGui::Text("Position:");
			ImGui::SliderFloat("X", &fufu.objectPosition.x, -10.0f, 10.0f, "%.2f");
			ImGui::SliderFloat("Y", &fufu.objectPosition.y, -10.0f, 10.0f, "%.2f");
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
		// glUniform1i(programForSAT->getUniformLocation("input_image"), 0);
		//glBindImageTexture(0, shadows.SAT[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
		// glUniform1i(programForSAT->getUniformLocation("output_image"), 1);
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

	void GLExample::renderPlane(){
		programForPlane->bind();

		glActiveTexture(GL_TEXTURE0);
		
		//glBindTexture(GL_TEXTURE_2D,  cubeDiffuse->getTexHandle());
		glBindTexture(GL_TEXTURE_2D, shadows[0].depthMap);
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

	void GLExample::renderGround(const std::vector<glm::mat4> & lightSpaceMatrixes){
		if(isPBR){

			mvpMatrix = cam.getViewProjectionMatrix() * ground->getModelMatrix();

			programForPBR->setUniformMat4fv("mvpMatrix", mvpMatrix);
       		programForPBR->setUniformMat4fv("modelMatrix", ground->getModelMatrix());
			programForPBR->setUniformi("useDiffuseTexture", 0);

			ground->draw(cam.getProjectionMatrix(), cam.getViewMatrix(), programForPBR, false, nullptr);

		 }
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

	void GLExample::renderLightBox()
	{
		programForLightBox->bind();
		for(int i = 0; i<lights.size();i++){
			mvpMatrix = cam.getViewProjectionMatrix() * lightboxes[i]->getModelMatrix();
			glUniform3fv(programForLightBox->getUniformLocation("objectColor"), 1,  &lights[i].radiance[0]);
			glUniformMatrix4fv(programForLightBox->getUniformLocation("mvpMatrix"), 1, GL_FALSE, &mvpMatrix[0][0]);
			lightboxes[i]->draw();
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

