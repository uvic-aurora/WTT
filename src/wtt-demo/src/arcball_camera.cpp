#include "arcball_camera.hpp"

#include <QVector2D>
#include <QDebug>

#include <cmath>


ArcballCamera::ArcballCamera(const QVector3D& pos,
                             const QVector3D& focus,
                             const QVector3D& up)
                             :pos_(pos),
                              focus_(focus),
                              up_(up)
{
  updateRadius();
}

QMatrix4x4 ArcballCamera::getViewMatrix() const
{
  QMatrix4x4 view;
  view.setToIdentity();
  view.lookAt(pos_, focus_, up_);
  return view;
}

QMatrix4x4 ArcballCamera::getViewRotation() const
{
  QMatrix4x4 r;
  r.setToIdentity();
  r.rotate(QQuaternion::fromDirection(pos_ - focus_, up_));
  return r;
}

qreal ArcballCamera::radius() const
{
  return radius_;
}

void ArcballCamera::updateRadius()
{
  radius_ = (pos_ - focus_).length();
}

void ArcballCamera::drag(const QVector2D &mov)
{
  QVector3D dest(mov, 0.0);
  
  if (dest.length() > 1.0)
  {
    dest.normalize();
  }

  dest.setX(-dest.x() * radius_);
  dest.setY(dest.y() * radius_);
  dest.setZ(std::sqrt(radius_ * radius_ - dest.lengthSquared()));

  QVector4D dest_4d {dest, 1.0};
  // Rotate dest_4d to camera space;
  dest_4d = getViewRotation() * dest_4d;
  dest = dest_4d.toVector3D();


  QVector3D axis(QVector3D::crossProduct(pos_ - focus_, dest));

  qreal angle = std::asin( axis.length() / ((pos_ - focus_).length() * dest.length())) * 2 *180.0 / M_PI;

  axis.normalize();

  QMatrix4x4 r;
  r.setToIdentity();
  r.rotate(angle, axis);

  pos_ = (r * QVector4D(pos_, 1.0)).toVector3D();
  up_ = (r * QVector4D(up_, 0.0)).toVector3D();
  updateRadius();
}

void ArcballCamera::shift(const QVector2D &shift)
{
  QVector3D shift_3d(shift, 0.0);

  if (shift_3d.length() > 1.0)
  {
    shift_3d.normalize();
  }

  shift_3d.setX(-shift_3d.x() * radius_);
  shift_3d.setY(shift_3d.y() * radius_);

  // Rotate shift_3d to camera space
  shift_3d = (getViewRotation() * QVector4D(shift_3d, 0.0)).toVector3D();

  pos_ += shift_3d;
  focus_ += shift_3d;

  updateRadius();
}

void ArcballCamera::setPosition(const QVector3D &pos)
{
  pos_ = pos;
  updateRadius();
}

void ArcballCamera::setPosition(const QVector4D &pos)
{
  pos_ = pos.toVector3D();
  updateRadius();
}

void ArcballCamera::setFocus(const QVector3D &focus)
{
  focus_ = focus;
  updateRadius();
}

void ArcballCamera::setFocus(const QVector4D &focus)
{
  focus_ = focus.toVector3D();
  updateRadius();
}

void ArcballCamera::setUp(const QVector3D &up)
{
  up_ = up;
}

void ArcballCamera::setUp(const QVector4D &up)
{
  up_ = up.toVector3D();
}


const QVector3D& ArcballCamera::pos() const
{
  return pos_;
}

const QVector3D& ArcballCamera::focus() const
{
  return focus_;
}

const QVector3D& ArcballCamera::up() const
{
  return up_;
}