#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initUI();
    initCardData();
    initWindowList();
    loadCardLibraryTable();

    // 初始化捕获线程
    m_captureThread = new CaptureThread(this);
    connect(m_captureThread, &CaptureThread::frameCaptured, this, &MainWindow::onFrameCaptured);
    connect(m_captureThread, &CaptureThread::captureError, this, &MainWindow::onCaptureError);
}

MainWindow::~MainWindow()
{
    if (m_captureThread->isRunning())
    {
        m_captureThread->stop();
    }
    delete ui;
}

// 初始化UI样式
void MainWindow::initUI()
{
    this->setWindowTitle("三国杀张嫙同礼最优收益计算器");
    this->setMinimumSize(1280, 800);

    // 手牌列表初始化
    ui->handTable->setColumnCount(3);
    ui->handTable->setHorizontalHeaderLabels(QStringList() << "卡牌名称" << "花色" << "收益权重");
    ui->handTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->handTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->handTable->setSelectionMode(QAbstractItemView::SingleSelection);

    // 结果列表初始化
    ui->resultTable->setColumnCount(4);
    ui->resultTable->setHorizontalHeaderLabels(QStringList() << "序号" << "卡牌名称" << "花色" << "收益权重");
    ui->resultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 卡牌库表格初始化
    ui->cardLibTable->setColumnCount(4);
    ui->cardLibTable->setHorizontalHeaderLabels(QStringList() << "卡牌名称" << "类型" << "收益权重" << "花色");
    ui->cardLibTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->cardLibTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->cardLibTable->setSelectionMode(QAbstractItemView::SingleSelection);

    // ROI滑块初始化
    ui->roiXSlider->setRange(0, 100);
    ui->roiYSlider->setRange(0, 100);
    ui->roiWSlider->setRange(0, 100);
    ui->roiHSlider->setRange(0, 100);
    ui->roiXSlider->setValue(10);
    ui->roiYSlider->setValue(80);
    ui->roiWSlider->setValue(80);
    ui->roiHSlider->setValue(20);
    updateROILabel();

    // 卡牌类型下拉框初始化
    ui->cardTypeCombo->addItems(QStringList() << "基本牌" << "普通锦囊" << "装备牌" << "延时锦囊");
}

// 初始化卡牌下拉框数据
void MainWindow::initCardData()
{
    CardLibrary& lib = CardLibrary::getInstance();
    std::vector<const Card*> allCards = lib.getAllCards();

    // 去重卡牌名称
    QSet<QString> cardNameSet;
    for (auto card : allCards)
    {
        cardNameSet.insert(QString::fromStdString(card->getName()));
    }

    // 填充卡牌名称下拉框
    QStringList cardNameList = cardNameSet.values();
    cardNameList.sort();
    ui->cardNameCombo->addItems(cardNameList);
    ui->customCardNameEdit->addItems(cardNameList);

    // 填充花色下拉框
    ui->suitCombo->addItems(QStringList() << "红桃" << "方块" << "黑桃" << "梅花");
    ui->customSuitCombo->addItems(QStringList() << "红桃" << "方块" << "黑桃" << "梅花");
}

// 初始化窗口列表
void MainWindow::initWindowList()
{
    ui->windowCombo->clear();
    auto windowList = WindowCapture::getAllWindowTitles();
    for (auto& pair : windowList)
    {
        QString title = QString::fromStdString(pair.second);
        ui->windowCombo->addItem(title, QVariant::fromValue((void*)pair.first));
    }

    // 自动选中含三国杀的窗口
    for (int i = 0; i < ui->windowCombo->count(); i++)
    {
        if (ui->windowCombo->itemText(i).contains("三国杀", Qt::CaseInsensitive))
        {
            ui->windowCombo->setCurrentIndex(i);
            break;
        }
    }
}

