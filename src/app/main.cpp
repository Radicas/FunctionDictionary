#include "core/database/databasemanager.h"
#include "core/services/parseservice.h"
#include "common/logger/logger.h"
#include "ui/mainwindow/mainwindow.h"
#include "common/theme/thememanager.h"
#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QStyleFactory>
#include <QWebEngineView>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  Q_INIT_RESOURCE(markdown);
  Q_INIT_RESOURCE(theme);

  app.setApplicationName("FunctionDB");
  app.setOrganizationName("FunctionDB");

  app.setStyle(QStyleFactory::create("Fusion"));

  ThemeManager::instance().init();
  ThemeManager::instance().applyTheme(&app);

  QString exeDir = QCoreApplication::applicationDirPath();
  QString dbDirPath = exeDir + "/database";
  QDir dbDir(dbDirPath);
  if (!dbDir.exists()) {
    if (!dbDir.mkpath(".")) {
      Logger::instance().error("无法创建数据库目录: " + dbDirPath);
      return -1;
    }
    Logger::instance().info("成功创建数据库目录: " + dbDirPath);
  }

  Logger::instance().info("应用程序启动");
  Logger::instance().info("可执行文件目录: " + exeDir);
  Logger::instance().info("数据库目录: " + dbDirPath);

  QString dbPath = dbDirPath + "/functions.db";
  if (!DatabaseManager::instance().init(dbPath)) {
    Logger::instance().error("数据库初始化失败，程序退出");
    return -1;
  }

  IDatabaseManager* dbManager = &DatabaseManager::instance();
  IParseService* parseService = new ParseService(dbManager, &app);

  MainWindow window(dbManager, parseService);
  window.show();

  int result = app.exec();

  Logger::instance().info("应用程序退出，返回码: " + QString::number(result));
  return result;
}
