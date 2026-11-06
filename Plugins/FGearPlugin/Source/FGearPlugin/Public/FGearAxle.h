// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FGearTelemetryDrawer.h"
#include "FGearWheelOptions.h"
#include "FGearVehicleState.h"
#include "FGearAxle.generated.h"

class UFGearWheel;
class AFGearVehicle;
class UFGearEngine;
class UFGearTransmission;

UENUM()
enum class FGearDifferentialType : uint8
{
	DF_OPEN 	UMETA(DisplayName = "Open"),
	DF_LOCKED 	UMETA(DisplayName = "Locked"),
	DF_LSD		UMETA(DisplayName = "LSD")
};

UCLASS(meta = (BlueprintSpawnableComponent), ClassGroup = (FGear))
class FGEARPLUGIN_API UFGearAxle : public UFGearTelemetryDrawer
{
	GENERATED_BODY()
	friend class FGearSerializer;
public:
	UFGearAxle();

	void init(AFGearVehicle* v, int index);
	void reset();
	void preWheelupdate(bool firstStep);
	void postWheelupdate();
	float getTorque(UFGearWheel* w);
	float getSteeringInput();

	void saveState(FFGearVehicleState& state);
	void loadState(FFGearVehicleState& state);

	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getCurRpm() const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getMaxRpm() const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getFeedbackTorque() const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getWheelRPMToKMH() const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getNormalizedSteering() const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FVector getLocalCenter() const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FVector getGlobalCenter() const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getKMHSpeed() const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getRPM() const;
	float getGroundRPM() const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getTrackWidth() const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getDrivetrainFriction() const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool hasSteering() const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setTargetSteer(float s);

	UFUNCTION(BlueprintCallable, Category = "FGear")
	FGearDifferentialType getDifferentialType() const { return mDifferentialType; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UFGearWheelOptions* getWheelOptions() const { return mWheelOptions; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setWheelOptions(UFGearWheelOptions* wo) { mWheelOptions = wo; mWheelOptions->mAxle = this; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UFGearWheel* getLeftWheel() const { return mWheelLeft; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setLeftWheel(UFGearWheel* w) { mWheelLeft = w; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UFGearWheel* getRightWheel() const { return mWheelRight; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setRightWheel(UFGearWheel* w) { mWheelRight = w; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getTorqueShare() const { return mTorqueShare; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getFinalGearRatio() const { return mFinalGearRatio; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getMaxSteerAngle() const { return mMaxSteerAngle; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getAckermanCoeff() const { return mAckermanCoeff; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	int getAckermanReferenceIndex() const { return mAckermanReferenceIndex; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool hasHandBrake() const { return mHasHandbrake; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getDiffStrength() const { return mDiffStrength; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getCamberAngle() const { return mCamberAngle; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getToeAngle() const { return mToeAngle; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	int getIndex() const { return mIndex; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setDifferentialType(FGearDifferentialType dt) { mDifferentialType = dt; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void applyWheelOptions();
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setTorqueShare(float f) { mTorqueShare = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setFinalGearRatio(float f) { mFinalGearRatio = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setMaxSteerAngle(float f) { mMaxSteerAngle = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setAckermanCoeff(float f) { mAckermanCoeff = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setAckermanReferenceIndex(int i) { mAckermanReferenceIndex = i; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setHasHandBrake(bool b) { mHasHandbrake = b; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setDiffStrength(float f) { mDiffStrength = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setCamberAngle(float f) { mCamberAngle = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setToeAngle(float f) { mToeAngle = f; }

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* Property) const override;
#endif
	virtual FVector2D drawTelemetry(UCanvas* Canvas, UFont* Font, FLinearColor textColor, FFontRenderInfo renderInfo, FVector2D p) override;

private:
	void steeringUpdate();
	void calculateAxleTorque();
	float getOpenDiffTorque();
	float getLockedDiffTorque(UFGearWheel* w);

private:
	/** Currently open, locked and lsd differentials are supported. */
	UPROPERTY(EditAnywhere, Category = "FGearAxle", DisplayName = "Differential Type")
	FGearDifferentialType mDifferentialType;
	UFGearWheelOptions* mWheelOptions;
	UFGearWheel* mWheelLeft;
	UFGearWheel* mWheelRight;
	/** This parameter is only visible if lsd type is selected and determines the magnitude of the extra force that is applied to the gripping wheel */
	UPROPERTY(EditAnywhere, Category = "FGearAxle", DisplayName = "Differential Strength", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float mDiffStrength;
	/** The ratio of the torque received from engine. In reality the sum of the shares should be 1 but it is not constraint */
	UPROPERTY(EditAnywhere, Category = "FGearAxle", DisplayName = "Torque Share", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float mTorqueShare;
	/** The transmission value is calculated from the current gear ratio and this final gear ratio(also called final drive). In real cars this is the gear ratio between the the driveshaft and the axle which can determine the acceleration and the top speed of the vehicle. Higher values result faster accelerations and lower top speeds. */
	UPROPERTY(EditAnywhere, Category = "FGearAxle", DisplayName = "Final Gear Ratio")
	float mFinalGearRatio;
	/** Maximum angle(degree) of steering for this axles wheels. For non steering wheels just leave it at 0. */
	UPROPERTY(EditAnywhere, Category = "FGearAxle", DisplayName = "Max Steer Angle", meta = (ClampMin = "-90.0", ClampMax = "90.0", UIMin = "-90.0", UIMax = "90.0"))
	float mMaxSteerAngle;
	/** Ackerman steering values are automatically calculated from this axle and the reference axle. This value determines the rate this calculation is applied. Providing negative values for anti ackerman is also possible. */
	UPROPERTY(EditAnywhere, Category = "FGearAxle", DisplayName = "Ackerman Coefficient", meta = (ClampMin = "-1.0", ClampMax = "1.0", UIMin = "-1.0", UIMax = "1.0"))
	float mAckermanCoeff;
	/** Ackerman calculation is based on the current axle and the axle that is referenced by this index value. Normally this value is the index of the rear axle */
	UPROPERTY(EditAnywhere, Category = "FGearAxle", DisplayName = "Ackerman Reference Axle")
	int mAckermanReferenceIndex;
	/** If set this axles wheels are effected by handbrake */
	UPROPERTY(EditAnywhere, Category = "FGearAxle", DisplayName = "Has Handbrake")
	bool mHasHandbrake;
	/** Camber angle value in degrees. This value visually changes the wheels roll angle and is fed to the tire model */
	UPROPERTY(EditAnywhere, Category = "FGearAxle", DisplayName = "Camber Angle", meta = (ClampMin = "-10.0", ClampMax = "10.0", UIMin = "-10.0", UIMax = "10.0"))
	float mCamberAngle;
	/** Toe angle value in degrees. This value visually change the wheels yaw angle and modifies the direction of the force output of the wheel. */
	UPROPERTY(EditAnywhere, Category = "FGearAxle", DisplayName = "Toe Angle", meta = (ClampMin = "-10.0", ClampMax = "10.0", UIMin = "-10.0", UIMax = "10.0"))
	float mToeAngle;

	int mIndex;
	AFGearVehicle* mVehicle;
	UFGearEngine* mEngine;
	UFGearTransmission* mTransmission;

	//torque
	float mAxleTorque;
	float mLeftTorque;
	float mRightTorque;

	//steering
	float mCurrentSteerLeft;
	float mCurrentSteerRight;
	float mTargetSteer;
};