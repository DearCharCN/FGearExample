// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearTire.h"
#include <limits>

UFGearTire::UFGearTire() :
	UObject(),
	mReferenceLoad(3000.0f),
	mReferenceTirePressure(2.0f),
	mForceSymmetry(false)
{
}

void UFGearTire::init()
{
	calculateMaxLongitudinalForce();
	calculateMaxLateralForce();
	mLongitudinalForce = mLateralForce = 0.0f;
	mCombinedTorque = FVector::ZeroVector;
}

void UFGearTire::calculate(float load, float slipRatio, float slipAngle, float camber, float pressure, float radius, float Vx)
{
	mLongitudinalForce = mLateralForce = 0.0f;
	mCombinedTorque = FVector::ZeroVector;
}

void UFGearTire::calculateMaxLongitudinalForce()
{
	mMaxLongitudinalForce = -std::numeric_limits<float>::max();

	for (float i = 0.0f; i <= 1.0f; i += 0.01f)
	{
		calculate(mReferenceLoad, i, 0.0f, 0.0f, mReferenceTirePressure, 0.0f, 0.0f);
		if (mLongitudinalForce > mMaxLongitudinalForce)
		{
			mMaxLongitudinalForce = mLongitudinalForce;
			mMaxLongitudinalForceSlip = i;
		}
	}
}

void UFGearTire::calculateMaxLateralForce()
{
	mMaxLateralForce = -std::numeric_limits<float>::max();

	for (float i = 0.0f; i <= 1.0f; i += 0.01f)
	{
		float angle = i * 0.5f * PI;
		calculate(mReferenceLoad, 0.0f, angle, 0.0f, mReferenceTirePressure, 0.0f, 0.0f);
		if (mLateralForce > mMaxLateralForce)
		{
			mMaxLateralForce = mLateralForce;
			mMaxLateralForceAngle = angle;
		}
	}
}

void UFGearTire::updatePreviewCurve()
{
	mLngCurve.EditorCurveData.Reset();
	mLatCurve.EditorCurveData.Reset();

	for (int i = -100; i <= 100; i++)
	{
		float fi = i / 100.0f;
		float angle = fi * 0.475f * PI;
		calculate(mReferenceLoad, fi, 0.0f, 0.0f, mReferenceTirePressure, 0.0f, 0.0f);
		mLngCurve.EditorCurveData.AddKey(fi, getLongitudinalForce());
		calculate(mReferenceLoad, 0.0f, angle, 0.0f, mReferenceTirePressure, 0.0f, 0.0f);
		mLatCurve.EditorCurveData.AddKey(angle, getLateralForce());
	}
}

#if WITH_EDITOR
void UFGearTire::PostEditChangeProperty(FPropertyChangedEvent& e)
{
	Super::PostEditChangeProperty(e);
	updatePreviewCurve();
}
#endif