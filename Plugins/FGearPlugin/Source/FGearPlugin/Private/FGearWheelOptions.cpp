// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearWheelOptions.h"
#include "FGearTire.h"
#include "FGearWheelOptions.h"
#include "FGearAxle.h"
#include "FGearTireSimple.h"

UFGearWheelOptions::UFGearWheelOptions() : 
	UFGearTelemetryDrawer(),
	mTireModelClass(nullptr),
	mTireModel(nullptr),
	mLateralFriction(1.0f),
	mLongitudinalFriction(1.0f),
	mLateralScale(1.0f),
	mLongitudinalScale(1.0f),
	mOverturnScale(0.0f),
	mRollMomentScale(0.0f),
	mSelfAlignScale(0.0f),
	mTirePressure(2.0f),
	mRadius(0.25f),
	mWidth(0.2f),
	mForwardOffset(0.0f),
	mSpacerOffset(0.0f),
	mVerticalOffset(0.0f),
	mRimOffset(0.0f),
	mCastOffset(0.0f),
	mMass(10.0f),
	mBrakeTorque(1000.0f),
	mSuspensionUpTravel(0.1f),
	mSuspensionDownTravel(0.1f),
	mSuspensionSpring(20.0f),
	mCompressionDamper(1000.0f),
	mRelaxationDamper(1000.0f),
	mSuspensionPreload(0.0f)
{
}

void UFGearWheelOptions::reset()
{
	mLateralFriction = 1.0f;
	mLongitudinalFriction = 1.0f;
	mLateralScale = 1.0f;
	mLongitudinalScale = 1.0f;
	mOverturnScale = 0.0f;
	mRollMomentScale = 0.0f;
	mSelfAlignScale = 0.0f;
	mTirePressure = 2.0f;
	mRadius = 0.25f;
	mWidth = 0.2f;
	mForwardOffset = 0.0f;
	mSpacerOffset = 0.0f;
	mVerticalOffset = 0.0f;
	mRimOffset = 0.0f;
	mCastOffset = 0.0f;
	mMass = 10.0f;
	mBrakeTorque = 1000.0f;
	mSuspensionUpTravel = 0.1f;
	mSuspensionDownTravel = 0.1f;
	mSuspensionSpring = 5000.0f;
	mCompressionDamper = 1000.0f;
	mRelaxationDamper = 1000.0f;
	mSuspensionPreload = 0.0f;
	checkNoTireState();
}

//TODO : delete previous?
void UFGearWheelOptions::setTireModel(UFGearTire* t)
{
	if (t)
	{
		mTireModel = t;
		t->init();
	}
}

void UFGearWheelOptions::checkNoTireState()
{
	if (mTireModel == nullptr)
	{
		if (mTireModelClass != nullptr)
		{
			mTireModel = NewObject<UFGearTire>(this, mTireModelClass);
		}
		else
		{
			forceCreateTire();
		}
		mTireModel->init();
	}
}

void UFGearWheelOptions::forceCreateTire()
{
	mTireModel = NewObject<UFGearTireSimple>();
}

void UFGearWheelOptions::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (mTireModel != nullptr && mTireModel->IsValidLowLevel())
	{
		mTireModel->ConditionalBeginDestroy();
		mTireModel = nullptr;
	}
	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

#if WITH_EDITOR
void UFGearWheelOptions::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (mAxle != nullptr) mAxle->applyWheelOptions();
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif