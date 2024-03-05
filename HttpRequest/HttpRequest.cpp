#include "curl.h"
#include "HttpRequest.h"
#include <codecvt>
#include <StringUtil.h>
//! 静态变量
bool               CHttpRequest::m_EnableProxy = false;
static std::string httpSeed                    = "nJi9o;/";

//! 构造函数
CHttpRequest::CHttpRequest(void) {}

//! 析构函数
CHttpRequest::~CHttpRequest(void) {}

//! Post回调函数
size_t WriteData(void* buffer, size_t size, size_t nmemb, void* userp)
{
    std::string* str = dynamic_cast<std::string*>((std::string*)userp);
    if (nullptr == str || nullptr == buffer)
    {
        return -1;
    }

    char* pData = (char*)buffer;
    str->append(pData, size * nmemb);

    return nmemb;
}

//! 读取图片文件回调函数
static size_t ReadCallback(void* ptr, size_t size, size_t nmemb, void* stream)
{
    size_t     retcode;
    curl_off_t nread;
    retcode = fread(ptr, size, nmemb, (FILE*)stream);

    nread = (curl_off_t)retcode;

    fprintf(stderr, "*** We read %" CURL_FORMAT_CURL_OFF_T " bytes from file\n", nread);

    return retcode;
}

//! 字符转换
static std::string string_To_UTF8(const std::string& str)
{
    int      nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
    wchar_t* pwBuf = new wchar_t[nwLen + 1];
    ZeroMemory(pwBuf, nwLen * 2 + 2);
    ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);
    int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

    char* pBuf = new char[nLen + 1];
    ZeroMemory(pBuf, nLen + 1);

    ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);
    std::string retStr(pBuf);

    delete[] pwBuf;
    delete[] pBuf;
    pwBuf = NULL;
    pBuf  = NULL;

    return retStr;
}

//! 同Http服务端通信
int CHttpRequest::CurlPostGetRequest(RequestType::List requestType, const std::string& url,
                                     std::map<std::string, std::string>& mapParams, std::list<std::string>& listRequestHeader,
                                     std::string& strResponse, std::string& strHeaderResponse, std::string& strErrorInfo,
                                     int nTimeout /* = 10*/, bool bJsonFormat /* = false*/)
{
    //! 返回值
    CURLcode retCode  = CURLE_OK;
    int      httpCode = -1;
    strResponse       = "";
    strHeaderResponse = "";
    strErrorInfo      = "";

    //! 初始化
    CURL* curl = curl_easy_init();
    if (nullptr == curl)
    {
        httpCode     = CURLE_FAILED_INIT;
        strErrorInfo = "curl_easy_init() return nullptr";

        return httpCode;
    }

    //! 设置参数值
    std::string                                  urlParam = "";
    std::map<std::string, std::string>::iterator iter;
    int                                          count = 0;
    for (iter = mapParams.begin(); iter != mapParams.end(); iter++)
    {
        if (count > 0)
        {
            urlParam.append("&");
        }

        //! 打点时参数比较特殊
        if (iter->first != "")
        {
            urlParam.append(iter->first);
            urlParam.append("=");
            urlParam.append(iter->second);
        }
        else
        {
            urlParam.append(iter->second);
        }

        count++;
    }

    //! json风格则需要重新设置参数
    if (bJsonFormat)
    {
        urlParam = "";
        count    = 0;
        std::map<std::string, std::string>::iterator iter;
        urlParam.append("{");
        for (iter = mapParams.begin(); iter != mapParams.end(); iter++)
        {
            urlParam.append("\"");
            urlParam.append(iter->first);
            urlParam.append("\":");
            if (iter->second.compare(std::string("").c_str()) == 0)
            {
                urlParam.append("\"\"");
            }
            else
            {
                if (iter->first.compare(std::string("keyword").c_str()) == 0)
                {
                    urlParam.append("\"");
                    urlParam.append(iter->second);
                    urlParam.append("\"");
                }
                else
                {
                    urlParam.append(iter->second);
                }
            }

            if (count < ((int)mapParams.size() - 1))
            {
                urlParam.append(",");
            }

            count++;
        }

        urlParam.append("}");

        //! 格式转换
        urlParam = string_To_UTF8(urlParam);
    }

    //! 加密
    if (count > 0)
    {
        if (m_EncryptFunc != nullptr)
        {
            std::string sign = (*m_EncryptFunc)(httpSeed + urlParam);
            urlParam         = urlParam + std::string("&sign=") + sign;
        }
    }

    //! Post请求模式
    if (requestType == RequestType::Post)
    {
        //! 设置url参数
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, urlParam.c_str());

        //! 设置为Post模式
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
    }
    //! Get请求模式
    else
    {
        //! url地址
        std::string combineUrl = url;
        if (!urlParam.empty())
        {
            combineUrl.append("?");
            combineUrl.append(urlParam);
        }

        curl_easy_setopt(curl, CURLOPT_URL, combineUrl.c_str());

        //! 设置为Get模式
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    }

    //! 构建HTTP报文头
    struct curl_slist* headers = nullptr;
    {
        if (listRequestHeader.size() > 0)
        {
            std::list<std::string>::iterator iter, iterEnd;
            iter    = listRequestHeader.begin();
            iterEnd = listRequestHeader.end();
            for (iter; iter != iterEnd; iter++)
            {
                headers = curl_slist_append(headers, iter->c_str());
            }

            if (headers != nullptr)
            {
                //! 设置http请求头信息
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            }
        }
    }

    //! 设置agent(curl version + window version)
    std::string agent = std::string("UXLive/1.9.4 (Windows 10)");
    curl_easy_setopt(curl, CURLOPT_USERAGENT, agent.c_str());

    //! 配置Get回调函数
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strResponse);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);

    curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void*)&strHeaderResponse);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WriteData);

    //! 是否配置代码(调试用)
    if (m_EnableProxy)
    {
        curl_easy_setopt(curl, CURLOPT_PROXY, "127.0.0.1:8888");
    }

    //! 其它设置
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, nTimeout);
    curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1L);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);

    //! 开始请求
    retCode = curl_easy_perform(curl);
    if (CURLE_OK == retCode)
    {
        retCode = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        if (CURLE_OK == retCode)
        {
            if (200 != httpCode)
            {
                strErrorInfo = "curl_easy_getinfo ok, but http ret is not 200";
                strErrorInfo.append(curl_easy_strerror((CURLcode)httpCode));
            }
        }
        else
        {
            httpCode     = (int)retCode;
            strErrorInfo = "curl_easy_getinfo() failed ";
            strErrorInfo.append(curl_easy_strerror(retCode));
        }
    }
    else
    {
        httpCode     = (int)retCode;
        strErrorInfo = "curl_easy_perform() failed ";
        strErrorInfo.append(curl_easy_strerror(retCode));
    }

    //! 释放资源
    if (headers != nullptr)
    {
        curl_slist_free_all(headers);
    }

    curl_easy_cleanup(curl);

    return httpCode;
}

