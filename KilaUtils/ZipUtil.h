#ifndef H_ZIPUTIL_H
#define H_ZIPUTIL_H

#include "KilaUtils.h"
#include <string>
#include <vector>
class KILAUTILS_API CZipUtil final
{
public:
    //将文件添加到zip文件中
    // fileNameInZip: 在zip文件中的文件名，包含相对路径
    // srcFile: 必须指定源文件全路径
    static int AddFileToZip(void* zf, const char* fileNameInZip, const char* srcFile, const bool isDir);

    static int zipOneFile(const std::wstring& wfileNameInZip, const std::wstring& wSrcFile,
                          const std::wstring& wOutputZipName);

    static std::vector<std::string> CreateZipFromDir_2depth_dirsAndfiles(const std::wstring& dirstr,
                                                                         const std::wstring& zipFileName,
                                                                         size_t              upload_range);

    static std::vector<std::string> GetDumpFiles(const std::wstring& dirstr, size_t upload_range);

    static int CollectDumpFilesInDir(const std::wstring& strPath, std::vector<std::wstring>& flist);
};

#endif