// 加载卡牌库表格
void MainWindow::loadCardLibraryTable(const QString& filter)
{
    ui->cardLibTable->setRowCount(0);
    CardLibrary& lib = CardLibrary::getInstance();
    std::vector<const Card*> allCards = lib.getAllCards();

    for (auto card : allCards)
    {
        QString fullName = QString::fromStdString(card->getFullName());
        if (!filter.isEmpty() && !fullName.contains(filter, Qt::CaseInsensitive))
        {
            continue;
        }

        int row = ui->cardLibTable->rowCount();
        ui->cardLibTable->insertRow(row);

        // 类型转换
        QString typeStr;
        switch (card->getType())
        {
            case CardType::BASIC: typeStr = "基本牌"; break;
            case CardType::NORMAL_TRICK: typeStr = "普通锦囊"; break;
            case CardType::EQUIP: typeStr = "装备牌"; break;
            case CardType::DELAYED_TRICK: typeStr = "延时锦囊"; break;
            default: typeStr = "未知"; break;
        }

        ui->cardLibTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(card->getName())));
        ui->cardLibTable->setItem(row, 1, new QTableWidgetItem(typeStr));
        ui->cardLibTable->setItem(row, 2, new QTableWidgetItem(QString::number(card->getProfit())));
        ui->cardLibTable->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(card->getSuitName())));
    }
}

// 更新ROI参数标签
void MainWindow::updateROILabel()
{
    ui->roiXLabel->setText(QString("左边界: %1%").arg(ui->roiXSlider->value()));
    ui->roiYLabel->setText(QString("上边界: %1%").arg(ui->roiYSlider->value()));
    ui->roiWLabel->setText(QString("宽度: %1%").arg(ui->roiWSlider->value()));
    ui->roiHLabel->setText(QString("高度: %1%").arg(ui->roiHSlider->value()));

    // 更新线程的ROI参数
    float x = ui->roiXSlider->value() / 100.0f;
    float y = ui->roiYSlider->value() / 100.0f;
    float w = ui->roiWSlider->value() / 100.0f;
    float h = ui->roiHSlider->value() / 100.0f;
    m_captureThread->setHandROIRatio(x, y, w, h);
}

// 显示计算结果
void MainWindow::showResult(const CalculateResult& result)
{
    ui->resultTable->setRowCount(0);
    ui->maxProfitLabel->setText(QString("<b>最大总收益：%1</b>").arg(result.maxTotalProfit));

    if (result.triggerIndex == -1)
    {
        ui->triggerInfoLabel->setText("<b style='color:red;'>当前手牌无法触发同礼，建议调整手牌</b>");
    }
    else
    {
        QString triggerInfo = QString("<b style='color:green;'>同礼触发位置：第%1张 | 触发卡牌：%2 | 额外执行%3次（总效果%4次）</b>")
                                  .arg(result.triggerIndex + 1)
                                  .arg(QString::fromStdString(result.triggerCardName))
                                  .arg(result.triggerExtraTimes)
                                  .arg(result.triggerExtraTimes + 1);
        ui->triggerInfoLabel->setText(triggerInfo);
    }

    // 填充结果表格
    for (int i = 0; i < result.optimalOrder.size(); i++)
    {
        const Card& card = result.optimalOrder[i];
        int row = ui->resultTable->rowCount();
        ui->resultTable->insertRow(row);

        QTableWidgetItem* indexItem = new QTableWidgetItem(QString::number(i + 1));
        QTableWidgetItem* nameItem = new QTableWidgetItem(QString::fromStdString(card.getName()));
        QTableWidgetItem* suitItem = new QTableWidgetItem(QString::fromStdString(card.getSuitName()));
        QTableWidgetItem* profitItem = new QTableWidgetItem(QString::number(card.getProfit()));

        // 标记触发卡牌
        if (i == result.triggerIndex)
        {
            indexItem->setBackground(QColor(0, 255, 0, 100));
            nameItem->setBackground(QColor(0, 255, 0, 100));
            suitItem->setBackground(QColor(0, 255, 0, 100));
            profitItem->setBackground(QColor(0, 255, 0, 100));
            nameItem->setText(nameItem->text() + " 【同礼触发】");
        }

        ui->resultTable->setItem(row, 0, indexItem);
        ui->resultTable->setItem(row, 1, nameItem);
        ui->resultTable->setItem(row, 2, suitItem);
        ui->resultTable->setItem(row, 3, profitItem);
    }
}

