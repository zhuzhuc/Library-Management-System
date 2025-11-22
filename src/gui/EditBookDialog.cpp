#include "EditBookDialog.h"
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QIntValidator>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include "UiTheme.h"

EditBookDialog::EditBookDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("编辑图书信息");
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
    QLabel* titleLabel = new QLabel("编辑图书信息", this);
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
    
    QFrame* formFrame = new QFrame(this);
    formFrame->setStyleSheet("QFrame { background-color: transparent; }");
    
    QFormLayout* formLayout = new QFormLayout(formFrame);
    formLayout->setSpacing(18);
    formLayout->setLabelAlignment(Qt::AlignRight);
    
    QString inputStyle = ui::inputFieldStyle();
    QString labelStyle = ui::labelStyle();
    
    idEdit = new QLineEdit(this);
    idEdit->setValidator(new QIntValidator(1, 1000000, this));
    idEdit->setReadOnly(true);
    idEdit->setStyleSheet(
        "QLineEdit { "
        "    padding: 8px 12px; "
        "    font-size: 13px; "
        "    border: 1px solid #bdc3c7; "
        "    border-radius: 4px; "
        "    background-color: #ecf0f1; "
        "    color: #7f8c8d; "
        "}"
    );
    
    titleEdit = new QLineEdit(this);
    titleEdit->setStyleSheet(inputStyle);
    
    authorEdit = new QLineEdit(this);
    authorEdit->setStyleSheet(inputStyle);
    
    isbnEdit = new QLineEdit(this);
    isbnEdit->setStyleSheet(inputStyle);
    
    categoryEdit = new QLineEdit(this);
    categoryEdit->setStyleSheet(inputStyle);
    
    copiesSpin = new QSpinBox(this);
    copiesSpin->setMinimum(1);
    copiesSpin->setMaximum(1000);
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
    
    formLayout->addRow(idLabel, idEdit);
    formLayout->addRow(titleLabel2, titleEdit);
    formLayout->addRow(authorLabel, authorEdit);
    formLayout->addRow(isbnLabel, isbnEdit);
    formLayout->addRow(categoryLabel, categoryEdit);
    formLayout->addRow(copiesLabel, copiesSpin);
    
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
    
    QPushButton* saveBtn = new QPushButton("保存", this);
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setStyleSheet(ui::primaryButtonStyle());
    
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(saveBtn);
    mainLayout->addWidget(buttonFrame);
    
    QDialogButtonBox* buttons = new QDialogButtonBox(this);
    buttons->addButton(saveBtn, QDialogButtonBox::AcceptRole);
    buttons->addButton(cancelBtn, QDialogButtonBox::RejectRole);
    
    connect(saveBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void EditBookDialog::setBookData(int id, const QString& title, const QString& author, 
                                  const QString& isbn, const QString& category, int copies) {
    idEdit->setText(QString::number(id));
    titleEdit->setText(title);
    authorEdit->setText(author);
    isbnEdit->setText(isbn);
    categoryEdit->setText(category);
    copiesSpin->setValue(copies);
}

int EditBookDialog::getId() const { 
    return idEdit->text().toInt(); 
}

QString EditBookDialog::getTitleStr() const { 
    return titleEdit->text(); 
}

QString EditBookDialog::getAuthor() const { 
    return authorEdit->text(); 
}

QString EditBookDialog::getIsbn() const { 
    return isbnEdit->text(); 
}

QString EditBookDialog::getCategory() const { 
    return categoryEdit->text(); 
}

int EditBookDialog::getCopies() const { 
    return copiesSpin->value(); 
}
