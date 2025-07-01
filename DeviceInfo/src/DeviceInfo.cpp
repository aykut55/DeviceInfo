#include "DeviceInfo.h"

CDeviceInfo::~CDeviceInfo()
{
    Cleanup();
}

CDeviceInfo::CDeviceInfo(std::shared_ptr<ILogger> pLogger) : m_pLogger(pLogger)
{
    Reset();

    FillWmiClassesList();
}

void CDeviceInfo::Log(const std::string& message)
{
    if (m_pLogger)
    {
        m_pLogger->Log(message);
    }
    else
    {

    }
}

void CDeviceInfo::Error(const std::string& message)
{
    if (m_pLogger)
    {
        m_pLogger->Error(message);
    }
    else
    {

    }
}

void CDeviceInfo::LogW(const std::wstring& message)
{
    if (m_pLogger)
    {

    }
    else
    {

    }
}

void CDeviceInfo::ErrorW(const std::wstring& message)
{
    if (m_pLogger)
    {

    }
    else
    {

    }
}

void CDeviceInfo::SetLogger(std::shared_ptr<ILogger> pLogger) 
{
    if (pLogger) m_pLogger = pLogger;
}

auto CDeviceInfo::GetLogger(void)
{
    return m_pLogger;
}

int CDeviceInfo::Reset(void)
{
    return 1;
}