// ------------------------------ 主界面槽函数 ------------------------------
// 添加手牌
void MainWindow::on_addCardBtn_clicked()
{
    QString cardName = ui->cardNameCombo->currentText();
    QString suitStr = ui->suitCombo->currentText();

    CardLibrary& lib = CardLibrary::getInstance();
    Suit suit = CardLibrary::stringToSuit(suitStr.toStdString());
    const Card* card = lib.getCard(cardName.toStdString(), suit);

    if (!card)
    {
        QMessageBox::warning(this, "错误", "该卡牌不存在！");
        return;
    }

    // 添加到手牌列表
    int row = ui->handTable->rowCount();
    ui->handTable->insertRow(row);
    ui->handTable->setItem(row, 0, new QTableWidgetItem(cardName));
    ui->handTable->setItem(row, 1, new QTableWidgetItem(suitStr));
    ui->handTable->setItem(row, 2, new QTableWidgetItem(QString::number(card->getProfit())));
}

// 删除选中手牌
void MainWindow::on_delCardBtn_clicked()
{
    int currentRow = ui->handTable->currentRow();
    if (currentRow < 0)
    {
        QMessageBox::warning(this, "提示", "请先选中要删除的手牌！");
        return;
    }
    ui->handTable->removeRow(currentRow);
}

// 清空手牌
void MainWindow::on_clearCardBtn_clicked()
{
    ui->handTable->setRowCount(0);
}

// 计算最优收益
void MainWindow::on_calcBtn_clicked()
{
    int rowCount = ui->handTable->rowCount();
    if (rowCount == 0)
    {
        QMessageBox::warning(this, "错误", "请先添加手牌！");
        return;
    }

    // 转换手牌数据
    std::vector<Card> handCards;
    CardLibrary& lib = CardLibrary::getInstance();

    for (int i = 0; i < rowCount; i++)
    {
        QString cardName = ui->handTable->item(i, 0)->text();
        QString suitStr = ui->handTable->item(i, 1)->text();
        Suit suit = CardLibrary::stringToSuit(suitStr.toStdString());
        const Card* card = lib.getCard(cardName.toStdString(), suit);
        if (card)
        {
            handCards.push_back(*card);
        }
    }

    // 计算结果
    CalculateResult result = m_calculator.calculateOptimalPlay(handCards);
    showResult(result);
}

// ------------------------------ 窗口捕获槽函数 ------------------------------
// 刷新窗口列表
void MainWindow::on_refreshWindowBtn_clicked()
{
    initWindowList();
    QMessageBox::information(this, "提示", "窗口列表已刷新！");
}

// 开始捕获
void MainWindow::on_startCaptureBtn_clicked()
{
    if (m_captureThread->isRunning())
    {
        return;
    }

    int currentIndex = ui->windowCombo->currentIndex();
    if (currentIndex < 0)
    {
        QMessageBox::warning(this, "错误", "请先选择游戏窗口！");
        return;
    }

    HWND hwnd = (HWND)ui->windowCombo->itemData(currentIndex).value<void*>();
    m_captureThread->setWindowHandle(hwnd);
    m_captureThread->setShowHandZoom(ui->zoomHandCheckBox->isChecked());
    m_captureThread->start();

    ui->startCaptureBtn->setEnabled(false);
    ui->stopCaptureBtn->setEnabled(true);
    ui->windowCombo->setEnabled(false);
    ui->refreshWindowBtn->setEnabled(false);
}

// 停止捕获
void MainWindow::on_stopCaptureBtn_clicked()
{
    if (!m_captureThread->isRunning())
    {
        return;
    }

    m_captureThread->stop();
    ui->previewLabel->clear();
    ui->bigPreviewLabel->clear();

    ui->startCaptureBtn->setEnabled(true);
    ui->stopCaptureBtn->setEnabled(false);
    ui->windowCombo->setEnabled(true);
    ui->refreshWindowBtn->setEnabled(true);
}

// 切换手牌放大显示
void MainWindow::on_zoomHandCheckBox_stateChanged(int arg1)
{
    m_captureThread->setShowHandZoom(arg1 == Qt::Checked);
}

// ROI滑块调整
void MainWindow::on_roiXSlider_valueChanged(int value) { updateROILabel(); }
void MainWindow::on_roiYSlider_valueChanged(int value) { updateROILabel(); }
void MainWindow::on_roiWSlider_valueChanged(int value) { updateROILabel(); }
void MainWindow::on_roiHSlider_valueChanged(int value) { updateROILabel(); }

