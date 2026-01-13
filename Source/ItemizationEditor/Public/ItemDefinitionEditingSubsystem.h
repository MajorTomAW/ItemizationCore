// Author: Tom Werner (MajorT), 2025 November

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "ItemDefinitionEditingSubsystem.generated.h"

class UItemDefinitionBase;
class FItemDefinitionViewModel;

UCLASS()
class ITEMIZATIONEDITOR_API UItemDefinitionEditingSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	UItemDefinitionEditingSubsystem();
	static UItemDefinitionEditingSubsystem* Get();
	TSharedRef<FItemDefinitionViewModel> FindOrAddViewModel(TNotNull<UItemDefinitionBase*> InItemDefinition);

	virtual void BeginDestroy() override;

private:
	void HandlePostGarbageCollect();

protected:
	TMap<FObjectKey, TSharedPtr<FItemDefinitionViewModel>> ItemViewModels;
	FDelegateHandle PostGarbageCollectHandle;
};
