#include "MyBorrowsDialog.h"
#include "LibraryController.h"
#include "src/db/DBManager.h"
#include "UiTheme.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QLabel>
#include <QFrame>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QDateTime>
#include <QBrush>
#include <QColor>
#include <algorithm>

MyBorrowsDialog::MyBorrowsDialog(const QString& borrowerId, LibraryController* controller, QWidget* parent)
    : QDialog(parent),
      borrowerId(borrowerId),
      controller(controller),
      statsLabel(nullptr),
      totalChipLabel(nullptr),
      activeChipLabel(nullptr),
      overdueChipLabel(nullptr) {
    setWindowTitle("我的借阅");
    setMinimumSize(1150, 720);
    resize(1150, 720);
    setSizeGripEnabled(true);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    
    setStyleSheet(ui::dialogBackground());
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    QFrame* titleFrame = new QFrame(this);
    titleFrame->setFixedHeight(64);
    titleFrame->setStyleSheet(ui::headerFrameStyle());
    QHBoxLayout* titleLayout = new QHBoxLayout(titleFrame);
    titleLayout->setContentsMargins(20, 15, 20, 15);
    QLabel* titleLabel = new QLabel("我的借阅", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: white;");
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    mainLayout->addWidget(titleFrame);
    
    QFrame* contentFrame = new QFrame(this);
    contentFrame->setStyleSheet(ui::cardFrameStyle());
    QVBoxLayout* contentLayout = new QVBoxLayout(contentFrame);
    contentLayout->setSpacing(15);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    
    auto makeChip = [&](const QString& text, const QString& bg, const QString& fg) {
        QLabel* chip = new QLabel(text, contentFrame);
        chip->setStyleSheet(QString(
            "QLabel { "
            "    padding: 4px 12px; "
            "    border-radius: 999px; "
            "    background-color: %1; "
            "    color: %2; "
            "    font-size: 12px; "
            "    font-weight: 600; "
            "}"
        ).arg(bg, fg));
        return chip;
    };
    
    QFrame* infoBanner = new QFrame(contentFrame);
    infoBanner->setStyleSheet(ui::softCardFrameStyle());
    QHBoxLayout* bannerLayout = new QHBoxLayout(infoBanner);
    bannerLayout->setSpacing(12);
    bannerLayout->setContentsMargins(12, 12, 12, 12);
    
    QLabel* borrowerInfoLabel = new QLabel(QString("借阅人ID: %1").arg(borrowerId), infoBanner);
    borrowerInfoLabel->setStyleSheet("color: #1f2937; font-weight: 600; font-size: 14px;");
    bannerLayout->addWidget(borrowerInfoLabel);
    
    totalChipLabel = makeChip("总借阅 --", "#e0f2fe", "#0369a1");
    activeChipLabel = makeChip("借阅中 --", "#dcfce7", "#166534");
    overdueChipLabel = makeChip("已逾期 --", "#fee2e2", "#991b1b");
    
    bannerLayout->addWidget(totalChipLabel);
    bannerLayout->addWidget(activeChipLabel);
    bannerLayout->addWidget(overdueChipLabel);
    bannerLayout->addStretch();
    
    statsLabel = new QLabel("数据加载中…", infoBanner);
    statsLabel->setStyleSheet(ui::subtleTextStyle());
    bannerLayout->addWidget(statsLabel);
    
    contentLayout->addWidget(infoBanner);
    
    QFrame* tableFrame = new QFrame(this);
    tableFrame->setStyleSheet(ui::softCardFrameStyle());
    
    QVBoxLayout* tableLayout = new QVBoxLayout(tableFrame);
    tableLayout->setSpacing(10);
    tableLayout->setContentsMargins(0, 0, 0, 0);
    
    QHBoxLayout* titleRowLayout = new QHBoxLayout();
    titleRowLayout->setContentsMargins(0, 0, 0, 0);
    QLabel* tableTitle = new QLabel("当前借阅列表", this);
    tableTitle->setStyleSheet(ui::sectionTitleStyle());
    titleRowLayout->addWidget(tableTitle);
    titleRowLayout->addStretch();
    
    QPushButton* refreshBtn = new QPushButton("刷新", this);
    refreshBtn->setCursor(Qt::PointingHandCursor);
    refreshBtn->setStyleSheet(ui::primaryButtonStyle());
    titleRowLayout->addWidget(refreshBtn);
    tableLayout->addLayout(titleRowLayout);
    
    connect(refreshBtn, &QPushButton::clicked, this, &MyBorrowsDialog::refreshData);
    
    model = new QStandardItemModel(this);
    model->setColumnCount(7);
    model->setHorizontalHeaderLabels({"图书ID", "书名", "作者", "借阅日期", "借阅天数", "应归还日期", "状态"});
    
    tableView = new QTableView(this);
    tableView->setModel(model);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setAlternatingRowColors(true);
    tableView->verticalHeader()->hide();
    tableView->setShowGrid(false);
    tableView->setWordWrap(false);
    tableView->setTextElideMode(Qt::ElideRight);
    tableView->setStyleSheet(ui::tableStyle());
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    tableLayout->addWidget(tableView);
    contentLayout->addWidget(tableFrame, 1);
    mainLayout->addWidget(contentFrame, 1);
    
    QFrame* buttonFrame = new QFrame(this);
    buttonFrame->setStyleSheet("QFrame { background-color: transparent; }");
    QHBoxLayout* buttonLayout = new QHBoxLayout(buttonFrame);
    buttonLayout->setContentsMargins(20, 15, 20, 15);
    buttonLayout->addStretch();
    
    QPushButton* closeBtn = new QPushButton("关闭", this);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(ui::primaryButtonStyle());
    
    buttonLayout->addWidget(closeBtn);
    mainLayout->addWidget(buttonFrame);
    
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    
    refreshData();
}

void MyBorrowsDialog::refreshData() {
    model->setRowCount(0);
    auto updateSummary = [&](int total, int active, int overdue, const QString& msg) {
        if (totalChipLabel) {
            totalChipLabel->setText(QString("总借阅 %1").arg(total));
        }
        if (activeChipLabel) {
            activeChipLabel->setText(QString("借阅中 %1").arg(active));
        }
        if (overdueChipLabel) {
            overdueChipLabel->setText(QString("已逾期 %1").arg(overdue));
        }
        if (statsLabel) {
            statsLabel->setText(msg);
        }
    };
    
    if (!controller) {
        return;
    }
    
    if (!controller->isDatabaseConnected()) {
        updateSummary(0, 0, 0, "数据库未连接");
        return;
    }
    
    db::DBManager* dbManager = controller->getDBManager();
    if (!dbManager) {
        updateSummary(0, 0, 0, "数据库管理器不可用");
        return;
    }
    
    if (borrowerId.isEmpty()) {
        updateSummary(0, 0, 0, "借阅人ID为空");
        return;
    }
    
    std::vector<std::map<std::string, std::string>> records;
    // 使用getBorrowRecordsByBorrower获取所有记录（包括已归还的）
    // 如果只想看当前借阅的，可以使用getActiveBorrowRecordsByBorrower
    bool success = false;
    if (dbManager->getBorrowRecordsByBorrower(borrowerId.toStdString(), records)) {
        success = true;
    } else if (dbManager->getActiveBorrowRecordsByBorrower(borrowerId.toStdString(), records)) {
        success = true;
    }
    
    if (success) {
        int totalRecords = records.size();
        int activeCount = 0;
        int overdueCount = 0;
        QDateTime currentDateTime = QDateTime::currentDateTime();
        
        for (const auto& record : records) {
            QString status = record.find("status") != record.end() ? 
                QString::fromStdString(record.at("status")) : "unknown";
            if (status == "borrowed") {
                activeCount++;
                QString expectedReturnStr = record.find("expected_return_date") != record.end() ? 
                    QString::fromStdString(record.at("expected_return_date")) : "";
                if (!expectedReturnStr.isEmpty()) {
                    QDateTime expectedReturn = QDateTime::fromString(expectedReturnStr.left(19), "yyyy-MM-dd HH:mm:ss");
                    if (expectedReturn.isValid() && expectedReturn < currentDateTime) {
                        overdueCount++;
                    }
                }
            }
        }
        
        if (records.empty()) {
            updateSummary(0, 0, 0, "暂无借阅记录");
            return;
        }
        
        updateSummary(totalRecords, activeCount, overdueCount,
            QString("更新于 %1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm")));
        
        for (const auto& record : records) {
            int row = model->rowCount();
            model->insertRow(row);
            
            model->setItem(row, 0, new QStandardItem(QString::fromStdString(record.at("book_id"))));
            model->setItem(row, 1, new QStandardItem(QString::fromStdString(record.at("title"))));
            model->setItem(row, 2, new QStandardItem(QString::fromStdString(record.at("author"))));
            
            QString borrowDate = QString::fromStdString(record.at("borrow_date"));
            borrowDate = borrowDate.left(19).replace("T", " ");
            model->setItem(row, 3, new QStandardItem(borrowDate));
            
            model->setItem(row, 4, new QStandardItem(QString::fromStdString(record.at("borrow_days")) + " 天"));
            
            QString expectedReturn = QString::fromStdString(record.at("expected_return_date"));
            expectedReturn = expectedReturn.left(19).replace("T", " ");
            
            // 检查是否逾期
            bool isOverdue = false;
            QString status = record.find("status") != record.end() ? 
                QString::fromStdString(record.at("status")) : "unknown";
            if (status == "borrowed" && !expectedReturn.isEmpty()) {
                QDateTime expectedReturnDate = QDateTime::fromString(expectedReturn, "yyyy-MM-dd HH:mm:ss");
                if (expectedReturnDate.isValid() && expectedReturnDate < currentDateTime) {
                    isOverdue = true;
                    expectedReturn = expectedReturn + " (已逾期)";
                }
            }
            
            QStandardItem* returnDateItem = new QStandardItem(expectedReturn);
            if (isOverdue) {
                returnDateItem->setForeground(QBrush(QColor("#e74c3c")));
            }
            model->setItem(row, 5, returnDateItem);
            
            QString statusText;
            if (status == "borrowed") {
                statusText = isOverdue ? "借阅中 · 已逾期" : "借阅中";
            } else {
                QString returnDate = record.find("return_date") != record.end() ? 
                    QString::fromStdString(record.at("return_date")).left(19).replace("T", " ") : "";
                statusText = returnDate.isEmpty() ? "已归还" : QString("已归还 · %1").arg(returnDate);
            }
            QStandardItem* statusItem = new QStandardItem(statusText);
            if (isOverdue) {
                statusItem->setForeground(QBrush(QColor("#e74c3c")));
            } else if (status == "borrowed") {
                statusItem->setForeground(QBrush(QColor("#27ae60")));
            } else {
                statusItem->setForeground(QBrush(QColor("#95a5a6")));
            }
            model->setItem(row, 6, statusItem);
        }
    } else {
        // 查询失败时显示详细错误信息
        updateSummary(0, 0, 0, "查询失败 - 请检查借阅人ID是否正确");
        // 在表格中显示错误信息
        int row = model->rowCount();
        model->insertRow(row);
        QStandardItem* errorItem = new QStandardItem("查询失败，请检查借阅人ID: " + borrowerId);
        errorItem->setForeground(QBrush(QColor("#e74c3c")));
        model->setItem(row, 0, errorItem);
        for (int col = 1; col < model->columnCount(); col++) {
            model->setItem(row, col, new QStandardItem(""));
        }
    }
}