// 捕获到新画面
void MainWindow::onFrameCaptured(QImage frame, QImage handFrame)
{
    // 主界面小预览
    if (!frame.isNull())
    {
        ui->previewLabel->setPixmap(QPixmap::fromImage(frame).scaled(ui->previewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    // 大预览窗口
    if (!handFrame.isNull())
    {
        ui->bigPreviewLabel->setPixmap(QPixmap::fromImage(handFrame).scaled(ui->bigPreviewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

// 捕获错误
void MainWindow::onCaptureError(QString msg)
{
    on_stopCaptureBtn_clicked();
    QMessageBox::critical(this, "捕获错误", msg);
}

// ------------------------------ 卡牌库槽函数 ------------------------------
// 搜索卡牌
void MainWindow::on_searchCardEdit_textChanged(const QString &arg1)
{
    loadCardLibraryTable(arg1);
}

// 修改卡牌收益
void MainWindow::on_modifyProfitBtn_clicked()
{
    int currentRow = ui->cardLibTable->currentRow();
    if (currentRow < 0)
    {
        QMessageBox::warning(this, "提示", "请先选中要修改的卡牌！");
        return;
    }

    QString cardName = ui->cardLibTable->item(currentRow, 0)->text();
    QString suitStr = ui->cardLibTable->item(currentRow, 3)->text();
    QString fullName = cardName + "_" + suitStr;

    bool ok;
    int newProfit = QInputDialog::getInt(this, "修改收益权重", "请输入新的收益权重：",
                                           ui->cardLibTable->item(currentRow, 2)->text().toInt(), 0, 100, 1, &ok);
    if (!ok) return;

    CardLibrary& lib = CardLibrary::getInstance();
    if (lib.modifyCardProfit(fullName.toStdString(), newProfit))
    {
        loadCardLibraryTable(ui->searchCardEdit->text());
        QMessageBox::information(this, "成功", "收益权重修改成功！");
    }
    else
    {
        QMessageBox::critical(this, "错误", "卡牌修改失败！");
    }
}

// 添加自定义卡牌
void MainWindow::on_addCustomCardBtn_clicked()
{
    QString cardName = ui->customCardNameEdit->currentText();
    QString suitStr = ui->customSuitCombo->currentText();
    int typeIndex = ui->cardTypeCombo->currentIndex();
    int profit = ui->customProfitSpin->value();

    if (cardName.isEmpty())
    {
        QMessageBox::warning(this, "错误", "请输入卡牌名称！");
        return;
    }

    Suit suit = CardLibrary::stringToSuit(suitStr.toStdString());
    CardType type = static_cast<CardType>(typeIndex);
    Card newCard(cardName.toStdString(), suit, type, profit);

    CardLibrary& lib = CardLibrary::getInstance();
    if (lib.addCard(newCard))
    {
        initCardData();
        loadCardLibraryTable();
        QMessageBox::information(this, "成功", "自定义卡牌添加成功！");
    }
    else
    {
        QMessageBox::critical(this, "错误", "该卡牌已存在！");
    }
}

// 删除卡牌
void MainWindow::on_delCardLibBtn_clicked()
{
    int currentRow = ui->cardLibTable->currentRow();
    if (currentRow < 0)
    {
        QMessageBox::warning(this, "提示", "请先选中要删除的卡牌！");
        return;
    }

    QString cardName = ui->cardLibTable->item(currentRow, 0)->text();
    QString suitStr = ui->cardLibTable->item(currentRow, 3)->text();
    QString fullName = cardName + "_" + suitStr;

    if (QMessageBox::question(this, "确认删除", "确定要删除该卡牌吗？", QMessageBox::Yes|QMessageBox::No) != QMessageBox::Yes)
    {
        return;
    }

    CardLibrary& lib = CardLibrary::getInstance();
    if (lib.removeCard(fullName.toStdString()))
    {
        loadCardLibraryTable(ui->searchCardEdit->text());
        QMessageBox::information(this, "成功", "卡牌删除成功！");
    }
    else
    {
        QMessageBox::critical(this, "错误", "卡牌删除失败！");
    }
}