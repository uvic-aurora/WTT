#ifndef WTT_DEMO_INCLUDE_ARCBALL_CAMERA_HPP
#define WTT_DEMO_INCLUDE_ARCBALL_CAMERA_HPP

#include <QMatrix4x4>

class ArcballCamera
{
public:
  explicit ArcballCamera(const QVector3D& pos = QVector3D(0.0, 0.0, 0.0),
                         const QVector3D& focus = QVector3D(0.0, 0.0, 0.0),
                         const QVector3D& up = QVector3D(0.0, 1.0, 0.0));

  QMatrix4x4 getViewMatrix() const;
  QMatrix4x4 getViewRotation() const;
  qreal radius() const;

  void drag(const QVector2D& drag);
  void shift(const QVector2D& shift);

  void setPosition(const QVector3D& pos);
  void setPosition(const QVector4D& pos);

  void setFocus(const QVector3D& focus);
  void setFocus(const QVector4D& focus);

  void setUp(const QVector3D& up);
  void setUp(const QVector4D& up);

  void updateRadius();

  const QVector3D& pos() const;
  const QVector3D& focus() const;
  const QVector3D& up() const;

protected:
  QVector3D pos_;
  QVector3D focus_;
  QVector3D up_;
  double radius_;
};


#endif  // define WTT_DEMO_INCLUDE_ARCBALL_CAMERA_HPP