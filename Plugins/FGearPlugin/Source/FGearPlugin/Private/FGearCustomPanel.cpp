// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearCustomPanel.h"

#if WITH_EDITOR

#include "Editor/PropertyEditor/Public/DetailLayoutBuilder.h"
#include "Editor/PropertyEditor/Public/DetailCategoryBuilder.h"
#include "Editor/PropertyEditor/Public/DetailWidgetRow.h"
#include "Runtime/Slate/Public/Widgets/Text/STextBlock.h"
#include "Runtime/Slate/Public/Widgets/Input/SButton.h"
#include "FGearUtility.h"
#include "FGearVehicle.h"
#include "FGearEngine.h"
#include "FGearWheelOptions.h"

#define LOCTEXT_NAMESPACE "FFGearPluginModule"

FGearCustomPanel::~FGearCustomPanel()
{
	UPackage::PackageDirtyStateChangedEvent.RemoveAll(this);
}

TSharedRef<IDetailCustomization> FGearCustomPanel::MakeInstance()
{
	return MakeShareable(new FGearCustomPanel);
}

void FGearCustomPanel::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> CustomizedObjects;
	DetailBuilder.GetObjectsBeingCustomized(CustomizedObjects);
	for (TWeakObjectPtr<UObject> Object : CustomizedObjects)
	{
		if (Object.IsValid())
		{
			if (Object->IsA(AFGearVehicle::StaticClass()))
			{
				AFGearVehicle* v = Cast<AFGearVehicle>(Object);
				if (v)
				{
					//hide default component categories
					DetailBuilder.HideCategory(FName("FGearAeroDynamics"));
					DetailBuilder.HideCategory(FName("FGearAxle"));
					DetailBuilder.HideCategory(FName("FGearEngine"));
					DetailBuilder.HideCategory(FName("FGearReplication"));
					DetailBuilder.HideCategory(FName("FGearStandardInput"));
					DetailBuilder.HideCategory(FName("FGearTransmission"));
					DetailBuilder.HideCategory(FName("FGearWheel"));
					DetailBuilder.HideCategory(FName("FGearWheelOptions"));

					IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("FGearVehicle", FText::GetEmpty(), ECategoryPriority::Important);
					Category.AddCustomRow(LOCTEXT("RowSearch", "Save&Load"), true)
					.NameContent()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("SerializeText", "Save or Load Settings"))
						.Font(IDetailLayoutBuilder::GetDetailFont())
					]
					.ValueContent()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						[
							SNew(SButton)
							.OnClicked_UObject(v, &AFGearVehicle::onSaveClicked)
							.HAlign(HAlign_Center)
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								[
									SNew(SBorder)
									[
										SNew(STextBlock)
										.Text(FText::FromString("Save"))
									]
								]
							]
						]
						+ SHorizontalBox::Slot()
						.FillWidth(0.1)
						+ SHorizontalBox::Slot()
						[
							SNew(SButton)
							.OnClicked_UObject(v, &AFGearVehicle::onLoadClicked)
							.HAlign(HAlign_Center)
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								[
									SNew(SBorder)
									[
										SNew(STextBlock)
										.Text(FText::FromString("Load"))
									]
								]
							]
						]
					];
				}
			}
			else
			{
				if (Object->IsA(UFGearEngine::StaticClass()))
				{
					mEngine = Cast<UFGearEngine>(Object);
					if (mEngine)
					{
						IDetailCategoryBuilder& cat = DetailBuilder.EditCategory("FGearEngine", FText::GetEmpty(), ECategoryPriority::Important);
						addTextInfoRow(cat, FText::FromString("Max Power"), &FGearCustomPanel::getMaxPower);
					}
				}
				else if (Object->IsA(UFGearWheelOptions::StaticClass()))
				{
					mWheelOpts = Cast<UFGearWheelOptions>(Object);
					if (mWheelOpts)
					{
						IDetailCategoryBuilder& cat = DetailBuilder.EditCategory("FGearWheelOptions", FText::GetEmpty(), ECategoryPriority::Important);
						addTextInfoRow(cat, FText::FromString("Total Length"), &FGearCustomPanel::getTotalLength);
						addTextInfoRow(cat, FText::FromString("Max Spring Force"), &FGearCustomPanel::getMaxSpringForce);
						addTextInfoRow(cat, FText::FromString("Preload Length"), &FGearCustomPanel::getPreloadLength);
					}
				}
				else if (Object->IsA(UFGearTransmission::StaticClass()))
				{
					mTransmission = Cast<UFGearTransmission>(Object);
					if (mTransmission)
					{
						IDetailCategoryBuilder& cat = DetailBuilder.EditCategory("FGearTransmission", FText::GetEmpty(), ECategoryPriority::Important);
						mTransmissionLayout = MakeShareable(new FInfoTextLayout(mTransmission));
						cat.AddCustomBuilder(mTransmissionLayout.ToSharedRef(), true);
					}
				}
			}
		}
	}
	UPackage::PackageDirtyStateChangedEvent.AddSP(this, &FGearCustomPanel::onEditorPackageModified);
}

