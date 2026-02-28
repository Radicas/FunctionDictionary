/**
 * @file function_dict_c_api.h
 * @brief 函数字典C语言接口，提供跨语言调用支持
 * @author Developer
 * @date 2026-02-28
 * @version 1.0
 *
 * 本模块提供C语言风格的API，支持Python等其他语言调用。
 * 所有接口均使用extern "C"导出，确保二进制兼容性。
 */

#ifndef FUNCTION_DICT_C_API_H
#define FUNCTION_DICT_C_API_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化函数字典库
 * @param db_path 数据库文件路径（UTF-8编码）
 * @return 初始化是否成功，0表示成功，非0表示失败
 */
int function_dict_init(const char* db_path);

/**
 * @brief 检查函数字典是否已初始化
 * @return 是否已初始化，1表示已初始化，0表示未初始化
 */
int function_dict_is_initialized();

/**
 * @brief 添加函数
 * @param key 函数名称（UTF-8编码，不能为空）
 * @param value 函数介绍（UTF-8编码，Markdown格式）
 * @return 添加是否成功，0表示成功，非0表示失败
 */
int function_dict_add_function(const char* key, const char* value);

/**
 * @brief 根据ID删除函数
 * @param id 函数ID
 * @return 删除是否成功，0表示成功，非0表示失败
 */
int function_dict_delete_function(int id);

/**
 * @brief 批量删除函数
 * @param ids 函数ID数组
 * @param count ID数组长度
 * @return 删除是否成功，0表示成功，非0表示失败
 */
int function_dict_delete_functions(const int* ids, int count);

/**
 * @brief 检查函数是否存在
 * @param key 函数名称（UTF-8编码）
 * @return 是否存在，1表示存在，0表示不存在
 */
int function_dict_function_exists(const char* key);

/**
 * @brief 获取最后一次错误信息
 * @return 错误信息字符串（UTF-8编码），如果没有错误返回NULL
 * 注意：返回的字符串由内部管理，调用者不应释放
 */
const char* function_dict_get_last_error();

/**
 * @brief 清理资源
 * 注意：调用此函数后，需要重新调用function_dict_init才能再次使用
 */
void function_dict_cleanup();

#ifdef __cplusplus
}
#endif

#endif // FUNCTION_DICT_C_API_H
