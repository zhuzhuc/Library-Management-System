#include "MainWindow.h"
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>
#include <QAction>
#include <QMessageBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QGridLayout>
#include <QStatusBar>
#include <QGroupBox>
#include <QFrame>
#include <QFont>
#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QListWidget>
#include <QTimer>
#include <QFile>
#include <QPixmap>
#include <QResizeEvent>
#include <QPalette>
#include <QBrush>
#include <QDate>
#include <QScrollArea>
#include <QSizePolicy>
#include <algorithm>
#include "BookTableModel.h"
#include "LibraryController.h"
#include "AddBookDialog.h"
#include "AddUserDialog.h"
#include "EditBookDialog.h"
#include "LoginDialog.h"
#include "ResetPasswordDialog.h"
#include "BorrowDaysDialog.h"
#include "MyBorrowsDialog.h"
#include "BookDetailDialog.h"
#include "UsersListDialog.h"
#include "Book.h"
#include "Student.h"
#include "Teacher.h"
#include "src/db/DBManager.h"
#include "RecommendBooK.h"
#include "AppearanceDialog.h"
#include "AppSettings.h"
#include "UiTheme.h"

namespace {
const char* kBaseMainWindowStyle =
    "QMainWindow { "
    "    font-family: 'PingFang SC', 'Microsoft YaHei', 'Segoe UI', sans-serif; "
    "}"
    "QWidget { "
    "    font-size: 13px; "
    "}";
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    isLoggedIn = false;
    currentUserType = "";
    currentUsername = "";
    currentBorrowerId = "";
    controller = new LibraryController(this);
    model = new BookTableModel(controller, this);
    recommendationList = nullptr;
    recommendationMetaLabel = nullptr;
    recommendationTimer = nullptr;
    recommendationCarouselIndex = -1;
    totalTitlesValueLabel = nullptr;
    totalCopiesValueLabel = nullptr;
    availableCopiesValueLabel = nullptr;
    borrowedCopiesValueLabel = nullptr;
    availableChipLabel = nullptr;
    borrowedChipLabel = nullptr;
    totalChipLabel = nullptr;
    appearanceSettingsAct = nullptr;
    setStyleSheet(kBaseMainWindowStyle);
    
    AppSettings& settings = AppSettings::instance();
    connect(&settings, &AppSettings::mainBackgroundChanged, this, &MainWindow::applyBackgroundFromSettings);
    
    if (!showLogin()) {
        QApplication::exit(0);
        return;
    }
    
