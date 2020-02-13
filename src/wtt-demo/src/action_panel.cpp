#include "action_panel.hpp"
#include "ui_action_panel.h"

#include <QFile>
#include <QDebug>
#include <QEvent>
#include <QScreen>
ActionPanel::ActionPanel(QWidget* parent):
QWidget(parent),
ui_ptr_(new Ui::ActionPanel)
{
  ui_ptr_->setupUi(this);
  initWidgets();
  initGeometry();
  initStyleSheet();
  initConnections();
}



ActionPanel::~ActionPanel()
{
  delete ui_ptr_;
}

void ActionPanel::initConnections() {
  connect(ui_ptr_->openfile_button, &QPushButton::clicked, std::bind(&ActionPanel::userAction, this, OPENMESH));
  connect(ui_ptr_->reset_button, &QPushButton::clicked, std::bind(&ActionPanel::userAction, this, RESETMESH));
  connect(ui_ptr_->fwt_button, &QPushButton::clicked, std::bind(&ActionPanel::userAction, this, FWT));
  connect(ui_ptr_->iwt_button, &QPushButton::clicked, std::bind(&ActionPanel::userAction, this, IWT));
  connect(ui_ptr_->compress_button, &QPushButton::clicked, std::bind(&ActionPanel::userAction, this, COMPRESS));
  connect(ui_ptr_->denoise_button, &QPushButton::clicked, std::bind(&ActionPanel::userAction, this, DENOISE));
  connect(ui_ptr_->type_button, &QPushButton::clicked, std::bind(&ActionPanel::userAction, this, SETTYPE));
}

void ActionPanel::initWidgets() {
  // fwt_effect_ = new QGraphicsOpacityEffect(ui_ptr_->fwt_button);
  // iwt_effect_ = new QGraphicsOpacityEffect(ui_ptr_->iwt_button);
  // compress_effect_ = new QGraphicsOpacityEffect(ui_ptr_->compress_button);
  // denoise_effect_ = new QGraphicsOpacityEffect(ui_ptr_->denoise_button);
  // fwt_effect_->setOpacity(0.25);
  // iwt_effect_->setOpacity(0.25);
  // compress_effect_->setOpacity(0.25);
  // denoise_effect_->setOpacity(0.25);
  // ui_ptr_->fwt_button->setGraphicsEffect(fwt_effect_);
  // ui_ptr_->iwt_button->setGraphicsEffect(iwt_effect_);
  // ui_ptr_->compress_button->setGraphicsEffect(compress_effect_);
  // ui_ptr_->denoise_button->setGraphicsEffect(denoise_effect_);

  ui_ptr_->reset_button->setDisabled(true);
  ui_ptr_->type_button->setDisabled(true);
  ui_ptr_->fwt_button->setDisabled(true);
  ui_ptr_->iwt_button->setDisabled(true);
  ui_ptr_->compress_button->setDisabled(true);
  ui_ptr_->denoise_button->setDisabled(true);
  ui_ptr_->type_text->setAlignment(Qt::AlignCenter);

  ui_ptr_->openfile_button->setToolTip("Load mesh");
  ui_ptr_->reset_button->setToolTip("Reset mesh");
  ui_ptr_->type_button->setToolTip("Set wavelet transform type");
  ui_ptr_->fwt_button->setToolTip("Do forward wavelet transform");
  ui_ptr_->iwt_button->setToolTip("Do inverse wavelet transform");
  ui_ptr_->compress_button->setToolTip("Set compression rate");
  ui_ptr_->denoise_button->setToolTip("Set denoise level");
}

void ActionPanel::initStyleSheet()
{
  QFile qss_file(":/qss/action_panel.qss");

  if (qss_file.open(QFile::ReadOnly))
  {
    this->setStyleSheet(QString(qss_file.readAll()));
  }
  else
  {
    qCritical() << "[GUI][ActionPanel] Error applying qss: " << qss_file.fileName();
  }

  ui_ptr_->openfile_icon->setPixmap(QPixmap(":/images/folder.png"));
  ui_ptr_->openfile_icon->setScaledContents(true);
  ui_ptr_->reset_icon->setPixmap(QPixmap(":/images/reset.png"));
  ui_ptr_->reset_icon->setScaledContents(true);
  ui_ptr_->iwt_icon->setPixmap(QPixmap(":/images/backward.png"));
  ui_ptr_->iwt_icon->setScaledContents(true);
  ui_ptr_->fwt_icon->setPixmap(QPixmap(":/images/forward.png"));
  ui_ptr_->fwt_icon->setScaledContents(true);
  ui_ptr_->compress_icon->setPixmap(QPixmap(":/images/sort.png"));
  ui_ptr_->compress_icon->setScaledContents(true);
  ui_ptr_->denoise_icon->setPixmap(QPixmap(":/images/equalizer.png"));
  ui_ptr_->denoise_icon->setScaledContents(true);
  ui_ptr_->type_icon->setPixmap(QPixmap(":/images/circle.png"));
  ui_ptr_->type_icon->setScaledContents(true);
}

