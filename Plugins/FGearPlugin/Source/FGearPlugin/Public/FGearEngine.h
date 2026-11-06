// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "FGearTelemetryDrawer.h"
#include "FGearVehicleState.h"
#include "FGearEngine.generated.h"

class UFGearTransmission;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGearEngineStartStopSignature, bool, started);

UCLASS()
class FGEARPLUGIN_API UFGearEngine : public UFGearTelemetryDrawer
{
	GENERATED_BODY()
	friend class FGearSerializer;
public:
	UFGearEngine();
	void init(AFGearVehicle* v);
	void myFixedUpdate(float dt);
	void setFeedback(float dt, float rpm, float torque);
	void setRevMatching(float time, float rpm);
	void updateStallState(float dt);
	bool canStall();

	void saveState(FFGearVehicleState& state);
	void loadState(FFGearVehicleState& state);

	UFUNCTION(BlueprintCallable, Category = "FGear")
	void calculatePeakTorqueRpm();
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void calculateMaxPower(float& maxPower, float& maxPowerRpm);
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UCurveFloat* getTorqueCurve() const { return mTorqueCurve; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getTorqueScale() const { return mTorqueScale; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getIdleRpm() const { return mIdleRpm; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getLimitRpm() const { return mLimitRpm; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getLimiterTime() const { return mLimiterTime; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getEngineInertia() const { return mEngineInertia; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getEngineStallTime() const { return mEngineStallTime; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getEngineStallRpm() const { return mEngineStallRpm; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getFrictionTorque() const { return mFrictionTorque; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getRpm() const { return mRpm; }
	float getFeedbackRpm() const { return mFeedbackRpm; }
	float getPeakTorqueRpm() const { return mPeakTorqueRpm; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getThrottle() const { return mThrottle; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getTorque() const { return mTorque; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getRpmRatio() const { return mRpm / mLimitRpm; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setTorqueCurve(UCurveFloat* curve) { mTorqueCurve = curve; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setTorqueScale(float f) { mTorqueScale = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setRpm(float f) { mRpm = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setIdleRpm(float f) { mIdleRpm = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setLimitRpm(float f) { mLimitRpm = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setLimiterTime(float f) { mLimiterTime = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setEngineInertia(float f) { mEngineInertia = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setEngineStallTime(float f) { mEngineStallTime = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setEngineStallRpm(float f) { mEngineStallRpm = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setFrictionTorque(float f) { mFrictionTorque = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setThrottle(float pedal);
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setLimiter(float l) { mLimiter = l; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void resetRpm() { mRpm = mIdleRpm; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool isLimiterOn() const { return mLimiter > 0.0f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool isRunning() const { return mRunning; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setRunning(bool b);

	virtual FVector2D drawTelemetry(UCanvas* Canvas, UFont* Font, FLinearColor textColor, FFontRenderInfo renderInfo, FVector2D p) override;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FGearEngineStartStopSignature OnStartStop;

private:
	/** The engine torque is directly the result of this curve. The X axis of the curve represents engine rpm and Y axis is the torque value in Nm. There are real world vehicles engine curves available on the internet, mimicking those curves is a good way to start. */
	UPROPERTY(EditAnywhere, Category = "FGearEngine", DisplayName = "Torque Curve")
	UCurveFloat* mTorqueCurve;
	/** This is the direct multiplier of engine torque curve. Final engine torque is : Curve Result x Scale. */
	UPROPERTY(EditAnywhere, Category = "FGearEngine", DisplayName = "Torque Scale")
	float mTorqueScale;
	/** This is a constant friction factor(multiplied with rpm) that slows down the engine. Higher values result more engine brake and faster rpm rev downs. Lower values has opposite effect so to determine a good value you can rev up in neutral gear and see how it revs down. */
	UPROPERTY(EditAnywhere, Category = "FGearEngine", DisplayName = "Friction Torque")
	float mFrictionTorque;
	/** Minumum rpm value of the engine, currently there is no stalling so the engine always keeps revving */
	UPROPERTY(EditAnywhere, Category = "FGearEngine", DisplayName = "Idle Rpm")
	float mIdleRpm;
	/** This is the maximum rpm that the engine can rev up to, after this point engine input is blocked for a little time, see limiter time. */
	UPROPERTY(EditAnywhere, Category = "FGearEngine", DisplayName = "Limit Rpm")
	float mLimitRpm;
	/** When the rpm value reaches the limit value the throttle input is blocked for this long. It is measured in miliseconds. Experiment with it but keep in mind that if the engine inertia is too low, engine rpm will fluctuate a lot so you better keep it as low as possible */
	UPROPERTY(EditAnywhere, Category = "FGearEngine", DisplayName = "Limiter Time")
	float mLimiterTime;
	/** This effects the acceleration/deceleration of the engine. Inertia has direct effect on the acceleration of the vehicle so avoid too low or high values. To determine a good value you can rev up/down in neutral gear and see how it behaves */
	UPROPERTY(EditAnywhere, Category = "FGearEngine", DisplayName = "Engine Inertia")
	float mEngineInertia;
	/** Engine is considered to be stalling when the current rpm falls below this value. */
	UPROPERTY(EditAnywhere, Category = "FGearEngine", DisplayName = "Engine Stall Rpm")
	float mEngineStallRpm;
	/** If the engine keeps stalling this amount of time(in seconds) then the engine is forced to stop. The engine won't stall if this value is negative.  */
	UPROPERTY(EditAnywhere, Category = "FGearEngine", DisplayName = "Engine Stall Duration", meta = (ClampMin = "-1", ClampMax = "5", UIMin = "-1", UIMax = "5"))
	float mEngineStallTime;

	AFGearVehicle* mVehicle;
	UFGearTransmission* mTransmission;
	bool mRunning;
	float mRpm;
	float mThrottle;
	float mLimiter;
	float mTorque;
	float mFeedbackRpm;
	float mFeedbackTorque;
	float mIdleRpmTorque;
	float mPeakTorqueRpm;
	float mMatchRpm;
	float mMatchTime;
	float mMatchDelta;
	float mStallTime;
};