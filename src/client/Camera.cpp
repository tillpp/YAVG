#include "Camera.hpp"

void Camera::update(Window& window, float delta){

    //camera rotation
    float sensitivity = 0.05f;
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    if(ypos*sensitivity > 89.0 || -89.0 >ypos*sensitivity){
        ypos = glm::clamp(ypos*sensitivity,-89.0,89.0)/sensitivity;
        glfwSetCursorPos(window,xpos,ypos);
    }
    
    auto rotation = glm::mat4(1.f);
    rotation = glm::rotate(rotation, glm::radians(-(float)xpos*sensitivity) , glm::vec3(0.0f, 1.0f, 0.0f));
    rotation = glm::rotate(rotation, glm::radians(-(float)ypos*sensitivity) , glm::vec3(1.0f, 0.0f, 0.0f));
    this->forward = rotation*glm::vec4(0.0f, 0, -1.0f,1.0f);
    this->right   = rotation*glm::vec4(1.0f, 0, 0.0f,1.0f);

    //camera position
    float speed = 4.f;
    if(glfwGetKey(window,GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS){
        speed = 20.f;
    }
    {
        auto forward = glm::normalize(glm::vec3(this->forward.x,0,this->forward.z));
        auto right   = glm::normalize(glm::vec3(this->right.x  ,0,this->right.z  ));
        if(glfwGetKey(window,GLFW_KEY_W) == GLFW_PRESS){
            pos += forward*delta*speed;
        }
        if(glfwGetKey(window,GLFW_KEY_S) == GLFW_PRESS){
            pos -= forward*delta*speed;
        }
        if(glfwGetKey(window,GLFW_KEY_D) == GLFW_PRESS){
            pos += right*delta*speed;
        }
        if(glfwGetKey(window,GLFW_KEY_A) == GLFW_PRESS){
            pos -= right*delta*speed;
        }
    }
    if(glfwGetKey(window,GLFW_KEY_SPACE) == GLFW_PRESS){
        pos += glm::vec3(0.f,1.f,0.f)*delta*speed;
    }
    if(glfwGetKey(window,GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
        pos -= glm::vec3(0.f,1.f,0.f)*delta*speed;
    }
}