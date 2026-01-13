// Author: Tom Werner (MajorT), 2025 November


#include "ItemDefinitionEditingSubsystem.h"

#include "Items/ItemDefinitionBase.h"
#include "Toolkits/ItemDefinitionViewModel.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(ItemDefinitionEditingSubsystem)

UItemDefinitionEditingSubsystem::UItemDefinitionEditingSubsystem()
{
	PostGarbageCollectHandle = FCoreUObjectDelegates::GetPostGarbageCollect().AddUObject(this, &ThisClass::HandlePostGarbageCollect);
}

UItemDefinitionEditingSubsystem* UItemDefinitionEditingSubsystem::Get()
{
	return GEditor->GetEditorSubsystem<UItemDefinitionEditingSubsystem>();
}

TSharedRef<FItemDefinitionViewModel> UItemDefinitionEditingSubsystem::FindOrAddViewModel(
	TNotNull<UItemDefinitionBase*> InItemDefinition)
{
	const FObjectKey Key(InItemDefinition);
	TSharedPtr<FItemDefinitionViewModel> ViewModelPtr = ItemViewModels.FindRef(Key);
	if (ViewModelPtr)
	{
		// The item definition could be re-instantiated.
		// Can occur when the object is destroyed and recreated in a pool or when reloaded in the editor.
		// The object might have the same pointer value or the same path,
		// but it's a new object and all weak references are invalid
		if (ViewModelPtr->GetItemDefinition() == InItemDefinition)
		{
			return ViewModelPtr.ToSharedRef();
		}

		ItemViewModels.Remove(Key);
		ViewModelPtr = nullptr;
	}

	TSharedRef<FItemDefinitionViewModel> NewViewModel = ItemViewModels.Add(Key, MakeShared<FItemDefinitionViewModel>()).ToSharedRef();
	NewViewModel->Init(InItemDefinition);
	return NewViewModel;
}

void UItemDefinitionEditingSubsystem::BeginDestroy()
{
	FCoreUObjectDelegates::GetPostGarbageCollect().Remove(PostGarbageCollectHandle);
	Super::BeginDestroy();
}

void UItemDefinitionEditingSubsystem::HandlePostGarbageCollect()
{
	for (TMap<FObjectKey, TSharedPtr<FItemDefinitionViewModel>>::TIterator It(ItemViewModels); It; ++It)
	{
		if (!It.Key().ResolveObjectPtr())
		{
			It.RemoveCurrent();
		}
		else if (!It.Value() || !It.Value()->GetItemDefinition())
		{
			It.RemoveCurrent();
		}
	}
}