//! 格式转换
static std::wstring utf8_to_utf16(const std::string& source)
{
    try
    {
        static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> cvt;
        return cvt.from_bytes(source);
    }
    catch (std::range_error&)
    {
        return std::wstring();
    }
}

//! 上传图片到服务端
int CHttpRequest::CurlUploadRequest(const std::string& url, const std::string& localfile, void* cookie,
                                    UploadProcessFunc progressfunc, std::list<std::string>& listRequestHeader,
                                    std::string& strResponse, std::string& strErrorInfo, bool& bNeedUpdateToken)
{
    //! 返回值
    CURLcode retCode  = CURLE_OK;
    int      httpCode = -1;
    strResponse       = "";
    strErrorInfo      = "";
    bNeedUpdateToken  = false;

    std::wstring wfile = utf8_to_utf16(localfile);

    FILE* file = nullptr;
    _wfopen_s(&file, wfile.c_str(), L"rb");
    if (nullptr == file)
    {
        strErrorInfo = "file is not exist or no permisson.";
        return -2;
    }

    _fseeki64(file, 0, SEEK_END);
    __int64 filesize = _ftelli64(file);
    _fseeki64(file, 0, SEEK_SET);

    //! 初始化
    CURL* curl = curl_easy_init();
    if (nullptr == curl)
    {
        httpCode     = CURLE_FAILED_INIT;
        strErrorInfo = "curl_easy_init() return nullptr";

        return httpCode;
    }

    //! 构建HTTP报文头
    struct curl_slist* headers = nullptr;
    {
        if (listRequestHeader.size() > 0)
        {
            std::list<std::string>::iterator iter, iterEnd;
            iter    = listRequestHeader.begin();
            iterEnd = listRequestHeader.end();
            for (iter; iter != iterEnd; iter++)
            {
                headers = curl_slist_append(headers, iter->c_str());
            }

            if (headers != nullptr)
            {
                //设置http请求头信息
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            }
        }
    }

    //! 配置图片文件读取函数
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, ReadCallback);
    curl_easy_setopt(curl, CURLOPT_READDATA, file);

    //! 配置上传图片后返回内容
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strResponse);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);

    //! 配置上传进度函数
    if (progressfunc != nullptr)
    {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, FALSE);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressfunc);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, cookie);
    }

    //! 设置url参数
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_PUT, 1L);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    //! 设置文件上传大小
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)filesize);

    //! 其它设置
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);

    //! 开始请求
    retCode = curl_easy_perform(curl);
    if (CURLE_OK == retCode)
    {
        retCode = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        if (CURLE_OK == retCode)
        {
            if (200 != httpCode)
            {
                strErrorInfo = "curl_easy_getinfo ok, but http ret is not 200";
                strErrorInfo.append(curl_easy_strerror((CURLcode)httpCode));

                //! 需要更新上传token
                bNeedUpdateToken = true;
            }
        }
        else
        {
            httpCode     = (int)retCode;
            strErrorInfo = "curl_easy_getinfo() failed ";
            strErrorInfo.append(curl_easy_strerror(retCode));
        }
    }
    else
    {
        httpCode     = (int)retCode;
        strErrorInfo = "curl_easy_perform() failed ";
        strErrorInfo.append(curl_easy_strerror(retCode));
    }

    //! 释放资源
    if (headers != nullptr)
    {
        curl_slist_free_all(headers);
    }

    curl_easy_cleanup(curl);

    //! 关闭文件
    if (file)
    {
        fclose(file);
    }

    return httpCode;
}