    setupUi();
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUi() {
    setWindowTitle("图书馆管理系统 - Library Management System");
    setMinimumSize(1500, 800);
    
    applyBackgroundFromSettings();

    setWindowOpacity(1.0);
    
    QWidget* central = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Header - 简洁风格
    QFrame* headerFrame = new QFrame(central);
    headerFrame->setStyleSheet(
        "QFrame { "
        "    background-color: #2c3e50; "
        "    padding: 20px 30px; "
        "}"
    );
    
    QHBoxLayout* headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setSpacing(20);
    
    QLabel* titleLabel = new QLabel("图书馆管理系统", central);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: white; background: transparent;");
    
    statusLabel = new QLabel(central);
    if (controller->isDatabaseConnected()) {
        statusLabel->setText("MySQL 已连接");
        statusLabel->setStyleSheet(
            "color: #27ae60; "
            "font-weight: 500; "
            "padding: 6px 12px; "
            "background-color: rgba(39, 174, 96, 0.2); "
            "border-radius: 4px; "
            "border: 1px solid #27ae60;"
        );
    } else {
        statusLabel->setText("文件存储模式");
        statusLabel->setStyleSheet(
            "color: #e67e22; "
            "font-weight: 500; "
            "padding: 6px 12px; "
            "background-color: rgba(230, 126, 34, 0.2); "
            "border-radius: 4px; "
            "border: 1px solid #e67e22;"
        );
    }
    
    userLabel = new QLabel(central);
    updateUserDisplay();
    userLabel->setStyleSheet(
        "color: #ecf0f1; "
        "font-weight: 500; "
        "padding: 6px 12px; "
        "background-color: rgba(52, 152, 219, 0.2); "
        "border-radius: 4px; "
        "border: 1px solid #3498db;"
    );
    
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(statusLabel);
    headerLayout->addWidget(userLabel);
    mainLayout->addWidget(headerFrame);

    // 查询与概览
    QFrame* searchFrame = new QFrame(central);
    searchFrame->setObjectName("queryFrame");
    searchFrame->setStyleSheet(
        "QFrame#queryFrame { "
        "    background-color: white; "
        "    border: 1px solid #e0e7ff; "
        "    border-radius: 14px; "
        "}"
    );
    QVBoxLayout* searchBoxLayout = new QVBoxLayout(searchFrame);
    searchBoxLayout->setSpacing(10);
    searchBoxLayout->setContentsMargins(20, 16, 20, 16);
    
    QHBoxLayout* searchLayout = new QHBoxLayout();
    searchLayout->setSpacing(12);
    searchEdit = new QLineEdit(central);
    searchEdit->setPlaceholderText("输入书名、作者、分类或ISBN进行搜索...");
    searchEdit->setStyleSheet(
        "QLineEdit { "
        "    padding: 10px 12px; "
        "    font-size: 14px; "
        "    border: 1px solid #bdc3c7; "
        "    border-radius: 4px; "
        "    background-color: white; "
        "    color: #2c3e50; "
        "}"
        "QLineEdit:focus { "
        "    border-color: #3498db; "
        "    border-width: 2px; "
        "}"
    );
    
    QPushButton* searchBtn = new QPushButton("搜索", central);
    QPushButton* clearBtn = new QPushButton("清空", central);
    searchBtn->setStyleSheet(
        "QPushButton { "
        "    padding: 10px 20px; "
        "    font-weight: 500; "
        "    font-size: 14px; "
        "    background-color: #3498db; "
        "    color: white; "
        "    border-radius: 4px; "
        "    border: none; "
        "}"
        "QPushButton:hover { "
        "    background-color: #2980b9; "
        "}"
        "QPushButton:pressed { "
        "    background-color: #21618c; "
        "}"
    );
    clearBtn->setStyleSheet(
        "QPushButton { "
        "    padding: 10px 20px; "
        "    font-weight: 500; "
        "    font-size: 14px; "
        "    background-color: white; "
        "    color: #34495e; "
        "    border-radius: 4px; "
        "    border: 1px solid #bdc3c7; "
        "}"
        "QPushButton:hover { "
        "    background-color: #ecf0f1; "
        "}"
    );
    
    searchLayout->addWidget(searchEdit, 1);
    searchLayout->addWidget(searchBtn);
    searchLayout->addWidget(clearBtn);
    searchBoxLayout->addLayout(searchLayout);
    
    QHBoxLayout* insightLayout = new QHBoxLayout();
    insightLayout->setContentsMargins(0, 0, 0, 0);
    insightLayout->setSpacing(12);
    
    QLabel* insightTitle = new QLabel("快速概览", searchFrame);
    insightTitle->setStyleSheet("color: #475569; font-weight: 600;");
    insightLayout->addWidget(insightTitle);
    
    auto createChip = [&](const QString& text, const QString& bg, const QString& fg) {
        QLabel* chip = new QLabel(text, searchFrame);
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
    
    totalChipLabel = createChip("馆藏 -- 种", "#e0f2fe", "#0369a1");
    availableChipLabel = createChip("可借 --", "#dcfce7", "#166534");
    borrowedChipLabel = createChip("借出 --", "#fee2e2", "#991b1b");
    
    insightLayout->addWidget(totalChipLabel);
    insightLayout->addWidget(availableChipLabel);
    insightLayout->addWidget(borrowedChipLabel);
    insightLayout->addStretch();
    
    QLabel* insightHint = new QLabel("支持书名/作者/分类/ISBN模糊搜索", searchFrame);
    insightHint->setStyleSheet("color: #94a3b8; font-size: 12px;");
    insightLayout->addWidget(insightHint);
    
    searchBoxLayout->addLayout(insightLayout);
    mainLayout->addWidget(searchFrame);
    
    // 动作集合：替代传统工具栏的分组按钮
    bool isAdmin = (currentUserType == "admin");
    
    QAction* borrowAct = new QAction("借阅", this);
    QAction* returnAct = new QAction("归还", this);
    QAction* recommendAct = new QAction("图书推荐", this);
    QAction* myBorrowsAct = new QAction("我的借阅", this);
    QAction* bookDetailAct = new QAction("图书详情", this);
    QAction* usersListAct = new QAction("用户管理", this);
    
    editBookAct = new QAction("编辑", this);
    reloadAct = new QAction("刷新", this);
    addBookAct = new QAction("添加", this);
    removeBookAct = new QAction("删除", this);
    addUserAct = new QAction("添加用户", this);
    resetPasswordAct = new QAction("重置密码", this);
    appearanceSettingsAct = new QAction("界面设置", this);
    logoutAct = new QAction("退出登录", this);
    
    editBookAct->setEnabled(isAdmin);
    addBookAct->setEnabled(isAdmin);
    removeBookAct->setEnabled(isAdmin);
    addUserAct->setEnabled(isAdmin);
    resetPasswordAct->setEnabled(isAdmin);
    usersListAct->setEnabled(isAdmin);
    appearanceSettingsAct->setEnabled(isAdmin);
    bookDetailAct->setEnabled(isAdmin);
    
    QWidget* actionsStrip = new QWidget(central);
    actionsStrip->setObjectName("actionsStrip");
    actionsStrip->setStyleSheet(
        "QWidget#actionsStrip { "
        "    background-color: rgba(255,255,255,0.95); "
        "    border: 1px solid #e0e7ff; "
        "    border-radius: 14px; "
        "}"
    );
    QHBoxLayout* actionsLayout = new QHBoxLayout(actionsStrip);
    actionsLayout->setSpacing(18);
    actionsLayout->setContentsMargins(20, 14, 20, 14);
    
    const QString primaryActionStyle =
        "QPushButton { "
        "    padding: 10px 20px; "
        "    border-radius: 999px; "
        "    background-color: #2563eb; "
        "    color: white; "
        "    font-weight: 600; "
        "    border: none; "
        "}"
        "QPushButton:hover { background-color: #1d4ed8; }";
    
    const QString neutralActionStyle =
        "QPushButton { "
        "    padding: 10px 18px; "
        "    border-radius: 999px; "
        "    background-color: #f1f5f9; "
        "    color: #1f2937; "
        "    font-weight: 600; "
        "    border: 1px solid #e2e8f0; "
        "}"
        "QPushButton:hover { background-color: #e2e8f0; }";
    
    const QString dangerActionStyle =
        "QPushButton { "
        "    padding: 10px 18px; "
        "    border-radius: 999px; "
        "    background-color: #fee2e2; "
        "    color: #991b1b; "
        "    font-weight: 600; "
        "    border: 1px solid #fecaca; "
        "}"
        "QPushButton:hover { background-color: #fecaca; }";
    
    auto createActionButton = [&](QAction* action, const QString& style) -> QPushButton* {
        if (!action) return nullptr;
        QPushButton* button = new QPushButton(action->text(), actionsStrip);
        button->setCursor(Qt::PointingHandCursor);
        button->setStyleSheet(style);
        button->setEnabled(action->isEnabled());
        connect(button, &QPushButton::clicked, action, &QAction::trigger);
        connect(action, &QAction::changed, this, [button, action]() {
            button->setEnabled(action->isEnabled());
            button->setText(action->text());
        });
        return button;
    };
    
    auto createActionGroup = [&](const QString& title,
                                 const QList<std::pair<QAction*, QString>>& entries) -> QWidget* {
        QList<std::pair<QAction*, QString>> validEntries;
        for (const auto& entry : entries) {
            if (entry.first) {
                validEntries.push_back(entry);
            }
        }
        if (validEntries.isEmpty()) {
            return nullptr;
        }
        QFrame* group = new QFrame(actionsStrip);
        group->setStyleSheet("QFrame { background-color: transparent; }");
        QVBoxLayout* groupLayout = new QVBoxLayout(group);
        groupLayout->setSpacing(6);
        groupLayout->setContentsMargins(0, 0, 0, 0);
        
        QLabel* titleLabel = new QLabel(title, group);
        titleLabel->setStyleSheet("color: #94a3b8; font-size: 12px; font-weight: 600;");
        groupLayout->addWidget(titleLabel);
        
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(8);
        for (const auto& entry : validEntries) {
            QPushButton* btn = createActionButton(entry.first, entry.second);
            row->addWidget(btn);
        }
        groupLayout->addLayout(row);
        return group;
    };
    
    if (QWidget* primaryGroup = createActionGroup("常用操作", {
            {borrowAct, primaryActionStyle},
            {returnAct, primaryActionStyle},
            {myBorrowsAct, neutralActionStyle},
            {recommendAct, neutralActionStyle}
        })) {
        actionsLayout->addWidget(primaryGroup);
    }
    
    if (isAdmin) {
        if (QWidget* collectionGroup = createActionGroup("馆藏管理", {
                {addBookAct, neutralActionStyle},
                {editBookAct, neutralActionStyle},
                {removeBookAct, dangerActionStyle},
                {bookDetailAct, neutralActionStyle}
            })) {
            actionsLayout->addWidget(collectionGroup);
        }
        
        if (QWidget* userGroup = createActionGroup("账户与系统", {
                {addUserAct, neutralActionStyle},
                {usersListAct, neutralActionStyle},
                {resetPasswordAct, neutralActionStyle},
                {appearanceSettingsAct, neutralActionStyle}
            })) {
            actionsLayout->addWidget(userGroup);
        }
    }
    
    if (QWidget* sessionGroup = createActionGroup("系统", {
            {reloadAct, neutralActionStyle},
            {logoutAct, dangerActionStyle}
        })) {
        actionsLayout->addWidget(sessionGroup);
    }
    
    actionsLayout->addStretch();
    mainLayout->addWidget(actionsStrip);
    
    QHBoxLayout* contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(20);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addLayout(contentLayout, 1);
    
    QWidget* tableContainer = new QWidget(central);
    QVBoxLayout* tableColumn = new QVBoxLayout(tableContainer);
    tableColumn->setSpacing(12);
    tableColumn->setContentsMargins(0, 0, 0, 0);
    contentLayout->addWidget(tableContainer, 1);
    
    QScrollArea* sideScrollArea = new QScrollArea(central);
    sideScrollArea->setWidgetResizable(true);
    sideScrollArea->setFrameShape(QFrame::NoFrame);
    sideScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sideScrollArea->setFixedWidth(360);
    sideScrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    
    QWidget* sideContainer = new QWidget(sideScrollArea);
    QVBoxLayout* sideColumn = new QVBoxLayout(sideContainer);
    sideColumn->setSpacing(12);
    sideColumn->setContentsMargins(0, 0, 0, 0);
    sideScrollArea->setWidget(sideContainer);
    contentLayout->addWidget(sideScrollArea);
    
    QPushButton* quickBorrowBtn = nullptr;
    QPushButton* quickReturnBtn = nullptr;
    QPushButton* quickRecommendBtn = nullptr;
    QPushButton* quickMyBorrowBtn = nullptr;
    
    QFrame* statsFrame = new QFrame(sideContainer);
    statsFrame->setObjectName("statsCard");
    statsFrame->setStyleSheet(
        "QFrame#statsCard { "
        "    background-color: white; "
        "    border: 1px solid #e2e8f0; "
        "    border-radius: 12px; "
        "    padding: 18px; "
        "}"
    );
    QVBoxLayout* statsLayout = new QVBoxLayout(statsFrame);
    statsLayout->setSpacing(10);
    statsLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* statsTitle = new QLabel("馆藏概览", statsFrame);
    QFont statsTitleFont = statsTitle->font();
    statsTitleFont.setPointSize(14);
    statsTitleFont.setBold(true);
    statsTitle->setFont(statsTitleFont);
    statsTitle->setStyleSheet("color: #1e293b;");
    statsLayout->addWidget(statsTitle);
    
    QLabel* statsDateLabel = new QLabel(QDate::currentDate().toString("yyyy年M月d日 dddd"), statsFrame);
    statsDateLabel->setStyleSheet("color: #94a3b8; font-size: 12px;");
    statsLayout->addWidget(statsDateLabel);
    
    bookCountLabel = new QLabel(statsFrame);
    bookCountLabel->setWordWrap(true);
    bookCountLabel->setStyleSheet("color: #475569; font-size: 13px;");
    statsLayout->addWidget(bookCountLabel);
    
    QGridLayout* statsGrid = new QGridLayout();
    statsGrid->setSpacing(12);
    statsGrid->setContentsMargins(0, 0, 0, 0);
    
    auto createMetricWidget = [&](const QString& title, QLabel*& valueLabel) -> QWidget* {
        QWidget* wrapper = new QWidget(statsFrame);
        QVBoxLayout* layout = new QVBoxLayout(wrapper);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(2);
        
        QLabel* label = new QLabel(title, wrapper);
        label->setStyleSheet("color: #94a3b8; font-size: 12px;");
        valueLabel = new QLabel("--", wrapper);
        QFont valueFont = valueLabel->font();
        valueFont.setPointSize(18);
        valueFont.setBold(true);
        valueLabel->setFont(valueFont);
        valueLabel->setStyleSheet("color: #0f172a;");
        
        layout->addWidget(label);
        layout->addWidget(valueLabel);
        return wrapper;
    };
    
    statsGrid->addWidget(createMetricWidget("图书种类", totalTitlesValueLabel), 0, 0);
    statsGrid->addWidget(createMetricWidget("馆藏册数", totalCopiesValueLabel), 0, 1);
    statsGrid->addWidget(createMetricWidget("可借数量", availableCopiesValueLabel), 1, 0);
    statsGrid->addWidget(createMetricWidget("借出数量", borrowedCopiesValueLabel), 1, 1);
    statsLayout->addLayout(statsGrid);
    
    sideColumn->addWidget(statsFrame);
    
    QFrame* quickActionsFrame = new QFrame(sideContainer);
    quickActionsFrame->setObjectName("quickActionsCard");
    quickActionsFrame->setStyleSheet(
        "QFrame#quickActionsCard { "
        "    background-color: white; "
        "    border: 1px solid #e0e7ff; "
        "    border-radius: 12px; "
        "    padding: 18px; "
        "}"
    );
    QVBoxLayout* quickLayout = new QVBoxLayout(quickActionsFrame);
    quickLayout->setSpacing(10);
    quickLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* quickTitle = new QLabel("快捷操作", quickActionsFrame);
    QFont quickFont = quickTitle->font();
    quickFont.setPointSize(13);
    quickFont.setBold(true);
    quickTitle->setFont(quickFont);
    quickTitle->setStyleSheet("color: #1f2a37;");
    quickLayout->addWidget(quickTitle);
    
    QLabel* quickHint = new QLabel("常用功能只需一键即可到达。", quickActionsFrame);
    quickHint->setStyleSheet("color: #94a3b8; font-size: 12px;");
    quickLayout->addWidget(quickHint);
    
    QGridLayout* quickGrid = new QGridLayout();
    quickGrid->setSpacing(10);
    QString quickButtonStyle =
        "QPushButton { "
        "    padding: 12px; "
        "    border-radius: 10px; "
        "    border: 1px solid #d7e0f5; "
        "    background-color: #f8fafc; "
        "    color: #1e293b; "
        "    font-weight: 600; "
        "}"
        "QPushButton:hover { "
        "    background-color: #eef2ff; "
        "    border-color: #bfcbee; "
        "}";
    
    auto createQuickButton = [&](const QString& text, QPushButton*& target) {
        target = new QPushButton(text, quickActionsFrame);
        target->setCursor(Qt::PointingHandCursor);
        target->setStyleSheet(quickButtonStyle);
        return target;
    };
    
    createQuickButton("借阅图书", quickBorrowBtn);
    createQuickButton("归还图书", quickReturnBtn);
    createQuickButton("我的借阅", quickMyBorrowBtn);
    createQuickButton("热门推荐", quickRecommendBtn);
    
    quickGrid->addWidget(quickBorrowBtn, 0, 0);
    quickGrid->addWidget(quickReturnBtn, 0, 1);
    quickGrid->addWidget(quickMyBorrowBtn, 1, 0);
    quickGrid->addWidget(quickRecommendBtn, 1, 1);
    quickLayout->addLayout(quickGrid);
    
    sideColumn->addWidget(quickActionsFrame);
    setupRecommendationPanel(sideColumn);
    sideColumn->addStretch();

    // 表格框架 - 简洁风格
    QFrame* tableFrame = new QFrame(central);
    tableFrame->setStyleSheet(
        "QFrame { "
        "    background-color: white; "
        "    padding: 0px; "
        "    border: 1px solid #ecf0f1; "
        "}"
    );

    
    QVBoxLayout* tableFrameLayout = new QVBoxLayout(tableFrame);
    tableFrameLayout->setContentsMargins(0, 0, 0, 0);
    
    tableView = new QTableView(central);
    tableView->setModel(model);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView->setAlternatingRowColors(true);
    tableView->verticalHeader()->hide();
    tableView->setShowGrid(false);
    tableView->setWordWrap(false);
    tableView->setTextElideMode(Qt::ElideRight);
    tableView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    tableView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    tableView->setStyleSheet(ui::tableStyle());
    
    tableFrameLayout->addWidget(tableView);
    tableColumn->addWidget(tableFrame, 1);

    updateBookCount();

    setCentralWidget(central);

    // Connect search
    connect(searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    connect(searchBtn, &QPushButton::clicked, this, &MainWindow::filterBooks);
    connect(clearBtn, &QPushButton::clicked, [this]() {
        searchEdit->clear();
        filterBooks();
    });

    // Connect model changes to update UI
    connect(controller, &LibraryController::libraryChanged, this, &MainWindow::handleLibraryChanged);
    
    connect(borrowAct, &QAction::triggered, [this]() {
        QModelIndex idx = tableView->currentIndex();
        if (!idx.isValid()) { 
            QMessageBox::warning(this, "提示", "请先选择要借阅的图书！"); 
            return; 
        }
        int bookId = model->bookIdAtRow(idx.row());
        Book* book = controller->getBookById(bookId);
        if (!book) {
            QMessageBox::warning(this, "错误", "未找到该图书！");
            return;
        }
        if (!book->getIsAvailable()) {
            QMessageBox::warning(this, "提示", QString("图书《%1》暂无可用副本！").arg(QString::fromStdString(book->getTitle())));
            return;
        }
        
        // Show borrow days dialog
        BorrowDaysDialog daysDlg(this);
        if (daysDlg.exec() != QDialog::Accepted) {
            return;
        }
        int borrowDays = daysDlg.getDays();
        
        // Use borrower ID from login
        QString borrowerId = currentBorrowerId;
        
        if (controller->borrowBook(bookId, borrowerId.toStdString(), borrowDays)) {
            QMessageBox::information(this, "成功", 
                QString("成功借阅图书《%1》！\n\n借阅天数: %2天").arg(QString::fromStdString(book->getTitle())).arg(borrowDays));
            model->refresh();
            updateBookCount();
        } else {
            QMessageBox::warning(this, "失败", QString(" 借阅图书《%1》失败，请稍后重试！").arg(QString::fromStdString(book->getTitle())));
        }
    });

    connect(returnAct, &QAction::triggered, [this]() {
        QModelIndex idx = tableView->currentIndex();
        if (!idx.isValid()) { 
            QMessageBox::warning(this, "提示", "请先选择要归还的图书！"); 
            return; 
        }
        int bookId = model->bookIdAtRow(idx.row());
        QString borrowerId = currentBorrowerId;
        
        if (borrowerId.isEmpty()) {
            QMessageBox::warning(this, "错误", "无法获取借阅人ID，请重新登录！");
            return;
        }
        
        // 确认对话框
        int ret = QMessageBox::question(this, "确认归还", 
            QString("确定要归还图书ID: %1 吗？").arg(bookId),
            QMessageBox::Yes | QMessageBox::No);
        if (ret != QMessageBox::Yes) {
            return;
        }
        
        if (controller->returnBook(bookId, borrowerId.toStdString())) {
            QMessageBox::information(this, "成功", "归还成功！");
            model->refresh();
            updateBookCount();
        } else {
            QMessageBox::warning(this, "失败", 
                QString("归还失败！\n\n可能的原因：\n1. 您未借阅过该图书\n2. 该图书已被归还\n3. 数据库连接失败\n\n借阅人ID: %1\n图书ID: %2")
                .arg(borrowerId).arg(bookId));
        }
    });

    connect(reloadAct, &QAction::triggered, [this]() {
        controller->loadFromDatabase();
        model->refresh();
        updateBookCount();
        QMessageBox::information(this, "刷新完成", "已从数据库重新加载数据！");
    });

    connect(editBookAct, &QAction::triggered, [this]() {
        if (currentUserType != "admin") {
            QMessageBox::warning(this, "你不配", "此操作需要管理员权限！");
            return;
        }
        QModelIndex idx = tableView->currentIndex();
        if (!idx.isValid()) {
            QMessageBox::warning(this, "提示", "请先选择要编辑的图书！");
            return;
        }
        int bookId = model->bookIdAtRow(idx.row());
        Book* book = controller->getBookById(bookId);
        if (!book) {
            QMessageBox::warning(this, "错误", "未找到该图书！");
            return;
        }
        
        EditBookDialog dlg(this);
        dlg.setBookData(book->getBookId(), 
                       QString::fromStdString(book->getTitle()),
                       QString::fromStdString(book->getAuthor()),
                       QString::fromStdString(book->getIsbn()),
                       QString::fromStdString(book->getCategory()),
                       book->getTotalCopies());
        
        if (dlg.exec() == QDialog::Accepted) {
            // Remove old book and add updated one
            controller->removeBook(bookId);
            Book updatedBook(dlg.getId(), dlg.getTitleStr().toStdString(), 
                           dlg.getAuthor().toStdString(), dlg.getIsbn().toStdString(),
                           dlg.getCategory().toStdString(), dlg.getCopies());
            // Preserve borrow status
            int borrowedCount = book->getTotalCopies() - book->getAvailableCopies();
            for (int i = 0; i < borrowedCount && i < dlg.getCopies(); i++) {
                updatedBook.borrowBook();
            }
            controller->addBook(updatedBook);
            model->refresh();
            updateBookCount();
            QMessageBox::information(this, "编辑成功", QString("图书《%1》信息已更新！").arg(dlg.getTitleStr()));
        }
    });
    
    connect(addBookAct, &QAction::triggered, [this]() {
        if (currentUserType != "admin") {
            QMessageBox::warning(this, "你不配", "此操作需要管理员权限！");
            return;
        }
        AddBookDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted) {
            Book b(dlg.getId(), dlg.getTitleStr().toStdString(), dlg.getAuthor().toStdString(), 
                   dlg.getIsbn().toStdString(), dlg.getCategory().toStdString(), dlg.getCopies());
            controller->addBook(b);
            model->refresh();
            updateBookCount();
            QMessageBox::information(this, "添加成功", QString("图书《%1》已成功添加到图书馆！").arg(dlg.getTitleStr()));
        }
    });

    connect(removeBookAct, &QAction::triggered, [this]() {
        if (currentUserType != "admin") {
            QMessageBox::warning(this, "你不配", "此操作需要管理员权限！");
            return;
        }
        QModelIndex idx = tableView->currentIndex();
        if (!idx.isValid()) { 
            QMessageBox::warning(this, "提示", "请先选择要删除的图书！"); 
            return; 
        }
        int bookId = model->bookIdAtRow(idx.row());
        Book* book = controller->getBookById(bookId);
        if (!book) {
            QMessageBox::warning(this, "错误", "未找到该图书！");
            return;
        }
        QMessageBox::StandardButton reply = QMessageBox::question(this, "确认删除", 
            QString("确定要删除图书《%1》吗？\n\n此操作不可恢复！").arg(QString::fromStdString(book->getTitle())), 
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            QString title = QString::fromStdString(book->getTitle());
            controller->removeBook(bookId);
            model->refresh();
            updateBookCount();
            QMessageBox::information(this, "删除成功", QString("图书《%1》已成功删除！").arg(title));
        }
    });

    connect(addUserAct, &QAction::triggered, [this]() {
        if (currentUserType != "admin") {
            QMessageBox::warning(this, "你不配", "此操作需要管理员权限！");
            return;
        }
        AddUserDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted) {
            QString type = dlg.getType();
            QString id = dlg.getId();
            QString name = dlg.getName();
            QString dept = dlg.getDept();
            QString extra = dlg.getExtra();
            QString password = dlg.getPassword();
            int limit = dlg.getLimit();
            
            // Validate password
            if (password.length() < 6) {
                QMessageBox::warning(this, "密码无效", "密码长度至少需要6位！");
                return;
            }
            
            // Check if username already exists
            db::DBManager* dbManager = controller->getDBManager();
            if (dbManager && dbManager->isConnected() && dbManager->userExists(id.toStdString())) {
                QMessageBox::warning(this, "用户名已存在", QString("用户名「%1」已存在，请使用其他用户名！").arg(id));
                return;
            }
            
            // Create borrower
            Borrower* borrower = nullptr;
            if (type == "student") {
                borrower = new Student(id.toStdString(), name.toStdString(), 
                    dept.toStdString(), extra.toStdString(), limit);
            } else {
                borrower = new Teacher(id.toStdString(), name.toStdString(), 
                    dept.toStdString(), extra.toStdString(), limit);
            }
            controller->addBorrower(borrower);
            
            // Create login user account
            if (dbManager && dbManager->isConnected()) {
                if (dbManager->createUser(id.toStdString(), password.toStdString(), "user", id.toStdString())) {
                    QMessageBox::information(this, "添加成功", 
                        QString("用户「%1」已成功添加！\n\n用户名: %2\n密码: %3")
                        .arg(name).arg(id).arg(password));
                } else {
                    QMessageBox::warning(this, "部分成功", 
                        QString("用户「%1」已添加，但登录账户创建失败！").arg(name));
                }
            } else {
                QMessageBox::information(this, "添加成功", QString("用户「%1」已成功添加！").arg(name));
            }
        }
    });
    
    connect(usersListAct, &QAction::triggered, [this]() {
        if (currentUserType != "admin") {
            QMessageBox::warning(this, "权限不足", "🔐 此操作需要管理员权限！");
            return;
        }
        UsersListDialog dlg(controller, this);
        dlg.exec();
    });
    
    connect(appearanceSettingsAct, &QAction::triggered, this, &MainWindow::openAppearanceSettings);
    
    auto wireQuickAction = [](QPushButton* button, QAction* action) {
        if (button && action) {
            QObject::connect(button, &QPushButton::clicked, action, &QAction::trigger);
        }
    };
    wireQuickAction(quickBorrowBtn, borrowAct);
    wireQuickAction(quickReturnBtn, returnAct);
    wireQuickAction(quickRecommendBtn, recommendAct);
    wireQuickAction(quickMyBorrowBtn, myBorrowsAct);
    
    connect(myBorrowsAct, &QAction::triggered, [this]() {
        MyBorrowsDialog dlg(currentBorrowerId, controller, this);
        dlg.exec();
    });
    
    connect(recommendAct, &QAction::triggered, this, &MainWindow::openRecommendationDialog);
    
    connect(bookDetailAct, &QAction::triggered, [this]() {
        if (currentUserType != "admin") {
            QMessageBox::warning(this, "权限不足", "此操作需要管理员权限！");
            return;
        }
        QModelIndex idx = tableView->currentIndex();
        if (!idx.isValid()) {
            QMessageBox::warning(this, "提示", "请先选择要查看的图书！");
            return;
        }
        int bookId = model->bookIdAtRow(idx.row());
        BookDetailDialog dlg(bookId, controller, this);
        dlg.exec();
    });
    
    connect(resetPasswordAct, &QAction::triggered, [this]() {
        if (currentUserType != "admin") {
            QMessageBox::warning(this, "你不配", "此操作需要管理员权限！");
            return;
        }
        ResetPasswordDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted) {
            QString username = dlg.getUsername();
            QString newPassword = dlg.getNewPassword();
            
            if (newPassword.length() < 6) {
                QMessageBox::warning(this, "密码无效", "密码长度至少需要6位！");
                return;
            }
            
            db::DBManager* dbManager = controller->getDBManager();
            if (!dbManager || !dbManager->isConnected()) {
                QMessageBox::warning(this, "错误", "数据库未连接，无法重置密码！");
                return;
            }
            
            if (!dbManager->userExists(username.toStdString())) {
                QMessageBox::warning(this, "用户不存在", QString("用户名「%1」不存在！").arg(username));
                return;
            }
            
            if (dbManager->updateUserPassword(username.toStdString(), newPassword.toStdString())) {
                QMessageBox::information(this, "重置成功", 
                    QString("用户「%1」的密码已成功重置！\n\n新密码: %2").arg(username).arg(newPassword));
            } else {
                QMessageBox::warning(this, "重置失败", "密码重置失败，请稍后重试！");
            }
        }
    });

