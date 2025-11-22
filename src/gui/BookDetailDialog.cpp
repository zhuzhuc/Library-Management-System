#include "BookDetailDialog.h"

#include "Book.h"
#include "LibraryController.h"
#include "UiTheme.h"
#include "src/db/DBManager.h"

#include <QDateTime>
#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>

namespace {

QString buildPillStyle(const QString& bgColor, const QString& textColor) {
    return QStringLiteral(
        "QLabel { "
        "    padding: 4px 12px; "
        "    border-radius: 999px; "
        "    font-size: 11px; "
        "    font-weight: 600; "
        "    background-color: %1; "
        "    color: %2; "
        "}"
    ).arg(bgColor, textColor);
}

QLabel* createPill(const QString& text, QWidget* parent, const QString& styleOverride = {}) {
    QLabel* label = new QLabel(text, parent);
    QString style = styleOverride.isEmpty() ? ui::pillStyle() : styleOverride;
    label->setStyleSheet(style);
    return label;
}

} // namespace

BookDetailDialog::BookDetailDialog(int bookId, LibraryController* controller, QWidget* parent)
    : QDialog(parent),
      bookId(bookId),
      controller(controller),
      idValueLabel(nullptr),
      titleValueLabel(nullptr),
      authorValueLabel(nullptr),
      categoryValueLabel(nullptr),
      isbnValueLabel(nullptr),
      availabilityValueLabel(nullptr),
      totalCopiesValueLabel(nullptr),
      borrowedCountValueLabel(nullptr),
      availabilityProgressBar(nullptr),
      categoryChipLabel(nullptr),
      statusChipLabel(nullptr),
      borrowerTableView(nullptr),
      borrowerModel(nullptr),
      statsLabel(nullptr) {
    setWindowTitle("图书详情");
    setMinimumSize(1200, 780);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setStyleSheet(ui::dialogBackground());

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QFrame* titleFrame = new QFrame(this);
    titleFrame->setStyleSheet(ui::headerFrameStyle());
    titleFrame->setFixedHeight(70);
    QHBoxLayout* titleLayout = new QHBoxLayout(titleFrame);
    titleLayout->setContentsMargins(24, 12, 24, 12);

    QLabel* titleLabel = new QLabel("图书详情", titleFrame);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #f8fafc;");
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    mainLayout->addWidget(titleFrame);

    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    mainLayout->addWidget(scrollArea, 1);

    QWidget* scrollContent = new QWidget(scrollArea);
    QVBoxLayout* contentLayout = new QVBoxLayout(scrollContent);
    contentLayout->setSpacing(18);
    contentLayout->setContentsMargins(24, 24, 24, 24);

    // Info card
    QFrame* infoFrame = new QFrame(scrollContent);
    infoFrame->setStyleSheet(ui::cardFrameStyle());
    QVBoxLayout* infoLayout = new QVBoxLayout(infoFrame);
    infoLayout->setSpacing(16);

    QLabel* infoTitle = new QLabel("图书基础信息", infoFrame);
    infoTitle->setStyleSheet(ui::sectionTitleStyle());
    infoLayout->addWidget(infoTitle);

    QGridLayout* grid = new QGridLayout();
    grid->setHorizontalSpacing(24);
    grid->setVerticalSpacing(12);
    grid->setColumnStretch(1, 1);
    grid->setColumnStretch(3, 1);

    auto addRow = [&](const QString& labelText, QLabel*& valueLabel, int row, int column, int columnSpan = 1) {
        QLabel* label = new QLabel(labelText, infoFrame);
        label->setStyleSheet(ui::labelStyle());
        valueLabel = createValueLabel("--");
        grid->addWidget(label, row, column);
        grid->addWidget(valueLabel, row, column + 1, 1, columnSpan);
    };

    addRow("图书ID", idValueLabel, 0, 0);
    addRow("当前状态", availabilityValueLabel, 0, 2);
    addRow("书名", titleValueLabel, 1, 0, 3);
    addRow("作者", authorValueLabel, 2, 0);
    addRow("ISBN", isbnValueLabel, 2, 2);

    QLabel* categoryLabel = new QLabel("分类", infoFrame);
    categoryLabel->setStyleSheet(ui::labelStyle());
    categoryValueLabel = createValueLabel("--");
    grid->addWidget(categoryLabel, 3, 0);
    grid->addWidget(categoryValueLabel, 3, 1);

    addRow("馆藏册数", totalCopiesValueLabel, 3, 2);
    addRow("已借出", borrowedCountValueLabel, 4, 2);

    infoLayout->addLayout(grid);

    // Availability progress
    QFrame* availabilityFrame = new QFrame(infoFrame);
    availabilityFrame->setStyleSheet(ui::softCardFrameStyle());
    QVBoxLayout* availabilityLayout = new QVBoxLayout(availabilityFrame);
    availabilityLayout->setSpacing(8);
    QLabel* availabilityTitle = new QLabel("可借进度", availabilityFrame);
    availabilityTitle->setStyleSheet(ui::labelStyle());
    availabilityLayout->addWidget(availabilityTitle);

    availabilityProgressBar = new QProgressBar(availabilityFrame);
    availabilityProgressBar->setRange(0, 100);
    availabilityProgressBar->setTextVisible(false);
    availabilityProgressBar->setFixedHeight(12);
    availabilityProgressBar->setStyleSheet(
        "QProgressBar { "
        "    background-color: #e2e8f0; "
        "    border-radius: 6px; "
        "}"
        "QProgressBar::chunk { "
        "    background-color: #34d399; "
        "    border-radius: 6px; "
        "}"
    );
    availabilityLayout->addWidget(availabilityProgressBar);

    infoLayout->addWidget(availabilityFrame);

    // Chip row
    QHBoxLayout* chipLayout = new QHBoxLayout();
    chipLayout->setSpacing(12);
    categoryChipLabel = createPill("分类 · --", infoFrame);
    statusChipLabel = createPill("状态 · --", infoFrame);
    chipLayout->addWidget(categoryChipLabel);
    chipLayout->addWidget(statusChipLabel);
    chipLayout->addStretch();
    infoLayout->addLayout(chipLayout);

    contentLayout->addWidget(infoFrame);

    // Borrower card
    QFrame* borrowersFrame = new QFrame(scrollContent);
    borrowersFrame->setStyleSheet(ui::cardFrameStyle());
    QVBoxLayout* borrowersLayout = new QVBoxLayout(borrowersFrame);
    borrowersLayout->setSpacing(10);

    QHBoxLayout* borrowersHeaderLayout = new QHBoxLayout();
    QLabel* borrowersTitle = new QLabel("借阅记录", borrowersFrame);
    borrowersTitle->setStyleSheet(ui::sectionTitleStyle());
    borrowersHeaderLayout->addWidget(borrowersTitle);

    statsLabel = new QLabel("--", borrowersFrame);
    statsLabel->setStyleSheet(ui::subtleTextStyle());
    borrowersHeaderLayout->addStretch();
    borrowersHeaderLayout->addWidget(statsLabel);

    QPushButton* refreshButton = new QPushButton("刷新", borrowersFrame);
    refreshButton->setCursor(Qt::PointingHandCursor);
    refreshButton->setStyleSheet(ui::primaryButtonStyle());
    borrowersHeaderLayout->addWidget(refreshButton);
    borrowersLayout->addLayout(borrowersHeaderLayout);

    connect(refreshButton, &QPushButton::clicked, this, &BookDetailDialog::refreshData);

    borrowerModel = new QStandardItemModel(this);
    borrowerModel->setColumnCount(8);
    borrowerModel->setHorizontalHeaderLabels(
        {"借阅者ID", "借阅者姓名", "院系", "借阅日期", "借阅天数", "应归还日期", "实际归还日期", "状态"});

    borrowerTableView = new QTableView(this);
    borrowerTableView->setModel(borrowerModel);
    borrowerTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    borrowerTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    borrowerTableView->horizontalHeader()->setStretchLastSection(false);
    borrowerTableView->setAlternatingRowColors(true);
    borrowerTableView->verticalHeader()->hide();
    borrowerTableView->setShowGrid(false);
    borrowerTableView->setStyleSheet(ui::tableStyle());
    borrowerTableView->setColumnWidth(0, 100);
    borrowerTableView->setColumnWidth(1, 150);
    borrowerTableView->setColumnWidth(2, 150);
    borrowerTableView->setColumnWidth(3, 160);
    borrowerTableView->setColumnWidth(4, 90);
    borrowerTableView->setColumnWidth(5, 160);
    borrowerTableView->setColumnWidth(6, 160);
    borrowerTableView->setColumnWidth(7, 110);
    borrowersLayout->addWidget(borrowerTableView);

    contentLayout->addWidget(borrowersFrame);
    contentLayout->addStretch();

    scrollArea->setWidget(scrollContent);

    refreshData();
}

