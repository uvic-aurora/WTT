#include <scene_object.hpp>

#include <QOpenGLShaderProgram>

SceneObject::SceneObject(QObject* parent)
: QObject(parent),
  glsl_program_(new QOpenGLShaderProgram(this))                                 
{
}

SceneObject::~SceneObject()
{
  delete glsl_program_;
}