#if ENGINE_MAJOR_VERSION != 5
void FGearCustomPanel::addTextInfoRow(IDetailCategoryBuilder& category, FText title, TDelegate<FText()>::TSPMethodDelegate_Const<FGearCustomPanel>::FMethodPtr inFunc)
#else
void FGearCustomPanel::addTextInfoRow(IDetailCategoryBuilder& category, FText title, TDelegate<FText()>::TConstMethodPtr<FGearCustomPanel> inFunc)
#endif
{
	category.AddCustomRow(title, true)
	.NameContent()
	[
		SNew(STextBlock)
		.Text(title)
		.Font(IDetailLayoutBuilder::GetDetailFont())
	]
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		[
			SNew(STextBlock)
			.Text(this, inFunc)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
	];
}

FText FGearCustomPanel::getMaxPower() const
{
	if (mEngine)
	{
		float maxPower, maxPowerRpm;
		mEngine->calculateMaxPower(maxPower, maxPowerRpm);
		maxPower *= FGearUtility::kw2hp;
		return FText::FromString(FString::Printf(TEXT("%.0fhp @%.0frpm"), maxPower, maxPowerRpm));
	}
	return FText();
}

FText FGearCustomPanel::getTotalLength() const
{
	if (mWheelOpts)
	{
		float springLength = mWheelOpts->getSuspensionUpTravel() + mWheelOpts->getSuspensionDownTravel();
		return FText::FromString(FString::SanitizeFloat(springLength) + "m");
	}
	return FText();
}

FText FGearCustomPanel::getMaxSpringForce() const
{
	if (mWheelOpts)
	{
		float springLength = mWheelOpts->getSuspensionUpTravel() + mWheelOpts->getSuspensionDownTravel();
		float springForce = 1000.0f * springLength * mWheelOpts->getSuspensionSpring();
		return FText::FromString(FString::SanitizeFloat(springForce) + "N");
	}
	return FText();
}

FText FGearCustomPanel::getPreloadLength() const
{
	if (mWheelOpts)
	{
		float springLength = mWheelOpts->getSuspensionUpTravel() + mWheelOpts->getSuspensionDownTravel();
		float preload = (mWheelOpts->getSuspensionPreload() / 100.0f) * springLength;
		float preloadForce = 1000.0f * preload * mWheelOpts->getSuspensionSpring();
		return FText::FromString(FString::Printf(TEXT("%.1fmm - Force %.1fN"), 1000.0f * preload, preloadForce));
	}
	return FText();
}

void FGearCustomPanel::onEditorPackageModified(UPackage * Package)
{
	if (mTransmissionLayout
		&& Package
		&& Package->IsDirty()
		&& (Package == mTransmission->GetOutermost())
		&& !mTransmissionLayout.IsValid())
	{
		mTransmissionLayout->refresh();
	}
}

void FInfoTextLayout::refresh()
{
	if (mTarget == nullptr) return;
	if (!mTarget->IsValidLowLevel()) return;

	onRebuildChildren.ExecuteIfBound();
}

void FInfoTextLayout::GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder)
{
	if (mTarget)
	{
		mChildren.Empty();
		TArray<float> maxSpeeds = mTarget->getMaxSpeeds();
		if (maxSpeeds.Num() > 0)
		{
			for (int i = 0; i < maxSpeeds.Num(); i++)
			{
				if (maxSpeeds.IsValidIndex(i))
				{
					TSharedRef<class FInfoTextArgLayout> argLayout = MakeShareable(new FInfoTextArgLayout(i + 1, maxSpeeds.Num(), maxSpeeds[i]));
					ChildrenBuilder.AddCustomBuilder(argLayout);
					mChildren.Add(argLayout);
				}
			}
		}
	}
}

void FInfoTextArgLayout::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
	NodeRow
	.NameContent()
	[
		SNew(STextBlock)
		.Text(this, &FInfoTextArgLayout::getGearTitleText)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		]
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		[
			SNew(STextBlock)
			.Text(this, &FInfoTextArgLayout::getGearValueText)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
	];
}

FText FInfoTextArgLayout::getGearTitleText() const
{
	FString gearTxt = mIndex == mMaxIndex ? "R" : FString::FromInt(mIndex);
	return FText::FromString("Gear " + gearTxt + " Top Speed");
}

FText FInfoTextArgLayout::getGearValueText() const
{
	return FText::FromString(FString::Printf(TEXT("%.1f km/h"), mSpeed));
}

#undef LOCTEXT_NAMESPACE

#endif