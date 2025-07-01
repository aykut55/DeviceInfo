#include "DeviceInfo.h"

int runExample1(std::auto_ptr<CDeviceInfo> pDeviceInfo)
{
    pDeviceInfo->Run();             // Standart çalıştırma
    pDeviceInfo->RunExtended();     // Genişletilmiş çalıştırma (tüm property'leri getirir)
    pDeviceInfo->RunExtended2();    // Genişletilmiş çalıştırma (tüm property'leri getirir) 
    return 1;
}

int runExample2(std::auto_ptr<CDeviceInfo> pDeviceInfo)
{
    auto bios = pDeviceInfo->QuerySingleItem(L"Win32_BIOS", {
        L"Manufacturer", L"Version", L"ReleaseDate"
        });

    std::wcout << L"\n[BIOS Bilgileri - Tek Nesne]\n";
    std::wcout << L"Üretici: " << bios[L"Manufacturer"] << L"\n";
    std::wcout << L"Sürüm: " << bios[L"Version"] << L"\n";
    std::wcout << L"Tarih: " << bios[L"ReleaseDate"] << L"\n";

    std::wcout << L"\n";
    std::wcout << L"\n";
    std::wcout << L"\n";
    std::wcout << L"\n";

    auto disks = pDeviceInfo->QueryMultiItem(L"Win32_LogicalDisk", {
        L"DeviceID", L"FileSystem", L"Size", L"FreeSpace"
        });

    std::wcout << L"\n[Disk Bilgileri - Tüm Diskler]\n";
    for (size_t i = 0; i < disks.size(); ++i) {
        std::wcout << L"Disk #" << i + 1 << L"\n";
        std::wcout << L"  ID: " << disks[i][L"DeviceID"] << L"\n";
        std::wcout << L"  FS: " << disks[i][L"FileSystem"] << L"\n";
        std::wcout << L"  Size: " << disks[i][L"Size"] << L"\n";
        std::wcout << L"  Free: " << disks[i][L"FreeSpace"] << L"\n";
    }


    std::wcout << L"\n";
    std::wcout << L"\n";
    std::wcout << L"\n";
    std::wcout << L"\n";

    auto physicalDisks = pDeviceInfo->QueryMultiItem(L"Win32_DiskDrive", {
    L"Model", L"SerialNumber", L"Size", L"InterfaceType"
        });

    for (const auto& disk : physicalDisks) {
        std::wcout << L"\nDisk:\n";
        std::wcout << L"  Model: " << disk.at(L"Model") << L"\n";
        std::wcout << L"  Serial: " << disk.at(L"SerialNumber") << L"\n";
        std::wcout << L"  Size: " << disk.at(L"Size") << L"\n";
        std::wcout << L"  Interface: " << disk.at(L"InterfaceType") << L"\n";
    }

    return 1;
}

int runExample3(std::auto_ptr<CDeviceInfo> pDeviceInfo)
{
    pDeviceInfo->RunExtended3_BunaGerekKalmadi();

    pDeviceInfo->RunExtended3();

    return 1;
}

int runExample_QueryLogicalToPhysicalDisks(std::auto_ptr<CDeviceInfo> pDeviceInfo)
{
    auto diskMap = pDeviceInfo->QueryLogicalToPhysicalDisks();

    std::wcout << L"\n[Volume -> PhysicalDisk eslesmesi]\n";
    for (const auto& disk : diskMap) {
        std::wcout << L"Drive: " << disk.at(L"LogicalDrive") << L"\n";
        std::wcout << L"  Model: " << disk.at(L"Model") << L"\n";
        std::wcout << L"  Serial: " << disk.at(L"SerialNumber") << L"\n";
        std::wcout << L"  Device: " << disk.at(L"DeviceID") << L"\n";
    }

    return 1;
}

