#ifndef DeviceInfoH
#define DeviceInfoH

#include <Windows.h>
#include <winreg.h>

#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")

#include <wincrypt.h>
#pragma comment(lib, "advapi32.lib")

#include "HeaderFiles.h"

#include "FileLogger.h"
#include "ConsoleLogger.h"
#include "CompositeLogger.h"

class CDeviceInfo {
public:
    virtual ~CDeviceInfo();
             CDeviceInfo(std::shared_ptr<ILogger> pLogger);

    auto GetLogger(void);
    int  Reset(void);
    int  Init(void);
    int  Run(void);
    int  RunExtended(void);
    int  RunExtended2(void);
    int  Cleanup(void);

protected:

private:
    std::shared_ptr<ILogger> m_pLogger;
    void SetLogger(std::shared_ptr<ILogger> logger);
    void Log(const std::string& message);
    void Error(const std::string& message);
    void LogW(const std::wstring& message);
    void ErrorW(const std::wstring& message);

    IWbemLocator* pLoc = nullptr;
    IWbemServices* pSvc = nullptr;
    IEnumWbemClassObject* ExecQuery(const std::wstring& wql);
    std::wstring GetProperty(IWbemClassObject* obj, const BSTR propName);
    void RunQueryAndPrint(const std::wstring& className, const std::vector<std::wstring>& properties, std::wofstream& logFile);
    void RunQueryAndPrintExtended(const std::wstring& className, std::wofstream& logFile);
    void RunQueryAndPrintExtended2(const std::wstring& className, const std::vector<std::wstring>& properties, std::wofstream& logFile);

    std::vector<std::pair<std::wstring, std::vector<std::wstring>>> wmiClasses;
    void FillWmiClassesList(void);

};

#endif