#include "videowindow.h"
#include "ui_videowindow.h"

namespace OCC {

VideoWindow::VideoWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::VideoWindow)
{
    ui->setupUi(this);
    QWebEnginePage *page = ui->webEngineView->page();
    QObject::connect(page, &QWebEnginePage::featurePermissionRequested, this, &VideoWindow::slotFeaturePermissionRequested);
}

VideoWindow::~VideoWindow()
{
    delete ui;
}

void VideoWindow::setUrl(const QUrl &url)
{
    ui->webEngineView->load(url);
}

void VideoWindow::slotFeaturePermissionRequested(const QUrl &securityOrigin, QWebEnginePage::Feature feature)
{
    ui->webEngineView->page()->setFeaturePermission(securityOrigin, feature, QWebEnginePage::PermissionGrantedByUser);
}

} // end namespace
