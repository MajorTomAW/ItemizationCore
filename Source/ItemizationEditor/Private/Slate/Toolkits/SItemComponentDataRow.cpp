// Copyright © 2025 Playton. All Rights Reserved.


#include "SItemComponentDataRow.h"

#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SItemComponentDataRow::Construct(
	const FArguments& InArgs,
	const TSharedRef<STableViewBase>& InOwnerTableView,
	TSharedPtr<FItemComponentInstanceListEntry> InInstance,
	const TSharedPtr<SScrollBox>& ViewBox)
{
	ConstructInternal(STableRow::FArguments(), InOwnerTableView);

	this->ChildSlot
	    .HAlign(HAlign_Fill)
	[
		SNew(SBox)
		.MinDesiredHeight(32.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Left)
			.AutoWidth()
			.Padding(FMargin(0.f, 0.f, 0.f, 0.f))
			[
				SNew(STextBlock)
				.Text_Lambda([InInstance]()
				{
					if (InInstance->DisplayName.IsEmpty())
					{
						return FText::FromString(TEXT("None"));
					}
					
					return InInstance->DisplayName;
				})
			]
		]
	];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
