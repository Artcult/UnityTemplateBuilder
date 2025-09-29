#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QLineEdit>
#include <QBoxLayout>
#include <QDir>


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void browse_unity_path();
    void installTemplate();

private:
    void setupUI();
    void create_connections();
    void copy_recursively(const QDir & src, const QDir & dst);
    bool validate_unity_path(const QString& path);
    bool create_tgz(const QString & src_dir,const QString& out_file);
    bool build_template(const QString& assetSandboxDir, const QString& projectDir, const QString& unityPath);


private:
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