int runExample4(std::auto_ptr<CDeviceInfo> pDeviceInfo)
{
    // Parametreli çağrım    
    // Kullanıcı tanımlı WMI sınıfları ve özellikler
    std::vector<std::pair<std::wstring, std::vector<std::wstring>>> selectedWMI = {
        std::make_pair(L"Win32_Processor", std::vector<std::wstring>{ L"Name", L"Architecture", L"NumberOfCores", L"NumberOfLogicalProcessors" }),
        std::make_pair(L"Win32_PhysicalMemory", std::vector<std::wstring>{ L"Manufacturer", L"Capacity", L"Speed", L"MemoryType", L"FormFactor" }),
        std::make_pair(L"Win32_BIOS", std::vector<std::wstring>{ L"Manufacturer", L"Version", L"ReleaseDate" })
    };

    pDeviceInfo->RunExtended4(selectedWMI);  

    return 1;
}

int runExample5(std::auto_ptr<CDeviceInfo> pDeviceInfo)
{
    pDeviceInfo->RunExtended5();

    return 1;
}

int runExample6(std::auto_ptr<CDeviceInfo> pDeviceInfo)
{
    // Parametreli çağrım    
    // Kullanıcı tanımlı WMI sınıfları ve özellikler
    std::vector<std::pair<std::wstring, std::vector<std::wstring>>> selectedWMI = {
        std::make_pair(L"Win32_Processor", std::vector<std::wstring>{ L"Name", L"Architecture", L"NumberOfCores", L"NumberOfLogicalProcessors" }),
        std::make_pair(L"Win32_PhysicalMemory", std::vector<std::wstring>{ L"Manufacturer", L"Capacity", L"Speed", L"MemoryType", L"FormFactor" }),
        std::make_pair(L"Win32_BIOS", std::vector<std::wstring>{ L"Manufacturer", L"Version", L"ReleaseDate" })
    };

    pDeviceInfo->RunExtended6(selectedWMI);

    return 1;
}

int runExample7(std::auto_ptr<CDeviceInfo> pDeviceInfo)
{
    pDeviceInfo->RunExtended7();

    return 1;
}

int runExample8(std::auto_ptr<CDeviceInfo> pDeviceInfo)
{
    pDeviceInfo->RunExtended8();

    return 1;
}

int runExample9(std::auto_ptr<CDeviceInfo> pDeviceInfo)
{
    pDeviceInfo->Run9();

    return 1;
}

int runExample10(std::auto_ptr<CDeviceInfo> pDeviceInfo)
{
    std::wofstream logFile(L"device_output.txt");
    logFile.imbue(std::locale(""));

    // ▶️ Veriyi al ve hem ekrana hem log dosyasına yaz
    std::vector<std::map<std::wstring, std::wstring>> results =
        pDeviceInfo->RunQueryAndPrint2(
            L"Win32_BIOS",
            { L"Manufacturer", L"Version", L"ReleaseDate" },
            logFile
        );

    // ▶️ Sonradan işlemek için dönen veriyi de kullan
    std::wcout << L"\n--- Programatik kullanım ---\n";
    for (const auto& item : results) {
        for (const auto& pair : item) {
            std::wcout << pair.first << L" = " << pair.second << std::endl;
        }
        std::wcout << L"--------------------------\n";
    }


    return 1;
}

int runExample11(std::auto_ptr<CDeviceInfo> pDeviceInfo)
{
    std::wofstream logFile2(L"allprops_bios.txt");
    logFile2.imbue(std::locale(""));

    auto result = pDeviceInfo->RunQueryAllProperties(L"Win32_BIOS", logFile2);

    // İstersen result içeriği üzerinde programatik işlem de yapabilirsin
    for (const auto& instance : result) {
        auto it = instance.find(L"Manufacturer");
        if (it != instance.end()) {
            if (it->second == L"Dell Inc.") {
                std::wcout << L"Dell BIOS bulundu.\n";
            }
        }
    }

    //veya
    std::wcout << L"Tüm BIOS Versiyonları:\n";
    for (const auto& instance : result) {
        auto it = instance.find(L"Version");
        if (it != instance.end()) {
            std::wcout << L" - " << it->second << std::endl;
        }
    }

    //veya
    if (!result.empty()) {
        const auto& firstInstance = result.front();
        for (const auto& kv : firstInstance) {
            std::wcout << kv.first << L" = " << kv.second << std::endl;
        }
    }

    // veya 
    std::wcout << L"[\n";
    for (size_t i = 0; i < result.size(); ++i) {
        std::wcout << L"  {\n";
        const auto& instance = result[i];
        size_t count = 0;
        for (const auto& kv : instance) {
            std::wcout << L"    \"" << kv.first << L"\": \"" << kv.second << L"\"";
            if (++count < instance.size()) std::wcout << L",";
            std::wcout << L"\n";
        }
        std::wcout << L"  }";
        if (i + 1 < result.size()) std::wcout << L",";
        std::wcout << L"\n";
    }
    std::wcout << L"]\n";

    logFile2.close();

    return 1;
}

