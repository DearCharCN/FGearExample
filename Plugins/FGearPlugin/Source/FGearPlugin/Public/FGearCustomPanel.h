// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#if WITH_EDITOR
#include "Runtime/Launch/Resources/Version.h"
#include "Editor/DetailCustomizations/Public/DetailCustomizations.h"
#include "Editor/PropertyEditor/Public/DetailCategoryBuilder.h"
#include "Editor/PropertyEditor/Public/IDetailChildrenBuilder.h"
#include "Editor/PropertyEditor/Public/IDetailCustomization.h"
#include "FGearTransmission.h"

class FInfoTextLayout : public IDetailCustomNodeBuilder, public TSharedFromThis<FInfoTextLayout>
{
public:
	FInfoTextLayout(UFGearTransmission* inTarget) : mTarget(inTarget)
	{
		mTarget->mLayout = this;
		mTarget->refreshParameters();
	}
	void refresh();

private:
	virtual void SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren) override { onRebuildChildren = InOnRegenerateChildren; };
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override {};
	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override;
	virtual void Tick(float DeltaTime) override {}
	virtual bool RequiresTick() const override { return false; }
	virtual FName GetName() const override { return NAME_None; }
	virtual bool InitiallyCollapsed() const override { return false; }

	UFGearTransmission* mTarget;
	FSimpleDelegate onRebuildChildren;
	TArray<TWeakPtr<class FInfoTextArgLayout>> mChildren;
};

class FInfoTextArgLayout : public IDetailCustomNodeBuilder, public TSharedFromThis<FInfoTextArgLayout>
{
public:
	FInfoTextArgLayout(int indx, int mindx, float speed) : mIndex(indx), mMaxIndex(mindx), mSpeed(speed)
	{
	}

private:
	virtual void SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren) override {};
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override;
	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override {};
	virtual void Tick(float DeltaTime) override {}
	virtual bool RequiresTick() const override { return false; }
	virtual FName GetName() const override { return NAME_None; }
	virtual bool InitiallyCollapsed() const override { return false; }

	FText getGearTitleText() const;
	FText getGearValueText() const;

	int mIndex;
	int mMaxIndex;
	float mSpeed;
};

class FGearCustomPanel : public IDetailCustomization
{
public:
	FGearCustomPanel() : mTransmissionLayout(nullptr) {}
	~FGearCustomPanel();
	static TSharedRef<IDetailCustomization> MakeInstance();
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
#if ENGINE_MAJOR_VERSION != 5
	void addTextInfoRow(IDetailCategoryBuilder& category, FText title, TDelegate<FText()>::TSPMethodDelegate_Const<FGearCustomPanel>::FMethodPtr inFunc);
#else
	void addTextInfoRow(IDetailCategoryBuilder& category, FText title, TDelegate<FText()>::TConstMethodPtr<FGearCustomPanel> inFunc);
#endif
	FText getMaxPower() const;
	FText getTotalLength() const;
	FText getMaxSpringForce() const;
	FText getPreloadLength() const;
	void onEditorPackageModified(UPackage* Package);

	class UFGearEngine* mEngine;
	class UFGearWheelOptions* mWheelOpts;
	UFGearTransmission* mTransmission;
	TSharedPtr<class FInfoTextLayout> mTransmissionLayout;
};

#endif