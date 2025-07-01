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
        RunQueryAndPrint1(className, properties, logFile);
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

void CDeviceInfo::RunQueryAndPrint1(const std::wstring& className, const std::vector<std::wstring>& properties, std::wofstream& logFile) 
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

std::vector<std::map<std::wstring, std::wstring>> CDeviceInfo::RunQueryAndPrint2(const std::wstring& className, const std::vector<std::wstring>& properties, std::wofstream& logFile)
{
    std::vector<std::map<std::wstring, std::wstring>> results;

    IEnumWbemClassObject* pEnumerator = nullptr;
    std::wstring query = L"SELECT * FROM " + className;
    HRESULT hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t(query.c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &pEnumerator
    );

    if (FAILED(hres) || !pEnumerator) {
        std::wcerr << L"Query failed for: " << className << std::endl;
        return results;
    }

    logFile << L"\n[" << className << L"]" << std::endl;
    std::wcout << L"\n[" << className << L"]" << std::endl;

    ULONG uReturn = 0;
    IWbemClassObject* pclsObj = nullptr;

    while (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK) {
        std::map<std::wstring, std::wstring> item;

        for (size_t i = 0; i < properties.size(); ++i) {
            const std::wstring& prop = properties[i];
            std::wstring val = GetProperty(pclsObj, prop.c_str());
            item[prop] = val;

            std::wcout << prop << L": " << val << std::endl;
            logFile << prop << L": " << val << std::endl;
        }

        std::wcout << std::endl;
        logFile << std::endl;

        results.push_back(item);
        pclsObj->Release();
    }

    pEnumerator->Release();
    return results;
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
            //driveMap[dev] = std::make_pair(model, serial);
            driveMap[NormalizeDeviceID(dev)] = std::make_pair(model, serial);
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
            const std::wstring normDeviceID = NormalizeDeviceID(deviceID);

            std::map<std::wstring, std::pair<std::wstring, std::wstring> >::const_iterator itDrive = driveMap.find(normDeviceID);

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

int CDeviceInfo::RunExtended6(const std::vector<std::pair<std::wstring, std::vector<std::wstring>>>& wmiClasses)
{
    std::wofstream logFile("device_output_extended6.txt");
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

int CDeviceInfo::RunExtended7() {
    std::wofstream logFile(L"device_output_extended7.txt");
    logFile.imbue(std::locale(""));

    const std::vector<std::pair<std::wstring, std::vector<std::wstring>>> classes = {
        { L"Win32_Processor",     { L"Name", L"Architecture", L"NumberOfCores", L"NumberOfLogicalProcessors" } },
        { L"Win32_BIOS",          { L"Manufacturer", L"Version", L"ReleaseDate" } },
        { L"Win32_LogicalDisk",   { L"DeviceID", L"Size", L"FreeSpace", L"FileSystem" } }
        // Daha fazla sınıf eklenebilir
    };

    for (size_t i = 0; i < classes.size(); ++i) {
        auto const& p = classes[i];
        auto results = RunQueryAndPrint2(p.first, p.second, logFile);

        std::wcout << L"\n[Programmatic - " << p.first << L"]\n";
        for (auto const& item : results) {
            for (auto const& kv : item) {
                std::wcout << kv.first << L" = " << kv.second << L"\n";
            }
            std::wcout << L"---------------------\n";
        }
    }

    logFile.close();

    return 1;
}

int CDeviceInfo::RunExtended8() {
    std::wofstream logFile(L"device_output_extended8.txt");
    logFile.imbue(std::locale(""));

    const std::vector<std::pair<std::wstring, std::vector<std::wstring>>> classes = {
        { L"Win32_Processor",     { L"Name", L"Architecture", L"NumberOfCores", L"NumberOfLogicalProcessors" } },
        { L"Win32_BIOS",          { L"Manufacturer", L"Version", L"ReleaseDate" } },
        { L"Win32_LogicalDisk",   { L"DeviceID", L"Size", L"FreeSpace", L"FileSystem" } }
    };

    for (size_t i = 0; i < classes.size(); ++i) {
        auto const& cls = classes[i];

        std::wcout << L"\n===== " << cls.first << L" =====\n";
        logFile << L"\n===== " << cls.first << L" =====\n";

        std::wcout << L"Available Properties:\n"; logFile << L"Available Properties:\n";
        for (auto const& prop : cls.second) {
            std::wcout << L"  - " << prop << L"\n";
            logFile << L"  - " << prop << L"\n";
        }

        auto results = RunQueryAndPrint2(cls.first, cls.second, logFile);

        int count = 0;
        for (auto const& item : results) {
            ++count;
            std::wstring hdr = L"[Instance #" + std::to_wstring(count) + L"]";
            std::wcout << hdr << L"\n"; logFile << hdr << L"\n";

            for (auto const& kv : item) {
                std::wcout << L"  " << kv.first << L": " << kv.second << L"\n";
                logFile << L"  " << kv.first << L": " << kv.second << L"\n";
            }
        }
    }

    logFile.close();

    return 1;
}

int CDeviceInfo::Run9() {
    std::wofstream logFile(L"device_output_9.txt");
    logFile.imbue(std::locale(""));

    const std::vector<std::pair<std::wstring, std::vector<std::wstring>>> wmiClasses = {
        std::make_pair(L"Win32_Processor", std::vector<std::wstring>{ L"Name", L"Architecture", L"NumberOfCores", L"NumberOfLogicalProcessors" }),
        std::make_pair(L"Win32_BIOS", std::vector<std::wstring>{ L"Manufacturer", L"Version", L"ReleaseDate" }),
        std::make_pair(L"Win32_BaseBoard", std::vector<std::wstring>{ L"Manufacturer", L"Product", L"SerialNumber" }),
        std::make_pair(L"Win32_LogicalDisk", std::vector<std::wstring>{ L"DeviceID", L"Size", L"FreeSpace", L"FileSystem" }),
        std::make_pair(L"Win32_PhysicalMemory", std::vector<std::wstring>{ L"Manufacturer", L"Capacity", L"Speed", L"MemoryType", L"FormFactor" }),
        std::make_pair(L"Win32_VideoController", std::vector<std::wstring>{ L"Description", L"AdapterRAM", L"DriverVersion" }),
        std::make_pair(L"Win32_OperatingSystem", std::vector<std::wstring>{ L"Caption", L"Version", L"OSArchitecture", L"InstallDate" }),
        std::make_pair(L"Win32_ComputerSystem", std::vector<std::wstring>{ L"Manufacturer", L"Model", L"SystemType", L"UserName" }),
        std::make_pair(L"Win32_NetworkAdapter", std::vector<std::wstring>{ L"Description", L"MACAddress", L"NetConnectionID" }),
        std::make_pair(L"Win32_Printer", std::vector<std::wstring>{ L"Name", L"PortName", L"Default", L"WorkOffline" }),
        std::make_pair(L"Win32_SoundDevice", std::vector<std::wstring>{ L"Name", L"Manufacturer", L"ProductName" }),
        std::make_pair(L"Win32_Keyboard", std::vector<std::wstring>{ L"Description", L"Layout", L"NumberOfFunctionKeys" }),
        std::make_pair(L"Win32_PointingDevice", std::vector<std::wstring>{ L"Description", L"NumberOfButtons" }),
        std::make_pair(L"Win32_Environment", std::vector<std::wstring>{ L"Name", L"VariableValue", L"UserName" }),
        std::make_pair(L"Win32_PnPEntity", std::vector<std::wstring>{ L"Name", L"DeviceID", L"Manufacturer", L"Service" }),
        std::make_pair(L"Win32_USBController", std::vector<std::wstring>{ L"DeviceID", L"Name" }),
        std::make_pair(L"Win32_Service", std::vector<std::wstring>{ L"Name", L"StartMode", L"State", L"Status" })
    };

    for (size_t i = 0; i < wmiClasses.size(); ++i) {
        const std::wstring& className = wmiClasses[i].first;
        const std::vector<std::wstring>& properties = wmiClasses[i].second;

        // Yeni hali: verileri al ve işle
        std::vector<std::map<std::wstring, std::wstring>> results = RunQueryAndPrint2(className, properties, logFile);

        // Örnek olarak ekrana döngüyle yazabiliriz (logFile zaten fonksiyonda kullanıldı)
        std::wcout << L"\n[Programmatic Access - " << className << L"]\n";
        for (const auto& item : results) {
            for (const auto& kv : item) {
                std::wcout << kv.first << L": " << kv.second << L"\n";
            }
            std::wcout << L"---\n";
        }
    }

    logFile.close();

    return 1;
}

std::vector<std::map<std::wstring, std::wstring>> CDeviceInfo::RunQueryAllProperties(
    const std::wstring& className,
    std::wofstream& logFile)
{
    std::vector<std::map<std::wstring, std::wstring>> results;
    IEnumWbemClassObject* pEnumerator = nullptr;

    std::wstring query = L"SELECT * FROM " + className;
    HRESULT hr = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t(query.c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &pEnumerator);

    if (FAILED(hr) || !pEnumerator) {
        std::wcerr << L"Query failed: " << className << std::endl;
        return results;
    }

    IWbemClassObject* pObj = nullptr;
    ULONG uReturn = 0;

    int instanceCount = 0;

    while (pEnumerator->Next(WBEM_INFINITE, 1, &pObj, &uReturn) == S_OK) {
        ++instanceCount;

        SAFEARRAY* namesArray = nullptr;
        hr = pObj->GetNames(nullptr, WBEM_FLAG_ALWAYS | WBEM_MASK_CONDITION_ORIGIN, nullptr, &namesArray);
        if (FAILED(hr) || !namesArray) {
            pObj->Release();
            continue;
        }

        long lBound = 0, uBound = 0;
        SafeArrayGetLBound(namesArray, 1, &lBound);
        SafeArrayGetUBound(namesArray, 1, &uBound);

        std::map<std::wstring, std::wstring> item;

        for (long i = lBound; i <= uBound; ++i) {
            BSTR name;
            SafeArrayGetElement(namesArray, &i, &name);
            std::wstring propName(name, SysStringLen(name));

            std::wstring value = GetProperty(pObj, name);
            item[propName] = value;

            SysFreeString(name);
        }

        SafeArrayDestroy(namesArray);
        pObj->Release();

        // Loglama
        std::wstring instanceHeader = L"[" + className + L" Instance #" + std::to_wstring(instanceCount) + L"]";
        std::wcout << instanceHeader << std::endl;
        logFile << instanceHeader << std::endl;

        for (auto const& kv : item) {
            std::wcout << L"  " << kv.first << L": " << kv.second << std::endl;
            logFile << L"  " << kv.first << L": " << kv.second << std::endl;
        }

        results.push_back(item);
    }

    pEnumerator->Release();
    return results;
}

void CDeviceInfo::RunVisualLayout() {
    std::wofstream logFile("visual_output.txt");
    logFile.imbue(std::locale(""));

    auto write = [&](const std::wstring& label, const std::wstring& value) {
        std::wcout << label << L": " << value << std::endl;
        logFile << label << L": " << value << std::endl;
        };

    auto writeHeader = [&](const std::wstring& header) {
        std::wcout << L"\n=== " << header << L" ===" << std::endl;
        logFile << L"\n=== " << header << L" ===" << std::endl;
        };

    // BIOS
    writeHeader(L"BIOS/UEFI");
    std::map<std::wstring, std::wstring> bios = QuerySingleItem(L"Win32_BIOS", { L"Manufacturer", L"Version" });
    write(L"Manufacturer", bios[L"Manufacturer"]);
    write(L"Version", bios[L"Version"]);

    // OS
    writeHeader(L"Operating System");
    std::map<std::wstring, std::wstring> os = QuerySingleItem(L"Win32_OperatingSystem", { L"Caption", L"Version", L"OSArchitecture" });
    write(L"Name", os[L"Caption"]);
    write(L"Version", os[L"Version"]);
    write(L"Architecture", os[L"OSArchitecture"]);

    // CPU
    writeHeader(L"CPU");
    std::map<std::wstring, std::wstring> cpu = QuerySingleItem(L"Win32_Processor", { L"Name", L"MaxClockSpeed" });
    write(L"Model", cpu[L"Name"]);
    write(L"Max Clock Speed (MHz)", cpu[L"MaxClockSpeed"]);

    // Motherboard
    writeHeader(L"Motherboard");
    std::map<std::wstring, std::wstring> board = QuerySingleItem(L"Win32_BaseBoard", { L"Manufacturer", L"Product" });
    write(L"Manufacturer", board[L"Manufacturer"]);
    write(L"Product", board[L"Product"]);

    // RAM
    writeHeader(L"RAM");
    std::vector<std::map<std::wstring, std::wstring>> ramList = QueryMultiItem(L"Win32_PhysicalMemory", { L"Capacity", L"Speed", L"DeviceLocator" });
    for (size_t i = 0; i < ramList.size(); ++i) {
        const std::map<std::wstring, std::wstring>& ram = ramList[i];
        std::wstring sizeGB = L"N/A";
        if (ram.count(L"Capacity") > 0) {
            sizeGB = std::to_wstring(_wtoi64(ram.at(L"Capacity").c_str()) / (1024 * 1024 * 1024)) + L" GB";
        }
        write(L"Slot " + ram.at(L"DeviceLocator"), sizeGB + L" @ " + ram.at(L"Speed") + L" MHz");
    }

    // GPU
    writeHeader(L"GPU");
    std::map<std::wstring, std::wstring> gpu = QuerySingleItem(L"Win32_VideoController", { L"Name", L"AdapterRAM" });
    std::wstring gpuMb = std::to_wstring(_wtoi64(gpu[L"AdapterRAM"].c_str()) / (1024 * 1024));
    write(L"Model", gpu[L"Name"]);
    write(L"Memory", gpuMb + L" MB");

    // Display
    writeHeader(L"Display");
    std::map<std::wstring, std::wstring> display = QuerySingleItem(L"Win32_DesktopMonitor", { L"Name", L"ScreenHeight", L"ScreenWidth" });
    write(L"Monitor", display[L"Name"]);
    write(L"Resolution", display[L"ScreenWidth"] + L"x" + display[L"ScreenHeight"]);

    // Disk
    writeHeader(L"Storage");
    std::vector<std::map<std::wstring, std::wstring>> drives = QueryMultiItem(L"Win32_DiskDrive", { L"Model", L"Size" });
    for (size_t i = 0; i < drives.size(); ++i) {
        const std::map<std::wstring, std::wstring>& d = drives[i];
        std::wstring size = std::to_wstring(_wtoi64(d.at(L"Size").c_str()) / (1024 * 1024 * 1024));
        write(d.at(L"Model"), size + L" GB");
    }

    // Optical
    writeHeader(L"Optical Drives");
    std::vector<std::map<std::wstring, std::wstring>> cds = QueryMultiItem(L"Win32_CDROMDrive", { L"Name" });
    for (size_t i = 0; i < cds.size(); ++i) {
        write(L"Drive", cds[i].at(L"Name"));
    }

    // Network
    writeHeader(L"Network");
    std::vector<std::map<std::wstring, std::wstring>> nets = QueryMultiItem(L"Win32_NetworkAdapterConfiguration", { L"Description", L"MACAddress" });
    for (size_t i = 0; i < nets.size(); ++i) {
        const std::map<std::wstring, std::wstring>& net = nets[i];
        write(net.at(L"Description"), net.count(L"MACAddress") ? net.at(L"MACAddress") : L"");
    }

    // Audio
    writeHeader(L"Sound");
    std::map<std::wstring, std::wstring> sound = QuerySingleItem(L"Win32_SoundDevice", { L"Name" });
    write(L"Audio Device", sound[L"Name"]);

    // Uptime
    writeHeader(L"Uptime");
    std::map<std::wstring, std::wstring> os2 = QuerySingleItem(L"Win32_OperatingSystem", { L"LastBootUpTime" });
    write(L"Last Boot", os2[L"LastBootUpTime"]);
    write(L"Last Boot", ConvertWmiDate(os2[L"LastBootUpTime"]));

    logFile.close();
}

std::wstring CDeviceInfo::ConvertWmiDate(const std::wstring& wmiDate) {
    if (wmiDate.length() < 14) return L"Unknown";

    SYSTEMTIME st = { 0 };
    st.wYear = std::stoi(wmiDate.substr(0, 4));
    st.wMonth = std::stoi(wmiDate.substr(4, 2));
    st.wDay = std::stoi(wmiDate.substr(6, 2));
    st.wHour = std::stoi(wmiDate.substr(8, 2));
    st.wMinute = std::stoi(wmiDate.substr(10, 2));
    st.wSecond = std::stoi(wmiDate.substr(12, 2));

    wchar_t buffer[100];
    swprintf(buffer, 100, L"%04d-%02d-%02d %02d:%02d:%02d",
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

    return std::wstring(buffer);
}

std::wstring CDeviceInfo::ComputeHash(const std::wstring& input) {
    HCRYPTPROV hProv = NULL;
    HCRYPTHASH hHash = NULL;
    BYTE hash[20]; // SHA1 = 160-bit
    DWORD hashLen = sizeof(hash);
    std::wstring result;

    if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        if (CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash)) {
            CryptHashData(hHash, reinterpret_cast<const BYTE*>(input.c_str()), static_cast<DWORD>(input.length() * sizeof(wchar_t)), 0);
            if (CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0)) {
                wchar_t buffer[3];
                for (DWORD i = 0; i < hashLen; ++i) {
                    swprintf(buffer, 3, L"%02X", hash[i]);
                    result += buffer;
                }
            }
            CryptDestroyHash(hHash);
        }
        CryptReleaseContext(hProv, 0);
    }

    return result;
}

