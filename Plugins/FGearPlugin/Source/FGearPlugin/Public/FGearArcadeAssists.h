// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FGearTelemetryDrawer.h"
#include "FGearArcadeAssists.generated.h"

USTRUCT()
struct FGearPID
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditAnywhere, Category = "FGear", DisplayName = "Kp")
	float mKp;
	UPROPERTY(EditAnywhere, Category = "FGear", DisplayName = "Ki")
	float mKi;
	UPROPERTY(EditAnywhere, Category = "FGear", DisplayName = "Kd")
	float mKd;
	float mPrevError;
	float mP, mI, mD;

	FGearPID() : mKp(1.0f), mKi(0.0f), mKd(0.0f)
	{
		mPrevError = mP = mI = mD = 0.0f;
	}

	float getOutput(float currentError, float dt)
	{
		mP = currentError;
		mI += mP * dt;
		mD = (mP - mPrevError) / dt;
		mPrevError = currentError;
		return mP * mKp + mI * mKi + mD * mKd;
	}
};

class AFGearVehicle;
class UFGearAxle;

UCLASS(meta = (BlueprintSpawnableComponent), ClassGroup = (FGear))
class FGEARPLUGIN_API UFGearArcadeAssists : public UFGearTelemetryDrawer
{
	GENERATED_BODY()
public:
	UFGearArcadeAssists();

	void init(AFGearVehicle* v);
	void substepTick(float dt);

	//set instant kmh speed
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setInstantSpeed(float kmhSpeed, int gear);

	//recover assists setters
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setRecoverAssists(bool enable) { mRecoverAssists = enable; }
	
	//antirollbars setters
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setAntirollBars(bool enable) { mAntirollBars = enable; }

	//traction assist setters
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setTractionAssist(bool enable) { mTractionAssist = enable; }

	//drift assist setters
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setDriftAssist(bool enable) { mDriftAssist = enable; }

	//torque splitter setters
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setTorqueSplitter(bool enable) { mTorqueSplitter = enable; }

	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setCruiseControl(bool active, float kmhSpeed);

