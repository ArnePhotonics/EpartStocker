#ifndef MPNSUGGESTIONWINDOW_H
#define MPNSUGGESTIONWINDOW_H

#include <QDialog>
namespace Ui {
    class MPNSuggestionWindow;
}

class SuggestionPartInfo {
    public:
    QString mpn;
    int part_id = 0;
    QString location;
};

class MPNSuggestionWindow : public QDialog {
    Q_OBJECT

    public:
    explicit MPNSuggestionWindow(QWidget *parent, QWidget *mainwindow_parent);
    ~MPNSuggestionWindow();
    void show_suggestions(QPoint position, int width, QList<SuggestionPartInfo> parts);

    private slots:
    void on_suggestionLabel_linkActivated(const QString &link);

    private:
    Ui::MPNSuggestionWindow *ui;
    QWidget *m_mainwindow_parent = nullptr;
};

#endif // MPNSUGGESTIONWINDOW_H
