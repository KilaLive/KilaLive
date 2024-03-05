#include "ZipUtil.h"
#define ZLIB_WINAPI
#include "zip.h"
#include <regex>
#define UXLOG_ERR
#include <StringUtil.h>
#include <regex>
#include <direct.h>
#include <io.h>
#include "JuceConfig.h"
#include "juce_core/juce_core.h"
#include <map>

#define UXLOG_INFO
int CZipUtil::AddFileToZip(void* zf, const char* fileNameInZip, const char* srcFile, const bool isDir)
{
    if (NULL == zf || NULL == fileNameInZip || NULL == srcFile) return -1;

    if (0 == strlen(fileNameInZip) || 0 == strlen(srcFile)) return -2;

    //初始化写入zip的文件信息
    zip_fileinfo zi;
    zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour = zi.tmz_date.tm_mday = zi.tmz_date.tm_mon =
        zi.tmz_date.tm_year                                                             = 0;
    zi.dosDate                                                                          = 0;
    zi.internal_fa                                                                      = 0;
    zi.external_fa                                                                      = 0;

    //在zip文件中创建新文件
    zipOpenNewFileInZip(zf, fileNameInZip, &zi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION);

    if (false == isDir)
    {
        //打开源文件
        FILE* srcfp = fopen(srcFile, "rb");
        if (srcfp == NULL)
        {
            UXLOG_ERR("can not add file [%s]", srcFile);
            zipCloseFileInZip(zf);  //关闭zip文件
            return -3;
        }

        //读入源文件并写入zip文件
        char buf[100 * 1024];  // buffer
        int  numBytes = 0;
        while (!feof(srcfp))
        {
            numBytes = fread(buf, 1, sizeof(buf), srcfp);
            zipWriteInFileInZip(zf, buf, numBytes);
            if (ferror(srcfp)) break;
        }

        //关闭源文件
        fclose(srcfp);
    }

    //关闭zip文件
    zipCloseFileInZip(zf);

    return 0;
}

int DiffFromNowInDay(std::tm& date)
{
    time_t logtime = mktime(&date);

    time_t    tt = time(NULL);
    struct tm tmTime;
    localtime_s(&tmTime, &tt);
    // 忽略时分秒，确保上一次直播那天的日志都上传
    tmTime.tm_hour = 0;
    tmTime.tm_min  = 0;
    tmTime.tm_sec  = 0;
    time_t nowtime = mktime(&tmTime);
    if (nowtime < logtime)
    {
        return -1;
    }
    int insec = difftime(nowtime, logtime);
    return insec / (24 * 3600);
};

int CZipUtil::zipOneFile(const std::wstring& wfileNameInZip, const std::wstring& wSrcFile, const std::wstring& wOutputZipName)
{
    std::string fileNameInZip = CStringUtil::utf16toAnsi(wfileNameInZip);
    std::string srcfile       = CStringUtil::utf16toAnsi(wSrcFile);
    std::string outputZipName = CStringUtil::utf16toAnsi(wOutputZipName);

    zipFile newZipFile = zipOpen(outputZipName.c_str(), APPEND_STATUS_CREATE);

    if (NULL == newZipFile)
    {
        UXLOG_ERR("zipOneFile zipOpen error [%s]", outputZipName.c_str());
        return -1;
    }

    int ret = CZipUtil::AddFileToZip(newZipFile, fileNameInZip.c_str(), srcfile.c_str(), false);
    if (0 == ret)
    {
        UXLOG_INFO("zipOneFile AddFileToZip [%s]", fileNameInZip.c_str());
    }
    else
    {
        UXLOG_ERR("zipOneFile AddFileToZip [%s] ret %d", fileNameInZip.c_str(), ret);
    }

    zipClose(newZipFile, NULL);
    return 0;
}