//! 提交表单
int CHttpRequest::CurlCommitFormRequst(const std::string& url, std::list<FormItem>& items, std::list<std::string>& pictures,
                                       std::list<std::string>& listRequestHeader, std::string& strResponse,
                                       std::string& strErrorInfo)
{
    //! 返回值
    CURLcode retCode  = CURLE_OK;
    int      httpCode = -1;
    strResponse       = "";
    strErrorInfo      = "";

    //! 初始化
    CURL* curl = curl_easy_init();
    if (nullptr == curl)
    {
        httpCode     = CURLE_FAILED_INIT;
        strErrorInfo = "curl_easy_init() return nullptr";

        return httpCode;
    }

    //! 构建HTTP报文头
    struct curl_slist* headers = nullptr;
    {
        if (listRequestHeader.size() > 0)
        {
            std::list<std::string>::iterator iter, iterEnd;
            iter    = listRequestHeader.begin();
            iterEnd = listRequestHeader.end();
            for (iter; iter != iterEnd; iter++)
            {
                headers = curl_slist_append(headers, iter->c_str());
            }

            if (headers != nullptr)
            {
                //! 设置http请求头信息
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            }
        }
    }

    //! 配置数据写入回调函数
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strResponse);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);

    //! 配置表单数据
    struct curl_httppost* post = nullptr;
    struct curl_httppost* last = nullptr;
    if (items.size() > 0)
    {
        std::list<FormItem>::iterator iter, iterEnd;
        iter    = items.begin();
        iterEnd = items.end();
        for (iter; iter != iterEnd; iter++)
        {
            curl_formadd(&post, &last, CURLFORM_COPYNAME, iter->name.c_str(), CURLFORM_COPYCONTENTS, iter->value.c_str(),
                         CURLFORM_END);
        }
    }

    //! 图片
    if (pictures.size() > 0)
    {
        std::list<std::string>::iterator iter, iterEnd;
        iter    = pictures.begin();
        iterEnd = pictures.end();
        for (iter; iter != iterEnd; iter++)
        {
            struct curl_forms forms[2];
            {
                forms[0].option = CURLFORM_FILE;
                forms[0].value  = iter->c_str();
            }

            forms[1].option = CURLFORM_END;
            curl_formadd(&post, &last, CURLFORM_COPYNAME, "imageFiles[]", CURLFORM_ARRAY, forms, CURLFORM_CONTENTTYPE,
                         "application/octet-stream", CURLFORM_END);
        }
    }

    curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);

    //! 设置url参数
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);

    //! 开始请求
    retCode = curl_easy_perform(curl);
    if (CURLE_OK == retCode)
    {
        retCode = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        if (CURLE_OK == retCode)
        {
            if (200 != httpCode)
            {
                strErrorInfo = "curl_easy_getinfo ok, but http ret is not 200";
                strErrorInfo.append(curl_easy_strerror((CURLcode)httpCode));
            }
        }
        else
        {
            httpCode     = (int)retCode;
            strErrorInfo = "curl_easy_getinfo() failed ";
            strErrorInfo.append(curl_easy_strerror(retCode));
        }
    }
    else
    {
        httpCode     = (int)retCode;
        strErrorInfo = "curl_easy_perform() failed ";
        strErrorInfo.append(curl_easy_strerror(retCode));
    }

    //! 释放资源
    if (headers != nullptr)
    {
        curl_slist_free_all(headers);
    }

    curl_easy_cleanup(curl);

    return httpCode;
}

int CHttpRequest::Url_Encode(std::string& inout)
{
    int   ret  = -1;
    CURL* curl = nullptr;
    curl       = curl_easy_init();
    if (curl)
    {
        char* out = curl_easy_escape(curl, inout.c_str(), inout.length());
        if (out)
        {
            inout = out;
            curl_free(out);
            ret = 0;
        }
        curl_easy_cleanup(curl);
    }
    return ret;
}

int CHttpRequest::Url_Decode(std::string& inout)
{
    // add by liangk,solve urldecode +&space error
    std::string url = CStringUtil::replace_str(inout, "+", "%20");

    int   ret  = -1;
    CURL* curl = nullptr;
    curl       = curl_easy_init();
    if (curl)
    {
        int   out_len = 0;
        char* out     = curl_easy_unescape(curl, url.c_str(), inout.length(), &out_len);
        if (out)
        {
            inout = out;
            curl_free(out);
            ret = 0;
        }
        curl_easy_cleanup(curl);
    }
    return ret;
}

void CHttpRequest::Init()
{
    curl_global_init(CURL_GLOBAL_ALL);
}

void CHttpRequest::Destroy()
{
    curl_global_cleanup();
}