	//bike helper setters
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setBikeHelper(bool enable) { mBikeHelper = enable; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setAntiRollSpring(float v) { mAntiRollSpring = v; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setDamperSmooth(float v) { mDamperSmooth = v; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setMinLeanSpeed(float v) { mMinLeanSpeed = v; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setMaxLeanAngle(float v) { mMaxLeanAngle = v; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setMaxPitchAngle(float v) { mMaxPitchAngle = v; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setLeanInAir(bool enable) { mLeanInAir = enable; }

private:
	//recover assists
	UPROPERTY(EditAnywhere, Category = "FGearArcadeAssists|Recovery", DisplayName = "Enable Recover Assist")
	bool mRecoverAssists;
	UPROPERTY(EditAnywhere, Category = "FGearArcadeAssists|Recovery", DisplayName = "UnderSteer Recover Power", meta = (EditCondition = "mRecoverAssists"))
	float mUnderSteerRecoverPower;
	UPROPERTY(EditAnywhere, Category = "FGearArcadeAssists|Recovery", DisplayName = "OverSteer Recover Power", meta = (EditCondition = "mRecoverAssists"))
	float mOverSteerRecoverPower;
	UPROPERTY(EditAnywhere, Category = "FGearArcadeAssists|Recovery", DisplayName = "Minumum Recover Assist Speed", meta = (EditCondition = "mRecoverAssists", ClampMin = "0.0", ClampMax = "1000.0", UIMin = "0.0", UIMax = "1000.0"))
	float mMinRecoverAssistSpeed;
	UPROPERTY(EditAnywhere, Category = "FGearArcadeAssists|Recovery", DisplayName = "Full Recover Assist Speed", meta = (EditCondition = "mRecoverAssists", ClampMin = "0.0", ClampMax = "1000.0", UIMin = "0.0", UIMax = "1000.0"))
	float mFullRecoverAssistSpeed;

	//antirollbars
	UPROPERTY(EditAnywhere, Category = "FGearArcadeAssists|AntirollBar", DisplayName = "Enable Antirollbars")
	bool mAntirollBars;
	UPROPERTY(EditAnywhere, Category = "FGearArcadeAssists|AntirollBar", DisplayName = "Front Antiroll Power", meta = (EditCondition = "mAntirollBars", ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
	float mFrontAntiRollPower;
	UPROPERTY(EditAnywhere, Category = "FGearArcadeAssists|AntirollBar", DisplayName = "Rear Antiroll Power", meta = (EditCondition = "mAntirollBars", ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
	float mRearAntiRollPower;

	//traction assist
	UPROPERTY(EditAnywhere, Category = "FGearArcadeAssists|TractionAssist", DisplayName = "Enable Traction Assist")
	bool mTractionAssist;
	UPROPERTY(EditAnywhere, Category = "FGearArcadeAssists|TractionAssist", DisplayName = "Traction Assist Ratio", meta = (EditCondition = "mTractionAssist", ClampMin = "0.0", ClampMax = "200.0", UIMin = "0.0", UIMax = "200.0"))
	float mTractionAssistRatio;
	UPROPERTY(EditAnywhere, Category = "FGearArcadeAssists|TractionAssist", DisplayName = "Traction Assist Max Speed", meta = (EditCondition = "mTractionAssist", ClampMin = "1.0", ClampMax = "50.0", UIMin = "1.0", UIMax = "50.0"))
	float mTractionAssistMaxSpeed;

	//drift assist
	UPROPERTY(EditAnywhere, Category = "FGearArcadeAssists|DriftAssist", DisplayName = "Enable Drift Assist")
	bool mDriftAssist;
	UPROPERTY(EditAnywhere, Category = "FGearArcadeAssists|DriftAssist", DisplayName = "Front Drift Assist Ratio", meta = (EditCondition = "mDriftAssist", ClampMin = "-100.0", ClampMax = "100.0", UIMin = "-100.0", UIMax = "100.0"))
	float mDriftAssistFrontRatio;
	UPROPERTY(EditAnywhere, Category = "FGearArcadeAssists|DriftAssist", DisplayName = "Rear Drift Assist Ratio", meta = (EditCondition = "mDriftAssist", ClampMin = "-100.0", ClampMax = "100.0", UIMin = "-100.0", UIMax = "100.0"))
	float mDriftAssistRearRatio;

	//torque splitter
	UPROPERTY(EditAnywhere, Category = "FGearArcadeAssists|TorqueSplitter", DisplayName = "Enable Torque Splitter")
	bool mTorqueSplitter;
	UPROPERTY(EditAnywhere, Category = "FGearArcadeAssists|TorqueSplitter", DisplayName = "Torque Splitter Ratio", meta = (EditCondition = "mTorqueSplitter", ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
	float mTorqueSplitterRatio;
	UPROPERTY(EditAnywhere, Category = "FGearArcadeAssists|TorqueSplitter", DisplayName = "Torque Splitter Response Time", meta = (EditCondition = "mTorqueSplitter", ClampMin = "1.0", ClampMax = "1000.0", UIMin = "1.0", UIMax = "1000.0"))
	float mTorqueSplitterResponseTime;

	//bike helper
	UPROPERTY(EditAnywhere, Category = "FGearArcadeAssists|BikeHelper", DisplayName = "Enable Bike Helper")
	bool mBikeHelper;
	UPROPERTY(EditAnywhere, Category = "FGearArcadeAssists|BikeHelper", DisplayName = "Anti Roll Spring", meta = (EditCondition = "mBikeHelper"))
	float mAntiRollSpring;
	UPROPERTY(EditAnywhere, Category = "FGearArcadeAssists|BikeHelper", DisplayName = "Damper Smooth", meta = (EditCondition = "mBikeHelper", ClampMin = "0.1", ClampMax = "10.0", UIMin = "0.1", UIMax = "10.0"))
	float mDamperSmooth;
	UPROPERTY(EditAnywhere, Category = "FGearArcadeAssists|BikeHelper", DisplayName = "Min Lean Speed", meta = (EditCondition = "mBikeHelper", ClampMin = "0.0", ClampMax = "1000.0", UIMin = "0.0", UIMax = "1000.0"))
	float mMinLeanSpeed;
	UPROPERTY(EditAnywhere, Category = "FGearArcadeAssists|BikeHelper", DisplayName = "Max Lean Angle", meta = (EditCondition = "mBikeHelper", ClampMin = "0.0", ClampMax = "90.0", UIMin = "0.0", UIMax = "90.0"))
	float mMaxLeanAngle;
	UPROPERTY(EditAnywhere, Category = "FGearArcadeAssists|BikeHelper", DisplayName = "Max Pitch Angle", meta = (EditCondition = "mBikeHelper", ClampMin = "0.0", ClampMax = "90.0", UIMin = "0.0", UIMax = "90.0"))
	float mMaxPitchAngle;
	float mLastPitch;
	UPROPERTY(EditAnywhere, Category = "FGearArcadeAssists|BikeHelper", DisplayName = "Lean In Air", meta = (EditCondition = "mBikeHelper"))
	bool mLeanInAir;

	//cruise control
	UPROPERTY(EditAnywhere, Category = "FGearArcadeAssists|CruiseControl", DisplayName = "Enable Cruise Control")
	bool mCruiseControl;
	UPROPERTY(EditAnywhere, Category = "FGearArcadeAssists|CruiseControl", DisplayName = "Cruise Speed", meta = (EditCondition = "mCruiseControl"))
	float mCruiseSpeed;
	UPROPERTY(EditAnywhere, Category = "FGearArcadeAssists|CruiseControl", DisplayName = "PID Settings", meta = (EditCondition = "mCruiseControl"))
	FGearPID mCruiseControlPID;

	AFGearVehicle* mVehicle;
	FBodyInstance* mBody;
	TArray<UFGearAxle*> mAxles;
	TArray<float> mFrictionCoeffs;
	TArray<float> mTorqueShares;

	void updateRecoverAssists();
	void updateAntiRollBars();
	void updateTractionAssist();
	void updateDriftAssist();
	void updateTorqueSplitter(float dt);
	void updateBikeHelper(float dt);

public:
	void updateCruiseControl(float dt);
};