QLabel* BookDetailDialog::createValueLabel(const QString& text) {
    QLabel* label = new QLabel(text, this);
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setStyleSheet("color: #0f172a; font-size: 14px;");
    return label;
}

void BookDetailDialog::refreshData() {
    Book* book = controller ? controller->getBookById(bookId) : nullptr;
    if (!book) {
        if (statsLabel) statsLabel->setText("未找到该图书");
        borrowerModel->removeRows(0, borrowerModel->rowCount());
        return;
    }

    const int totalCopies = book->getTotalCopies();
    const int availableCopies = book->getAvailableCopies();
    const int borrowedCopies = totalCopies - availableCopies;

    idValueLabel->setText(QString::number(book->getBookId()));
    titleValueLabel->setText(QString::fromStdString(book->getTitle()));
    authorValueLabel->setText(QString::fromStdString(book->getAuthor()));
    isbnValueLabel->setText(QString::fromStdString(book->getIsbn()));
    QString categoryText = QString::fromStdString(book->getCategory());
    categoryValueLabel->setText(categoryText);
    availabilityValueLabel->setText(book->getIsAvailable() ? "可借" : "不可借");
    totalCopiesValueLabel->setText(QString::number(totalCopies));
    borrowedCountValueLabel->setText(QString::number(std::max(0, borrowedCopies)));
    availabilityProgressBar->setValue(totalCopies == 0 ? 0 : static_cast<int>((availableCopies * 100.0) / totalCopies));

    if (categoryChipLabel) {
        categoryChipLabel->setText(QString("分类 · %1").arg(categoryText.isEmpty() ? "未分类" : categoryText));
        categoryChipLabel->setStyleSheet(buildPillStyle("#e0f2fe", "#0369a1"));
    }
    if (statusChipLabel) {
        const bool available = book->getIsAvailable();
        statusChipLabel->setText(QString("状态 · %1").arg(available ? "可借" : "已借完"));
        statusChipLabel->setStyleSheet(available
            ? buildPillStyle("#dcfce7", "#166534")
            : buildPillStyle("#fee2e2", "#991b1b"));
    }

    borrowerModel->removeRows(0, borrowerModel->rowCount());
    db::DBManager* dbManager = controller ? controller->getDBManager() : nullptr;
    int recordCount = 0;
    QString latestBorrower;

    auto getField = [](const std::map<std::string, std::string>& record,
                       std::initializer_list<const char*> keys,
                       const char* fallback = "") -> QString {
        for (auto key : keys) {
            auto it = record.find(key);
            if (it != record.end()) {
                return QString::fromStdString(it->second);
            }
        }
        return QString::fromUtf8(fallback);
    };

    if (dbManager && dbManager->isConnected()) {
        std::vector<std::map<std::string, std::string>> records;
        if (dbManager->getBorrowRecordsByBook(book->getBookId(), records)) {
            for (const auto& record : records) {
                QList<QStandardItem*> rowItems;
                auto borrowerId = getField(record, {"borrower_id"}, "--");
                auto borrowerName = getField(record, {"borrower_name"}, "未知借阅人");
                auto dept = getField(record, {"department", "borrower_dept"}, "未知院系");
                auto borrowDate = getField(record, {"borrow_date"}, "--");
                auto borrowDays = getField(record, {"borrow_days"}, "--");
                auto dueDate = getField(record, {"due_date", "expected_return_date"}, "--");
                auto returnDate = getField(record, {"return_date"}, "-");
                auto status = getField(record, {"status"}, "unknown");

                rowItems << new QStandardItem(borrowerId)
                         << new QStandardItem(borrowerName)
                         << new QStandardItem(dept)
                         << new QStandardItem(borrowDate)
                         << new QStandardItem(borrowDays)
                         << new QStandardItem(dueDate)
                         << new QStandardItem(returnDate.isEmpty() ? "-" : returnDate)
                         << new QStandardItem(status);
                borrowerModel->appendRow(rowItems);
                recordCount++;
                if (latestBorrower.isEmpty()) {
                    latestBorrower = borrowerName;
                }
            }
        }
    }

    if (statsLabel) {
        if (recordCount == 0) {
            statsLabel->setText("暂无借阅记录");
        } else {
            statsLabel->setText(QString("共 %1 条记录 · 最近由 %2 借阅").arg(recordCount).arg(latestBorrower));
        }
    }
}
#include <algorithm>
