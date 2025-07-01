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

int CDeviceInfo::RunExtended3_BunaGerekKalmadi(void) 
{
    std::wofstream logFile("device_output_extended3.txt");
    logFile.imbue(std::locale(""));

    std::vector<std::wstring> extendedClasses = {
        L"Win32_Processor", L"Win32_BIOS", L"Win32_BaseBoard",
        L"Win32_LogicalDisk", L"Win32_PhysicalMemory", L"Win32_VideoController",
        L"Win32_OperatingSystem", L"Win32_ComputerSystem", L"Win32_NetworkAdapter",
        L"Win32_Printer", L"Win32_SoundDevice", L"Win32_Keyboard",
        L"Win32_PointingDevice", L"Win32_Environment", L"Win32_PnPEntity",
        L"Win32_USBController", L"Win32_Service"
    };

    for (size_t i = 0; i < extendedClasses.size(); ++i) {
        const std::wstring& className = extendedClasses[i];
        std::wcout << L"\n===== " << className << L" =====\n";
        logFile << L"\n===== " << className << L" =====\n";

        IEnumWbemClassObject* pEnumerator = ExecQuery(L"SELECT * FROM " + className);
        if (!pEnumerator) continue;

        IWbemClassObject* pObj = nullptr;
        ULONG ret = 0;

        if (pEnumerator->Next(WBEM_INFINITE, 1, &pObj, &ret) == S_OK) {
            // İlk nesne geldi, şimdi tüm property adlarını al
            SAFEARRAY* namesArray = nullptr;
            HRESULT hr = pObj->GetNames(NULL, WBEM_FLAG_ALWAYS, NULL, &namesArray);

            if (SUCCEEDED(hr)) {
                LONG lBound = 0, uBound = 0;
                SafeArrayGetLBound(namesArray, 1, &lBound);
                SafeArrayGetUBound(namesArray, 1, &uBound);

                // Önce sadece property adlarını yaz (indentli)
                std::wcout << L"  [Properties]\n";
                logFile << L"  [Properties]\n";
                for (LONG i = lBound; i <= uBound; ++i) {
                    BSTR propName;
                    SafeArrayGetElement(namesArray, &i, &propName);
                    std::wcout << L"    - " << propName << L"\n";
                    logFile << L"    - " << propName << L"\n";
                    SysFreeString(propName);
                }

                std::wcout << L"  [Values]\n";
                logFile << L"  [Values]\n";

                // Şimdi tekrar dönüp ad + değer çiftlerini yaz
                for (LONG i = lBound; i <= uBound; ++i) {
                    BSTR propName;
                    SafeArrayGetElement(namesArray, &i, &propName);
                    std::wstring value = GetProperty(pObj, propName);
                    std::wcout << L"    " << propName << L": " << value << L"\n";
                    logFile << L"    " << propName << L": " << value << L"\n";
                    SysFreeString(propName);
                }

                SafeArrayDestroy(namesArray);
            }

            pObj->Release();
        }

        pEnumerator->Release();
    }

    logFile.close();

    return 1;
}

int CDeviceInfo::RunExtended3(void)
{
    std::wofstream logFile(L"device_output_extended3_1.txt");
    logFile.imbue(std::locale(""));

    for (size_t i = 0; i < wmiClasses.size(); ++i) {
        const std::wstring& className = wmiClasses[i].first;
        const std::vector<std::wstring>& properties = wmiClasses[i].second;
        RunQueryAndPrintExtended3(className, properties, logFile);
    }

    logFile.close();

    return 1;
}

int CDeviceInfo::RunExtended4(const std::vector<std::pair<std::wstring, std::vector<std::wstring>>>& wmiClasses) 
{
    std::wofstream logFile("device_output_extended4.txt");
    logFile.imbue(std::locale(""));

    for (size_t i = 0; i < wmiClasses.size(); ++i) {
        const std::wstring& className = wmiClasses[i].first;
        const std::vector<std::wstring>& properties = wmiClasses[i].second;

        // Property başlıkları
        std::wstring header = L"[" + className + L" Available Properties]";
        std::wcout << header << std::endl;
        logFile << header << std::endl;

        for (size_t j = 0; j < properties.size(); ++j) {
            std::wstring line = L"  - " + properties[j];
            std::wcout << line << std::endl;
            logFile << line << std::endl;
        }

        // Değerleri getir
        std::vector<std::map<std::wstring, std::wstring>> results = QueryMultiItem(className, properties);

        for (size_t k = 0; k < results.size(); ++k) {
            std::wstring instanceHeader = L"[" + className + L" Instance #" + std::to_wstring(k + 1) + L"]";
            std::wcout << instanceHeader << std::endl;
            logFile << instanceHeader << std::endl;

            for (std::map<std::wstring, std::wstring>::const_iterator it = results[k].begin(); it != results[k].end(); ++it) {
                std::wstring entry = it->first + L": " + it->second;
                std::wcout << entry << std::endl;
                logFile << entry << std::endl;
            }

            std::wcout << std::endl;
            logFile << std::endl;
        }
    }

    logFile.close();

    return 1;
}

