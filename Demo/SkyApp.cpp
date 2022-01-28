//
//  SkyApp.cpp
//
//  Created by srm, April 2020
//

#define _USE_MATH_DEFINES
#include <cmath>

#include "SkyApp.hpp"

#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/slider.h>
#include <nanogui/label.h>

#include <cpplocate/cpplocate.h>


SkyApp::SkyApp() :
nanogui::Screen(nanogui::Vector2i(600, 600), "Sky Demo", false),
sky(30 * M_PI/180, 4) {

  const std::string resourcePath =
        cpplocate::locatePath("resources", "", nullptr) + "resources/";

  // Set up a shader program to visualize the sky
  prog.reset(new GLWrap::Program("program", { 
      { GL_VERTEX_SHADER, resourcePath + "shaders/fsq.vs" },
      { GL_FRAGMENT_SHADER, resourcePath + "shaders/sunsky.fs" },
      { GL_FRAGMENT_SHADER, resourcePath + "shaders/skyvis.fs" }
  }));

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

  fsqMesh->setAttribute(0, positions);
  fsqMesh->setAttribute(1, texCoords);

  // Build the control panel
  buildGUI();
  // Arrange windows in the layout we have described
  perform_layout();
  // Make the window visible and start the application's main loop
  set_visible(true);
}



void SkyApp::buildGUI() {

  // Creates a window that has this screen as the parent.
  // NB: even though this is a raw pointer, nanogui manages this resource internally.
  // Do not delete it!
  controlPanel = new nanogui::Window(this, "Control Panel");
  controlPanel->set_fixed_width(220);
  controlPanel->set_position(nanogui::Vector2i(15, 15));
  controlPanel->set_layout(new nanogui::BoxLayout(nanogui::Orientation::Vertical,
                                                 nanogui::Alignment::Middle,
                                                 5, 5));

  // Create a slider widget that adjusts the sun angle parameter
  nanogui::Widget* elevWidget = new nanogui::Widget(controlPanel);
  elevWidget->set_layout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal,
                                               nanogui::Alignment::Middle,
                                               0, 5));
  new nanogui::Label(elevWidget, "Sun Theta:");
  nanogui::Slider* elevSlider = new nanogui::Slider(elevWidget);
  elevSlider->set_range(std::make_pair(0.0f, 90.0f));
  elevSlider->set_value(30.0f);
  elevSlider->set_callback([this] (float value) {
    sky.setThetaSun(value * M_PI / 180);
  });

  // Create a slider that adjusts the turbidity
  nanogui::Widget* turbWidget = new nanogui::Widget(controlPanel);
  turbWidget->set_layout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal,
                                               nanogui::Alignment::Middle,
                                               0, 5));
  new nanogui::Label(turbWidget, "Turbidity:");
  nanogui::Slider* turbSlider = new nanogui::Slider(turbWidget);
  turbSlider->set_range(std::make_pair(2.0f, 6.0f));
  turbSlider->set_value(4.0f);
  turbSlider->set_callback([this] (float value) {
    sky.setTurbidity(value);
  });
}



bool SkyApp::keyboard_event(int key, int scancode, int action, int modifiers) {
  // First, see if the parent method accepts the keyboard event.
  // If so, the event is already handled.
  if (Screen::keyboard_event(key, scancode, action, modifiers))
    return true;

  // If the user presses the escape key...
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    // ...exit the application.
    set_visible(false);
    return true;
  }

  // Space to show/hide control panel
  if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
    controlPanel->set_visible(!controlPanel->visible());
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

  // The event is not handled here.
  return false;
}


void SkyApp::draw_contents() {

  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT);

  prog->use();
  sky.setUniforms(*prog);
  prog->uniform("exposure", exposure);
  fsqMesh->drawArrays(GL_TRIANGLE_FAN, 0, 4);
  prog->unuse();

}
