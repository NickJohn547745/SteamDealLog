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

    void init();

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void runSteamScripts();

    void runHumbleBundleScripts();

public slots:
    void webpageReady(bool isReady);
    void replyFinished(QNetworkReply* reply);
    void webPageProgress(int value);

signals:
    void readyForNext();

    void done();

    void nextOne();

    void finish();

private slots:
    void on_pushButton_clicked();

    void on_radioButtonPage_toggled(bool checked);

private:
    Ui::MainWindow *ui;
    QWebEngineView *view;
    QNetworkAccessManager *manager;

    QString currentAppId = "";
    QString currentUrl = "";

    QStringList appIds;
    QStringList platforms;
    QStringList humbleList;

    bool apiResponseReady = false;
    bool showHeader = true;

    int pageCount = 0;
    int websiteType = 0;//0 = Steam 1 == HumbleBundle
};

#endif // MAINWINDOW_H