    connect(logoutAct, &QAction::triggered, this, &MainWindow::onLogout);
    
    // 状态栏
    statusBar = QMainWindow::statusBar();
    statusBar->setStyleSheet(
        "QStatusBar { "
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "        stop:0 #f8fafc, stop:1 white); "
        "    border-top: 2px solid #e0e7ff; "
        "    color: #64748b; "
        "    font-size: 12px; "
        "    padding: 8px; "
        "}"
    );
    statusBar->showMessage(QString("就绪 | 欢迎 %1 使用图书馆管理系统").arg(currentUsername));
}

void MainWindow::setupRecommendationPanel(QVBoxLayout* containerLayout) {
    QFrame* recommendationFrame = new QFrame(this);
    recommendationFrame->setObjectName("recommendationFrame");
    recommendationFrame->setStyleSheet(
        "QFrame#recommendationFrame { "
        "    background-color: white; "
        "    border: 1px solid #ecf0f1; "
        "    border-radius: 10px; "
        "}"
    );
    
    QVBoxLayout* recommendationLayout = new QVBoxLayout(recommendationFrame);
    recommendationLayout->setContentsMargins(20, 16, 20, 16);
    recommendationLayout->setSpacing(8);
    
    QHBoxLayout* headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* titleLabel = new QLabel("智能推荐", recommendationFrame);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #1f2a37;");
    
    recommendationMetaLabel = new QLabel("正在加载推荐...", recommendationFrame);
    recommendationMetaLabel->setStyleSheet("color: #64748b;");
    
    QPushButton* moreButton = new QPushButton("查看更多", recommendationFrame);
    moreButton->setCursor(Qt::PointingHandCursor);
    moreButton->setStyleSheet(
        "QPushButton { "
        "    padding: 6px 14px; "
        "    background-color: #3498db; "
        "    color: white; "
        "    border-radius: 4px; "
        "    border: none; "
        "    font-weight: 500; "
        "}"
        "QPushButton:hover { "
        "    background-color: #2980b9; "
        "}"
    );
    connect(moreButton, &QPushButton::clicked, this, &MainWindow::openRecommendationDialog);
    
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(recommendationMetaLabel);
    headerLayout->addSpacing(10);
    headerLayout->addWidget(moreButton);
    
    recommendationLayout->addLayout(headerLayout);
    
    recommendationList = new QListWidget(recommendationFrame);
    recommendationList->setObjectName("recommendationList");
    recommendationList->setSelectionMode(QAbstractItemView::SingleSelection);
    recommendationList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    recommendationList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    recommendationList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    recommendationList->setFocusPolicy(Qt::StrongFocus);
    recommendationList->setSpacing(8);
    recommendationList->setUniformItemSizes(false);
    recommendationList->setMouseTracking(true);
    recommendationList->setSelectionRectVisible(false);
    recommendationList->setMinimumHeight(220);
    recommendationList->setMaximumHeight(260);
    recommendationList->setStyleSheet(
        "QListWidget#recommendationList { "
        "    background-color: transparent; "
        "    border: none; "
        "    padding: 6px 2px; "
        "}"
        "QListWidget#recommendationList::item { "
        "    margin: 2px 0; "
        "    padding: 2px; "
        "    border-radius: 10px; "
        "    color: #0f172a; "
        "}"
        "QListWidget#recommendationList::item:selected { "
        "    background-color: rgba(37, 99, 235, 0.12); "
        "    color: #0f172a; "
        "}"
    );
    connect(recommendationList, &QListWidget::itemClicked, this, &MainWindow::handleRecommendationActivated);
    connect(recommendationList, &QListWidget::itemActivated, this, &MainWindow::handleRecommendationActivated);
    
    recommendationLayout->addWidget(recommendationList);
    containerLayout->addWidget(recommendationFrame);
    
    if (!recommendationTimer) {
        recommendationTimer = new QTimer(this);
        recommendationTimer->setInterval(4000);
        connect(recommendationTimer, &QTimer::timeout, this, &MainWindow::advanceRecommendationCarousel);
    }
    
    refreshEmbeddedRecommendations();
}

