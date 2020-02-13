#ifndef WTT_DEMO_INCLUDE_LOGGER_HPP
#define WTT_DEMO_INCLUDE_LOGGER_HPP

#include <QDebug>

class DebugLogger {
public:
  DebugLogger(const QString& name):name_(name) {}
  QDebug operator()() {
    return qDebug().noquote() << name_;
  }
private:
  QString name_;
};

class FatalLogger {
public:
  FatalLogger(const QString& name):name_(name) {}
  QDebug operator()() {
    return qDebug().noquote() << name_;
  }
private:
  QString name_;
};

#endif