void FilterFunc(juce::Array<juce::File>& filterd, juce::Array<juce::File>& subfiles, size_t upload_range,
                bool isRootLogDir = false)
{
    std::map<int, std::vector<juce::File>> filesinrange;
    for (auto& f : subfiles)
    {
        std::string filename = f.getFileNameWithoutExtension().toStdString();

        if (f.hasFileExtension("log") || f.hasFileExtension("xlog") || f.hasFileExtension("dmp"))
        {
            if (isRootLogDir)
            {
                size_t pos = filename.find_first_of('_');
                if (pos == std::string::npos)
                {
                    filterd.add(f);
                    continue;
                }
                else
                {
                    if (filename.length() < 3)
                    {
                        continue;
                    }
                    /*std::string prestr = filename.substr(0, 2);
                    if (prestr != "PC")
                    {
                        continue;
                    }*/
                }
            }

            if (f.hasFileExtension("dmp"))
            {
                filterd.add(f);
            }

            std::regex pattern1("^[a-zA-Z]+_[0-9]{4}_[0-9]{1,2}_[0-9]{1,2}_[0-9]{1,2}_[0-9]{1,2}_[0-9]{1,2}");
            if (regex_match(filename, pattern1))
            {
                size_t pos = filename.find_first_of('_');
                if (pos == std::string::npos)
                {
                    continue;
                }
                std::string datestr = filename.substr(pos + 1, filename.length() - pos);

                std::tm date;
                sscanf(datestr.c_str(), "%d_%d_%d_%d_%d_%d", &date.tm_year, &date.tm_mon, &date.tm_mday, &date.tm_hour,
                       &date.tm_min, &date.tm_sec);
                date.tm_year -= 1900;
                date.tm_mon -= 1;
                // 忽略时分秒，确保上一次直播那天的日志都上传
                date.tm_hour = 0;
                date.tm_min  = 0;
                date.tm_sec  = 0;

                int diffinday = DiffFromNowInDay(date);
                if (diffinday < 0)
                {
                    UXLOG_ERR("CreateZipFromDir3 file filter error, invalid log file time: %s", filename.c_str());
                    continue;
                }
                filesinrange[diffinday].push_back(f);
                continue;
            }

            std::regex pattern2("^[a-zA-Z]+_[0-9]{8}");
            if (regex_match(filename, pattern2))
            {
                size_t pos = filename.find_first_of('_');
                if (pos == std::string::npos)
                {
                    continue;
                    ;
                }
                std::string datestr  = filename.substr(pos + 1, filename.length() - pos);
                std::string yearstr  = datestr.substr(0, 4);
                std::string monthstr = datestr.substr(4, 2);
                std::string daystr   = datestr.substr(6, 2);

                std::tm date;
                sscanf(yearstr.c_str(), "%d", &date.tm_year);
                sscanf(monthstr.c_str(), "%d", &date.tm_mon);
                sscanf(daystr.c_str(), "%d", &date.tm_mday);
                date.tm_year -= 1900;
                date.tm_mon -= 1;
                // 忽略时分秒，确保上一次直播那天的日志都上传
                date.tm_hour = 0;
                date.tm_min  = 0;
                date.tm_sec  = 0;

                int diffinday = DiffFromNowInDay(date);
                if (diffinday < 0)
                {
                    UXLOG_ERR("CreateZipFromDir3 file filter error, invalid log file time: %s", filename.c_str());
                    continue;
                }
                filesinrange[diffinday].push_back(f);
                continue;
            }
        }
        else if (f.hasFileExtension("mmap2"))
        {
            filterd.add(f);
        }
    }

    int rangecount = 0;
    for (auto& kv : filesinrange)
    {
        if (kv.first == 0)
        {
            auto& files = kv.second;
            for (auto& f : files)
            {
                filterd.add(f);
            }
        }
        else if (rangecount < upload_range)
        {
            rangecount++;
            auto& files = kv.second;
            for (auto& f : files)
            {
                filterd.add(f);
            }
        }
        else
        {
            break;
        }
    }
}

