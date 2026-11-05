// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CollisionQueryParams.h"
#include "AIController.h"
#include "FGearAutoDrive.generated.h"

class UCurveFloat;
class AFGearVehicle;
class AFGearSpline;

UCLASS(ClassGroup = (FGear))
class FGEARPLUGIN_API AFGearAutoDrive : public AAIController
{
	GENERATED_BODY()
public:
	AFGearAutoDrive();
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* Property) const override;
#endif

	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool isActive() const { return mActive; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setActive(bool b);
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getCurrentSplineParam() const { return mCurrentSplineParam; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setSpline(AFGearSpline* s) { mSpline = s; }

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void myTick(float dt);
	void updateInputs(float dt);
	void applyInputs(float dt);
	void updateRaycasts(float dt, float speedCoeff);

	FCollisionQueryParams mTraceParams;
	AFGearVehicle* mVehicle;

	UPROPERTY(EditAnywhere, Category = "FGearAutoDrive|General", DisplayName = "Active")
	bool mActive;
	UPROPERTY(EditAnywhere, Category = "FGearAutoDrive|General", DisplayName = "Spline", meta = (EditCondition = "mActive"))
	AFGearSpline* mSpline;
	UPROPERTY(EditAnywhere, Category = "FGearAutoDrive|General", DisplayName = "Stuck Reset Time")
	float mStuckResetTime;
	UPROPERTY(EditAnywhere, Category = "FGearAutoDrive|General", DisplayName = "Reset Z-Offset")
	float mResetZOffset;

	UPROPERTY(EditAnywhere, Category = "FGearAutoDrive|Drive", DisplayName = "Speed By Angle Curve", meta = (EditCondition = "mActive"))
	UCurveFloat* mSpeedByAngleCurve;
	UPROPERTY(EditAnywhere, Category = "FGearAutoDrive|Drive", DisplayName = "Speed Coefficient Curve", meta = (EditCondition = "mActive"))
	UCurveFloat* mSpeedCoeffCurve;
	UPROPERTY(EditAnywhere, Category = "FGearAutoDrive|Drive", DisplayName = "Steering Distance", meta = (EditCondition = "mActive"))
	float mSteeringDistance;
	UPROPERTY(EditAnywhere, Category = "FGearAutoDrive|Drive", DisplayName = "Braking Distance", meta = (EditCondition = "mActive"))
	float mBrakingDistance;
	UPROPERTY(EditAnywhere, Category = "FGearAutoDrive|Drive", DisplayName = "Full Steer Angle", meta = (EditCondition = "mActive"))
	float mSteeringLimit;

	UPROPERTY(EditAnywhere, Category = "FGearAutoDrive|Avoidance", DisplayName = "Forward Check Distance", meta = (EditCondition = "mActive"))
	float mForwardCheckDistance;
	UPROPERTY(EditAnywhere, Category = "FGearAutoDrive|Avoidance", DisplayName = "Side Check Distance", meta = (EditCondition = "mActive"))
	float mSideCheckDistance;
	UPROPERTY(EditAnywhere, Category = "FGearAutoDrive|Avoidance", DisplayName = "Cross Check Distance", meta = (EditCondition = "mActive"))
	float mCrossCheckDistance;
	UPROPERTY(EditAnywhere, Category = "FGearAutoDrive|Avoidance", DisplayName = "Collision Avoid Distance", meta = (EditCondition = "mActive"))
	float mAvoidanceDistance;

	UPROPERTY(EditAnywhere, Category = "FGearAutoDrive|Debug", DisplayName = "Debug", meta = (EditCondition = "mActive"))
	bool mDebug;
	UPROPERTY(EditAnywhere, Category = "FGearAutoDrive|Debug", DisplayName = "Debug Thickness")
	float mDebugThickness;
	UPROPERTY(EditAnywhere, Category = "FGearAutoDrive|Debug", DisplayName = "Steering Debug Color")
	FColor mSteeringDebugColor;
	UPROPERTY(EditAnywhere, Category = "FGearAutoDrive|Debug", DisplayName = "Braking Debug Color")
	FColor mBrakingDebugColor;
	UPROPERTY(EditAnywhere, Category = "FGearAutoDrive|Debug", DisplayName = "Avoidance Debug Color")
	FColor mAvoidanceDebugColor;

	//inputs
	float mThrottleInput;
	float mBrakeInput;
	float mSteeringInput;
	float mNoVelocityTime;
	float mLateralDelta;
	float mCurrentSplineParam;
};