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
    int  RunExtended3_BunaGerekKalmadi(void);
    int  RunExtended3(void);
    int  RunExtended4(const std::vector<std::pair<std::wstring, std::vector<std::wstring>>>& wmiClasses);
    int  RunExtended5(void);
    int  Cleanup(void);

    std::map<std::wstring, std::wstring> QuerySingleItem(const std::wstring& className, const std::vector<std::wstring>& properties);

    std::vector<std::map<std::wstring, std::wstring>> QueryMultiItem(const std::wstring& className, const std::vector<std::wstring>& properties);

    // QueryMultiItem() için diski model + volume + serial olarak birleştiren özel fonksiyon yaz
    std::vector<std::map<std::wstring, std::wstring>> QueryLogicalToPhysicalDisks();

    std::wstring GetProperty(IWbemClassObject* pObj, const wchar_t* propertyName);

    void Release(IWbemClassObject* pObj);
    void Release(IEnumWbemClassObject* pObj);    

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
    void RunQueryAndPrintExtended3(const std::wstring& className, const std::vector<std::wstring>& properties, std::wofstream& logFile);

    std::vector<std::pair<std::wstring, std::vector<std::wstring>>> wmiClasses;
    void FillWmiClassesList(void);

    std::wstring VariantToWString(const VARIANT& vt);
};

#endif