void MainWindow::refreshEmbeddedRecommendations() {
    if (!recommendationList) {
        return;
    }
    recommendationList->clear();
    embeddedRecommendations = controller->recommendBooks(8);
    recommendationCarouselIndex = -1;
    
    if (embeddedRecommendations.empty()) {
        if (recommendationMetaLabel) {
            recommendationMetaLabel->setText("暂无推荐数据");
        }
        if (recommendationTimer && recommendationTimer->isActive()) {
            recommendationTimer->stop();
        }
        QListWidgetItem* placeholder = new QListWidgetItem("暂无数据，稍后再试...");
        placeholder->setFlags(Qt::NoItemFlags);
        recommendationList->addItem(placeholder);
        return;
    }
    
    if (recommendationMetaLabel) {
        recommendationMetaLabel->setText(QString("为你推荐 %1 本热门图书 · 自动轮播").arg(embeddedRecommendations.size()));
    }
    
    auto createCard = [&](const Book& book) -> QWidget* {
        QWidget* card = new QWidget(recommendationList);
        card->setObjectName("recommendationCard");
        QHBoxLayout* layout = new QHBoxLayout(card);
        layout->setContentsMargins(12, 8, 12, 8);
        layout->setSpacing(12);

        QVBoxLayout* textLayout = new QVBoxLayout();
        textLayout->setSpacing(4);

        QLabel* titleLabel = new QLabel(QString("《%1》").arg(QString::fromStdString(book.getTitle())), card);
        titleLabel->setStyleSheet("color: #0f172a; font-weight: 600; font-size: 14px;");

        QString meta = QString("%1 · 分类 %2 · ISBN %3")
            .arg(QString::fromStdString(book.getAuthor()))
            .arg(QString::fromStdString(book.getCategory()))
            .arg(QString::fromStdString(book.getIsbn()));
        QLabel* metaLabel = new QLabel(meta, card);
        metaLabel->setStyleSheet("color: #64748b; font-size: 12px;");

        textLayout->addWidget(titleLabel);
        textLayout->addWidget(metaLabel);

        QVBoxLayout* rightLayout = new QVBoxLayout();
        rightLayout->setSpacing(4);
        rightLayout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        QLabel* availabilityLabel = new QLabel(
            QString("可借 %1 / %2").arg(book.getAvailableCopies()).arg(book.getTotalCopies()), card);
        availabilityLabel->setStyleSheet("color: #0f172a; font-weight: 600;");

        QLabel* statusChip = new QLabel(book.getAvailableCopies() > 0 ? "可借" : "等候中", card);
        statusChip->setStyleSheet(book.getAvailableCopies() > 0 ?
            "QLabel { padding: 2px 10px; border-radius: 999px; background-color: #dcfce7; color: #166534; font-size: 11px; font-weight: 600; }" :
            "QLabel { padding: 2px 10px; border-radius: 999px; background-color: #fee2e2; color: #991b1b; font-size: 11px; font-weight: 600; }");

        rightLayout->addWidget(availabilityLabel, 0, Qt::AlignRight);
        rightLayout->addWidget(statusChip, 0, Qt::AlignRight);

        layout->addLayout(textLayout, 1);
        layout->addLayout(rightLayout);
        return card;
    };

    for (const auto& book : embeddedRecommendations) {
        QListWidgetItem* item = new QListWidgetItem(recommendationList);
        item->setData(Qt::UserRole, book.getBookId());
        item->setToolTip(QString("%1\n分类：%2\nISBN：%3")
            .arg(QString::fromStdString(book.getTitle()))
            .arg(QString::fromStdString(book.getCategory()))
            .arg(QString::fromStdString(book.getIsbn())));

        QWidget* cardWidget = createCard(book);
        item->setSizeHint(cardWidget->sizeHint());
        recommendationList->setItemWidget(item, cardWidget);
    }
    
    if (recommendationList->count() == 1 && recommendationTimer) {
        recommendationTimer->stop();
    } else if (recommendationTimer && !recommendationTimer->isActive()) {
        recommendationTimer->start();
    }
    
    advanceRecommendationCarousel();
}

