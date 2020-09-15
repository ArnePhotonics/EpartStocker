#ifndef MPNSUGGESTIONWINDOW_H
#define MPNSUGGESTIONWINDOW_H

#include <QDialog>

namespace Ui {
    class MPNSuggestionWindow;
}

class MPNSuggestionWindow : public QDialog {
    Q_OBJECT

    public:
    explicit MPNSuggestionWindow(QWidget *parent = nullptr);
    ~MPNSuggestionWindow();
    void show_suggestions(QPoint position, int width, QStringList mpns);

    private:
    Ui::MPNSuggestionWindow *ui;
};

#endif // MPNSUGGESTIONWINDOW_H
