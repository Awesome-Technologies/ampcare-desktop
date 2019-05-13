#include "configfile.h"
#include "videowindow.h"
#include "ui_videowindow.h"

#include <QDesktopWidget>
#include <QStyle>
#include <QWebEngineProfile>

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

    // persist cookies to avoid constant need for login
    ConfigFile cfg;
    QString path = cfg.configPath();
    QWebEngineProfile::defaultProfile()->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);
    QWebEngineProfile *defaultProfile = QWebEngineProfile::defaultProfile();
    defaultProfile->setCachePath(cfg.configPath() + "webcache/");
    defaultProfile->setPersistentStoragePath(cfg.configPath() + "webpersistent/");

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
