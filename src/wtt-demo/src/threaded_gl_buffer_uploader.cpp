#include "threaded_gl_buffer_uploader.hpp"

#include <QOpenGLContext>
#include <QOffscreenSurface>

ThreadedGLBufferUploader::ThreadedGLBufferUploader():
context_(new QOpenGLContext()),
surface_(new QOffscreenSurface()),
debug(DebugLogger("[ThreadedGLBufferUploader]")),
fatal(FatalLogger("[ThreadedGLBufferUploader]"))
{
  connect(this, &ThreadedGLBufferUploader::started, this, &ThreadedGLBufferUploader::initGL);
}

ThreadedGLBufferUploader::~ThreadedGLBufferUploader() {
  if (context_) {
    context_->doneCurrent();
    delete context_;
  }
  if (surface_) {
    delete surface_;
  }
}

void ThreadedGLBufferUploader::initGL() {
  debug() << "Initialize GL";
  context_->makeCurrent(surface_);
  initializeOpenGLFunctions();
  context_->doneCurrent();
  debug() << "Initialize GL Done";
}