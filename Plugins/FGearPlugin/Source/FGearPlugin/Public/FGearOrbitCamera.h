// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FGearTelemetryDrawer.h"
#include "Camera/CameraTypes.h"
#include "FGearOrbitCamera.generated.h"

class AFGearVehicle;

UCLASS(meta = (BlueprintSpawnableComponent), ClassGroup = (FGear))
class FGEARPLUGIN_API UFGearOrbitCamera : public UFGearTelemetryDrawer
{
	GENERATED_BODY()
public:
	UFGearOrbitCamera();
	void update(float deltaTime, FMinimalViewInfo & outResult);
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "FGear")
	void injectMouseDX(float mx);
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void injectMouseDY(float my);
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void injectMouseDZ(float mz);
	
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setDriftMode(bool dm) { mDriftMode = dm; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setFreeze(bool f) { mFreeze = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setOrbitMode(bool orbit) { } // Legacy Blueprint hook; mouse movement now controls orbit mode.

	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setSpring(float s) { mSpring = s; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setDistance(float d) { mDistance = d; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setTargetZOffset(float z) { mTargetZOffset = z; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setTiltAngle(float t) { mTiltAngle = t; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setRotationOffset(float angle) { mRotationOffset = angle; }

private:
	AFGearVehicle* mTarget;
	UPROPERTY(EditAnywhere, Category = "FGearCamera", DisplayName = "Drift Mode")
	bool mDriftMode;
	UPROPERTY(EditAnywhere, Category = "FGearCamera", DisplayName = "Freeze")
	bool mFreeze;
	UPROPERTY(EditAnywhere, Category = "FGearCamera", DisplayName = "Spring")
	float mSpring;
	UPROPERTY(EditAnywhere, Category = "FGearCamera", DisplayName = "Distance")
	float mDistance;
	UPROPERTY(EditAnywhere, Category = "FGearCamera", DisplayName = "Target Z-Offset")
	float mTargetZOffset;
	UPROPERTY(EditAnywhere, Category = "FGearCamera", DisplayName = "Tilt Angle")
	float mTiltAngle;

	FVector mLastPos;
	float mRotationY;
	float mRotationOffset;
	float mCurrentTilt;
	float mMouseOrbitTimeRemaining;
};
