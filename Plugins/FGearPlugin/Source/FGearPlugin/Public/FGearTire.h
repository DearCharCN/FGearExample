// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Curves/CurveFloat.h"
#include "FGearTire.generated.h"

UENUM()
enum class FGearForceCombineMode : uint8
{
	GRIP 	UMETA(DisplayName = "Grip"),
	SIMPLE 	UMETA(DisplayName = "Simple"),
	SUM		UMETA(DisplayName = "Sum")
};

UCLASS(abstract, Blueprintable, BlueprintType)
class FGEARPLUGIN_API UFGearTire : public UObject
{
	GENERATED_BODY()
public:
	UFGearTire();
	virtual void init();
	virtual void calculate(float load, float slipRatio, float slipAngle, float camber, float pressure, float radius, float Vx);
	
	FGearForceCombineMode getCombineMode() const { return mCombineMode; }
	float getMaxLongitudinalForce() const { return mMaxLongitudinalForce; }
	float getMaxLongitudinalForceSlip() const { return mMaxLongitudinalForceSlip; }
	float getMaxLateralForce() const { return mMaxLateralForce; }
	float getMaxLateralForceAngle() const { return mMaxLateralForceAngle; }
	float getLongitudinalForce() const { return mLongitudinalForce; }
	float getLateralForce() const { return mLateralForce; }
	FVector getCombinedTorque() const { return mCombinedTorque; }

private:
	void calculateMaxLongitudinalForce();
	void calculateMaxLateralForce();
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& e) override;
#endif

protected:
	void updatePreviewCurve();

	UPROPERTY(VisibleAnywhere, Category = "FGearTire", DisplayName = "Longitudinal Preview")
	FRuntimeFloatCurve mLngCurve;
	UPROPERTY(VisibleAnywhere, Category = "FGearTire", DisplayName = "Lateral Preview")
	FRuntimeFloatCurve mLatCurve;
	UPROPERTY(BlueprintReadOnly, Category = "FGearTire", DisplayName = "Force Combine Mode")
	FGearForceCombineMode mCombineMode;
	UPROPERTY(EditAnywhere, Category = "FGearTire", DisplayName = "Reference Load")
	float mReferenceLoad;
	UPROPERTY(EditAnywhere, Category = "FGearTire", DisplayName = "Reference Tire Pressure")
	float mReferenceTirePressure;
	UPROPERTY(EditAnywhere, Category = "FGearTire", DisplayName = "Force Symmetry")
	bool mForceSymmetry;

	//output
	float mLongitudinalForce;
	float mLateralForce;
	FVector mCombinedTorque;

private:
	//extremas
	float mMaxLongitudinalForce;
	float mMaxLongitudinalForceSlip;
	float mMaxLateralForce;
	float mMaxLateralForceAngle;
};