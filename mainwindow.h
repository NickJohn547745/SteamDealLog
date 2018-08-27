#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWebEngineWidgets/QWebEngineView>
#include <qDebug>
#include <QEventLoop>
#include <QTimer>
#include <QMessageBox>
#include <QUrlQuery>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    void waitForAppIds();

    void getDataFromIds(QStringList ids);

    QString priceToString(int price);

    void reset();

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void webpageReady(bool isReady);
    void replyFinished(QNetworkReply* reply);
    void webPageProgress(int value);

signals:
    void readyForNext();

private slots:
    void on_pushButton_clicked();

    void on_radioButtonPage_toggled(bool checked);

private:
    QWebEngineView *view;
    Ui::MainWindow *ui;
    QStringList appIds;
    QStringList platforms;
    QNetworkAccessManager *manager;
    bool apiResponseReady = false;
    QString currentAppId = "";
    int pageCount = 0;
    QString currentUrl = "";
    bool showHeader = true;
};

#endif // MAINWINDOW_H
