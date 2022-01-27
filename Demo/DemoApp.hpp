//
//  DemoApp.hpp
//  Demo
//
//  Created by eschweic on 1/23/19.
//

#ifndef DemoApp_hpp
#define DemoApp_hpp

#include <nanogui/screen.h>

/// This is a simple app that demonstrates how to use OpenGL commands with nanogui.
/// It inherits from nanogui::Screen, which means it can be used as an application.
class DemoApp : public nanogui::Screen {
public:
  /// Default constructor.
  /// This sets the window size to @c windowWidth by @c windowHeight pixels.
  /// This also sets up child windows and starts the application.
  DemoApp();

  /// Capture a keyboard event.
  /// This simply listens for a press of the @c escape key and exits; all other keys are ignored.
  /// See https://www.glfw.org/docs/latest/input_guide.html#input_key for more info.
  /// @arg key The GLFW key value of the event (GLFW_KEY_A, etc.).
  /// @arg scancode The scancode of the event; unique for every key.
  /// @arg action The GLFW action value of the event (GLFW_PRESS, etc.).
  /// @arg modifiers GLFW modifier flags for any active modifiers (GLFW_MOD_SHIFT, etc.).
  virtual bool keyboard_event(int key, int scancode, int action, int modifiers) override;

  /// Draws the UI and all child windows with the given context.
  virtual void draw(NVGcontext *ctx) override {
    Screen::draw(ctx);
  }

  /// Draws the background of this window.
  /// This is where all OpenGL calls for this window should happen.
  virtual void draw_contents() override;

private:
  /// The width of the application window in pixels.
  static const int windowWidth;
  /// The height of the application window in pixels.
  static const int windowHeight;

  /// The color that is referenced when draw
  nanogui::Color backgroundColor;
};


#endif /* DemoApp_hpp */
