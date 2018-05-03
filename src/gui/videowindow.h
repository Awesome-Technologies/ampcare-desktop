#ifndef VIDEOWINDOW_H
#define VIDEOWINDOW_H

#include <QMainWindow>
#include <QWebEnginePage>

namespace OCC {

namespace Ui {
class VideoWindow;
}

class VideoWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit VideoWindow(QWidget *parent = 0);
    ~VideoWindow();

    void setUrl(const QUrl &url);

public slots:
	void slotFeaturePermissionRequested(const QUrl &securityOrigin, QWebEnginePage::Feature feature);

private:
    Ui::VideoWindow *ui;
};
}

#endif // VIDEOWINDOW_H
