#include "databasemanager.h"
#include "logger.h"
#include "mainwindow.h"
#include "thememanager.h"
#include <QApplication>
#include <QDir>
#include <QStandardPaths>

#ifdef Q_OS_WIN
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

int main(int argc, char *argv[]) {
#ifdef Q_OS_WIN
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
  
  int oldMode = _setmode(_fileno(stdout), _O_U8TEXT);
  if (oldMode == -1) {
    oldMode = _setmode(_fileno(stdout), _O_TEXT);
  }
  
  oldMode = _setmode(_fileno(stderr), _O_U8TEXT);
  if (oldMode == -1) {
    oldMode = _setmode(_fileno(stderr), _O_TEXT);
  }
#endif

  QApplication app(argc, argv);

  app.setApplicationName("FunctionDB");
  app.setOrganizationName("FunctionDB");

  ThemeManager::instance().init();
  ThemeManager::instance().applyTheme(&app);

  QString appDataPath =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir dir(appDataPath);
  if (!dir.exists()) {
    dir.mkpath(".");
  }

  QString logPath = appDataPath + "/functiondb.log";
//   Logger::instance().init(logPath);
  Logger::instance().info("应用程序启动");

  QString dbPath = appDataPath + "/functions.db";
  if (!DatabaseManager::instance().init(dbPath)) {
    Logger::instance().error("数据库初始化失败，程序退出");
    return -1;
  }

  MainWindow window;
  window.show();

  int result = app.exec();

  Logger::instance().info("应用程序退出，返回码: " + QString::number(result));
  return result;
}
