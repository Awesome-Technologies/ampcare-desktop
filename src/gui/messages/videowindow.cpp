#include "videowindow.h"
#include "ui_videowindow.h"

#include <QDesktopWidget>
#include <QStyle>

namespace OCC {

VideoWindow::VideoWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::VideoWindow)
{
    ui->setupUi(this);
    setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            QSize(1200, 750),
            qApp->desktop()->availableGeometry()));
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