void MainWindow::advanceRecommendationCarousel() {
    if (!recommendationList || recommendationList->count() == 0) {
        if (recommendationTimer) {
            recommendationTimer->stop();
        }
        return;
    }
    int count = recommendationList->count();
    if (count <= 0) return;
    recommendationCarouselIndex = (recommendationCarouselIndex + 1) % count;
    recommendationList->setCurrentRow(recommendationCarouselIndex);
    QListWidgetItem* item = recommendationList->item(recommendationCarouselIndex);
    if (item) {
        recommendationList->scrollToItem(item, QAbstractItemView::PositionAtCenter);
    }
}

void MainWindow::handleLibraryChanged() {
    updateBookCount();
    refreshEmbeddedRecommendations();
}

void MainWindow::openRecommendationDialog() {
    RecommendBooK dlg(controller, this);
    dlg.exec();
    refreshEmbeddedRecommendations();
}

void MainWindow::handleRecommendationActivated(QListWidgetItem* item) {
    if (!item || !model || !tableView) {
        return;
    }
    bool ok = false;
    int bookId = item->data(Qt::UserRole).toInt(&ok);
    if (!ok || bookId <= 0) {
        return;
    }
    
    for (int row = 0; row < model->rowCount(); ++row) {
        if (model->bookIdAtRow(row) == bookId) {
            QModelIndex idx = model->index(row, 0);
            tableView->setCurrentIndex(idx);
            tableView->scrollTo(idx, QAbstractItemView::PositionAtCenter);
            break;
        }
    }
}

