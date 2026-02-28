#include "../core/database/databasemanager.h"
#include "../common/logger/logger.h"
#include "../ui/mainwindow/mainwindow.h"
#include <QApplication>
#include <QDir>
#include <QStandardPaths>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  app.setApplicationName("FunctionDB");
  app.setOrganizationName("FunctionDB");

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
