#ifndef WTT_DEMO_INCLUDE_THREADED_GL_BUFFER_UPLOADER_HPP
#define WTT_DEMO_INCLUDE_THREADED_GL_BUFFER_UPLOADER_HPP

#include "logger.hpp"

#include <QThread>
#include <QOpenGLFunctions>
class QOpenGLContext;
class QOffscreenSurface;
class ThreadedGLBufferUploader: public QThread, public QOpenGLFunctions
{
  Q_OBJECT
public:
  explicit ThreadedGLBufferUploader();
  virtual ~ThreadedGLBufferUploader();

  virtual QOpenGLContext* getContext() {return context_;}
  virtual QOffscreenSurface* getSurface() {return surface_;}

signals:
  void bufferUploaded();

public slots:
  void initGL();

protected:
  QOpenGLContext* context_;
  QOffscreenSurface* surface_;

  DebugLogger debug;
  FatalLogger fatal;

};
#endif  // define WTT_DEMO_INCLUDE_DATA_LOADER_HPP