int CDeviceInfo::RunExtended5(void) 
{
    std::wofstream logFile("device_output_extended5.txt");
    logFile.imbue(std::locale(""));

    std::vector<std::wstring> extendedClasses = {
        L"Win32_Processor", L"Win32_BIOS", L"Win32_BaseBoard",
        L"Win32_LogicalDisk", L"Win32_PhysicalMemory", L"Win32_VideoController",
        L"Win32_OperatingSystem", L"Win32_ComputerSystem", L"Win32_NetworkAdapter",
        L"Win32_Printer", L"Win32_SoundDevice", L"Win32_Keyboard",
        L"Win32_PointingDevice", L"Win32_Environment", L"Win32_PnPEntity",
        L"Win32_USBController", L"Win32_Service"
    };

    std::wstring wql = L"";

    for (size_t c = 0; c < extendedClasses.size(); ++c) {
        const std::wstring& className = extendedClasses[c];

        IEnumWbemClassObject* pEnumerator = NULL;
        wql = (L"SELECT * FROM ") + className;
        HRESULT hr = pSvc->ExecQuery(
            SysAllocString(L"WQL"),
            BSTR(wql.c_str()),//(L"SELECT * FROM " + className).c_str(),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            NULL,
            &pEnumerator
        );

        if (FAILED(hr) || !pEnumerator) {
            std::wcerr << L"Query failed for class: " << className << std::endl;
            logFile << L"Query failed for class: " << className << std::endl;
            continue;
        }

        // İlk instance'ı çekip property isimlerini al
        IWbemClassObject* pObj = NULL;
        ULONG returned = 0;
        std::vector<std::wstring> properties;

        if (pEnumerator->Next(WBEM_INFINITE, 1, &pObj, &returned) == S_OK) {
            SAFEARRAY* pNames = NULL;
            hr = pObj->GetNames(NULL, WBEM_FLAG_ALWAYS | WBEM_MASK_CONDITION_ORIGIN, NULL, &pNames);
            if (SUCCEEDED(hr) && pNames) {
                LONG lBound = 0, uBound = 0;
                SafeArrayGetLBound(pNames, 1, &lBound);
                SafeArrayGetUBound(pNames, 1, &uBound);
                for (LONG i = lBound; i <= uBound; ++i) {
                    BSTR name;
                    SafeArrayGetElement(pNames, &i, &name);
                    properties.push_back(name);
                    SysFreeString(name);
                }
                SafeArrayDestroy(pNames);
            }
            pObj->Release();
        }
        pEnumerator->Release();

        // Property listesi
        logFile << L"\n[" << className << L" Available Properties]\n";
        std::wcout << L"\n[" << className << L" Available Properties]\n";
        for (size_t i = 0; i < properties.size(); ++i) {
            logFile << L"  - " << properties[i] << std::endl;
            std::wcout << L"  - " << properties[i] << std::endl;
        }

        // Instance değerlerini tekrar çek
        wql = (L"SELECT * FROM ") + className;
        hr = pSvc->ExecQuery(
            SysAllocString(L"WQL"),
            BSTR(wql.c_str()),//(L"SELECT * FROM " + className).c_str(),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            NULL,
            &pEnumerator
        );

        if (FAILED(hr) || !pEnumerator) {
            continue;
        }

        int instanceCount = 0;
        while (pEnumerator->Next(WBEM_INFINITE, 1, &pObj, &returned) == S_OK) {
            ++instanceCount;
            logFile << L"\n[" << className << L" Instance #" << instanceCount << L"]\n";
            std::wcout << L"\n[" << className << L" Instance #" << instanceCount << L"]\n";

            for (size_t i = 0; i < properties.size(); ++i) {
                const std::wstring& prop = properties[i];
                VARIANT vtProp;
                VariantInit(&vtProp);
                HRESULT hRes = pObj->Get(prop.c_str(), 0, &vtProp, NULL, NULL);
                if (SUCCEEDED(hRes)) {
                    std::wstring val = VariantToWString(vtProp);
                    logFile << L"  " << prop << L": " << val << std::endl;
                    std::wcout << L"  " << prop << L": " << val << std::endl;
                }
                VariantClear(&vtProp);
            }
            pObj->Release();
        }

        if (pEnumerator)
            pEnumerator->Release();
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

void CDeviceInfo::Release(IWbemClassObject* pObj)
{
    pObj->Release();
}

void CDeviceInfo::Release(IEnumWbemClassObject* pObj)
{
    pObj->Release();
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

void CDeviceInfo::RunQueryAndPrintExtended3(const std::wstring& className, const std::vector<std::wstring>& properties, std::wofstream& logFile)
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

            // Önce sadece property adlarını yaz (indentli)
            std::wcout << L"  [Properties]\n";
            logFile << L"  [Properties]\n";

            for (LONG i = lLower; i <= lUpper; ++i) {
                BSTR propName;
                SafeArrayGetElement(namesArray, &i, &propName);
                std::wcout << L"    - " << propName << L"\n";
                logFile << L"    - " << propName << L"\n";
                SysFreeString(propName);
            }

            std::wcout << L"  [Values]\n";
            logFile << L"  [Values]\n";

            // Şimdi tekrar dönüp ad + değer çiftlerini yaz
            for (LONG i = lLower; i <= lUpper; ++i) {
                BSTR propName;
                SafeArrayGetElement(namesArray, &i, &propName);
                std::wstring value = GetProperty(pObj, propName);
                std::wcout << L"    " << propName << L": " << value << L"\n";
                logFile << L"    " << propName << L": " << value << L"\n";
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

std::map<std::wstring, std::wstring> CDeviceInfo::QuerySingleItem(const std::wstring& className, const std::vector<std::wstring>& properties)
{
    std::map<std::wstring, std::wstring> result;

    IEnumWbemClassObject* pEnumerator = ExecQuery(L"SELECT * FROM " + className);
    if (!pEnumerator) return result;

    IWbemClassObject* pObj = nullptr;
    ULONG ret = 0;
    if (pEnumerator->Next(WBEM_INFINITE, 1, &pObj, &ret) == S_OK) {
        for (size_t i = 0; i < properties.size(); ++i) {
            const std::wstring& prop = properties[i];
            std::wstring val = GetProperty(pObj, const_cast<BSTR>(prop.c_str()));
            result[prop] = val;
        }
        pObj->Release();
    }

    pEnumerator->Release();

    return result;
}

std::vector<std::map<std::wstring, std::wstring>> CDeviceInfo::QueryMultiItem(const std::wstring& className, const std::vector<std::wstring>& properties)
{
    std::vector<std::map<std::wstring, std::wstring>> resultList;

    IEnumWbemClassObject* pEnumerator = ExecQuery(L"SELECT * FROM " + className);
    if (!pEnumerator) return resultList;

    IWbemClassObject* pObj = nullptr;
    ULONG ret = 0;

    while (pEnumerator->Next(WBEM_INFINITE, 1, &pObj, &ret) == S_OK) {
        std::map<std::wstring, std::wstring> item;

        for (size_t i = 0; i < properties.size(); ++i) {
            const std::wstring& prop = properties[i];
            std::wstring val = GetProperty(pObj, const_cast<BSTR>(prop.c_str()));
            item[prop] = val;
        }

        resultList.push_back(item);
        pObj->Release();
    }

    pEnumerator->Release();
    return resultList;
}


std::vector<std::map<std::wstring, std::wstring>> CDeviceInfo::QueryLogicalToPhysicalDisks() 
{
    std::vector<std::map<std::wstring, std::wstring>> result;

    // 1. Win32_DiskDrive: get DeviceID → \\.\PHYSICALDRIVE0
    std::map<std::wstring, std::pair<std::wstring, std::wstring>> driveMap; // DeviceID -> {Model, Serial}
    {
        IEnumWbemClassObject* pEnum = ExecQuery(L"SELECT * FROM Win32_DiskDrive");
        IWbemClassObject* obj = nullptr;
        ULONG ret = 0;

        while (pEnum && pEnum->Next(WBEM_INFINITE, 1, &obj, &ret) == S_OK) {
            std::wstring dev = GetProperty(obj, L"DeviceID");
            std::wstring model = GetProperty(obj, L"Model");
            std::wstring serial = GetProperty(obj, L"SerialNumber");
            driveMap[dev] = std::make_pair(model, serial);
            obj->Release();
        }

        if (pEnum) pEnum->Release();
    }

    // 2. Win32_DiskDriveToDiskPartition: Antecedent → Dependent
    std::map<std::wstring, std::wstring> partitionMap; // Partition → PhysicalDeviceID
    {
        IEnumWbemClassObject* pEnum = ExecQuery(L"SELECT * FROM Win32_DiskDriveToDiskPartition");
        IWbemClassObject* obj = nullptr;
        ULONG ret = 0;

        while (pEnum && pEnum->Next(WBEM_INFINITE, 1, &obj, &ret) == S_OK) {
            std::wstring antecedent = GetProperty(obj, L"Antecedent"); // Disk
            std::wstring dependent = GetProperty(obj, L"Dependent");   // Partition
            size_t start = antecedent.find(L"\"") + 1;
            size_t end = antecedent.find_last_of(L"\"");
            std::wstring deviceID = antecedent.substr(start, end - start);

            start = dependent.find(L"\"") + 1;
            end = dependent.find_last_of(L"\"");
            std::wstring partitionID = dependent.substr(start, end - start);

            partitionMap[partitionID] = deviceID;
            obj->Release();
        }

        if (pEnum) pEnum->Release();
    }

    // 3. Win32_LogicalDiskToPartition: Antecedent → Dependent
    std::map<std::wstring, std::wstring> logicalMap; // LogicalDisk → PartitionID
    {
        IEnumWbemClassObject* pEnum = ExecQuery(L"SELECT * FROM Win32_LogicalDiskToPartition");
        IWbemClassObject* obj = nullptr;
        ULONG ret = 0;

        while (pEnum && pEnum->Next(WBEM_INFINITE, 1, &obj, &ret) == S_OK) {
            std::wstring antecedent = GetProperty(obj, L"Antecedent"); // Partition
            std::wstring dependent = GetProperty(obj, L"Dependent");   // LogicalDisk
            size_t start = dependent.find(L"\"") + 1;
            size_t end = dependent.find_last_of(L"\"");
            std::wstring logicalID = dependent.substr(start, end - start);

            start = antecedent.find(L"\"") + 1;
            end = antecedent.find_last_of(L"\"");
            std::wstring partitionID = antecedent.substr(start, end - start);

            logicalMap[logicalID] = partitionID;
            obj->Release();
        }

        if (pEnum) pEnum->Release();
    }

    // 4. Birleştir: logical → partition → physical
    for (std::map<std::wstring, std::wstring>::const_iterator itLM = logicalMap.begin(); itLM != logicalMap.end(); ++itLM) {
        const std::wstring& logical = itLM->first;
        const std::wstring& partition = itLM->second;

        std::map<std::wstring, std::wstring>::const_iterator itPM = partitionMap.find(partition);
        if (itPM != partitionMap.end()) {
            const std::wstring& deviceID = itPM->second;
            std::map<std::wstring, std::pair<std::wstring, std::wstring> >::const_iterator itDrive = driveMap.find(deviceID);

            if (itDrive != driveMap.end()) {
                const std::pair<std::wstring, std::wstring>& driveInfo = itDrive->second;

                std::map<std::wstring, std::wstring> diskItem;
                diskItem[L"LogicalDrive"] = logical;
                diskItem[L"Model"] = driveInfo.first;
                diskItem[L"SerialNumber"] = driveInfo.second;
                diskItem[L"DeviceID"] = deviceID;

                result.push_back(diskItem);
            }
        }
    }

    return result;
}

std::wstring CDeviceInfo::GetProperty(IWbemClassObject* pObj, const wchar_t* propertyName) 
{
    BSTR bstrProp = SysAllocString(propertyName);
    std::wstring result = GetProperty(pObj, bstrProp);
    SysFreeString(bstrProp);
    return result;
}



std::wstring CDeviceInfo::VariantToWString(const VARIANT& vt) {
    switch (vt.vt) {
    case VT_BSTR:
        return vt.bstrVal ? std::wstring(vt.bstrVal) : L"";
    case VT_I4:
        return std::to_wstring(vt.intVal);
    case VT_UI4:
        return std::to_wstring(vt.uintVal);
    case VT_BOOL:
        return vt.boolVal ? L"True" : L"False";
    case VT_NULL:
    case VT_EMPTY:
        return L"";
    default:
        return L"[Unsupported Type]";
    }
}