#include "UsersListDialog.h"

#include "LibraryController.h"
#include "BorrowRecordsDialog.h"
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
#include <QMessageBox>

UsersListDialog::UsersListDialog(LibraryController* controller, QWidget* parent)
    : QDialog(parent), controller(controller) {
    setWindowTitle("用户管理");
    setMinimumSize(1100, 680);
    resize(1100, 680);
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
    QLabel* titleLabel = new QLabel("用户管理", this);
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
    contentLayout->setContentsMargins(20, 20, 20, 16);
    
    QFrame* tableFrame = new QFrame(this);
    tableFrame->setStyleSheet(ui::softCardFrameStyle());
    QVBoxLayout* tableLayout = new QVBoxLayout(tableFrame);
    tableLayout->setSpacing(10);
    tableLayout->setContentsMargins(0, 0, 0, 0);
    
    QHBoxLayout* headerRow = new QHBoxLayout();
    QLabel* tableTitle = new QLabel("系统用户列表", this);
    tableTitle->setStyleSheet(ui::sectionTitleStyle());
    headerRow->addWidget(tableTitle);
    headerRow->addStretch();
    
    QLabel* hintLabel = new QLabel("提示: 双击用户可查看其借阅记录", this);
    hintLabel->setStyleSheet(ui::subtleTextStyle());
    headerRow->addWidget(hintLabel);
    tableLayout->addLayout(headerRow);
    
    model = new QStandardItemModel(this);
    model->setColumnCount(6);
    model->setHorizontalHeaderLabels({"用户名", "用户类型", "借阅人ID", "借阅人姓名", "院系", "创建时间"});
    
    tableView = new QTableView(this);
    tableView->setModel(model);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->horizontalHeader()->setStretchLastSection(false);
    tableView->setAlternatingRowColors(true);
    tableView->verticalHeader()->hide();
    tableView->setShowGrid(false);
    tableView->setWordWrap(false);
    tableView->setTextElideMode(Qt::ElideRight);
    tableView->setStyleSheet(ui::tableStyle());
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    connect(tableView, &QTableView::doubleClicked, this, &UsersListDialog::showUserBorrowRecords);
    
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

void UsersListDialog::refreshData() {
    model->setRowCount(0);
    
    if (!controller) {
        return;
    }
    
    if (!controller->isDatabaseConnected()) {
        return;
    }
    
    db::DBManager* dbManager = controller->getDBManager();
    if (!dbManager) {
        return;
    }
    
    std::vector<std::map<std::string, std::string>> users;
    if (dbManager->getAllUsers(users)) {
        for (const auto& user : users) {
            int row = model->rowCount();
            model->insertRow(row);
            
            model->setItem(row, 0, new QStandardItem(QString::fromStdString(user.at("username"))));
            
            QString userType = QString::fromStdString(user.at("user_type"));
            model->setItem(row, 1, new QStandardItem(userType == "admin" ? "管理员" : "普通用户"));
            
            QString borrowerId = user.find("borrower_id") != user.end() && !user.at("borrower_id").empty() ?
                QString::fromStdString(user.at("borrower_id")) : "-";
            model->setItem(row, 2, new QStandardItem(borrowerId));
            
            QString borrowerName = user.find("borrower_name") != user.end() && !user.at("borrower_name").empty() ?
                QString::fromStdString(user.at("borrower_name")) : "-";
            model->setItem(row, 3, new QStandardItem(borrowerName));
            
            QString dept = user.find("borrower_dept") != user.end() && !user.at("borrower_dept").empty() ?
                QString::fromStdString(user.at("borrower_dept")) : "-";
            model->setItem(row, 4, new QStandardItem(dept));
            
            QString createdAt = QString::fromStdString(user.at("created_at"));
            createdAt = createdAt.left(19);
            model->setItem(row, 5, new QStandardItem(createdAt));
        }
    }
}

void UsersListDialog::showUserBorrowRecords(const QModelIndex& index) {
    if (!index.isValid()) return;
    
    int row = index.row();
    QString borrowerId = model->item(row, 2)->text();
    QString borrowerName = model->item(row, 3)->text();
    
    if (borrowerId == "-" || borrowerId.isEmpty()) {
        QMessageBox::information(this, "提示", "该用户没有关联的借阅人信息。");
        return;
    }
    
    BorrowRecordsDialog dlg(borrowerId, borrowerName, controller, this);
    dlg.exec();
}
