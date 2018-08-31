#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    manager = new QNetworkAccessManager(this);
    view = new QWebEngineView();

    connect(view->page(), SIGNAL(loadProgress(int)), this, SLOT(webPageProgress(int)));

    connect(view, SIGNAL(loadFinished(bool)), this, SLOT(webpageReady(bool)));
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

    init();
    reset();
}

MainWindow::~MainWindow()
{
    delete view;
    delete ui;
}

void MainWindow::webpageReady(bool isReady)
{
    if (isReady)
    {
        if (websiteType == 0)
            runSteamScripts();
        else if (websiteType == 1)
        {
            /*QEventLoop loop;
            QTimer *timer = new QTimer(this);

            connect(timer, SIGNAL(timeout()), &loop, SLOT(quit()));
            connect(this, SIGNAL(finish()), &loop, SLOT(quit()));
            connect(this, SIGNAL(finish()), timer, SLOT(stop()));

            timer->start(5000);*/

            runHumbleBundleScripts();
            //loop.exec();
            qDebug() << "DONE!";
        }
    }
}
void MainWindow::runSteamScripts()
{
    appIds.clear();
    if (showHeader)
    {
        view->page()->runJavaScript("for(var elements=document.getElementsByClassName(\"search_pagination_right\")[0].innerText.replace(\"...\",\"\").replace(\"<\",\"\").replace(\">\",\"\").replace(/\t/g,\" \").split(\" \"),pages=[],i=0;i<elements.length;i++)\"\"!=elements[i].trim()&&pages.push(parseInt(elements[i].trim()));Math.max.apply(null,pages);",
                                    [&](const QVariant &v) {
            qDebug() << 1111;
            pageCount = v.toString().toInt();
            ui->spinBoxStart->setMaximum(pageCount);
            ui->spinBoxEnd->setMaximum(pageCount);
        });
    }

    view->page()->runJavaScript("var elements=document.getElementById(\"search_result_container\").getElementsByTagName(\"div\")[1].getElementsByTagName(\"a\");var platforms=[];for(var i=0;i<elements.length;i++){var e=elements[i].getElementsByClassName(\"responsive_search_name_combined\")[0].getElementsByClassName(\"search_name\")[0].getElementsByTagName(\"p\")[0].getElementsByTagName(\"span\");var vrString=\"\";for(var x=0;x<e.length;x++){if(e[x].getAttribute(\"title\")){vrString+=e[x].getAttribute(\"title\");vrString+=\" \"}}platforms.push(vrString)}platforms;",
                                [&](const QVariant &v) {
        qDebug() << 2222;
        platforms = v.toStringList();
    });

    view->page()->runJavaScript("var elements=document.getElementById(\"search_result_container\").getElementsByTagName(\"div\")[1].getElementsByTagName(\"a\");var idList=[];for(var i=0;i<elements.length;i++){idList.push(elements[i].getAttribute(\"data-ds-appid\"))}idList;",
                                [&](const QVariant &v) {
        qDebug() << 3333;
        appIds = v.toStringList();
        waitForAppIds();
    });
}

void MainWindow::runHumbleBundleScripts()
{
    humbleList.clear();
    view->page()->runJavaScript("if(\"display: none;\"!=document.getElementsByClassName(\"js-loading-overlay overlay\")[0].getAttribute(\"style\")){\"false\"}else{for(var e=document.getElementsByClassName(\"entities-list\")[0].getElementsByClassName(\"entity-block-container js-entity-container\"),t=[],a=0;a<e.length;a++){for(var r=document.getElementsByClassName(\"entities-list\")[0].getElementsByTagName(\"li\")[0].getElementsByClassName(\"operating-system\"),n=\"\",l=0;l<r.length;l++)n+=r[l].title+\" \";t.push(n.trim()+\"|\"+e[a].getElementsByClassName(\"entity-title\")[0].getAttribute(\"title\")+\"|\"+e[a].getElementsByClassName(\"price\")[0].innerHTML.replace(\"$\",\"\").trim())}t}",
                                [&](const QVariant &v) {
        if (v.toString() == "false")
            runHumbleBundleScripts();
        else
        {
            qDebug() << 4444;
            humbleList = v.toStringList();
            qDebug() << humbleList;
            emit finish();
            waitForAppIds();
        }
    });
}