int runExample12(std::auto_ptr<CDeviceInfo> pDeviceInfo)
{
    // Yeni fonksiyon: Görsel sunum
    pDeviceInfo->RunVisualLayout();

    return 1;
}

int runExample13(std::auto_ptr<CDeviceInfo> pDeviceInfo)
{
    auto registryInfo = pDeviceInfo->ReadSystemInformationRegistry();
    for (const auto& kv : registryInfo) {
        std::wcout << kv.first << L": " << kv.second << std::endl;
    }

    return 1;
}

int runExample14(std::auto_ptr<CDeviceInfo> pDeviceInfo)
{
    // Sadece sabit donanım bilgilerine dayalı fingerprint
    std::wstring fp1 = pDeviceInfo->HardwareFingerprint2(false, false);

    // Donanım + OS bilgileri
    std::wstring fp2 = pDeviceInfo->HardwareFingerprint2(true, false);

    // Donanım + OS + Registry bilgileri
    std::wstring fp3 = pDeviceInfo->HardwareFingerprint2(true, true);

    std::wcout << L"Fingerprint 1 (donanim)           : " << fp1 << std::endl;
    std::wcout << L"Fingerprint 2 (donanim + OS)      : " << fp2 << std::endl;
    std::wcout << L"Fingerprint 3 (donanim + OS + reg): " << fp3 << std::endl;

    return 1;
}

int runExample15(std::auto_ptr<CDeviceInfo> pDeviceInfo)
{
    std::wstring hwId1 = pDeviceInfo->GenerateHardwareId(false);  // Sadece donanım
    std::wstring hwId2 = pDeviceInfo->GenerateHardwareId(true);   // Donanım + OS

    std::wcout << L"Hardware ID (persistent): " << hwId1 << std::endl;
    std::wcout << L"Hardware ID (with OS):   " << hwId2 << std::endl;

    return 1;
}

int main()
{
    int fncResult = -1;

	std::cout << "Hello World!" << std::endl;

    auto pConsoleLogger = std::make_shared<ConsoleLogger>();
    auto pFileLogger = std::make_shared<FileLogger>("DeviceInfoLog.txt");
    auto pLogger = std::make_shared<CompositeLogger>();
    pLogger->AddLogger(pConsoleLogger);
    pLogger->AddLogger(pFileLogger);

    std::auto_ptr<CDeviceInfo> pDeviceInfo(new CDeviceInfo(pLogger));

    pDeviceInfo->Reset();

    pDeviceInfo->Init();

    //runExample1(pDeviceInfo);

    //runExample2(pDeviceInfo);

    //runExample3(pDeviceInfo);

    //runExample_QueryLogicalToPhysicalDisks(pDeviceInfo);
    
    //runExample4(pDeviceInfo);

    //runExample5(pDeviceInfo);

    //runExample6(pDeviceInfo);

    //runExample7(pDeviceInfo);

    //runExample8(pDeviceInfo);

    //runExample9(pDeviceInfo);

    //runExample10(pDeviceInfo);

    //runExample11(pDeviceInfo);

    //runExample12(pDeviceInfo);

    //runExample13(pDeviceInfo);

    runExample14(pDeviceInfo);  // lisanslama için kullanılabilecek metod

    //runExample15(pDeviceInfo);

    //pDeviceInfo->Cleanup();

    return 1;
}