void ActionPanel::initGeometry()
{
  qreal scale = qApp->primaryScreen()->logicalDotsPerInch() / 96.0;
  ui_ptr_->openfile_button->setMinimumSize(QSize(48 * scale, 48 * scale));
  ui_ptr_->reset_button->setMinimumSize(QSize(48 * scale, 48 * scale));
  ui_ptr_->iwt_button->setMinimumSize(QSize(48 * scale, 48 * scale));
  ui_ptr_->fwt_button->setMinimumSize(QSize(48 * scale, 48 * scale));
  ui_ptr_->compress_button->setMinimumSize(QSize(48 * scale, 48 * scale));
  ui_ptr_->denoise_button->setMinimumSize(QSize(48 * scale, 48 * scale));
  ui_ptr_->type_button->setMinimumSize(QSize(48 * scale, 48 * scale));
}

QSize ActionPanel::minimumSizeHint() const
{
  QSize min_size_hint(0, 0);
  int max_height = -1;

  for (int index = 0; index < ui_ptr_->h_layout->count(); ++index)
  {
    QWidget* item = ui_ptr_->h_layout->itemAt(index)->widget();
    if (!item) {
      continue;
    }
    min_size_hint.rwidth() += item->width();
    if (item->height() > max_height)
    {
      max_height = item->height();
    }
  }

  min_size_hint.rheight() = max_height;
  qDebug() << "[Floating Widget] size hint: " << min_size_hint;
  return min_size_hint;
}

void ActionPanel::resizeEvent(QResizeEvent* e)
{
  QWidget::resizeEvent(e);
}

void ActionPanel::onOpenMeshDone(bool succ) {
  if (succ) {
    ui_ptr_->reset_button->setEnabled(true);
  } else {
    ui_ptr_->reset_button->setDisabled(true);
  }
  ui_ptr_->type_icon->setPixmap(QPixmap(":/images/circle.png"));
  ui_ptr_->type_button->setDisabled(false);
  ui_ptr_->fwt_button->setDisabled(true);
  ui_ptr_->iwt_button->setDisabled(true);
  ui_ptr_->compress_button->setDisabled(true);
  ui_ptr_->denoise_button->setDisabled(true);
}


void ActionPanel::onTypeSelected(int t) {
  if (t == 0) {
    ui_ptr_->type_icon->setPixmap(QPixmap(":/images/loop.png"));
  } else {
    ui_ptr_->type_icon->setPixmap(QPixmap(":/images/butterfly.png"));
  }
  ui_ptr_->fwt_button->setEnabled(true);
  ui_ptr_->iwt_button->setEnabled(true);
  ui_ptr_->compress_button->setEnabled(false);
  ui_ptr_->denoise_button->setEnabled(false);
}

void ActionPanel::onFWTDone(bool succ) {
  // if (succ) {
  //   ui_ptr_->fwt_button->setEnabled(false);
  //   ui_ptr_->iwt_button->setEnabled(true);
  //   ui_ptr_->compress_button->setEnabled(true);
  //   ui_ptr_->denoise_button->setEnabled(true);
  //   ui_ptr_->type_button->setEnabled(false);
  // }
  ui_ptr_->denoise_button->setEnabled(true);
  ui_ptr_->compress_button->setEnabled(true);
}

void ActionPanel::onIWTDone(bool succ) {
  // if (succ) {
  //   ui_ptr_->type_button->setEnabled(true);
  //   ui_ptr_->iwt_button->setEnabled(false);
  //   ui_ptr_->fwt_button->setEnabled(true);
  //   ui_ptr_->compress_button->setDisabled(true);
  //   ui_ptr_->denoise_button->setDisabled(true);
  // }
  ui_ptr_->denoise_button->setEnabled(false);
  ui_ptr_->compress_button->setEnabled(false);
}