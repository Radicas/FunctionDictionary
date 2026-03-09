/**
 * @file modellistfetcher.cpp
 * @brief 模型列表获取器实现
 * @author Developer
 * @date 2026-03-09
 * @version 1.0
 */

#include "core/ai/modellistfetcher.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>
#include "common/logger/logger.h"

ModelListFetcher::ModelListFetcher(QObject* parent)
    : QObject(parent), m_networkManager(new QNetworkAccessManager(this)), m_currentReply(nullptr)
{
    Logger::instance().info("模型列表获取器初始化完成");
}

ModelListFetcher::~ModelListFetcher()
{
    if (m_currentReply)
    {
        m_currentReply->abort();
        m_currentReply->deleteLater();
    }
}

ModelListFetcher& ModelListFetcher::instance()
{
    static ModelListFetcher instance;
    return instance;
}

void ModelListFetcher::fetchModels(const QString& baseUrl, const QString& apiKey)
{
    if (baseUrl.isEmpty() || apiKey.isEmpty())
    {
        emit fetchFailed("Base URL或API Key不能为空");
        Logger::instance().warning("获取模型列表失败：Base URL或API Key为空");
        return;
    }

    if (m_currentReply)
    {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }

    QString url = baseUrl;
    if (!url.endsWith("/"))
    {
        url += "/";
    }
    url += "models";

    QNetworkRequest request((QUrl(url)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + apiKey).toUtf8());

    m_currentReply = m_networkManager->get(request);

    QTimer* timer = new QTimer(this);
    timer->setSingleShot(true);

    connect(timer, &QTimer::timeout, this,
            [this, timer]()
            {
                if (m_currentReply)
                {
                    m_currentReply->abort();
                    emit fetchFailed("请求超时，请检查网络连接或Base URL是否正确");
                    Logger::instance().warning("获取模型列表超时");
                }
                timer->deleteLater();
            });
    timer->start(15000);

    connect(m_currentReply, &QNetworkReply::finished, this,
            [this, timer]()
            {
                timer->stop();
                timer->deleteLater();
                onReplyFinished(m_currentReply);
            });

    Logger::instance().info("开始获取模型列表，URL: " + url);
}

void ModelListFetcher::onReplyFinished(QNetworkReply* reply)
{
    if (!reply)
    {
        return;
    }

    reply->deleteLater();
    m_currentReply = nullptr;

    if (reply->error() != QNetworkReply::NoError)
    {
        QString errorString = reply->errorString();
        emit fetchFailed("获取模型列表失败：" + errorString);
        Logger::instance().error("获取模型列表失败: " + errorString);
        return;
    }

    QByteArray data = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError)
    {
        emit fetchFailed("解析响应数据失败：" + parseError.errorString());
        Logger::instance().error("解析模型列表JSON失败: " + parseError.errorString());
        return;
    }

    QStringList models;
    QJsonObject rootObj = doc.object();

    if (rootObj.contains("data"))
    {
        QJsonArray dataArray = rootObj["data"].toArray();
        for (const QJsonValue& value : dataArray)
        {
            QJsonObject modelObj = value.toObject();
            if (modelObj.contains("id"))
            {
                models.append(modelObj["id"].toString());
            }
        }
    }
    else if (rootObj.contains("models"))
    {
        QJsonArray modelsArray = rootObj["models"].toArray();
        for (const QJsonValue& value : modelsArray)
        {
            QJsonObject modelObj = value.toObject();
            if (modelObj.contains("id"))
            {
                models.append(modelObj["id"].toString());
            }
            else if (modelObj.contains("name"))
            {
                models.append(modelObj["name"].toString());
            }
        }
    }
    else
    {
        for (auto it = rootObj.begin(); it != rootObj.end(); ++it)
        {
            if (it.value().isString())
            {
                models.append(it.value().toString());
            }
        }
    }

    if (models.isEmpty())
    {
        emit fetchFailed("未找到可用的模型列表");
        Logger::instance().warning("API响应中未找到模型列表");
        return;
    }

    models.sort();
    emit modelsFetched(models);
    Logger::instance().info("成功获取模型列表，共 " + QString::number(models.size()) + " 个模型");
}
