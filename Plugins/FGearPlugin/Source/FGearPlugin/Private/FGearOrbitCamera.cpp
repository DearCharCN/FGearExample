// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearOrbitCamera.h"
#include "FGearVehicle.h"

UFGearOrbitCamera::UFGearOrbitCamera() :
	UFGearTelemetryDrawer(),
	mDriftMode(false),
	mFreeze(false),
	mSpring(1.0f),
	mDistance(600.0f),
	mTargetZOffset(0.0f),
	mTiltAngle(15.0f),
	mRotationY(0.0f),
	mRotationOffset(0.0f),
	mCurrentTilt(0.0f),
	mOrbitMode(false)
{
}

void UFGearOrbitCamera::BeginPlay()
{
	Super::BeginPlay();
	mTarget = Cast<AFGearVehicle>(GetOwner());
	if (mTarget == nullptr) return;
	mTarget->mOrbitCamera = this;
	mLastPos = mTarget->GetActorLocation();
}

void UFGearOrbitCamera::injectMouseDX(float mx)
{
	if (mOrbitMode) mRotationY += mx;
}

void UFGearOrbitCamera::injectMouseDY(float my)
{
	if (mOrbitMode) mTiltAngle += my;
}

void UFGearOrbitCamera::injectMouseDZ(float mz)
{
	mDistance -= mz;
}

void UFGearOrbitCamera::update(float deltaTime, FMinimalViewInfo& outResult)
{
	if (mTarget == nullptr) return;

	//no spring in orbit mode
	float cspring = mOrbitMode ? 0.0f : mSpring;

	//rotation update when spring is non zero
	{
		//lerp towards current tilt angle
		float tilt = mTarget->GetActorRotation().Euler().Y - mTiltAngle;
		if (cspring > 0.0f) mCurrentTilt = FGearUtility::LerpAngle(mCurrentTilt, tilt, cspring * deltaTime);
		else mCurrentTilt = -mTiltAngle;
		//lerp towards current yaw angle
		float targetAngle = mTarget->GetActorRotation().Euler().Z;
		if (mDriftMode)
		{
			FVector delta = mTarget->GetActorLocation() - mLastPos;
			float speed = (delta.Size() / deltaTime) * FGearUtility::cm2meter * FGearUtility::ms2kmh;
			if (speed > 5.0f) targetAngle = delta.Rotation().Euler().Z;
		}
		mRotationY = FGearUtility::LerpAngle(mRotationY, targetAngle, cspring * deltaTime);
		FQuat rot = FGearUtility::Euler(mCurrentTilt, mRotationY + mRotationOffset, 0.0f);
		FVector targetPos = mTarget->GetActorLocation() + mTargetZOffset * FVector::UpVector;
		FVector camPos = targetPos + mDistance * (rot * FVector::BackwardVector);
		if (mFreeze) camPos = outResult.Location;
		if (cspring > 0.0f || mFreeze) outResult.Rotation = (targetPos - camPos).Rotation();
		mLastPos = mTarget->GetActorLocation();
	}

	//update rotation when spring is zero
	{
		FQuat rot = FGearUtility::Euler(mCurrentTilt, mRotationY + mRotationOffset, 0.0f);
		FVector targetPos = mTarget->GetActorLocation() + mTargetZOffset * FVector::UpVector;
		FVector camPos = targetPos + mDistance * (rot * FVector::BackwardVector);
		if (cspring <= 0.0f) camPos = targetPos + mDistance * (rot * FVector::BackwardVector);
		if (!mFreeze) outResult.Location = camPos;
		if (cspring <= 0.0f) outResult.Rotation = (targetPos - camPos).Rotation();
	}
}