void MainWindow::openAppearanceSettings() {
    if (currentUserType != "admin") {
        QMessageBox::warning(this, "权限不足", "此操作需要管理员权限！");
        return;
    }
    AppearanceDialog dlg(this);
    dlg.exec();
}

void MainWindow::onSearchTextChanged(const QString& text) {
    filterBooks();
    updateBookCount();
}

void MainWindow::applyBackgroundFromSettings() {
    QString backgroundPath = AppSettings::instance().mainBackgroundPath();
    hasCustomMainBackground = false;
    mainBackgroundPixmap = QPixmap();
    if (!backgroundPath.isEmpty() && QFile::exists(backgroundPath)) {
        QPixmap pix(backgroundPath);
        if (!pix.isNull()) {
            mainBackgroundPixmap = pix;
            hasCustomMainBackground = true;
        }
    }
    updateWindowBackgroundBrush();
}

void MainWindow::updateWindowBackgroundBrush() {
    QPalette pal = palette();
    if (hasCustomMainBackground && !mainBackgroundPixmap.isNull()) {
        QPixmap scaled = mainBackgroundPixmap.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        pal.setBrush(QPalette::Window, scaled);
        setAutoFillBackground(true);
    } else {
        pal.setBrush(QPalette::Window, QBrush());
        setAutoFillBackground(false);
    }
    setPalette(pal);
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    if (hasCustomMainBackground) {
        updateWindowBackgroundBrush();
    }
}

