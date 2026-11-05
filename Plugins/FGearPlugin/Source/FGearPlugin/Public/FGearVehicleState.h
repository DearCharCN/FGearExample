// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FGearVehicleState.generated.h"

USTRUCT(BlueprintType)
struct FFGearInputState
{
	GENERATED_USTRUCT_BODY()

	int mLife;
	UPROPERTY()
	int mTick;
	UPROPERTY()
	float mThrottle;
	UPROPERTY()
	float mSteer;
	UPROPERTY()
	float mBrake;
	UPROPERTY()
	float mClutch;
	UPROPERTY()
	int8 mThrottleState;
	UPROPERTY()
	int8 mGear;
	UPROPERTY()
	bool mHandbrake;

	FFGearInputState();

	uint64 compress()
	{
		return	(uint64)(mThrottle * 65535.0f) << 48 |
				(uint64)(mBrake * 65535.0f) << 32 |
				(uint64)((mSteer + 1.0f) * 32767.0f) << 16 |
				(uint64)(mClutch * 255.0f) << 8 |
				(uint64)(((mGear + 1) & 0x7F) | (mHandbrake ? 0x80 : 0x00));
	}

	void uncompress(uint64 pckt)
	{
		mThrottle = (float)((pckt >> 48) & 0xFFFF) / 65535.0f;
		mBrake = (float)((pckt >> 32) & 0xFFFF) / 65535.0f;
		mSteer = ((float)((pckt >> 16) & 0xFFFF) / 32767.0f) - 1.0f;
		mClutch = (float)((pckt >> 8) & 0xFF) / 255.0f;
		mGear = (pckt & 0x7F) - 1;
		mHandbrake = (pckt & 0x80) != 0;
	}

	bool isSame(FFGearInputState s)
	{
		return	mThrottle == s.mThrottle &&
				mSteer == s.mSteer &&
				mBrake == s.mBrake &&
				mClutch == s.mClutch &&
				mThrottleState == s.mThrottleState &&
				mGear == s.mGear &&
				mHandbrake == s.mHandbrake;
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
	void reset();
};

//needed or NetSerialize will not be called
template<> struct TStructOpsTypeTraits<FFGearInputState> : public TStructOpsTypeTraitsBase2<FFGearInputState>
{
	enum { WithNetSerializer = true };
};

USTRUCT(BlueprintType)
struct FFGearVehicleState
{
	GENERATED_USTRUCT_BODY()

	FFGearVehicleState();
	void reset();

	//state
	float mStateTime;

	//kinematic state
	FVector mPosition;
	FQuat mRotation;
	FVector mVelocity;
	FVector mAngularVelocity;

	//vehicle state
	float mVehicleTime;

	//engine state
	float mRpm;
	float mLimiter;
	float mFeedbackRpm;

	//transmission state
	int mCurGear;
	int mNextGear;
	float mPauseTime;

	//input state
	FFGearInputState mInput;

	//wheel states
	TArray<bool> mActive;
	TArray<float> mLastCompressLength;
	TArray<float> mPitch;
	TArray<float> mRealSpeed;
};