void MainWindow::replyFinished(QNetworkReply *reply)
{
    QByteArray data = reply->readAll();

    QJsonObject apiResponse = QJsonDocument::fromJson(data).object();
    //
    QString platform = QString(platforms[appIds.indexOf(currentAppId)]).replace("HTC Vive", "Vive").replace("Oculus Rift", "Rift").replace("Windows Mixed Reality", "WMR").trimmed().replace(" ", "/").leftJustified(15, ' ');
    QString name = "[" + apiResponse[currentAppId].toObject()["data"].toObject()["name"].toString().leftJustified(55, ' ') + "](" + "https://store.steampowered.com/app/" + currentAppId + ")";
    QString originalprice = QString(priceToString(apiResponse[currentAppId].toObject()["data"].toObject()["price_overview"].toObject()["initial"].toInt())).leftJustified(10, ' ');
    QString discountPercent = QString(QString::number(apiResponse[currentAppId].toObject()["data"].toObject()["price_overview"].toObject()["discount_percent"].toInt())).leftJustified(10, ' ');
    QString newPrice = QString(priceToString(apiResponse[currentAppId].toObject()["data"].toObject()["price_overview"].toObject()["final"].toInt())).leftJustified(10, ' ');

    if (ui->checkBox->isChecked() && name.trimmed() != "")
        ui->plainTextEdit->setPlainText(ui->plainTextEdit->toPlainText() + "| " + platform + " | " + name + " | " + originalprice + " | " + discountPercent + " | " + newPrice + " |\n");
    else if (!ui->checkBox->isChecked() && name.trimmed() != "")
        ui->plainTextEdit->setPlainText(ui->plainTextEdit->toPlainText() + "| " + name + " | " + originalprice + " | " + discountPercent + " | " + newPrice + " |\n");

    apiResponseReady = true;
    emit readyForNext();
}

void MainWindow::webPageProgress(int value)
{
    ui->progressBar->setMaximum(100);
    ui->progressBar->setValue(value);
}

void MainWindow::waitForAppIds()
{
    QTime dieTime = QTime::currentTime().addMSecs(10);
    while (QTime::currentTime() < dieTime)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
    if (websiteType == 0)
    {
        if (appIds.isEmpty() || platforms.isEmpty() || pageCount == 0)
            waitForAppIds();
        if (!ui->radioButtonRange->isChecked())
        {
            if (ui->checkBox->isChecked())
            {
                ui->plainTextEdit->setPlainText("| PLATFORM        | NAME                                                    | ORIGINAL $ | PERCENT    | NEW $      |\n");
                ui->plainTextEdit->setPlainText(ui->plainTextEdit->toPlainText() + "|-----------------|---------------------------------------------------------|------------|------------|------------|\n");
            }
            else
            {
                ui->plainTextEdit->setPlainText("| NAME                                                    | ORIGINAL $ | PERCENT    | NEW $      |\n");
                ui->plainTextEdit->setPlainText(ui->plainTextEdit->toPlainText() + "|---------------------------------------------------------|------------|------------|------------|\n");
            }
        }
        else
        {
            if (showHeader)
            {
                if (ui->checkBox->isChecked())
                {
                    ui->plainTextEdit->setPlainText(ui->plainTextEdit->toPlainText() + "| PLATFORM        | NAME                                                    | ORIGINAL $ | PERCENT    | NEW $      |\n");
                    ui->plainTextEdit->setPlainText(ui->plainTextEdit->toPlainText() + "|-----------------|---------------------------------------------------------|------------|------------|------------|\n");
                }
                else
                {
                    ui->plainTextEdit->setPlainText(ui->plainTextEdit->toPlainText() + "| NAME                                                    | ORIGINAL $ | PERCENT    | NEW $      |\n");
                    ui->plainTextEdit->setPlainText(ui->plainTextEdit->toPlainText() + "|---------------------------------------------------------|------------|------------|------------|\n");
                }
            }
        }
        getDataFromIds(appIds);
    }
    else if (websiteType == 1)
    {
        if (humbleList.isEmpty())
            waitForAppIds();
        if (showHeader)
        {
            if (ui->checkBox->isChecked())
            {
                ui->plainTextEdit->setPlainText(ui->plainTextEdit->toPlainText() + "| PLATFORM        | NAME                                                    | NEW $      |\n");
                ui->plainTextEdit->setPlainText(ui->plainTextEdit->toPlainText() + "|-----------------|---------------------------------------------------------|------------|\n");
            }
            else
            {
                ui->plainTextEdit->setPlainText(ui->plainTextEdit->toPlainText() + "| NAME                                                    | NEW $      |\n");
                ui->plainTextEdit->setPlainText(ui->plainTextEdit->toPlainText() + "|---------------------------------------------------------|------------|\n");
            }
        }
        foreach (QString line, humbleList)
        {
            QStringList parameters = line.split("|");
            if (ui->checkBox->isChecked())
                ui->plainTextEdit->setPlainText(ui->plainTextEdit->toPlainText() + "| " + QString(parameters[0]).replace("Windows", "").replace("Android", "").replace("Mac", "").replace("Linux", "").replace("Oculus Rift", "Rift").replace("HTC Vive", "Vive").trimmed().replace(" ", "/").leftJustified(15, ' ') + " | " + QString(parameters[1]).leftJustified(55, ' ')+ " | " /* + QString(parameters[2]).leftJustified(10, ' ') + " | " + QString(parameters[3]).leftJustified(10, ' ') + " | " + */ + QString(parameters[2]).leftJustified(10, ' ') + " |\n");
            else
                ui->plainTextEdit->setPlainText(ui->plainTextEdit->toPlainText() + "| " + QString(parameters[1]).leftJustified(55, ' ') + " | " + QString(parameters[2]).leftJustified(10, ' ')/* + " | " + QString(parameters[3]).leftJustified(10, ' ') + " | " + QString(parameters[4]).leftJustified(10, ' ') */+ " |\n");
        }
        showHeader = false;
        emit readyForNext();
    }
}