void MainWindow::filterBooks() {
    QString searchText = searchEdit->text().toLower();
    if (searchText.isEmpty()) {
        model->refresh();
        statusBar->showMessage("显示所有图书");
        return;
    }
    

    auto allBooks = controller->allBooks();
    std::vector<Book*> filtered;
    for (auto* book : allBooks) {
        QString title = QString::fromStdString(book->getTitle()).toLower();
        QString author = QString::fromStdString(book->getAuthor()).toLower();
        QString category = QString::fromStdString(book->getCategory()).toLower();
        QString isbn = QString::fromStdString(book->getIsbn()).toLower();
        
        if (title.contains(searchText) || author.contains(searchText) || 
            category.contains(searchText) || isbn.contains(searchText)) {
            filtered.push_back(book);
        }
    }
    
    // Update model with filtered results
    model->setFilteredBooks(filtered);
    statusBar->showMessage(QString("搜索关键词: \"%1\" | 找到 %2 本图书").arg(searchText).arg(filtered.size()));
}

void MainWindow::updateBookCount() {
    auto allBooks = controller->allBooks();
    int totalCopies = 0, available = 0, borrowed = 0;
    for (auto* book : allBooks) {
        totalCopies += book->getTotalCopies();
        available += book->getAvailableCopies();
        borrowed += (book->getTotalCopies() - book->getAvailableCopies());
    }
    
    int totalTitles = static_cast<int>(allBooks.size());
    if (bookCountLabel) {
        bookCountLabel->setText(
            QString("当前馆藏: %1 种图书，共 %2 册。").arg(totalTitles).arg(totalCopies));
    }
    if (totalTitlesValueLabel) {
        totalTitlesValueLabel->setText(QString::number(totalTitles));
    }
    if (totalCopiesValueLabel) {
        totalCopiesValueLabel->setText(QString::number(totalCopies));
    }
    if (availableCopiesValueLabel) {
        availableCopiesValueLabel->setText(QString::number(available));
    }
    if (borrowedCopiesValueLabel) {
        borrowedCopiesValueLabel->setText(QString::number(borrowed));
    }
    if (totalChipLabel) {
        totalChipLabel->setText(QString("馆藏 %1 种").arg(totalTitles));
    }
    if (availableChipLabel) {
        availableChipLabel->setText(QString("可借 %1").arg(available));
    }
    if (borrowedChipLabel) {
        borrowedChipLabel->setText(QString("借出 %1").arg(borrowed));
    }
}

