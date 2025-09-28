#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QCoreApplication>

#include "mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(new QWidget(this))
    , m_mainLayout(new QVBoxLayout(m_centralWidget))
    , m_pathLayout(new QHBoxLayout())
    , m_buttonLayout(new QHBoxLayout())
    , m_pathLabel(new QLabel("Путь к версии движка Unity:", this))
    , m_pathEdit(new QLineEdit(this))
    , m_browseButton(new QPushButton("Обзор...", this))
    , m_installButton(new QPushButton("Установить", this))
    , m_okButton(new QPushButton("OK", this))
    , m_progressBar(new QProgressBar(this))
{
    setupUI();
    createConnections();

    setCentralWidget(m_centralWidget);
    setWindowTitle("Unity Template Installer");
    setMinimumSize(500, 150);

    m_okButton->setVisible(false);
    m_progressBar->setVisible(false);
}

void MainWindow::setupUI()
{
    // Настройка пути
    m_pathLayout->addWidget(m_pathLabel);
    m_pathLayout->addWidget(m_pathEdit);
    m_pathLayout->addWidget(m_browseButton);

    // Настройка кнопок
    m_buttonLayout->addWidget(m_installButton);
    m_buttonLayout->addWidget(m_okButton);
    m_buttonLayout->addStretch();

    // Основной layout
    m_mainLayout->addLayout(m_pathLayout);
    m_mainLayout->addWidget(m_progressBar);
    m_mainLayout->addStretch();
    m_mainLayout->addLayout(m_buttonLayout);

    // Настройка элементов
    m_pathEdit->setPlaceholderText("Выберите путь к папке с Unity...");
    m_installButton->setDefault(true);
    m_progressBar->setRange(0, 0);
}

void MainWindow::createConnections()
{
    connect(m_browseButton, &QPushButton::clicked, this, &MainWindow::browseUnityPath);
    connect(m_installButton, &QPushButton::clicked, this, &MainWindow::installTemplate);
    connect(m_okButton, &QPushButton::clicked, this, &QMainWindow::close);
}

bool MainWindow::validateUnityPath(const QString& path)
{
    if (path.isEmpty()) {
        return false;
    }

    QDir unityDir(path);
    if (!unityDir.exists()) {
        return false;
    }

    // Проверяем типичные признаки папки Unity
    QStringList expectedDirs = {"Editor", "MonoBleedingEdge", "Unity.app"};
    for (const QString& dir : expectedDirs) {
        if (unityDir.exists(dir)) {
            return true;
        }
    }

    return false;
}

void MainWindow::browseUnityPath()
{
    QString path = QFileDialog::getExistingDirectory(
        this,
        "Выберите папку с Unity",
        "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );

    if (!path.isEmpty()) {
        m_pathEdit->setText(QDir::toNativeSeparators(path));
    }
}

void MainWindow::installTemplate()
{
    QString unityPath = m_pathEdit->text().trimmed();

    // Проверка пути
    if (!validateUnityPath(unityPath)) {
        QMessageBox::warning(this, "Ошибка",
                             "Пожалуйста, выберите корректный путь к версии движка Unity.");
        return;
    }

    // Путь к архиву шаблона (рядом с исполняемым файлом)
    QString archivePath = QCoreApplication::applicationDirPath() + "/template.tgz";

    if (!QFile::exists(archivePath)) {
        QMessageBox::critical(this, "Ошибка",
                              "Архив шаблона (template.tgz) не найден рядом с исполняемым файлом.");
        return;
    }

    // Целевой путь для установки (куда нужно скопировать .tgz файл)
    QString targetDirPath = QDir(unityPath).absoluteFilePath("Editor/Data/Resources/PackageManager/ProjectTemplates");
    QString targetFilePath = targetDirPath + "/template.tgz";

    // Создаем целевую директорию если она не существует
    QDir targetDir(targetDirPath);
    if (!targetDir.exists()) {
        if (!targetDir.mkpath(".")) {
            QMessageBox::critical(this, "Ошибка",
                                  "Не удалось создать целевую директорию. Проверьте права доступа.");
            return;
        }
    }

    // Блокируем UI на время копирования
    m_installButton->setEnabled(false);
    m_browseButton->setEnabled(false);
    m_progressBar->setVisible(true);

    // Просто копируем файл (не распаковываем!)
    if (QFile::copy(archivePath, targetFilePath)) {
        QMessageBox::information(this, "Успех", "Шаблон успешно установлен в Unity!");

        // Показываем кнопку OK и скрываем кнопку Установить
        m_installButton->setVisible(false);
        m_okButton->setVisible(true);
        m_okButton->setDefault(true);
    } else {
        QMessageBox::critical(this, "Ошибка",
                              "Не удалось скопировать файл шаблона. Проверьте права доступа.");
    }

    // Разблокируем UI
    m_installButton->setEnabled(true);
    m_browseButton->setEnabled(true);
    m_progressBar->setVisible(false);
}
