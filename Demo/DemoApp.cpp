//
//  DemoApp.cpp
//  Demo
//
//  Created by eschweic on 1/23/19.
//

#include "DemoApp.hpp"

#include <nanogui/window.h>
#include <nanogui/opengl.h>
#include <nanogui/layout.h>
#include <nanogui/button.h>

#include <glm/gtc/random.hpp>

const int DemoApp::windowWidth = 800;
const int DemoApp::windowHeight = 600;

DemoApp::DemoApp() :
nanogui::Screen(nanogui::Vector2i(windowWidth, windowHeight), "NanoGUI Demo", false),
backgroundColor(0.4f, 0.4f, 0.7f, 1.0f) {
  // Creates a window that has this screen as the parent.
  // NB: even though this is a raw pointer, nanogui manages this resource internally.
  // Do not delete it!
  nanogui::Window *window = new nanogui::Window(this, "Window Demo");
  window->set_position(nanogui::Vector2i(15, 15));
  window->set_layout(new nanogui::GroupLayout());

  // Create a button with the window as its parent.
  // NB: even though this is a raw pointer, nanogui manages this resource internally.
  // Do not delete it!
  nanogui::Button *button = new nanogui::Button(window, "Random Color");
  button->set_callback([&]() {
    nanogui::Vector3f color = nanogui::Vector3f(glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f));
    backgroundColor = nanogui::Color(color);
  });

  // Arrange windows in the layout we have described
  perform_layout();
  // Make the window visible and start the application's main loop
  set_visible(true);
}

bool DemoApp::keyboard_event(int key, int scancode, int action, int modifiers) {
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

  // Otherwise, the event is not handled here.
  return false;
}

void DemoApp::draw_contents() {
  // All OpenGL code goes here!
  glClearColor(backgroundColor.r(), backgroundColor.g(), backgroundColor.b(), backgroundColor.w());
  glClear(GL_COLOR_BUFFER_BIT);
}
