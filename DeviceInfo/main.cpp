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

    std::wcout << L"\n[Volume → PhysicalDisk eşleşmesi]\n";
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

    //runExample_QueryLogicalToPhysicalDisks(pDeviceInfo);   // Bu calismiyor
    
    //runExample4(pDeviceInfo);

    //runExample5(pDeviceInfo);

    pDeviceInfo->Cleanup();

    return 1;
}