std::wstring CDeviceInfo::GenerateHardwareId(bool useOsData) {
    std::wstring rawData;

    // Donanım bileşenlerini çek
    std::vector<std::map<std::wstring, std::wstring>> cpu = QueryMultiItem(L"Win32_Processor", { L"ProcessorId" });
    std::vector<std::map<std::wstring, std::wstring>> board = QueryMultiItem(L"Win32_BaseBoard", { L"SerialNumber" });
    std::vector<std::map<std::wstring, std::wstring>> bios = QueryMultiItem(L"Win32_BIOS", { L"SerialNumber" });
    std::vector<std::map<std::wstring, std::wstring>> ram = QueryMultiItem(L"Win32_PhysicalMemory", { L"SerialNumber" });
    std::vector<std::map<std::wstring, std::wstring>> disk = QueryMultiItem(L"Win32_DiskDrive", { L"SerialNumber" });

    // Temel donanım
    if (!cpu.empty())   rawData += cpu[0][L"ProcessorId"];
    if (!board.empty()) rawData += board[0][L"SerialNumber"];
    if (!bios.empty())  rawData += bios[0][L"SerialNumber"];
    if (!ram.empty())   rawData += ram[0][L"SerialNumber"];
    if (!disk.empty())  rawData += disk[0][L"SerialNumber"];

    // Eğer OS bilgileri de dahil edilecekse
    if (useOsData) {
        std::vector<std::map<std::wstring, std::wstring>> os = QueryMultiItem(L"Win32_OperatingSystem", { L"SerialNumber" });
        std::vector<std::map<std::wstring, std::wstring>> cs = QueryMultiItem(L"Win32_ComputerSystem", { L"UserName" });
        std::vector<std::map<std::wstring, std::wstring>> csp = QueryMultiItem(L"Win32_ComputerSystemProduct", { L"UUID" });

        if (!os.empty())  rawData += os[0][L"SerialNumber"];
        if (!cs.empty())  rawData += cs[0][L"UserName"];
        if (!csp.empty()) rawData += csp[0][L"UUID"];
    }

    return ComputeHash(rawData);
}