void MainWindow::getDataFromIds(QStringList ids)
{
    showHeader = false;

    foreach (QString id, ids)
    {
        currentAppId = id;
        apiResponseReady = false;
        manager->get(QNetworkRequest(QUrl("https://store.steampowered.com/api/appdetails?appids=" + id)));
loopStart:
        QTime dieTime = QTime::currentTime().addMSecs(10);
        while (QTime::currentTime() < dieTime)
        {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        }
        if (!apiResponseReady)
            goto loopStart;
        else
            continue;
    }
}

QString MainWindow::priceToString(int price)
{
    QString priceString = QString::number(price);
    QString newString = priceString.insert(priceString.length() - 2, '.');
    if (newString[0] == '.')
        return "0" + newString;
    else
        return newString;
}

void MainWindow::reset()
{
    currentAppId = "";
    currentUrl = "";

    appIds.clear();
    platforms.clear();
    humbleList.clear();

    apiResponseReady = false;
    showHeader = true;

    pageCount = 0;
    websiteType = 0;
}

void MainWindow::init()
{
    ui->plainTextEdit->clear();
    ui->radioButtonPage->setChecked(true);
    ui->checkBox->setChecked(true);
    ui->spinBoxEnd->setValue(1);
    ui->spinBoxPage->setValue(1);
    ui->spinBoxStart->setValue(1);
    ui->spinBoxStart->hide();
    ui->spinBoxEnd->hide();
    ui->label_2->hide();
}

void MainWindow::on_pushButton_clicked()
{
    if (!ui->lineEdit->text().contains("https://store.steampowered.com/search/") && !ui->lineEdit->text().contains("https://www.humblebundle.com/store/search"))
    {
        if (!ui->lineEdit->text().contains("https://www.humblebundle.com/store/search"))
            QMessageBox::warning(this, "Error!", "Invalid URL, Should Be Formatted as \"https://www.humblebundle.com/store/search\"");
        else
            QMessageBox::warning(this, "Error!", "Invalid URL, Should Be Formatted as \"https://store.steampowered.com/search/\"");
    }
    else
    {
        if (ui->lineEdit->text().contains("https://store.steampowered.com/search/"))
            websiteType = 0;
        else
            websiteType = 1;
        qDebug() << websiteType;

        QUrlQuery query(QUrl(ui->lineEdit->text()));
        if (query.hasQueryItem("page"))
            query.removeQueryItem("page");

        QUrl url = QUrl(ui->lineEdit->text());
        url.setQuery(query);

        currentUrl = url.toString();
        ui->lineEdit->setText(currentUrl);

        if (ui->radioButtonRange->isChecked())
        {
            for (int x = ui->spinBoxStart->value(); x < ui->spinBoxEnd->value()+1; x++)
            {
                ui->progressBarPages->setMinimum(ui->spinBoxStart->value());
                ui->progressBarPages->setMaximum(ui->spinBoxEnd->value());
                ui->progressBarPages->setValue(x+1);

                if (websiteType == 0)
                    view->load(QUrl(currentUrl + "&page=" + QString::number(x)));
                else
                    view->load(QUrl(currentUrl + "&page=" + QString::number(x - 1)));

                QEventLoop nextLoop;
                connect(this, SIGNAL(readyForNext()), &nextLoop, SLOT(quit()));
                nextLoop.exec();
            }
        }
        else
        {
            if (websiteType == 0)
                view->load(QUrl(currentUrl + "&page=" + QString::number(ui->spinBoxPage->value())));
            else
                view->load(QUrl(currentUrl + "&page=" + QString::number(ui->spinBoxPage->value() - 1)));
        }

    }

    ui->progressBar->hide();
    ui->progressBarPages->hide();
}

void MainWindow::on_radioButtonPage_toggled(bool checked)
{
    if (checked)
    {
        ui->spinBoxPage->show();
        ui->spinBoxStart->hide();
        ui->spinBoxEnd->hide();
        ui->label_2->hide();
    }
    else
    {
        ui->spinBoxPage->hide();
        ui->spinBoxStart->show();
        ui->spinBoxEnd->show();
        ui->label_2->show();
    }
}