void MainWindow::updateStatusBar() {
    QString status = controller->isDatabaseConnected() ? 
        "数据库: MySQL 已连接" : "数据库: 文件存储模式";
    statusBar->showMessage(status);
}

QString MainWindow::ensureBorrowerBinding(const QString& username, const QString& userType, const QString& displayNameFallback) {
    if (!controller || !controller->isDatabaseConnected()) {
        return displayNameFallback.isEmpty() ? username : displayNameFallback;
    }
    db::DBManager* dbManager = controller->getDBManager();
    if (!dbManager) {
        return displayNameFallback.isEmpty() ? username : displayNameFallback;
    }
    
    const std::string borrowerId = username.toStdString();
    const std::string displayName = displayNameFallback.isEmpty() ? username.toStdString() : displayNameFallback.toStdString();
    
    if (userType == "admin") {
        Teacher adminBorrower(borrowerId, displayName, "管理员", "系统管理员", 10);
        dbManager->upsertBorrower(&adminBorrower);
    } else {
        Student defaultReader(borrowerId, displayName, "普通院系", "游客", 5);
        dbManager->upsertBorrower(&defaultReader);
    }
    dbManager->updateUserBorrowerId(username.toStdString(), borrowerId);
    
    return QString::fromStdString(borrowerId);
}

bool MainWindow::showLogin() {
    LoginDialog dlg(this);
    while (true) {
        if (dlg.exec() != QDialog::Accepted) {
            return false;
        }
        
        QString username = dlg.getUsername();
        QString password = dlg.getPassword();
        QString userType = dlg.getUserType();
        QString borrowerId;
        
        if (authenticateUser(username, password, userType, borrowerId)) {
            currentUsername = username;
            currentUserType = userType;
            currentBorrowerId = borrowerId.isEmpty() ? username : borrowerId; // Fallback to username if no borrower_id
            isLoggedIn = true;
            return true;
        } else {
            QMessageBox::warning(this, "登录失败", 
                QString("用户名或密码错误！\n\n用户类型: %1").arg(userType == "admin" ? "管理员" : "普通用户"));
        }
    }
}

bool MainWindow::authenticateUser(const QString& username, const QString& password, const QString& userType, QString& outBorrowerId) {
    if (!controller) {
        if (userType == "admin" && username == "admin" && password == "admin123") {
            outBorrowerId = "";
            return true;
        }
        if (userType == "user" && username == "user" && password == "user123") {
            outBorrowerId = "user";
            return true;
        }
        return false;
    }
    
    db::DBManager* dbManager = controller->getDBManager();
    
    // If database is connected, use database authentication
    if (dbManager && dbManager->isConnected()) {
        std::string dbUserType;
        std::string dbBorrowerId;
        if (dbManager->authenticateUser(username.toStdString(), password.toStdString(), dbUserType, dbBorrowerId)) {
            // Check if user type matches
            QString expectedType = (userType == "admin") ? "admin" : "user";
            if (QString::fromStdString(dbUserType) == expectedType) {
                QString borrowerId = QString::fromStdString(dbBorrowerId);
                if (borrowerId.isEmpty()) {
                    borrowerId = ensureBorrowerBinding(username, expectedType, username);
                }
                outBorrowerId = borrowerId;
                return true;
            }
        }
        return false;
    }
    
    // Fallback to hardcoded authentication if database is not connected
    if (userType == "admin") {
        if (username == "admin" && password == "admin123") {
            outBorrowerId = "";
            return true;
        }
    } else {
        if (username == "user" && password == "user123") {
            outBorrowerId = "user";
            return true;
        }
        // Also allow login with borrower ID (any borrower ID with password "123456")
        if (password == "123456") {
            outBorrowerId = username;
            return true;
        }
    }
    return false;
}

void MainWindow::updateUserDisplay() {
    QString userText;
    if (currentUserType == "admin") {
        userText = QString("Admin: %1").arg(currentUsername);
    } else {
        userText = QString("User: %1").arg(currentUsername);
    }
    userLabel->setText(userText);
}

void MainWindow::onLogout() {
    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认退出", 
        "确定要退出登录吗？", QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        isLoggedIn = false;
        currentUsername = "";
        currentUserType = "";
        currentBorrowerId = "";
        
        if (!showLogin()) {
            QApplication::exit(0);
        } else {
            updateUserDisplay();
            bool isAdmin = (currentUserType == "admin");
            editBookAct->setEnabled(isAdmin);
            addBookAct->setEnabled(isAdmin);
            removeBookAct->setEnabled(isAdmin);
            addUserAct->setEnabled(isAdmin);
            resetPasswordAct->setEnabled(isAdmin);
        }
    }
}


// void MainWindow::RecommendBooks() {
//     RecommendBooK dlg(controller, this);
//     dlg.exec();
// }
