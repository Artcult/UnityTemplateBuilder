#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QCoreApplication>
#include <archive.h>
#include <archive_entry.h>

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
    create_connections();

    setCentralWidget(m_centralWidget);
    setWindowTitle("Unity Template Installation");
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

void MainWindow::create_connections()
{
    connect(m_browseButton, &QPushButton::clicked, this, &MainWindow::browse_unity_path);
    connect(m_installButton, &QPushButton::clicked, this, &MainWindow::installTemplate);
    connect(m_okButton, &QPushButton::clicked, this, &QMainWindow::close);
}

bool MainWindow::validate_unity_path(const QString& path)
{
    // if (path.isEmpty()) {
    //     return false;
    // }

    // QDir unityDir(path);
    // if (!unityDir.exists()) {
    //     return false;
    // }

    // // Проверяем типичные признаки папки Unity
    // QStringList expectedDirs = {"Editor", "MonoBleedingEdge", "Unity.app"};
    // for (const QString& dir : expectedDirs) {
    //     if (unityDir.exists(dir)) {
    //         return true;
    //     }
    // }

    // return false;
    return true;
}

void MainWindow::browse_unity_path()
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

void MainWindow::copy_recursively(const QDir & src, const QDir & dst)
{
    if (!dst.exists()) dst.mkpath(".");
    for (QFileInfo fi : src.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries)) {
        QString targetPath = dst.filePath(fi.fileName());
        if (fi.isDir()) {
            copy_recursively(QDir(fi.absoluteFilePath()), QDir(targetPath));
        } else {
            QFile::copy(fi.absoluteFilePath(), targetPath);
        }
    }
}

bool MainWindow::create_tgz(const QString & src_dir,const QString& out_file)
{
    struct archive* a = archive_write_new();
    archive_write_set_format_pax_restricted(a); // tar
    archive_write_add_filter_gzip(a);           // gzip

    if (archive_write_open_filename(a, out_file.toUtf8().constData()) != ARCHIVE_OK) {
        archive_write_free(a);
        return false;
    }

    std::function<void(const QDir&, const QString&)> addDir;
    addDir = [&](const QDir &dir, const QString &basePath) {
        for (QFileInfo fi : dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries)) {
            QString relativePath = basePath + "/" + fi.fileName();
            struct archive_entry* entry = archive_entry_new();
            archive_entry_set_pathname(entry, relativePath.toUtf8().constData());
            archive_entry_set_filetype(entry, fi.isDir() ? AE_IFDIR : AE_IFREG);
            archive_entry_set_perm(entry, fi.isDir() ? 0755 : 0644);
            if (fi.isFile()) archive_entry_set_size(entry, fi.size());
            archive_write_header(a, entry);

            if (fi.isFile()) {
                QFile f(fi.absoluteFilePath());
                if (f.open(QIODevice::ReadOnly)) {
                    QByteArray buf;
                    while (!(buf = f.read(8192)).isEmpty()) {
                        archive_write_data(a, buf.constData(), buf.size());
                    }
                }
            }

            archive_entry_free(entry);

            if (fi.isDir()) addDir(QDir(fi.absoluteFilePath()), relativePath);
        }
    };

    addDir(QDir(src_dir), ""); // пустой basePath чтобы архив начинался с содержимого srcDir
    archive_write_close(a);
    archive_write_free(a);
    return true;
}

bool MainWindow::build_template(const QString& assetSandboxDir, const QString& projectDir, const QString& unityPath)
{
    QDir tempDir(QDir::tempPath() + "/package");
    if (tempDir.exists()) tempDir.removeRecursively();
    tempDir.mkpath("ProjectData~");

    // Копируем Assets, Packages, ProjectSettings
    QStringList foldersToCopy = {"Assets", "Packages", "ProjectSettings"};
    for (const QString &folder : foldersToCopy) {
        QDir srcDir(assetSandboxDir + "/" + folder);
        QDir dstDir(tempDir.filePath("ProjectData~/" + folder));
        copy_recursively(srcDir, dstDir);
    }

    // Удаляем ProjectVersion.txt если есть
    QString versionFile = tempDir.filePath("ProjectData~/ProjectSettings/ProjectVersion.txt");
    if (QFile::exists(versionFile)) QFile::remove(versionFile);

    // Собираем tgz
    QString archivePath = QCoreApplication::applicationDirPath() + "/com.unity.template.urp-lab.tgz";
    if (!create_tgz(tempDir.path(), archivePath)) return false;

    // Копируем в Unity
    QString targetDirPath = QDir(unityPath).absoluteFilePath(
        "Editor/Data/Resources/PackageManager/ProjectTemplates"
        );
    QDir targetDir(targetDirPath);
    if (!targetDir.exists()) targetDir.mkpath(".");

    QString targetFilePath = targetDir.filePath("com.unity.template.urp-lab.tgz");
    if (!QFile::copy(archivePath, targetFilePath)) return false;

    // Очистка временной папки
    tempDir.removeRecursively();

    return true;
}

void MainWindow::installTemplate()
{
    QString unityPath = m_pathEdit->text().trimmed();
    if (!validate_unity_path(unityPath)) {
        QMessageBox::warning(this, "Ошибка", "Укажите корректный путь к Unity.");
        return;
    }

    m_progressBar->setVisible(true);
    m_installButton->setEnabled(false);

    QDir exeDir(QCoreApplication::applicationDirPath());
    exeDir.cdUp();
    QString assetSandboxDir = exeDir.filePath("Asset Sandbox");

    QString projectDir = QDir::current().absoluteFilePath("Template/project");

    bool ok = build_template(assetSandboxDir, projectDir, unityPath);

    m_progressBar->setVisible(false);

    if (ok) {
        QMessageBox::information(this, "Успех", "Шаблон успешно установлен.");
        m_okButton->setVisible(true);
        m_installButton->setVisible(false);
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось установить шаблон.");
        m_installButton->setEnabled(true);
    }
}
