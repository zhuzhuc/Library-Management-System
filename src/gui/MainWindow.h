#pragma once

#include <QMainWindow>
#include <vector>
#include <QPixmap>

#include "Book.h"

class QTableView;
class QLineEdit;
class QLabel;
class QStatusBar;
class QAction;
class QVBoxLayout;
class BookTableModel;
class LibraryController;
class QListWidget;
class QListWidgetItem;
class QTimer;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
    bool showLogin();

private slots:
    void onSearchTextChanged(const QString& text);
    void updateStatusBar();
    void onLogout();
    void handleLibraryChanged();
    void openRecommendationDialog();
    void handleRecommendationActivated(QListWidgetItem* item);
    void openAppearanceSettings();

private:
    void setupUi();
    void setupRecommendationPanel(QVBoxLayout* containerLayout);
    void refreshEmbeddedRecommendations();
    void advanceRecommendationCarousel();
    void filterBooks();
    void updateBookCount();
    void updateUserDisplay();
    bool authenticateUser(const QString& username, const QString& password, const QString& userType, QString& outBorrowerId);
    void applyBackgroundFromSettings();
    void updateWindowBackgroundBrush();
    QString ensureBorrowerBinding(const QString& username, const QString& userType, const QString& displayNameFallback);
    void resizeEvent(QResizeEvent* event) override;
    
    
    QTableView* tableView;
    QLineEdit* searchEdit;
    QLabel* statusLabel;
    QLabel* userLabel;
    QStatusBar* statusBar;
    QLabel* bookCountLabel;
    BookTableModel* model;
    LibraryController* controller;
    
    QString currentUsername;
    QString currentUserType;  // "admin" or "user"
    QString currentBorrowerId;  // borrower_id from users table
    bool isLoggedIn;
    
    //actions
    QAction* editBookAct;
    QAction* addBookAct;
    QAction* removeBookAct;
    QAction* addUserAct;
    QAction* resetPasswordAct;
    QAction* reloadAct;
    QAction* logoutAct;
    QAction* appearanceSettingsAct;
    
    // 推荐展示
    QListWidget* recommendationList;
    QLabel* recommendationMetaLabel;
    QTimer* recommendationTimer;
    std::vector<Book> embeddedRecommendations;
    int recommendationCarouselIndex = -1;
    
    // 统计展示
    QLabel* totalTitlesValueLabel;
    QLabel* totalCopiesValueLabel;
    QLabel* availableCopiesValueLabel;
    QLabel* borrowedCopiesValueLabel;
    QLabel* availableChipLabel;
    QLabel* borrowedChipLabel;
    QLabel* totalChipLabel;
    
    QPixmap mainBackgroundPixmap;
    bool hasCustomMainBackground = false;
};