#define CLOGFILE_SUFFIX "clog"
void FilterFuncForLog4CPP(juce::Array<juce::File>& filterd, juce::Array<juce::File>& subfiles, size_t upload_range)
{
    std::map<int, std::vector<juce::File>> filesinrange;
    int                                    dateLen = std::string("2022_01_01_01_01_01").length();
    for (auto& f : subfiles)
    {
        std::string filename = f.getFileName().toStdString();
        std::string pstr     = "^[a-zA-Z]+_[0-9]{4}_[0-9]{1,2}_[0-9]{1,2}_[0-9]{1,2}_[0-9]{1,2}_[0-9]{1,2}\.("
                           + std::string(CLOGFILE_SUFFIX) + ")(\.[0-9]+)?";
        std::regex pattern(pstr.c_str());
        if (regex_match(filename, pattern))
        {
            size_t pos = filename.find_first_of('_');
            if (pos == std::string::npos)
            {
                continue;
            }

            if (filename.length() < dateLen + pos + 1) continue;

            std::string datestr = filename.substr(pos + 1, dateLen);

            std::tm date;
            sscanf(datestr.c_str(), "%d_%d_%d_%d_%d_%d", &date.tm_year, &date.tm_mon, &date.tm_mday, &date.tm_hour,
                   &date.tm_min, &date.tm_sec);
            date.tm_year -= 1900;
            date.tm_mon -= 1;
            // 忽略时分秒，确保上一次直播那天的日志都上传
            date.tm_hour = 0;
            date.tm_min  = 0;
            date.tm_sec  = 0;

            int diffinday = DiffFromNowInDay(date);
            if (diffinday < 0)
            {
                UXLOG_ERR("CreateZipFromDir3 file filter error, invalid log file time: %s", filename.c_str());
                continue;
            }

            filesinrange[diffinday].push_back(f);
        }
    }

    int rangecount = 0;
    for (auto& kv : filesinrange)
    {
        if (kv.first == 0)
        {
            auto& files = kv.second;
            for (auto& f : files)
            {
                filterd.add(f);
            }
        }
        else if (rangecount < upload_range)
        {
            rangecount++;
            auto& files = kv.second;
            for (auto& f : files)
            {
                filterd.add(f);
            }
        }
        else
        {
            break;
        }
    }
}

#define LOG_UPLOAD_RTC 1
std::vector<std::string> CZipUtil::CreateZipFromDir_2depth_dirsAndfiles(const std::wstring& dirstr,
                                                                        const std::wstring& zipFileName, size_t upload_range)
{
    std::vector<std::string> zipedfiles;
    juce::File               target(juce::String(dirstr.c_str()));

    if (false == target.exists()) return zipedfiles;

    std::string zipfile = CStringUtil::utf16toAnsi(zipFileName);

    zipFile newZipFile = zipOpen(zipfile.c_str(), APPEND_STATUS_CREATE);

    if (NULL == newZipFile)
    {
        UXLOG_ERR("CreateZipFromDir3 zipOpen error [%s]", zipfile.c_str());
        return zipedfiles;
    }

    juce::Array<juce::File> files = target.findChildFiles(juce::File::findFiles, false);
    juce::Array<juce::File> filterdfiles;
    FilterFunc(filterdfiles, files, upload_range, true);
    for (auto f : filterdfiles)
    {
        if (f.hasFileExtension("log") || f.hasFileExtension("xlog") || f.hasFileExtension("mmap2"))
        {
            std::string fileNameInZip   = CStringUtil::wstring2string(f.getRelativePathFrom(target).toWideCharPointer());
            std::string srcFullFileName = CStringUtil::wstring2string(f.getFullPathName().toWideCharPointer());
            int         ret = CZipUtil::AddFileToZip(newZipFile, fileNameInZip.c_str(), srcFullFileName.c_str(), false);
            if (0 == ret)
            {
                zipedfiles.push_back(srcFullFileName);
                UXLOG_INFO("CreateZipFromDir3 AddFileToZip [%s]", fileNameInZip.c_str());
            }
            else
            {
                UXLOG_ERR("CreateZipFromDir3 AddFileToZip [%s] ret %d", fileNameInZip.c_str(), ret);
            }
        }
    }

    juce::Array<juce::File> subdirs = target.findChildFiles(juce::File::findDirectories, false);
    for (auto dir : subdirs)
    {
        std::string dirNameInZip = CStringUtil::wstring2string((dir.getRelativePathFrom(target) + "/").toWideCharPointer());
        if (dirNameInZip == "rtc/" && LOG_UPLOAD_RTC == 0)
        {
            continue;
        }
        std::string srcFullFileName = CStringUtil::wstring2string(dir.getFullPathName().toWideCharPointer());
        int         ret             = CZipUtil::AddFileToZip(newZipFile, dirNameInZip.c_str(), srcFullFileName.c_str(), true);
        if (0 != ret)
        {
            UXLOG_ERR("CreateZipFromDir3 AddFileToZip [%s] ret %d", dirNameInZip.c_str(), ret);
            continue;
        }

        UXLOG_INFO("CreateZipFromDir3 AddFileToZip [%s]", dirNameInZip.c_str());
        juce::Array<juce::File> subfiles = dir.findChildFiles(juce::File::findFiles, false);
        juce::Array<juce::File> filterdfiles;

        if (dirNameInZip == "daesrv/" || dirNameInZip == "pc/" || dirNameInZip == "uxsdk/" || dirNameInZip == "zdae/"
            || dirNameInZip == "capRender/")
        {
            FilterFuncForLog4CPP(filterdfiles, subfiles, upload_range);
        }
        else
        {
            FilterFunc(filterdfiles, subfiles, upload_range);
        }

        for (auto f : filterdfiles)
        {
            std::string fileNameInZip   = dirNameInZip + CStringUtil::wstring2string(f.getFileName().toWideCharPointer());
            std::string srcFullFileName = CStringUtil::wstring2string(f.getFullPathName().toWideCharPointer());
            int         ret = CZipUtil::AddFileToZip(newZipFile, fileNameInZip.c_str(), srcFullFileName.c_str(), false);
            if (0 == ret)
            {
                UXLOG_INFO("CreateZipFromDir3 AddFileToZip [%s]", fileNameInZip.c_str());
                zipedfiles.push_back(srcFullFileName);
            }
            else
            {
                UXLOG_ERR("CreateZipFromDir3 AddFileToZip [%s] ret %d", fileNameInZip.c_str(), ret);
            }
        }
    }

    zipClose(newZipFile, NULL);
    return zipedfiles;
}

