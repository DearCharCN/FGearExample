// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CollisionShape.h"
#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
#include "Engine/HitResult.h"
#endif
#include "FGearTelemetryDrawer.h"
#include "FGearVehicleState.h"
#include "FGearUtility.h"
#include "FGearWheel.generated.h"

class AFGearVehicle;
class UFGearAxle;
class UFGearTire;
class UFGearAnimInstance;

UENUM()
enum class FGearWheelSide : uint8
{
	WS_LEFT 	UMETA(DisplayName = "Left"),
	WS_RIGHT 	UMETA(DisplayName = "Right")
};

UCLASS(meta = (BlueprintSpawnableComponent), ClassGroup = (FGear))
class FGEARPLUGIN_API UFGearWheel : public UFGearTelemetryDrawer
{
	GENERATED_BODY()
	friend class FGearSerializer;
public:
	UFGearWheel();
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* Property) const override;
#endif
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

	void init(AFGearVehicle* v, UFGearAxle* axle);
	void refreshParameters();
	void refreshVehicleReferences();
	void reset();
	void myFixedUpdate(float dt, float dts, int step, FVector& force, FVector& torque);
	void visualsUpdate(float dt);

	FVector combineLegacy();
	FVector combinePOR();
	FVector combineSum();
	void applyVerticalForce(float force);

	void saveState(FFGearVehicleState& state);
	void loadState(FFGearVehicleState& state);

	UFUNCTION(BlueprintCallable, Category = "FGear")
	FGearWheelSide getWheelSide() const { return mSide; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UFGearAxle* getAxle() const { return mAxle; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FHitResult getRayHit() const { return mRayHit; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FVector getHubPosition() const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FVector getLocalHubPosition() const { return mLocalHubPosition; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FVector getWheelPosition() const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FQuat getWheelRotation() const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FVector getLongitudinalForce() const { return mLongitudinalForce; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FVector getLateralForce() const { return mLateralForce; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FVector getCombinedTorque() const { return mCombinedTorque; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FVector getGlobalForward() const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FVector getGlobalRight() const;
	FVector getGroundVelocity() const { return mGroundVelocity; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FVector getLocalSpring() const { return mLocalSpringForce; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getGroundSpeed() const { return mLocalVelocity.X / mRadius; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getRadius() const { return mRadius; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getPressure() const { return mPressure; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getSpeed() const { return mRealSpeed; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getRpm() const { return mRealSpeed * FGearUtility::rads2Rpm; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getGroundRpm() const { return getGroundSpeed() * FGearUtility::rads2Rpm; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getKMHSpeed() const { return getRpm() * mWheelRPMToKMH; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getWheelRPMToKMH() const { return mWheelRPMToKMH; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getLongitudinalSlip() const { return mLongitudinalSlip; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getLateralSlip() const { return mLateralSlip; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getSlipRatio() const { return mSlipRatio; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getSlipAngle() const { return mSlipAngle; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getCurrentLoad() const { return mCurrentLoad; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getSuspensionTotalLength() const { return mSuspensionUpTravel + mSuspensionDownTravel; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getSuspensionCompressedLength() const { return mCompressedLength; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getSuspensionSpring() const { return mSuspensionSpring; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getSuspensionCompressRatio() const { return mCompressRatio; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getFeedbackTorque() const { return mFeedbackTorque; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getBraking() const { return mBraking; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getFrictionFactor() const { return mFrictionFactor; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getAdditionalDrag() const { return mAdditionalDrag; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getInherentDrag() const { return mInherentDrag; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getCurrentSteer() const { return mCurrentSteer; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getPitch() const { return -FMath::RadiansToDegrees(mPitch); }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool isEnabled() const { return mIsActive; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool hasContact() const { return mHasContact; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool hasDrive() const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool isABSActive() const { return mABSActive; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool isASRActive() const { return mASRActive; }
	
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setEnable(bool b) { mIsActive = b; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setWheelSide(FGearWheelSide s) { mSide = s; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setSpeed(float s) { mRealSpeed = s; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setCurrentSteer(float s) { mCurrentSteer = s; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setBraking(float b) { mBraking = b; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setFrictionFactor(float f) { mFrictionFactor = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setAdditionalDrag(float f) { mAdditionalDrag = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setInherentDrag(float f) { mInherentDrag = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setSpringCoeff(float f) { mSpringCoeff = f; }
	
	//individual setters for wheel options
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setLatFriction(float f) {  mLatFriction = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setLngFriction(float f) {  mLngFriction = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setLatScale(float f) { mLatScale = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setLngScale(float f) { mLngScale = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setOverturnScale(float f) { mOverturnScale = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setRollScale(float f) { mRollScale = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setAlignScale(float f) { mAlignScale = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setRadius(float f);
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setWidth(float f) { mWidth = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setWheelOffsets(float forward, float spacer, float vertical, float rim);
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setBoneName(const FString& NewName);
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setMass(float f) { mMass = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setPressure(float f) { mPressure = FMath::Max(2.0f * FGearUtility::epsilon, f); }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setBrakeTorque(float f) { mMaxBrakeTorque = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setSuspensionUpTravel(float f) { mSuspensionUpTravel = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setSuspensionDownTravel(float f) { mSuspensionDownTravel = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setSuspensionSpring(float f) { mSuspensionSpring = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setCompressionDamper(float f) { mCompressionDamper = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setRelaxationDamper(float f) { mRelaxationDamper = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setSuspensionPreload(float f);

	virtual FVector2D drawTelemetry(UCanvas* Canvas, UFont* Font, FLinearColor textColor, FFontRenderInfo renderInfo, FVector2D p) override;
	void drawGizmo(FColor color);

private:
	void initConvexCaster();
	void setConvexTransform();
	void updateComponentTransform();
	void suspensionUpdate(float dt);
	void updatePhysicsTransform();
	void updateLocalVelocity();
	void stickyTireUpdate();
	void updateWheelTorque();
	void updateWheelSpeed(float dt);
	void updateSlipValues();
	void updateTireFrictionValues();
	void applyRpmLimit(float dt);
	void updateWheelFriction(float dt);
	FVector calcWheelDragForce(float dt);
	FVector getGlobalVelocity() const;
	FVector getLocalVelocity() const;

private:
	UPROPERTY(EditAnywhere, Category = "FGearWheel", DisplayName = "Enabled")
	bool mIsActive;
	UPROPERTY(EditDefaultsOnly, Category = "FGearWheel", DisplayName = "Side")
	FGearWheelSide mSide;
	UPROPERTY(EditAnywhere, Category = "FGearWheel", DisplayName = "Bone Name")
	FName mBoneName;
	UPROPERTY()
	UStaticMeshComponent* mConvexShape;
	UFGearAnimInstance* mAnim;

	//wheel options
	float mLatFriction;
	float mLngFriction;
	float mLatScale;
	float mLngScale;
	float mOverturnScale;
	float mRollScale;
	float mAlignScale;
	float mRadius;
	float mWidth;
	float mForwardOffset;
	float mSpacerOffset;
	float mVerticalOffset;
	float mRimOffset;
	float mCastOffset;
	float mMass;
	float mPressure;
	float mMaxBrakeTorque;
	float mSuspensionUpTravel;
	float mSuspensionDownTravel;
	float mSuspensionSpring;
	float mCompressionDamper;
	float mRelaxationDamper;
	float mSuspensionPreload;

	//objects
	AFGearVehicle* mVehicle;
	UFGearAxle* mAxle;
	UFGearTire* mTire;
	FBodyInstance* mBody;
	FHitResult mRayHit;
	FCollisionShape mColShape; //for sphere casting
	FTransform mPhysicsTrans; //for chaos
	FVector mLocalHubPosition;

	//forces
	FVector mSpringForce;
	FVector mLocalSpringForce;
	FVector mLateralForce;
	FVector mLongitudinalForce;
	FVector mCombinedTorque;
	FVector mLocalVelocity;
	FVector mGroundVelocity;

	//suspension
	float mSpringCoeff; //adjusted for roll stiffness
	float mCompressedLength;
	float mLastCompressLength;
	float mCompressRatio;
	float mCurrentLoad;
	bool mHasContact;

	//steering
	float mCurrentSteer;
	float mToeAngle;
	float mCamberAngle;
	float mPitch;
	FQuat mSteerQuat;

	//friction
	float mLateralSlip;
	float mLongitudinalSlip;
	float mSlipRatio;
	float mSlipAngle;
	float mLngSlipOverride;
	float mS, mA, mP; //The Physics of Racing, Part 25: Combination Grip
	float mFrictionFactor;
	float mLatFrictionForce;
	float mLngFrictionForce;

	//drag
	float mInherentDrag;
	float mAdditionalDrag;

	//feedback
	float mFeedbackTorque;
	bool mABSActive;
	bool mASRActive;
	bool mHandbrakeActive;

	//others
	bool mAccelerating;
	float mRealSpeed; //rad/s
	float mWheelTorque;
	float mBrakeTorque;
	float mCircumference;
	float mWheelRPMToKMH;
	float mBraking;
	float mSpeedOverflow;
};