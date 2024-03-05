#ifndef H_HTTPREQUEST_H
#define H_HTTPREQUEST_H

#ifdef HTTPREQUEST_EXPORTS
#define HTTPREQUEST_API __declspec(dllexport)
#else
#define HTTPREQUEST_API __declspec(dllimport)
#endif

#include <string>
#include <list>
#include <map>

//! 文件上传进度函数指针
typedef size_t (*UploadProcessFunc)(void* ptr, double totalToDownload, double nowDownloaded, double totalToUpload,
                                    double nowUploaded);
typedef std::string (*EncryptFunc)(const std::string& input);

//! 表单项目
struct FormItem
{
    //! 数据成员
    std::string name;
    std::string value;

    //! 构造函数
    FormItem(void)
    {
        name  = "";
        value = "";
    }

    //! 构造函数
    FormItem(const std::string& name, const std::string& value)
    {
        this->name  = name;
        this->value = value;
    }
};

//! Http请求类型
struct RequestType
{
    enum List
    {
        Post,
        Get
    };
};

//! Http请求类
class HTTPREQUEST_API CHttpRequest
{
public:
    //! 构造函数
    CHttpRequest(void);

    //! 析构函数
    ~CHttpRequest(void);

    /**
     *  @brief          Http请求
     *
     *  @param          RequestType::List      requestType	            请求类型
     *  @param          const std::string&     url	                    网址
     *  @param          std::list<std::string> mapParams	                请求参数列表
     *  @param          std::list<std::string> listRequestHeader	        请求头
     *  @param          std::string&           strResponse	            返回值(消息体)
     *  @param          std::string&           strHeaderResponse	        返回值(头部)
     *  @param          std::string&           strErrorInfo	            返回的错误信息
     *  @param          int                    nTimeout                  超时时间
     *  @param          bool                   bJsonFormat               是否为Json风格
     *  @return         int
     */
    int CurlPostGetRequest(RequestType::List requestType, const std::string& url,
                           std::map<std::string, std::string>& mapParams, std::list<std::string>& listRequestHeader,
                           std::string& strResponse, std::string& strHeaderResponse, std::string& strErrorInfo,
                           int nTimeout = 10, bool bJsonFormat = false);

    /**
     *  @brief          上传文件到服务端
     *
     *  @param          const std::string&     url	                    网址
     *  @param          const std::string&     localfile	                本地文件地址
     *  @param          void*                  cookie	                回调函数参数
     *  @param          UploadProcessFunc      progressfunc              上传文件回调函数
     *  @param          std::list<std::string> listRequestHeader	        请求头
     *  @param          std::string&           strResponse	            返回值(消息体)
     *  @param          std::string&           strErrorInfo	            返回的错误信息
     *  @param          bool&                  bNeedUpdateToken	        文件上传失败后是否需要更新token
     *  @return         int
     */
    int CurlUploadRequest(const std::string& url, const std::string& localfile, void* cookie, UploadProcessFunc progressfunc,
                          std::list<std::string>& listRequestHeader, std::string& strResponse, std::string& strErrorInfo,
                          bool& bNeedUpdateToken);

    /**
     *  @brief          提交表单
     *
     *  @param          const std::string&      url	                    网址
     *  @param          std::list<FormItem>&    items	                待提交的表单数据
     *  @param          std::list<std::string>& pictures	                待提交的图片文件路径
     *  @param          std::list<std::string>  listRequestHeader	    请求头
     *  @param          std::string&            strResponse	            返回值(消息体)
     *  @param          std::string&            strErrorInfo	            返回的错误信息
     *  @return         int
     */
    int CurlCommitFormRequst(const std::string& url, std::list<FormItem>& items, std::list<std::string>& pictures,
                             std::list<std::string>& listRequestHeader, std::string& strResponse, std::string& strErrorInfo);

    /**
     *  @brief          配置加密函数
     *
     *  @param          EncryptFunc             func	                    加密函数
     *  @return         void
     */
    void SetEncryptFunc(EncryptFunc func)
    {
        m_EncryptFunc = func;
    }

    //! 设置是否支持代理
    static void SetProxy(bool proxy)
    {
        m_EnableProxy = proxy;
    }

    static int Url_Encode(std::string& inout);

    static int Url_Decode(std::string& inout);

    static void Init();
    static void Destroy();

private:
    //! 是否开启代理
    static bool m_EnableProxy;

    //! 加密函数
    EncryptFunc m_EncryptFunc = nullptr;
};

#endif