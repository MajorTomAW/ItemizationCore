// Author: Tom Werner (MajorT), 2025

#pragma once

#include "ItemComponentData.h"

#include "ItemComponentData_Icon.generated.h"

/** Item data for adding an icon for the item to display in the UI. */
USTRUCT(DisplayName="UI Icon Item Data")
struct FItemComponentData_Icon : public FItemComponentData
{
	GENERATED_BODY()

public:
	FItemComponentData_Icon();

	/** Icon texture to display in the UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Icon)
	TSoftObjectPtr<UTexture2D> Icon;

protected:
	//~ Begin FItemComponentData Interface
#if WITH_EDITOR
	virtual FText GetDescription() const override;
#endif
	//~ End FItemComponentData Interface
};