std::wstring CDeviceInfo::ReadMachineGuidFromRegistry() {
    HKEY hKey;
    wchar_t value[256];
    DWORD value_length = sizeof(value);
    const wchar_t* subkey = L"SOFTWARE\\Microsoft\\Cryptography";
    const wchar_t* valueName = L"MachineGuid";

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, subkey, 0, KEY_READ | KEY_WOW64_64KEY, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueExW(hKey, valueName, nullptr, nullptr, reinterpret_cast<LPBYTE>(&value), &value_length) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return std::wstring(value);
        }
        RegCloseKey(hKey);
    }
    return L"";
}

std::wstring CDeviceInfo::HardwareFingerprint(bool useOsData) {
    std::wstring fingerprint;

    // Ana bileşenler
    auto cpu = QuerySingleItem(L"Win32_Processor", { L"ProcessorId" });
    auto bios = QuerySingleItem(L"Win32_BIOS", { L"SerialNumber" });
    auto baseBoard = QuerySingleItem(L"Win32_BaseBoard", { L"SerialNumber" });
    auto disk = QuerySingleItem(L"Win32_DiskDrive", { L"SerialNumber" });

    fingerprint += cpu[L"ProcessorId"];
    fingerprint += bios[L"SerialNumber"];
    fingerprint += baseBoard[L"SerialNumber"];
    fingerprint += disk[L"SerialNumber"];

    // OS bilgileri dahil edilecekse
    if (useOsData) {
        auto os = QuerySingleItem(L"Win32_OperatingSystem", { L"SerialNumber", L"Version", L"InstallDate" });
        fingerprint += os[L"SerialNumber"];
        fingerprint += os[L"Version"];
        fingerprint += os[L"InstallDate"];

        fingerprint += ReadMachineGuidFromRegistry();  // Registry tabanlı MachineGuid
    }

    return fingerprint;
}

std::wstring CDeviceInfo::HardwareFingerprint2(bool useOsData, bool includeRegistryInfo) {
    std::wstring fingerprint;

    // Donanım bileşenlerinden alınan sabit bilgiler
    auto cpu = QuerySingleItem(L"Win32_Processor", { L"ProcessorId" });
    auto bios = QuerySingleItem(L"Win32_BIOS", { L"SerialNumber" });
    auto baseBoard = QuerySingleItem(L"Win32_BaseBoard", { L"SerialNumber" });
    auto disk = QuerySingleItem(L"Win32_DiskDrive", { L"SerialNumber" });

    fingerprint += cpu[L"ProcessorId"];
    fingerprint += bios[L"SerialNumber"];
    fingerprint += baseBoard[L"SerialNumber"];
    fingerprint += disk[L"SerialNumber"];

    if (useOsData) {
        auto os = QuerySingleItem(L"Win32_OperatingSystem", { L"SerialNumber", L"Version", L"InstallDate" });
        fingerprint += os[L"SerialNumber"];
        fingerprint += os[L"Version"];
        fingerprint += os[L"InstallDate"];

        fingerprint += ReadMachineGuidFromRegistry();
    }

    if (includeRegistryInfo) {
        fingerprint += L"\t";
        auto regInfo = ReadSystemInformationRegistry();
        for (const auto& kv : regInfo) {
            fingerprint += kv.second;
        }
    }

    return fingerprint;
}



