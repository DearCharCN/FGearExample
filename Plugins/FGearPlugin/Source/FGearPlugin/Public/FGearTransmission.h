// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "FGearTelemetryDrawer.h"
#include "FGearVehicleState.h"
#include "FGearTransmission.generated.h"

class AFGearVehicle;
class UFGearAxle;
#if WITH_EDITOR
class FInfoTextLayout;
#endif

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGearTransmissionSignature, int, prev, int, next);

UCLASS()
class FGEARPLUGIN_API UFGearTransmission : public UFGearTelemetryDrawer
{
	GENERATED_BODY()
	friend class FGearSerializer;
#if WITH_EDITOR
	friend class FInfoTextLayout;
#endif
public:
	UFGearTransmission();
	void init(AFGearVehicle* v);
	void myFixedUpdate(float dt);
	void updateClutchState(float dt);

	void saveState(FFGearVehicleState& state);
	void loadState(FFGearVehicleState& state);

	UFUNCTION(BlueprintCallable, Category = "FGear")
	void refreshParameters();
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setCurrentGear(int gear, bool force);
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void gearUp(bool skipN);
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void gearDown(bool skipN);
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getCurrentGearRatio() const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getTransmissionRatio(const UFGearAxle* axle) const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getGearRatio(int gear) const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getGearMaxSpeed(int gear) const { return mMaxSpeeds.IsValidIndex(gear) ? mMaxSpeeds[gear] : -1.0f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool isAutoChange() const { return mAutoChange; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool isAutoReverse() const { return mAutoReverse; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool isAutoClutch() const { return mAutoClutch; }
	bool isAutoClutchActive() const { return mAutoClutchActive; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool isRevMatching() const { return mRevMatching; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getChangeTime() const { return mChangeTime; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getClutchTime() const { return mClutchTime; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getGearUpRatio() const { return mGearUpRatio; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getGearDownRatio() const { return mGearDownRatio; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getGearUpVelocityBias() const { return mGearUpVelocityBias; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getGearDownVelocityBias() const { return mGearDownVelocityBias; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getClutchState() const { return mClutchState; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getClutchScale() const { return mClutchScale; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	TArray<float> getMaxSpeeds() const { return mMaxSpeeds; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	int getCurGear() const { return mCurGear; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	int getMaxGear() const { return mMaxGear; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	int getGearCount() const { return mGearRatios.Num(); }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setGearCount(int size);
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setGearRatio(int gear, float ratio) { mGearRatios[gear] = ratio; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setAutoChange(bool b) { mAutoChange = b; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setAutoReverse(bool b) { mAutoReverse = b; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setAutoClutch(bool b) { mAutoClutch = b; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setRevMatching(bool b) { mRevMatching = b; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setChangeTime(float t) { mChangeTime = t; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setClutchTime(float t) { mClutchTime = t; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setGearUpRatio(float f) { mGearUpRatio = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setGearDownRatio(float f) { mGearDownRatio = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setGearUpVelocityBias(float f) { mGearUpVelocityBias = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setGearDownVelocityBias(float f) { mGearDownVelocityBias = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setThrottleState(int ts) { mThrottleState = ts; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setClutchState(float cs);
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setClutchScale(float cs) { mClutchScale = cs; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setPauseTime(float f) { mPauseTime = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool isChanging() const { return mPauseTime > 0.0f; }

	virtual FVector2D drawTelemetry(UCanvas* Canvas, UFont* Font, FLinearColor textColor, FFontRenderInfo renderInfo, FVector2D p) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool CanEditChange(const FProperty* Property) const override;
#endif

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FGearTransmissionSignature OnGearChanged;

protected:
	virtual void gearChanged(int prev, int next);

private:
	/** When set to true gears are changed automatically according to the gear up/down values.*/
	UPROPERTY(EditAnywhere, Category = "FGearTransmission", DisplayName = "Auto Change")
	bool mAutoChange;
	/** When set to true the transmission automatically goes to rear gear when you brake and stop. Also the same thing happens when you are going backwards and push the throttle, this time the transmission automatically goes to first gear. This is only active when Auto Change is set. */
	UPROPERTY(EditAnywhere, Category = "FGearTransmission", DisplayName = "Auto Reverse", meta = (EditCondition = "mAutoChange"))
	bool mAutoReverse;
	/** When set to true the transmission automatically manages the clutch input for you. Set this to false if you want to manually operate the clutch. */
	UPROPERTY(EditAnywhere, Category = "FGearTransmission", DisplayName = "Auto Clutch")
	bool mAutoClutch;
	bool mAutoClutchActive;
	/** When set to true the rpm of the engine is interpolated towards the perfect match of your next gear ratio while changing gears. The time to interpolate is based on change time. */
	UPROPERTY(EditAnywhere, Category = "FGearTransmission", DisplayName = "Rev Matching")
	bool mRevMatching;
	/** When a gear change happens first the transmission goes to neutral and then switches to the next gear, this value determines how long this process takes in miliseconds. */
	UPROPERTY(EditAnywhere, Category = "FGearTransmission", DisplayName = "Change Time", meta = (ClampMin = "0.0", ClampMax = "1000.0", UIMin = "0.0", UIMax = "1000.0"))
	float mChangeTime;
	/** Suppose you drive in 1st gear, the maximum speed that you can achieve with this gear is calculated(mxs) and if you go over (gearup ratio * mxs) then the gear is changed */
	UPROPERTY(EditAnywhere, Category = "FGearTransmission", DisplayName = "Gear Up Ratio", meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
	float mGearUpRatio;
	/** Suppose you drive in 2nd gear, the maximum speed that you can achieve with the previous gear(1st) is calculated(mxs) and if you go below (geardown ratio * mxs) then the gear is changed */
	UPROPERTY(EditAnywhere, Category = "FGearTransmission", DisplayName = "Gear Down Ratio", meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
	float mGearDownRatio;
	/** Gear(up) change can be based on vehicle's velocity or the wheel speed. If this is zero it will be %100 based on wheel speed and if this is 1 then it will be %100 based on velocity. */
	UPROPERTY(EditAnywhere, Category = "FGearTransmission", DisplayName = "Gear Up Velocity Bias", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float mGearUpVelocityBias;
	/** Gear(down) change can be based on vehicle's velocity or the wheel speed. If this is zero it will be %100 based on wheel speed and if this is 1 then it will be %100 based on velocity. */
	UPROPERTY(EditAnywhere, Category = "FGearTransmission", DisplayName = "Gear Down Velocity Bias", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float mGearDownVelocityBias;
	/** Before changing gears, the clutch is fully disengaged. After gear change is completed, it takes this amount of time to reengage the clutch. Usually this value should be equal or lower than the change time. */
	UPROPERTY(EditAnywhere, Category = "FGearTransmission", DisplayName = "Clutch Engage Time", meta = (ClampMin = "1.0", ClampMax = "1000.0", UIMin = "1.0", UIMax = "1000.0"), meta = (EditCondition = "mAutoClutch"))
	float mClutchTime;
	/** This is a coefficient to the power transfer of the clutch(between the engine and the wheels). Do not change it if you really need to. Lower values will result high differences between engine and wheel rpms. Higher values will force both rpm values to be in sync but the engine rpm can behave erratic. */
	UPROPERTY(EditAnywhere, Category = "FGearTransmission", DisplayName = "Clutch Power Scale (%)", meta = (ClampMin = "1.0", ClampMax = "1000.0", UIMin = "1.0", UIMax = "1000.0"), meta = (EditCondition = "!mAutoClutch || !mClutchAssist"))
	float mClutchScale;
	/** This is an array of values which are used for calculating the final transmission ratio along with final drive. If you need a 5 speed car then the size should be 6 with the addition of rear gear. So the first 5 values are forward ratios and the last one is the rear gear ratio. All values should be positive and you can find real cars values on the internet. The default vehicle has only 1 forward and 1 backward gear so you can try the values in the image above. */
	UPROPERTY(EditAnywhere, Category = "FGearTransmission", DisplayName = "Gear Ratios")
	TArray<float> mGearRatios;
	/** This is an experimental feature. When you first launch the vehicle or when you try to climb a steep slope,
	    the rpm can stay at a low level until you reach a certain speed since there is not enough torque and there is no engine stalling.
		This is quite observable when you have a long first fear ratio. This assist will scale the power of the clutch in order to fight with this behavior.
		With lower clutch power the rpm can rise up and you can get receive more torque from the engine resulting faster accelerations.
		This is only activated with the first(1) and the rear(R) gears.
	*/
	UPROPERTY(EditAnywhere, Category = "FGearTransmission", DisplayName = "Stall Assist", meta = (EditCondition = "mAutoClutch"))
	bool mClutchAssist;
	UPROPERTY(EditAnywhere, Category = "FGearTransmission", DisplayName = "Clutch Scale Min (%)", meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"), meta = (EditCondition = "mClutchAssist"))
	float mClutchScaleMin;
	UPROPERTY(EditAnywhere, Category = "FGearTransmission", DisplayName = "Clutch Scale Max (%)", meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"), meta = (EditCondition = "mClutchAssist"))
	float mClutchScaleMax;

#if WITH_EDITOR
	FInfoTextLayout* mLayout;
#endif
	AFGearVehicle* mVehicle;
	TArray<float> mMaxSpeeds;
	int mCurGear;
	int mNextGear;
	int mMaxGear;
	int mThrottleState;
	float mPauseTime;
	float mUpSpeed;
	float mDownSpeed;
	float mClutchState;
};