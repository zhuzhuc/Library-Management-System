#include "RecommendBooK.h"

#include "Book.h"
#include "LibraryController.h"
#include "UiTheme.h"

#include <QColor>
#include <QDateTime>
#include <QFrame>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>
#include <algorithm>

namespace {

QLabel* createChip(const QString& text, QWidget* parent, const QString& bg, const QString& fg) {
    QLabel* chip = new QLabel(text, parent);
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
}

} // namespace

RecommendBooK::RecommendBooK(LibraryController* controller, QWidget* parent)
    : QDialog(parent),
      controller(controller),
      model(new QStandardItemModel(this)),
      tableview(new QTableView(this)),
      refreshButton(nullptr),
      autoScrollTimer(new QTimer(this)),
      currentRow(-1),
      statsLabel(nullptr),
      totalChipLabel(nullptr),
      availableChipLabel(nullptr),
      tightChipLabel(nullptr) {
    setWindowTitle("图书推荐 | 热门榜单");
    setModal(true);
    setMinimumSize(1180, 720);
    setSizeGripEnabled(true);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setStyleSheet(ui::dialogBackground());

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QFrame* titleFrame = new QFrame(this);
    titleFrame->setFixedHeight(72);
    titleFrame->setStyleSheet(ui::headerFrameStyle());
    QVBoxLayout* titleLayout = new QVBoxLayout(titleFrame);
    titleLayout->setContentsMargins(24, 14, 24, 14);
    titleLayout->setSpacing(4);

    QLabel* titleLabel = new QLabel("智能推荐榜单", titleFrame);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #f8fafc;");

    QLabel* subtitleLabel = new QLabel("结合借阅热度、库存健康度与可借优先级实时生成", titleFrame);
    subtitleLabel->setStyleSheet("color: #cbd5f5; font-size: 12px;");

    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(subtitleLabel);
    mainLayout->addWidget(titleFrame);

    QFrame* contentFrame = new QFrame(this);
    contentFrame->setStyleSheet(ui::cardFrameStyle());
    QVBoxLayout* contentLayout = new QVBoxLayout(contentFrame);
    contentLayout->setSpacing(18);
    contentLayout->setContentsMargins(24, 24, 24, 24);

    QFrame* infoBanner = new QFrame(contentFrame);
    infoBanner->setStyleSheet(ui::softCardFrameStyle());
    QHBoxLayout* infoLayout = new QHBoxLayout(infoBanner);
    infoLayout->setSpacing(12);
    infoLayout->setContentsMargins(12, 12, 12, 12);

    totalChipLabel = createChip("推荐 --", infoBanner, "#e0f2fe", "#0369a1");
    availableChipLabel = createChip("可借 --", infoBanner, "#dcfce7", "#166534");
    tightChipLabel = createChip("紧俏 --", infoBanner, "#fee2e2", "#991b1b");

    infoLayout->addWidget(totalChipLabel);
    infoLayout->addWidget(availableChipLabel);
    infoLayout->addWidget(tightChipLabel);
    infoLayout->addStretch();

    statsLabel = new QLabel("数据加载中…", infoBanner);
    statsLabel->setStyleSheet(ui::subtleTextStyle());
    infoLayout->addWidget(statsLabel);

    contentLayout->addWidget(infoBanner);

    QFrame* tableFrame = new QFrame(contentFrame);
    tableFrame->setStyleSheet("QFrame { background-color: transparent; }");
    QVBoxLayout* tableLayout = new QVBoxLayout(tableFrame);
    tableLayout->setSpacing(10);
    tableLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* headerRow = new QHBoxLayout();
    headerRow->setContentsMargins(0, 0, 0, 0);

    QLabel* tableTitle = new QLabel("推荐书单", tableFrame);
    tableTitle->setStyleSheet(ui::sectionTitleStyle());
    headerRow->addWidget(tableTitle);
    headerRow->addStretch();

    refreshButton = new QPushButton("刷新推荐", tableFrame);
    refreshButton->setCursor(Qt::PointingHandCursor);
    refreshButton->setStyleSheet(ui::primaryButtonStyle());
    headerRow->addWidget(refreshButton);
    tableLayout->addLayout(headerRow);

    model->setColumnCount(6);
    model->setHorizontalHeaderLabels(
        {"图书ID", "书名", "作者", "分类", "可借数量", "馆藏总数"});

    tableview->setModel(model);
    tableview->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableview->setSelectionMode(QAbstractItemView::SingleSelection);
    tableview->setAlternatingRowColors(true);
    tableview->verticalHeader()->hide();
    tableview->setShowGrid(false);
    tableview->setWordWrap(false);
    tableview->setTextElideMode(Qt::ElideRight);
    tableview->setStyleSheet(ui::tableStyle());
    tableview->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableview->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    tableview->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    tableLayout->addWidget(tableview);
    contentLayout->addWidget(tableFrame, 1);

    mainLayout->addWidget(contentFrame, 1);

    QFrame* buttonFrame = new QFrame(this);
    buttonFrame->setStyleSheet("QFrame { background-color: transparent; }");
    QHBoxLayout* buttonLayout = new QHBoxLayout(buttonFrame);
    buttonLayout->setContentsMargins(24, 18, 24, 24);
    buttonLayout->addStretch();

    QPushButton* closeBtn = new QPushButton("关闭", this);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(ui::secondaryButtonStyle());

    buttonLayout->addWidget(closeBtn);
    mainLayout->addWidget(buttonFrame);

    connect(refreshButton, &QPushButton::clicked, this, &RecommendBooK::refreshData);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    autoScrollTimer->setInterval(3800);
    connect(autoScrollTimer, &QTimer::timeout, this, &RecommendBooK::advanceCarousel);

    refreshData();
}

