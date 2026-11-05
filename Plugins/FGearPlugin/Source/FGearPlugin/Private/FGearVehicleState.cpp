// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearVehicleState.h"

FFGearInputState::FFGearInputState()
	: mLife(0)
	, mTick(0)
	, mThrottle(0.0f)
	, mSteer(0.0f)
	, mBrake(0.0f)
	, mClutch(0.0f)
	, mThrottleState(0)
	, mGear(0)
	, mHandbrake(false)
{
}

FFGearVehicleState::FFGearVehicleState()
	: mStateTime(0.0f)
	, mVehicleTime(0.0f)
	, mRpm(0.0f)
	, mLimiter(0.0f)
	, mFeedbackRpm(0.0f)
	, mCurGear(0)
	, mNextGear(0)
	, mPauseTime(0.0f)
{
}

void FFGearInputState::reset()
{
	mLife = 0;
	mTick = 0;
	mThrottle = 0.0f;
	mSteer = 0.0f;
	mBrake = 0.0f;
	mClutch = 0.0f;
	mThrottleState = 0;
	mGear = 0;
	mHandbrake = false;
}

void FFGearVehicleState::reset()
{
	//state
	mStateTime = 0.0f;

	//kinematic state
	mPosition = FVector::ZeroVector;
	mRotation = FQuat::Identity;
	mVelocity = FVector::ZeroVector;
	mAngularVelocity = FVector::ZeroVector;

	//vehicle state
	mVehicleTime = 0.0f;

	//engine state
	mRpm = 0.0f;
	mLimiter = 0.0f;
	mFeedbackRpm = 0.0f;

	//transmission state
	mCurGear = 0;
	mNextGear = 0;
	mPauseTime = 0.0f;

	//input state
	mInput.reset();

	//wheel states
	mActive.Empty();
	mLastCompressLength.Empty();
	mPitch.Empty();
	mRealSpeed.Empty();
}

bool FFGearInputState::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	//tick
	Ar << mTick;

	//throttle
	uint8 B = (mThrottle != 0);
	Ar.SerializeBits(&B, 1);
	if (B) Ar << mThrottle; else mThrottle = 0;

	//steer
	B = (mSteer != 0);
	Ar.SerializeBits(&B, 1);
	if (B) Ar << mSteer; else mSteer = 0;

	//brake
	B = (mBrake != 0);
	Ar.SerializeBits(&B, 1);
	if (B) Ar << mBrake; else mBrake = 0;

	//clutch
	B = (mClutch != 0);
	Ar.SerializeBits(&B, 1);
	if (B) Ar << mClutch; else mClutch = 0;

	//gear
	B = (mGear != 0);
	Ar.SerializeBits(&B, 1);
	if (B) Ar << mGear; else mGear = 0;

	//handbrake
	B = mHandbrake;
	Ar.SerializeBits(&B, 1);
	if (B) Ar << mHandbrake; else mHandbrake = false;

	bOutSuccess = true;
	return true;
}