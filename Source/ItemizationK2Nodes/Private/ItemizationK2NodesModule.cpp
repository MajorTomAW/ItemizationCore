#include "ItemComponentDataPinFactory.h"
#include "Modules/ModuleManager.h"

class FItemizationK2NodesModule : public IModuleInterface
{
public:
	//~ Begin IModuleInterface Interface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//~ End IModuleInterface Interface

private:
	TSharedPtr<FItemComponentDataPinFactory> MyPinFactory;
};
IMPLEMENT_MODULE(FItemizationK2NodesModule, ItemizationK2Nodes)


void FItemizationK2NodesModule::StartupModule()
{
	MyPinFactory = MakeShared<FItemComponentDataPinFactory>();
	FEdGraphUtilities::RegisterVisualPinFactory(MyPinFactory);
}

void FItemizationK2NodesModule::ShutdownModule()
{
	FEdGraphUtilities::UnregisterVisualPinFactory(MyPinFactory);
}