RecommendBooK::~RecommendBooK() = default;

void RecommendBooK::refreshData() {
    model->setRowCount(0);

    std::vector<Book> books;
    if (controller) {
        books = controller->recommendBooks(20);
    }

    int row = 0;
    for (const auto& book : books) {
        model->insertRow(row);
        model->setItem(row, 0, new QStandardItem(QString::number(book.getBookId())));
        model->setItem(row, 1, new QStandardItem(QString::fromStdString(book.getTitle())));
        model->setItem(row, 2, new QStandardItem(QString::fromStdString(book.getAuthor())));
        model->setItem(row, 3, new QStandardItem(QString::fromStdString(book.getCategory())));

        auto* availableItem = new QStandardItem(QString::number(book.getAvailableCopies()));
        availableItem->setData(book.getAvailableCopies() > 0 ? QColor("#16a34a") : QColor("#dc2626"),
                               Qt::ForegroundRole);
        model->setItem(row, 4, availableItem);

        model->setItem(row, 5, new QStandardItem(QString::number(book.getTotalCopies())));
        ++row;
    }

    updateSummary(books);

    currentRow = -1;
    if (row == 0) {
        autoScrollTimer->stop();
        return;
    }

    if (row == 1) {
        autoScrollTimer->stop();
    } else if (!autoScrollTimer->isActive()) {
        autoScrollTimer->start();
    }

    advanceCarousel();
}

void RecommendBooK::advanceCarousel() {
    int rows = model->rowCount();
    if (rows <= 0) {
        autoScrollTimer->stop();
        return;
    }
    currentRow = (currentRow + 1) % rows;
    tableview->selectRow(currentRow);
    tableview->scrollTo(model->index(currentRow, 0), QAbstractItemView::PositionAtCenter);
}

void RecommendBooK::updateSummary(const std::vector<Book>& books) {
    const int total = static_cast<int>(books.size());
    int totalAvailable = 0;
    int tightCount = 0;
    for (const auto& book : books) {
        const int available = book.getAvailableCopies();
        totalAvailable += available;
        const int tightThreshold = std::max(1, book.getTotalCopies() / 4);
        if (available <= tightThreshold) {
            ++tightCount;
        }
    }

    if (totalChipLabel) {
        totalChipLabel->setText(QString("推荐 %1").arg(total));
    }
    if (availableChipLabel) {
        availableChipLabel->setText(QString("可借 %1").arg(totalAvailable));
    }
    if (tightChipLabel) {
        tightChipLabel->setText(QString("紧俏 %1").arg(tightCount));
    }
    if (statsLabel) {
        if (total == 0) {
            statsLabel->setText("暂无推荐数据");
        } else {
            statsLabel->setText(QString("最近刷新：%1")
                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));
        }
    }
}
