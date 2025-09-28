#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QLineEdit>
#include <QBoxLayout>


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void browseUnityPath();
    void installTemplate();

private:
    void setupUI();
    void createConnections();
    bool validateUnityPath(const QString& path);

    QWidget *m_centralWidget;
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_pathLayout;
    QHBoxLayout *m_buttonLayout;

    QLabel *m_pathLabel;
    QLineEdit *m_pathEdit;
    QPushButton *m_browseButton;
    QPushButton *m_installButton;
    QPushButton *m_okButton;
    QProgressBar *m_progressBar;
};
#endif // MAINWINDOW_H
