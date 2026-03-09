/**
 * @file modellistfetcher.h
 * @brief 模型列表获取器，用于从AI API获取支持的模型列表
 * @author Developer
 * @date 2026-03-09
 * @version 1.0
 */

#ifndef MODELLISTFETCHER_H
#define MODELLISTFETCHER_H

#include <QNetworkAccessManager>
#include <QObject>
#include <QStringList>

/**
 * @brief 模型列表获取器类
 */
class ModelListFetcher : public QObject
{
    Q_OBJECT

   public:
    /**
     * @brief 获取ModelListFetcher的单例实例
     * @return ModelListFetcher的引用
     */
    static ModelListFetcher& instance();

    ModelListFetcher(const ModelListFetcher&) = delete;
    ModelListFetcher& operator=(const ModelListFetcher&) = delete;

    /**
     * @brief 获取模型列表（异步）
     * @param baseUrl API基础URL
     * @param apiKey API密钥
     */
    void fetchModels(const QString& baseUrl, const QString& apiKey);

   signals:
    /**
     * @brief 模型列表获取成功信号
     * @param models 模型列表
     */
    void modelsFetched(const QStringList& models);

    /**
     * @brief 模型列表获取失败信号
     * @param error 错误信息
     */
    void fetchFailed(const QString& error);

   private slots:
    /**
     * @brief 网络请求完成槽函数
     * @param reply 网络回复对象
     */
    void onReplyFinished(QNetworkReply* reply);

   private:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit ModelListFetcher(QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~ModelListFetcher();

    QNetworkAccessManager* m_networkManager;
    QNetworkReply* m_currentReply;
};

#endif
