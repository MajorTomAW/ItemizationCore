// Author: Tom Werner (MajorT), 2025

#include "Items/Data/ItemComponentData_Icon.h"

FItemComponentData_Icon::FItemComponentData_Icon()
{
}


#if WITH_EDITOR
FText FItemComponentData_Icon::GetDescription() const
{
	if (Icon.IsNull())
	{
		return FText::FromString(TEXT("None"));
	}

	return FText::Format(INVTEXT("Icon: {0}"), FText::FromString(Icon.ToSoftObjectPath().ToString()));
}
#endif
