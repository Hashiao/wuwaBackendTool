#include "debugform.h"
#include "ui_debugform.h"

DebugForm::DebugForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DebugForm)
{
    ui->setupUi(this);
    lockWidgetList.push_back(ui->colorEditText);
    lockWidgetList.push_back(ui->getGameWin);
    lockWidgetList.push_back(ui->tempThres);

    lockWidgetList.push_back(ui->testFindColor);
    lockWidgetList.push_back(ui->testFindPic);

    lockWidgetList.push_back(ui->testRebootGame);
    lockWidgetList.push_back(ui->activateBtn);
    lockWidgetList.push_back(ui->deactivateBtn);
    lockWidgetList.push_back(ui->isActivate);
    lockWidgetList.push_back(ui->saveImgPath);
    lockWidgetList.push_back(ui->capImgWaitMs);

    for(auto widget : lockWidgetList){
        widget->setVisible(false);
    }

    m_debugBackendWorker.moveToThread(&m_backendThread);
    m_backendThread.start();

    connect(this, &DebugForm::startActivateCap, &this->m_debugBackendWorker, &DebugBackendWorker::onStartActivateCap);
    connect(&this->m_debugBackendWorker, &DebugBackendWorker::activateCapDone, this, &DebugForm::onActivateCapDone);
}

DebugForm::~DebugForm()
{
    m_debugBackendWorker.stopWorker();
    m_backendThread.quit();
    m_backendThread.wait();
    delete ui;
}


void DebugForm::on_testFindPic_clicked(){
    qInfo() << QString("GeneralPanel::on_testFindPic_clicked");

    QString tempImgFullPath = QFileDialog::getOpenFileName(this, "打开待匹配的小图像");
    if(tempImgFullPath.isEmpty()){
        return;
    }
    cv::Mat templateImg = cv::imread(tempImgFullPath.toLocal8Bit().toStdString(), cv::IMREAD_UNCHANGED);
    if(templateImg.empty()){
        qWarning() << QString("failed to load %1").arg(tempImgFullPath);
        return;
    }

    if(templateImg.channels() == 3){

    }
    else if(templateImg.channels() == 4)  {
        cv::cvtColor(templateImg, templateImg, cv::COLOR_BGRA2BGR);
    }
    else{
        qWarning() << QString("%1 is not rgb image").arg(tempImgFullPath);
        return;
    }

    QString capImgFullPath = Utils::IMAGE_DIR() + "/" + "capWuwa.png";
    cv::Mat capImg = cv::imread(capImgFullPath.toLocal8Bit().toStdString(), cv::IMREAD_UNCHANGED);
    if(capImg.empty()){
        qWarning() << QString("failed to load %1").arg(capImgFullPath);
        return;
    }

    if(capImg.channels() == 3){

    }
    else if(capImg.channels() == 4)  {
        cv::cvtColor(capImg, capImg, cv::COLOR_BGRA2BGR);
    }
    else{
        qWarning() << QString("%1 is not rgb image").arg(capImgFullPath);
        return;
    }

    int X, Y;
    QElapsedTimer timer;
    timer.start();
    double similarity;
    bool isFind = Utils::findPic(capImg, templateImg, ui->tempThres->value(), X, Y, similarity);
    auto timeCostMs = timer.elapsed();
    qDebug() << QString("GeneralPanel::on_testFindPic_clicked is done, timeCost %1 ms, X %2, Y %3, isFound %4").arg(timeCostMs).arg(X).arg(Y).arg(isFind);

    // 如果找到模板，绘制蓝色矩形
    if (isFind) {
        cv::Mat markedImg = capImg.clone();
        cv::rectangle(
                    markedImg,
                    cv::Point(X, Y),
                    cv::Point(X + templateImg.cols, Y + templateImg.rows),
                    cv::Scalar(255, 0, 0), // 蓝色
                    2 // 线条宽度
                    );

        // 显示标记后的图像
        cv::imshow("Matched Image", markedImg);
        cv::waitKey(0); // 等待按键
        cv::destroyAllWindows(); // 关闭窗口
    } else {
        qWarning() << "Template not found in the source image.";
    }
    qInfo() << QString("similarity %1").arg(similarity);
}