std::vector<std::string> CZipUtil::GetDumpFiles(const std::wstring& dirstr, size_t upload_range)
{
    std::vector<std::string> files;
    juce::File               target(juce::String(dirstr.c_str()));

    if (false == target.exists()) return files;

    juce::Array<juce::File> subdirs = target.findChildFiles(juce::File::findDirectories, false);
    for (auto dir : subdirs)
    {
        std::string dirNameInZip = CStringUtil::wstring2string((dir.getRelativePathFrom(target) + "/").toWideCharPointer());
        if (dirNameInZip != "dmps/")
        {
            continue;
        }

        juce::Array<juce::File> subfiles = dir.findChildFiles(juce::File::findFiles, false);
        juce::Array<juce::File> filterdfiles;

        FilterFunc(filterdfiles, subfiles, upload_range);

        for (auto f : filterdfiles)
        {
            if (f.hasFileExtension("dmp"))
            {
                std::string fileNameInZip   = dirNameInZip + CStringUtil::wstring2string(f.getFileName().toWideCharPointer());
                std::string srcFullFileName = CStringUtil::wstring2string(f.getFullPathName().toWideCharPointer());
                files.push_back(srcFullFileName);
            }
        }
    }

    return files;
}

bool isDirectoryExist(const wchar_t* lpPath)
{
    if (!lpPath) return false;

    DWORD dwAttr = GetFileAttributes(lpPath);
    if (INVALID_FILE_ATTRIBUTES == dwAttr || (dwAttr & FILE_ATTRIBUTE_DIRECTORY))
    {
        return true;
    }
    return false;
}

int CZipUtil::CollectDumpFilesInDir(const std::wstring& strPath, std::vector<std::wstring>& flist)
{
    struct _wfinddata_t c_file;
    flist.clear();
    int          count    = 0;
    std::wstring wstrPath = strPath;
    if (false == isDirectoryExist(wstrPath.c_str()))
    {
        UXLOG_ERR("unable to locate the directory [%s]", wstrPath.c_str());
        return 0;
    }
    wstrPath += L"\\*.dmp";

    long hFile = _wfindfirst(wstrPath.c_str(), &c_file);
    if (hFile == -1)
    {
        UXLOG_ERR("_wfindfirst return -1");
        return 0;
    }
    else
    {
        flist.push_back(std::wstring(c_file.name));
        count++;
    }

    while (_wfindnext(hFile, &c_file) == 0)
    {
        flist.push_back(std::wstring(c_file.name));
        count++;
    }
    _findclose(hFile);

    return count;
}