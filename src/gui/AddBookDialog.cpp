#include "AddBookDialog.h"
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QIntValidator>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QSpinBox>
#include "UiTheme.h"

AddBookDialog::AddBookDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("添加图书");
    setMinimumSize(450, 550);
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
    QLabel* titleLabel = new QLabel("添加新图书", this);
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
    contentLayout->setSpacing(20);
    contentLayout->setContentsMargins(30, 25, 30, 25);
    
    // Frame for form
    QFrame* formFrame = new QFrame(this);
    formFrame->setStyleSheet("QFrame { background-color: transparent; }");
    
    QFormLayout* layout = new QFormLayout(formFrame);
    layout->setSpacing(18);
    layout->setLabelAlignment(Qt::AlignRight);
    
    QString inputStyle = ui::inputFieldStyle();
    QString labelStyle = ui::labelStyle();
    
    idEdit = new QLineEdit(this);
    idEdit->setValidator(new QIntValidator(1, 1000000, this));
    idEdit->setPlaceholderText("输入图书ID");
    idEdit->setStyleSheet(inputStyle);
    
    titleEdit = new QLineEdit(this);
    titleEdit->setPlaceholderText("输入书名");
    titleEdit->setStyleSheet(inputStyle);
    
    authorEdit = new QLineEdit(this);
    authorEdit->setPlaceholderText("输入作者");
    authorEdit->setStyleSheet(inputStyle);
    
    isbnEdit = new QLineEdit(this);
    isbnEdit->setPlaceholderText("输入ISBN");
    isbnEdit->setStyleSheet(inputStyle);
    
    categoryEdit = new QLineEdit(this);
    categoryEdit->setPlaceholderText("输入分类");
    categoryEdit->setStyleSheet(inputStyle);
    
    copiesSpin = new QSpinBox(this);
    copiesSpin->setMinimum(1);
    copiesSpin->setMaximum(1000);
    copiesSpin->setValue(1);
    copiesSpin->setStyleSheet(inputStyle);
    
    QLabel* idLabel = new QLabel("图书ID:", this);
    idLabel->setStyleSheet(labelStyle);
    QLabel* titleLabel2 = new QLabel("书名:", this);
    titleLabel2->setStyleSheet(labelStyle);
    QLabel* authorLabel = new QLabel("作者:", this);
    authorLabel->setStyleSheet(labelStyle);
    QLabel* isbnLabel = new QLabel("ISBN:", this);
    isbnLabel->setStyleSheet(labelStyle);
    QLabel* categoryLabel = new QLabel("分类:", this);
    categoryLabel->setStyleSheet(labelStyle);
    QLabel* copiesLabel = new QLabel("副本数:", this);
    copiesLabel->setStyleSheet(labelStyle);

    layout->addRow(idLabel, idEdit);
    layout->addRow(titleLabel2, titleEdit);
    layout->addRow(authorLabel, authorEdit);
    layout->addRow(isbnLabel, isbnEdit);
    layout->addRow(categoryLabel, categoryEdit);
    layout->addRow(copiesLabel, copiesSpin);
    
    contentLayout->addWidget(formFrame);
    mainLayout->addWidget(contentFrame, 1);

    // 按钮区域
    QFrame* buttonFrame = new QFrame(this);
    buttonFrame->setStyleSheet("QFrame { background-color: transparent; }");
    QHBoxLayout* buttonLayout = new QHBoxLayout(buttonFrame);
    buttonLayout->setContentsMargins(30, 15, 30, 20);
    buttonLayout->addStretch();
    
    QPushButton* cancelBtn = new QPushButton("取消", this);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(ui::secondaryButtonStyle());
    
    QPushButton* addBtn = new QPushButton("保存", this);
    addBtn->setCursor(Qt::PointingHandCursor);
    addBtn->setStyleSheet(ui::primaryButtonStyle());
    
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(addBtn);
    mainLayout->addWidget(buttonFrame);
    
    QDialogButtonBox* buttons = new QDialogButtonBox(this);
    buttons->addButton(addBtn, QDialogButtonBox::AcceptRole);
    buttons->addButton(cancelBtn, QDialogButtonBox::RejectRole);
    
    connect(addBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

int AddBookDialog::getId() const { return idEdit->text().toInt(); }
QString AddBookDialog::getTitleStr() const { return titleEdit->text(); }
QString AddBookDialog::getAuthor() const { return authorEdit->text(); }
QString AddBookDialog::getIsbn() const { return isbnEdit->text(); }
QString AddBookDialog::getCategory() const { return categoryEdit->text(); }
int AddBookDialog::getCopies() const { return copiesSpin->value(); }
