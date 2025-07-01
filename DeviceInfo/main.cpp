#include "DeviceInfo.h"

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

    pDeviceInfo->Init();

    pDeviceInfo->Run();
    pDeviceInfo->RunExtended();
    pDeviceInfo->RunExtended2();

    pDeviceInfo->Cleanup();
}