std::map<std::wstring, std::wstring> CDeviceInfo::ReadSystemInformationRegistry() {
    std::map<std::wstring, std::wstring> infoMap;

    HKEY hKey;
    const wchar_t* subkey = L"SYSTEM\\CurrentControlSet\\Control\\SystemInformation";
    const std::vector<std::wstring> keys = {
        L"SystemManufacturer",
        L"SystemProductName",
        L"BIOSVersion",
        L"ComputerHardwareId"  // Bazı sistemlerde mevcut
    };

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, subkey, 0, KEY_READ | KEY_WOW64_64KEY, &hKey) == ERROR_SUCCESS) {
        for (const auto& name : keys) {
            wchar_t value[512];
            DWORD size = sizeof(value);
            DWORD type = 0;
            if (RegQueryValueExW(hKey, name.c_str(), nullptr, &type, reinterpret_cast<LPBYTE>(value), &size) == ERROR_SUCCESS && type == REG_SZ) {
                infoMap[name] = value;
            }
        }
        RegCloseKey(hKey);
    }

    return infoMap;
}

std::wstring CDeviceInfo::NormalizeDeviceID(const std::wstring& dev)
{
    std::wstring norm;
    bool last_was_backslash = false;

    for (size_t i = 0; i < dev.length(); ++i) {
        if (dev[i] == L'\\') {
            if (!last_was_backslash) {
                norm += L'\\';
                last_was_backslash = true;
            }
        }
        else {
            norm += dev[i];
            last_was_backslash = false;
        }
    }
    return norm;
}