void DebugForm::on_testFindColor_clicked(){
    QString capImgFullPath = Utils::IMAGE_DIR() + "/" + "capWuwa.png";
    cv::Mat capImg = cv::imread(capImgFullPath.toLocal8Bit().toStdString(), cv::IMREAD_UNCHANGED);

    int x, y;
    bool isFindColorEx = Utils::findColorEx(capImg, 450, 39, 454, 43, ui->colorEditText->toPlainText(), ui->tempThres->value(), x, y);
    qDebug() << QString("isFindColorEx %1. x = %2, y = %3").arg(isFindColorEx).arg(x).arg(y);
}

void DebugForm::on_confirmPwd_clicked(){
    QString pwd = ui->pwd->toPlainText();
    QByteArray correctPwdHash = QCryptographicHash::hash("IloveWuwa666", QCryptographicHash::Sha256);
    QByteArray inputPwdHash = QCryptographicHash::hash(pwd.toUtf8(), QCryptographicHash::Sha256);
    if(inputPwdHash == correctPwdHash){
    //if(1){
        for(auto widget : lockWidgetList){
            widget->setVisible(true);
        }
    }
    else{
        QMessageBox::critical(this, "密码错误", "请重新输入密码");
    }
}

void DebugForm::on_testRebootGame_clicked(){
    qInfo() << "DebugForm::on_testRebootGame_clicked";
    QStringList windowTitleList = Utils::getAllWindowTitles();
    qDebug() << windowTitleList;
    //onst QString title = "launcher";
    const QString title = "鸣潮";
    emit startTestRebootGame(title);

}


void DebugForm::on_activateBtn_clicked(){
    if(m_debugBackendWorker.isBusy()){
        qWarning() << "请先停止，然后修改参数，最后再次激活启动";
        //QMessageBox::warning(this, "已经激活 截图中", "请先停止，然后修改参数，最后再次激活启动");
        return;
    }
    ui->isActivate->setChecked(true);
    emit startActivateCap(ui->saveImgPath->toPlainText(), ui->capImgWaitMs->value());
}

void DebugForm::on_deactivateBtn_clicked(){
    m_debugBackendWorker.stopWorker();
}

void DebugForm::on_scrCap_clicked(){
    QElapsedTimer timer;
    timer.start();
    bool isWuwaInit = Utils::initWuwaHwnd();
    if(!isWuwaInit){
        qWarning() << QString("未能绑定鸣潮窗口句柄");
        return;
    }

    QImage image = Utils::captureWindowToQImage(Utils::hwnd);
    if(image.isNull()){
        qWarning() << QString("未能获取鸣潮窗口图像");
        return;
    }

    /*
    // 尝试后台激活窗口（并不强制将窗口置前）
    DWORD threadId = GetWindowThreadProcessId(Utils::hwnd, nullptr);
    DWORD currentThreadId = GetCurrentThreadId();
    if (AttachThreadInput(currentThreadId, threadId, TRUE)) {
        // 激活窗口
        SendMessage(Utils::hwnd, WM_ACTIVATE, WA_ACTIVE, 0);
    }
    else{
        qWarning() << QString("激活鸣潮窗体失败 仍会截图");
    }
    */

    QString capImagePath = QString("%1").arg(ui->saveImgPath->toPlainText());
    bool isSave = image.save(capImagePath);
    int timeCostMs = timer.elapsed();
    if(isSave){
        qInfo() << QString("成功保存截图到 %1, 耗时 %2ms").arg(capImagePath).arg(timeCostMs);
    }
    else{
        qWarning() << QString("保存截图到 %1 失败, 耗时 %2ms").arg(capImagePath).arg(timeCostMs);
    }
}

void DebugForm::onActivateCapDone(const bool& isOK, const QString& msg){
    ui->isActivate->setChecked(false);
    if(isOK){
        qInfo() << "停止激活成功";
        //QMessageBox::information(this, "激活和捕获图像停止", msg);
        return;
    }
    else{
        qWarning() << "停止激活成功 但后台有报错信息 " << msg;
        //QMessageBox::warning(this, "激活和捕获图像错误", msg);
        return;
    }
}
