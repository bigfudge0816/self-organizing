#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <chrono>

#include "BoundingBox.hpp"
#include "Color.hpp"
#include "Tree.hpp"
#include "Types.hpp"
#include "UserAction.hpp"

class OpenGlWindow {
  GLFWwindow *window = nullptr;

  GLuint openGlCylinderProgram = -1;
  GLuint openGlCylinderVertexBufferArray = -1;
  GLuint openGlCylinderProgramVertexColorUniformLocation = -1;

  GLuint openGlCylinderProgramModelMatrixUniformLocation = -1;
  GLuint openGlCylinderProgramModelInverseTransposedMatrixUniformLocation = -1;
  GLuint openGlCylinderProgramViewMatrixUniformLocation = -1;
  GLuint openGlCylinderProgramProjectionMatrixUniformLocation = -1;

  GLuint openGlCylinderProgramCameraPositionInWorldUniformLocation = -1;
  GLuint openGlCylinderProgramLightPositionInWorldUniformLocation = -1;
  GLuint openGlCylinderProgramAmbientLightIntensityUniformLocation = -1;

  U64 drawCalls = 0;

  std::chrono::steady_clock::time_point lastUpdate = std::chrono::steady_clock::now();

  float fov = 2.0f * std::atan(1.0f);

  Point cameraPosition{0.0f, 0.5f, 1.0f};
  Point lookAtPosition{0.0f, 0.0f, 1.0f};

  void initializePrograms();

  void setUpVertexArrays();

  void drawMetamers(const std::unique_ptr<Metamer> &metamer);

  void updateCameraPosition();

public:
  static constexpr U32 DefaultWindowSide = 1024;

  OpenGlWindow();

  virtual ~OpenGlWindow();

  void setCameraForBoundingBox(BoundingBox boundingBox);

  void drawTree(const Tree &tree);

  void setShouldClose();

  bool shouldClose();

  void startDrawing();

  void pollEvents();

  void swapBuffers();
};
