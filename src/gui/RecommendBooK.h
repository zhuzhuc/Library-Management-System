#pragma once

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTableView>
#include <QTimer>
#include <vector>

class LibraryController;
class Book;

class RecommendBooK : public QDialog {
    Q_OBJECT
public:
    explicit RecommendBooK(LibraryController* controller, QWidget* parent = nullptr);
    ~RecommendBooK();

private slots:
    void refreshData();
    void advanceCarousel();

private:
    void updateSummary(const std::vector<Book>& books);

    LibraryController* controller;
    QStandardItemModel* model;
    QTableView* tableview;
    QPushButton* refreshButton;
    QTimer* autoScrollTimer;
    int currentRow;
    
    QLabel* statsLabel;
    QLabel* totalChipLabel;
    QLabel* availableChipLabel;
    QLabel* tightChipLabel;
};
