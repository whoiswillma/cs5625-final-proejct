//
//  ImgGUI.cpp
//  Demo
//
//  Created by eschweic on 1/23/19.
//

#include "ImgGUI.hpp"
#include <nanogui/window.h>
#include <glm/glm.hpp>

#include <cpplocate/cpplocate.h>

#include "GLWrap/Util.hpp"
#include "GLWrap/Program.hpp"
#include "GLWrap/Texture2D.hpp"
#include "GLWrap/Mesh.hpp"

namespace RTUtil {

ImgGUI::ImgGUI(int width, int height) :
windowWidth(width),
windowHeight(height),
nanogui::Screen(nanogui::Vector2i(width, height), "NanoGUI Demo", false) {

  // Look up paths to shader source code
  const std::string resourcePath =
    cpplocate::locatePath("resources/Common", "", nullptr) + "resources/";
  const std::string fsqVertSrcPath = resourcePath + "Common/shaders/fsq.vert";
  const std::string srgbFragSrcPath = resourcePath + "Common/shaders/srgb.frag";

  // Compile shader program
  try {
    srgbShader.reset(new GLWrap::Program("srgb", {
      { GL_VERTEX_SHADER, fsqVertSrcPath }, 
      { GL_FRAGMENT_SHADER, srgbFragSrcPath }
    }));
  } catch (std::runtime_error& error) {
    std::cerr << error.what() << std::endl;
    exit(1);
  }

  // Upload a two-triangle mesh for drawing a full screen quad
  std::vector<glm::vec3> positions = {
    glm::vec3(-1.0f, -1.0f, 0.0f),
    glm::vec3(1.0f, -1.0f, 0.0f),
    glm::vec3(1.0f, 1.0f, 0.0f),
    glm::vec3(-1.0f, 1.0f, 0.0f),
  };

  std::vector<glm::vec2> texCoords = {
    glm::vec2(0.0f, 0.0f),
    glm::vec2(1.0f, 0.0f),
    glm::vec2(1.0f, 1.0f),
    glm::vec2(0.0f, 1.0f),
  };

  fsqMesh.reset(new GLWrap::Mesh());

  fsqMesh->setAttribute(srgbShader->getAttribLocation("vert_position"), positions);
  fsqMesh->setAttribute(srgbShader->getAttribLocation("vert_texCoord"), texCoords);

  // Allocate texture
  imgTex.reset(new GLWrap::Texture2D(
    glm::ivec2(windowWidth, windowHeight), GL_RGBA32F));

  // Allocate image for display
  img_data = std::vector<float>(0.0f, windowWidth * windowHeight * 3);

  // Arrange windows in the layout we have described
  perform_layout();
  // Make the window visible and start the application's main loop
  set_visible(true);
}



ImgGUI::~ImgGUI() {
}


bool ImgGUI::keyboard_event(int key, int scancode, int action, int modifiers) {

  // Parent gets first chance to handle event.
  if (Screen::keyboard_event(key, scancode, action, modifiers))
    return true;

  // If the user presses the escape key...
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    // ...exit the application.
    set_visible(false);
    return true;
  }

  // Up and down arrows to adjust exposure
  if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
    exposure *= 2.0;
    return true;
  }
  if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
    exposure /= 2.0;
    return true;
  }

  // Otherwise, the event is not handled here.
  return false;
}


void ImgGUI::draw_contents() {

  // Clear (hardly necessary but makes it easier to recognize viewport issues)
  glClearColor(0.0, 0.2, 1.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);

  // Call subclass to update the displayed image
  compute_image();

  // Copy image data to OpenGL
  glBindTexture(GL_TEXTURE_2D, imgTex->id());
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, img_data.data()); 

  // Set up shader to convert to sRGB and write to default framebuffer
  srgbShader->use();
  imgTex->bindToTextureUnit(0);
  srgbShader->uniform("image", 0);
  srgbShader->uniform("exposure", exposure);

  // Set viewport
  const nanogui::Vector2i framebuffer_size = Screen::framebuffer_size();
  glViewport(0, 0, framebuffer_size.x(), framebuffer_size.y());

  // Draw the full screen quad
  fsqMesh->drawArrays(GL_TRIANGLE_FAN, 0, 4);

  GLWrap::checkGLError();

}

}
