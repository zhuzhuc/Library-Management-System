#include "BorrowRecordsDialog.h"
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
#include <QDialogButtonBox>
#include <QPushButton>
#include <QBrush>
#include <QColor>
#include <QDateTime>

BorrowRecordsDialog::BorrowRecordsDialog(const QString& borrowerId, const QString& borrowerName,
                                        LibraryController* controller, QWidget* parent)
    : QDialog(parent),
      borrowerId(borrowerId),
      borrowerName(borrowerName),
      controller(controller),
      statsLabel(nullptr),
      recordsChipLabel(nullptr),
      borrowedChipLabel(nullptr),
      returnedChipLabel(nullptr) {
    setWindowTitle(QString("借阅记录 - %1").arg(borrowerName));
    setMinimumSize(1150, 700);
    resize(1150, 700);
    setSizeGripEnabled(true);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    
    setStyleSheet(ui::dialogBackground());
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // 标题区域
    QFrame* titleFrame = new QFrame(this);
    titleFrame->setFixedHeight(64);
    titleFrame->setStyleSheet(ui::headerFrameStyle());
    QHBoxLayout* titleLayout = new QHBoxLayout(titleFrame);
    titleLayout->setContentsMargins(20, 15, 20, 15);
    QLabel* titleLabel = new QLabel(QString("%1 的借阅记录").arg(borrowerName), this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: white; background: transparent;");
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    mainLayout->addWidget(titleFrame);
    
    // 内容区域
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
    QHBoxLayout* infoLayout = new QHBoxLayout(infoBanner);
    infoLayout->setSpacing(12);
    infoLayout->setContentsMargins(12, 12, 12, 12);
    
    QLabel* borrowerLabel = new QLabel(QString("借阅人: %1 (%2)").arg(borrowerName).arg(borrowerId), infoBanner);
    borrowerLabel->setStyleSheet("color: #1f2937; font-weight: 600; font-size: 14px;");
    infoLayout->addWidget(borrowerLabel);
    
    recordsChipLabel = makeChip("记录 --", "#e0f2fe", "#0369a1");
    borrowedChipLabel = makeChip("借阅中 --", "#dcfce7", "#166534");
    returnedChipLabel = makeChip("已归还 --", "#fef3c7", "#b45309");
    
    infoLayout->addWidget(recordsChipLabel);
    infoLayout->addWidget(borrowedChipLabel);
    infoLayout->addWidget(returnedChipLabel);
    infoLayout->addStretch();
    
    statsLabel = new QLabel("数据加载中…", infoBanner);
    statsLabel->setStyleSheet(ui::subtleTextStyle());
    infoLayout->addWidget(statsLabel);
    
    contentLayout->addWidget(infoBanner);
    
    QFrame* tableFrame = new QFrame(this);
    tableFrame->setStyleSheet(ui::softCardFrameStyle());
    
    QVBoxLayout* tableLayout = new QVBoxLayout(tableFrame);
    tableLayout->setSpacing(10);
    tableLayout->setContentsMargins(0, 0, 0, 0);
    
    // 标题行：统计信息和刷新按钮
    QHBoxLayout* titleRowLayout = new QHBoxLayout();
    titleRowLayout->setContentsMargins(0, 0, 0, 0);
    titleRowLayout->addStretch();
    
    QPushButton* refreshBtn = new QPushButton("刷新", this);
    refreshBtn->setCursor(Qt::PointingHandCursor);
    refreshBtn->setStyleSheet(ui::primaryButtonStyle());
    titleRowLayout->addWidget(refreshBtn);
    tableLayout->addLayout(titleRowLayout);
    
    connect(refreshBtn, &QPushButton::clicked, this, &BorrowRecordsDialog::refreshData);
    
    model = new QStandardItemModel(this);
    model->setColumnCount(8);
    model->setHorizontalHeaderLabels({"图书ID", "书名", "作者", "ISBN", "借阅日期", "借阅天数", "应归还日期", "归还日期/状态"});
    
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
    
    // 按钮区域
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

void BorrowRecordsDialog::refreshData() {
    model->setRowCount(0);
    auto updateSummary = [&](int total, int borrowed, int returned, const QString& msg) {
        if (recordsChipLabel) {
            recordsChipLabel->setText(QString("记录 %1").arg(total));
        }
        if (borrowedChipLabel) {
            borrowedChipLabel->setText(QString("借阅中 %1").arg(borrowed));
        }
        if (returnedChipLabel) {
            returnedChipLabel->setText(QString("已归还 %1").arg(returned));
        }
        if (statsLabel) {
            statsLabel->setText(msg);
        }
    };
    
    if (!controller) {
        updateSummary(0, 0, 0, "控制器不可用");
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
    if (dbManager->getBorrowRecordsByBorrower(borrowerId.toStdString(), records)) {
        // 统计信息
        int totalRecords = records.size();
        int borrowedCount = 0;
        int returnedCount = 0;
        for (const auto& record : records) {
            QString status = record.find("status") != record.end() ? 
                QString::fromStdString(record.at("status")) : "unknown";
            if (status == "borrowed") {
                borrowedCount++;
            } else {
                returnedCount++;
            }
        }
        
        if (records.empty()) {
            updateSummary(0, 0, 0, "暂无借阅记录");
            return;
        }
        
        updateSummary(totalRecords, borrowedCount, returnedCount,
            QString("更新于 %1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm")));
        
        for (const auto& record : records) {
            int row = model->rowCount();
            model->insertRow(row);
            
            model->setItem(row, 0, new QStandardItem(QString::fromStdString(record.at("book_id"))));
            model->setItem(row, 1, new QStandardItem(QString::fromStdString(record.at("title"))));
            model->setItem(row, 2, new QStandardItem(QString::fromStdString(record.at("author"))));
            model->setItem(row, 3, new QStandardItem(QString::fromStdString(record.at("isbn"))));
            
            QString borrowDate = QString::fromStdString(record.at("borrow_date"));
            borrowDate = borrowDate.left(19).replace("T", " ");
            model->setItem(row, 4, new QStandardItem(borrowDate));
            
            model->setItem(row, 5, new QStandardItem(QString::fromStdString(record.at("borrow_days")) + " 天"));
            
            QString expectedReturn = QString::fromStdString(record.at("expected_return_date"));
            expectedReturn = expectedReturn.left(19).replace("T", " ");
            model->setItem(row, 6, new QStandardItem(expectedReturn));
            
            QString status = QString::fromStdString(record.at("status"));
            QString returnDateStr = record.find("return_date") != record.end() && !record.at("return_date").empty() ?
                QString::fromStdString(record.at("return_date")).left(19).replace("T", " ") : "";
            
            QString statusText = status == "borrowed" ? "借阅中" : ("已归还" + (returnDateStr.isEmpty() ? "" : " (" + returnDateStr + ")"));
            QStandardItem* statusItem = new QStandardItem(statusText);
            if (status == "borrowed") {
                statusItem->setForeground(QBrush(QColor("#27ae60")));
            } else {
                statusItem->setForeground(QBrush(QColor("#95a5a6")));
            }
            model->setItem(row, 7, statusItem);
        }
    } else {
        updateSummary(0, 0, 0, "查询失败");
    }
}