int CDeviceInfo::Init(void)
{
    HRESULT hr;

    hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hr)) return false;

    hr = CoInitializeSecurity(NULL, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
    if (FAILED(hr)) return false;

    hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc);
    if (FAILED(hr)) return false;

    hr = pLoc->ConnectServer(BSTR(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
    if (FAILED(hr)) return false;

    hr = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
        RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
    return SUCCEEDED(hr);

    return 1;
}

int CDeviceInfo::Run(void)
{
    std::wofstream logFile(L"device_output.txt");
    logFile.imbue(std::locale(""));

    for (size_t i = 0; i < wmiClasses.size(); ++i) {
        const std::wstring& className = wmiClasses[i].first;
        const std::vector<std::wstring>& properties = wmiClasses[i].second;
        RunQueryAndPrint(className, properties, logFile);
    }

    logFile.close();

    return 1;
}

int CDeviceInfo::RunExtended(void)
{
    std::wofstream logFile(L"device_output_extended.txt");
    logFile.imbue(std::locale(""));

    for (size_t i = 0; i < wmiClasses.size(); ++i) {
        const std::wstring& className = wmiClasses[i].first;
        RunQueryAndPrintExtended(className, logFile);
    }

    logFile.close();

    return 1;
}

int CDeviceInfo::RunExtended2(void)
{
    std::wofstream logFile(L"device_output_extended2.txt");
    logFile.imbue(std::locale(""));

    for (size_t i = 0; i < wmiClasses.size(); ++i) {
        const std::wstring& className = wmiClasses[i].first;
        const std::vector<std::wstring>& properties = wmiClasses[i].second;
        RunQueryAndPrintExtended2(className, properties, logFile);
    }

    logFile.close();

    return 1;
}

int CDeviceInfo::Cleanup(void)
{
    if (pSvc) {
        pSvc->Release(); 
        pSvc = 0;
    }

    if (pLoc) {
        pLoc->Release(); 
        pLoc = 0;
    }

    CoUninitialize();

    return 1;
}

IEnumWbemClassObject* CDeviceInfo::ExecQuery(const std::wstring& wql) 
{
    IEnumWbemClassObject* pEnumerator = nullptr;
    HRESULT hr = pSvc->ExecQuery(BSTR(L"WQL"), BSTR(wql.c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
    return SUCCEEDED(hr) ? pEnumerator : nullptr;
}

std::wstring CDeviceInfo::GetProperty(IWbemClassObject* obj, const BSTR propName) 
{
    VARIANT vtProp;
    VariantInit(&vtProp);
    std::wstring result;

    if (SUCCEEDED(obj->Get(propName, 0, &vtProp, 0, 0))) {
        if (vtProp.vt == VT_BSTR && vtProp.bstrVal)
            result = vtProp.bstrVal;
        else if (vtProp.vt == VT_I4)
            result = std::to_wstring(vtProp.intVal);
        else if (vtProp.vt == VT_BOOL)
            result = vtProp.boolVal ? L"True" : L"False";
        else if (vtProp.vt == VT_UI8)
            result = std::to_wstring(vtProp.ullVal);
    }

    VariantClear(&vtProp);
    return result;
}

void CDeviceInfo::RunQueryAndPrint(const std::wstring& className, const std::vector<std::wstring>& properties, std::wofstream& logFile) 
{
    IEnumWbemClassObject* pEnumerator = ExecQuery(L"SELECT * FROM " + className);
    if (!pEnumerator) return;

    IWbemClassObject* pObj = nullptr;
    ULONG ret;
    while (pEnumerator->Next(WBEM_INFINITE, 1, &pObj, &ret) == S_OK) {
        std::wcout << L"\n[" << className << L"]\n";
        logFile << L"\n[" << className << L"]\n";

        for (size_t i = 0; i < properties.size(); ++i) {
            const std::wstring& prop = properties[i];
            std::wstring val = GetProperty(pObj, const_cast<BSTR>(prop.c_str()));
            std::wcout << prop << L": " << val << L"\n";
            logFile << prop << L": " << val << L"\n";
        }

        pObj->Release();
    }

    pEnumerator->Release();
}

void CDeviceInfo::RunQueryAndPrintExtended(const std::wstring& className, std::wofstream& logFile) 
{
    IEnumWbemClassObject* pEnumerator = ExecQuery(L"SELECT * FROM " + className);
    if (!pEnumerator) return;

    IWbemClassObject* pObj = nullptr;
    ULONG ret;
    while (pEnumerator->Next(WBEM_INFINITE, 1, &pObj, &ret) == S_OK) 
    {
        std::wcout << L"\n[" << className << L"]\n";
        logFile << L"\n[" << className << L"]\n";

        SAFEARRAY* namesArray = nullptr;
        HRESULT hr = pObj->GetNames(NULL, WBEM_FLAG_ALWAYS, NULL, &namesArray);
        if (SUCCEEDED(hr)) {
            LONG lLower, lUpper;
            SafeArrayGetLBound(namesArray, 1, &lLower);
            SafeArrayGetUBound(namesArray, 1, &lUpper);

            for (LONG i = lLower; i <= lUpper; ++i) {
                BSTR propName;
                SafeArrayGetElement(namesArray, &i, &propName);

                std::wstring val = GetProperty(pObj, propName);
                std::wcout << propName << L": " << val << L"\n";
                logFile << propName << L": " << val << L"\n";

                SysFreeString(propName);
            }

            SafeArrayDestroy(namesArray);
        }

        pObj->Release();
    }

    pEnumerator->Release();
}

void CDeviceInfo::RunQueryAndPrintExtended2(const std::wstring& className, const std::vector<std::wstring>& properties, std::wofstream& logFile)
{
    IEnumWbemClassObject* pEnumerator = ExecQuery(L"SELECT * FROM " + className);
    if (!pEnumerator) return;

    IWbemClassObject* pObj = nullptr;
    ULONG ret;
    while (pEnumerator->Next(WBEM_INFINITE, 1, &pObj, &ret) == S_OK) 
    {
        std::wcout << L"\n[" << className << L"]\n";
        logFile << L"\n[" << className << L"]\n";

        std::wcout << L"\n";
        std::wcout << L"\n";

        for (size_t i = 0; i < properties.size(); ++i) {
            const std::wstring& prop = properties[i];
            std::wstring val = GetProperty(pObj, const_cast<BSTR>(prop.c_str()));
            std::wcout << prop << L": " << val << L"\n";
            logFile << prop << L": " << val << L"\n";
        }

        std::wcout << L"\n";
        std::wcout << L"\n";

        SAFEARRAY* namesArray = nullptr;
        HRESULT hr = pObj->GetNames(NULL, WBEM_FLAG_ALWAYS, NULL, &namesArray);
        if (SUCCEEDED(hr)) {
            LONG lLower, lUpper;
            SafeArrayGetLBound(namesArray, 1, &lLower);
            SafeArrayGetUBound(namesArray, 1, &lUpper);

            for (LONG i = lLower; i <= lUpper; ++i) {
                BSTR propName;
                SafeArrayGetElement(namesArray, &i, &propName);

                std::wstring val = GetProperty(pObj, propName);
                std::wcout << propName << L": " << val << L"\n";
                logFile << propName << L": " << val << L"\n";

                SysFreeString(propName);
            }

            SafeArrayDestroy(namesArray);
        }

        std::wcout << L"\n";
        std::wcout << L"\n";

        pObj->Release();
    }

    pEnumerator->Release();
}


void CDeviceInfo::FillWmiClassesList(void)
{
    wmiClasses = {
        std::make_pair(L"Win32_Processor",       std::vector<std::wstring>{ L"Name", L"Architecture", L"NumberOfCores", L"NumberOfLogicalProcessors" }),
        std::make_pair(L"Win32_BIOS",            std::vector<std::wstring>{ L"Manufacturer", L"Version", L"ReleaseDate" }),
        std::make_pair(L"Win32_BaseBoard",       std::vector<std::wstring>{ L"Manufacturer", L"Product", L"SerialNumber" }),
        std::make_pair(L"Win32_LogicalDisk",     std::vector<std::wstring>{ L"DeviceID", L"Size", L"FreeSpace", L"FileSystem" }),
        std::make_pair(L"Win32_PhysicalMemory",  std::vector<std::wstring>{ L"Manufacturer", L"Capacity", L"Speed", L"MemoryType", L"FormFactor" }),
        std::make_pair(L"Win32_VideoController", std::vector<std::wstring>{ L"Description", L"AdapterRAM", L"DriverVersion" }),
        std::make_pair(L"Win32_OperatingSystem", std::vector<std::wstring>{ L"Caption", L"Version", L"OSArchitecture", L"InstallDate" }),
        std::make_pair(L"Win32_ComputerSystem",  std::vector<std::wstring>{ L"Manufacturer", L"Model", L"SystemType", L"UserName" }),
        std::make_pair(L"Win32_NetworkAdapter",  std::vector<std::wstring>{ L"Description", L"MACAddress", L"NetConnectionID" }),
        std::make_pair(L"Win32_Printer",         std::vector<std::wstring>{ L"Name", L"PortName", L"Default", L"WorkOffline" }),
        std::make_pair(L"Win32_SoundDevice",     std::vector<std::wstring>{ L"Name", L"Manufacturer", L"ProductName" }),
        std::make_pair(L"Win32_Keyboard",        std::vector<std::wstring>{ L"Description", L"Layout", L"NumberOfFunctionKeys" }),
        std::make_pair(L"Win32_PointingDevice",  std::vector<std::wstring>{ L"Description", L"NumberOfButtons" }),
        std::make_pair(L"Win32_Environment",     std::vector<std::wstring>{ L"Name", L"VariableValue", L"UserName" }),
        std::make_pair(L"Win32_PnPEntity",       std::vector<std::wstring>{ L"Name", L"DeviceID", L"Manufacturer", L"Service" }),
        std::make_pair(L"Win32_USBController",   std::vector<std::wstring>{ L"DeviceID", L"Name" }),
        std::make_pair(L"Win32_Service",         std::vector<std::wstring>{ L"Name", L"StartMode", L"State", L"Status" })
    };
}