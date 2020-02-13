#ifndef WTT_DEMO_GLVIEW_CONTROL_PANEL_HPP
#define WTT_DEMO_GLVIEW_CONTROL_PANEL_HPP

#include "control_panel.hpp"

class QPushButton;
class GLViewControlPanel: public ControlPanel {
  Q_OBJECT
public:
  enum GLViewActions {
    RESETCAM = 0,
    TOGGLEEDGE = 1,
    SMOOTHSHADING = 2,
    FLATSHADING = 3,
    CAPTUREFRAME = 4
  };
  explicit GLViewControlPanel(QWidget* parent = 0);
  ~GLViewControlPanel();

  void initChildren();
  void initSize();

  QPushButton* toggle_edge_button_;
  QPushButton* reset_cam_button_;
  QPushButton* smooth_shading_button_;
  QPushButton* flat_shading_button_;
  QPushButton* capture_button_;
};
#endif