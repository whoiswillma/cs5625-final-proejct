#ifndef CAMERACONTROLLER_HEADER
#define CAMERACONTROLLER_HEADER

// by Eston

#include <memory>
#include <glm/glm.hpp>
#include <nanogui/vector.h>
#include "Camera.hpp"

namespace RTUtil {


/// Translates user input into camera movement around a virtual scene.
/// This class is designed to be compatible with the GLFW interface, but
/// does not explicitly depend on GLFW.
class CameraController {
public:
  /// @param camera The Camera to be controlled.
  CameraController(std::shared_ptr<Camera> camera) : camera(camera) { }

  virtual ~CameraController() { }

  /// Moves the Camera in response to a scroll event.
  /// The default implementation ignores the event.
  /// @param p The position of the mouse in the window.
  /// @param rel The subpixel relative motion of the scrolling event.
  /// @param modifiers Any Modifiers active during the event. See Modifier.
  /// @return @c false if the event is ignored; @c true otherwise.
  virtual bool scroll_event(const nanogui::Vector2i& p, const nanogui::Vector2f& rel,
                           int modifiers = 0) {
    return false;
  }

  /// Moves the Camera in response to a keyboard event.
  /// The default implementation ignores the event.
  /// @param key An identifier of the specific key that was pressed.
  /// @param scancode The system-specific scancode of the key.
  /// @param action The action of the event. See Action.
  /// @param modifiers Any Modifiers active during the event. See Modifier.
  /// @return @c false if the event is ignored; @c true otherwise.
  virtual bool keyboard_event(int key, int scancode, int action,
                             int modifiers) {
    return false;
  }

  /// Moves the Camera in response to a mouse button event.
  /// The default implementation ignores the event.
  /// @param p The position of the mouse in the window.
  /// @param button The identifier of the button pressed. See MouseButton.
  /// @param down @c true if the mouse is down, @c false otherwise.
  /// @param modifiers Any Modifiers active during the event. See Modifier.
  /// @return @c false if the event is ignored; @c true otherwise.
  virtual bool mouse_button_event(const nanogui::Vector2i& p, int button,
                                bool down, int modifiers) {
    return false;
  }

  /// Moves the Camera in response to a mouse motion event.
  /// The default implementation ignores the event.
  /// @param p The position of the mouse in the window.
  /// @param rel The relative motion of the event.
  /// @param button The identifier of the button pressed. See MouseButton.
  /// @param modifiers Any Modifiers active during the event. See Modifier.
  /// @return @c false if the event is ignored; @c true otherwise.
  virtual bool mouse_motion_event(const nanogui::Vector2i& p,
                                const nanogui::Vector2i& rel, int button,
                                int modifiers) {
    return false;
  }

  /// The action defining an input event.
  /// These are compatible with @c GLFW_PRESS etc.
  enum Action {
    Press = 1,
    Release = 0,
    Repeat = 2,
  };

  /// Modifiers accompanying an input event.
  /// These are compatible with @c GLFW_MOD_SHIFT etc.
  enum Modifier {
    Shift = 0x0001,
    Control = 0x0002,
    Alt = 0x0004,
    Super = 0x0008,
  };

  /// Identifiers for specific mouse buttons.
  /// These are compatible with @c GLFW_MOUSE_BUTTON_1 etc.
  enum MouseButton {
    Button1 = 0,
    Button2 = 1,
    Button3 = 2,
    Button4 = 3,
    Button5 = 4,
    Button6 = 5,
    Button7 = 6,
    Button8 = 7,
  };


protected:
  std::shared_ptr<Camera> camera;
};



/// A CameraController that is useful with a mouse interface.
class DefaultCC : public CameraController {
public:
  DefaultCC(std::shared_ptr<Camera> camera) : CameraController(camera) { }

  /// Moves the camera in response to a scroll event.
  /// This zooms the camera using the y-magnitude of the scroll event.
  /// @param p The position of the mouse in the window. This is ignored.
  /// @param rel The subpixel relative motion of the scrolling event.
  /// @param modifiers Any Modifiers active during the event. See Modifier.
  ///        This is ignored.
  /// @return @c true
  virtual bool scroll_event(const nanogui::Vector2i& p, const nanogui::Vector2f& rel,
                           int modifiers = 0) override {
    zoom(rel.y() * 0.01);
    return true;
  }

  /// Moves the Camera in response to a mouse motion event.
  /// If <tt>button == Button2</tt>, this orbits the Camera->
  /// If <tt>button == Button3</tt>, this pans the Camera->
  /// Otherwise, the event is ignored.
  /// @param p The position of the mouse in the window. This is ignored.
  /// @param rel The relative motion of the event.
  /// @param button The identifier of the button pressed. See MouseButton.
  /// @param modifiers Any Modifiers active during the event. See Modifier.
  ///        This is ignored.
  /// @return @c false if the event is ignored; @c true otherwise.
  virtual bool mouse_motion_event(const nanogui::Vector2i& p,
                                const nanogui::Vector2i& rel, int button,
                                int modifiers) override {
    if (button == MouseButton::Button2) {
      orbit(glm::vec2(-rel.x(), -rel.y()) * 0.01f);
      return true;
    }
    if (button == MouseButton::Button3) {
      float scale = glm::length(camera->getEye() - camera->getTarget()) * 0.0005f;
      pan(glm::vec2(-rel.x() * scale, rel.y() * scale));
      return true;
    }
    return false;
  }


protected:
  
  /// Moves the Camera closer or further from its target.
  /// @param delta The distance between the Camera and its target is scaled by
  /// this parameter.
  virtual void zoom(float delta) {
    glm::vec3 gaze = camera->getTarget() - camera->getEye();
    camera->setEye(camera->getEye() + gaze * delta);
  }

  /// Moves the Camera and its target in the direction of the Camera's gaze.
  /// @param d The distance to move the Camera and its target.
  virtual void dolly(float d) {
    glm::vec3 gaze = camera->getTarget() - camera->getEye();
    translate(gaze * d);
  }

  /// Moves the Camera and its target in a given direction.
  /// @param delta The vector to add to the Camera's position and target.
  virtual void translate(const glm::vec3& delta) {
    camera->setEye(camera->getEye() + delta);
    camera->setTarget(camera->getTarget() + delta);
  }

  /// Move the Camera and its target within the view plane.
  /// @param delta The distance to move the camera within the view plane.
  virtual void pan(const glm::vec2& delta) {
    const glm::vec3& r = camera->getRight();
    const glm::vec3& v = camera->getVertical();
    glm::vec3 t = delta.x * r + delta.y * v;
    translate(t);
  }

  /// Orbit the Camera around its target.
  /// @param delta The amount (in radians) to rotate the camera around the
  ///              vertical and right camera axes.
  virtual void orbit(const glm::vec2& delta) {
    const glm::vec3& v = camera->getVertical();
    const glm::vec3& r = camera->getRight();
    const glm::vec3& t = camera->getTarget();

    glm::mat4 T = glm::mat4(1.0);
    T = glm::translate(T, t);
    T = glm::rotate(T, delta.x, v);
    T = glm::rotate(T, delta.y, r);
    T = glm::translate(T, -t);

    camera->setEye(T * glm::vec4(camera->getEye(), 1.0));
  }
};


} // namespace

#endif // CAMERACONTROLLER_HEADER
