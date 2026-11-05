// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "GameFramework/Pawn.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MAJOR_VERSION == 5
#include "PBDRigidsSolver.h"
#include "PhysicsProxy/SingleParticlePhysicsProxy.h"
#endif
#include "FGearUtility.h"
#include "FGearSerializer.h"
#include "FGearVehicleState.h"
#include "FGearVehicle.generated.h"

class UFGearEngine;
class UFGearTransmission;
class UFGearAxle;
class UFGearAeroDynamics;
class UFGearWheelOptions;
class UFGearWheel;
class UFGearAnimInstance;
class UFGearSkeletalMeshComponent;
class UFGearStandardInput;
class UFGearOrbitCamera;
class UFGearArcadeAssists;
class UFGearReplication;
class UFGearSuspensionConstraint;
#if WITH_EDITOR
class FReply;
#endif

USTRUCT()
struct FGearTelemetryOptions
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "FGearTelemetry", DisplayName = "Text Color")
	FLinearColor mTextColor;
	UPROPERTY(EditAnywhere, Category = "FGearTelemetry", DisplayName = "Show")
	bool mShow;
	UPROPERTY(EditAnywhere, Category = "FGearTelemetry", DisplayName = "Show Gauge")
	bool mShowGauge;
	UPROPERTY(EditAnywhere, Category = "FGearTelemetry", DisplayName = "Show Aero Dynamics")
	bool mShowAeroDynamics;
	UPROPERTY(EditAnywhere, Category = "FGearTelemetry", DisplayName = "Show Axles")
	bool mShowAxles;
	UPROPERTY(EditAnywhere, Category = "FGearTelemetry", DisplayName = "Show Engine")
	bool mShowEngine;
	UPROPERTY(EditAnywhere, Category = "FGearTelemetry", DisplayName = "Show Input")
	bool mShowInput;
	UPROPERTY(EditAnywhere, Category = "FGearTelemetry", DisplayName = "Show Transmission")
	bool mShowTransmission;
	UPROPERTY(EditAnywhere, Category = "FGearTelemetry", DisplayName = "Show Vehicle")
	bool mShowVehicle;
	UPROPERTY(EditAnywhere, Category = "FGearTelemetry", DisplayName = "Show Wheels")
	bool mShowWheels;
	UPROPERTY(EditAnywhere, Category = "FGearTelemetry", DisplayName = "Show Replication")
	bool mShowReplication;
	UPROPERTY(EditAnywhere, Category = "FGearTelemetry", DisplayName = "Show Statistics")
	bool mShowStats;
	UPROPERTY(EditAnywhere, Category = "FGearTelemetry", DisplayName = "Show Gizmos")
	bool mShowGizmos;
	UPROPERTY(EditAnywhere, Category = "FGearTelemetry", DisplayName = "Print Collision Logs")
	bool mPrintCollisionLogs;

	FGearTelemetryOptions() :
		mTextColor(FLinearColor::White),
		mShow(false),
		mShowGauge(true),
		mShowAeroDynamics(false),
		mShowAxles(false),
		mShowEngine(false),
		mShowInput(false),
		mShowTransmission(false),
		mShowVehicle(false),
		mShowWheels(false),
		mShowReplication(false),
		mShowStats(false),
		mShowGizmos(false),
		mPrintCollisionLogs(false)
	{
	}
};

#if ENGINE_MAJOR_VERSION == 5
class FFGearAsyncCallback : public Chaos::TSimCallbackObject<Chaos::FSimCallbackNoInput, Chaos::FSimCallbackNoOutput>
{
public:
	void init(AFGearVehicle* v) { mVehicle = v; }

private:
	virtual void OnPreSimulate_Internal() override;
	AFGearVehicle* mVehicle;
};
#endif

UENUM()
enum class FGearQueryType : uint8
{
	Line 		UMETA(DisplayName = "Line"),
	Sphere 		UMETA(DisplayName = "Sphere"),
	Convex 		UMETA(DisplayName = "Convex")
};

UENUM()
enum class FGearUpDirection : uint8
{
	X_UP 		UMETA(DisplayName = "X-UP"),
	Y_UP 		UMETA(DisplayName = "Y-UP"),
	Z_UP 		UMETA(DisplayName = "Z-UP")
};

UENUM()
enum class FGearQueryMobilityType : uint8
{
	Any 		UMETA(DisplayName = "Any"),
	Static 		UMETA(DisplayName = "Static"),
	Dynamic		UMETA(DisplayName = "Dynamic")
};

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class FGearWakeUpSource : uint8
{
	None = 0		UMETA(Hidden),
	Throttle = 1	UMETA(DisplayName = "Throttle"),
	Braking = 2		UMETA(DisplayName = "Braking"),
	Steering = 4	UMETA(DisplayName = "Steering"),
	Shifting = 8	UMETA(DisplayName = "Shifting")
};
ENUM_CLASS_FLAGS(FGearWakeUpSource);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGearSleepSignature, AFGearVehicle*, vehicle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGearWakeSignature, AFGearVehicle*, vehicle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGearInputSignature, float, deltaTime);

UCLASS(ClassGroup = (FGear))
class FGEARPLUGIN_API AFGearVehicle : public APawn
{
	GENERATED_BODY()
	friend class UFGearOrbitCamera;
	friend class FGearSerializer;
public:
	AFGearVehicle();
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
	virtual void PostInitializeComponents() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void OnRep_Controller() override;
	virtual void PostRenderFor(class APlayerController* PC, class UCanvas* Canvas, FVector CameraPosition, FVector CameraDir) override;
	virtual void PostInitProperties() override;
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool CanEditChange(const FProperty* Property) const override;
	FReply onSaveClicked();
	FReply onLoadClicked();
#endif
	virtual void CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaTime) override;
	void substepTick(float dt);

	FTransform getBoneTransform(FName nm) const;
	UStaticMesh* getConvexAsset();
	FGearUpDirection getConvexUp() const { return mConvexUp; }
	FRotator getConvexRotation() const { return mConvexRotation; }
	bool getConvexVisible() const { return mConvexVisible; }
	UStaticMesh* getDefaultCylinderAsset() const { return mDefaultCylinderAsset; }

	void addForce(FVector f);
	void addRelativeForce(FVector f);
	void addForceAtPosition(FVector f, FVector p);
	void addTorque(FVector t);
	void addImpulse(FVector i);
	void addImpulseAtPosition(FVector i, FVector p);
	void addImpulsiveTorque(FVector it);

	UFUNCTION(BlueprintCallable, Category = "FGear")
	void saveState(FFGearVehicleState& state);
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void loadState(FFGearVehicleState state);
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void reset(FVector position, FRotator rotation);
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void sleep();
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void wakeUp();
	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool getIsSleeping() const { return mInSleep; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getSleepLinearThreshold() const { return mSleepLinearThreshold; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setSleepLinearThreshold(const float threshold) { mSleepLinearThreshold = threshold; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getSleepAngularThreshold() const { return mSleepAngularThreshold; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setSleepAngularThreshold(const float threshold) { mSleepAngularThreshold = threshold; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getSleepWaitingTime() const { return mSleepWaitingTime; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setSleepWaitingTime(const float time) { mSleepWaitingTime = time; }
	bool canWakeByThrottle() const { return (mWakeUpSources & (uint8)FGearWakeUpSource::Throttle) != 0; }
	bool canWakeByBraking() const { return (mWakeUpSources & (uint8)FGearWakeUpSource::Braking) != 0; }
	bool canWakeBySteering() const { return (mWakeUpSources & (uint8)FGearWakeUpSource::Steering) != 0; }
	bool canWakeByShifting() const { return (mWakeUpSources & (uint8)FGearWakeUpSource::Shifting) != 0; }

	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setMass(float mass);
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setCenterOfMassOffset(FVector com);
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setInertiaScale(FVector iscale);
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setUseCustomGravity(bool use) { mCustomGravity = use; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setGravity(FVector gravity) { mGravity = gravity; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setBraking(float brakeInput, bool handbrake);
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setSteering(float steerInput);
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getNormalizedSteering() const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getSteerDeltaAngle(bool front) const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getMaxWheelKMHSpeed(bool onlyPowered, bool absValue) const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getMinWheelKMHSpeed(bool onlyPowered) const;
	float getFeedbackWheelRPM() const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getWheelSpeedLimitKMH() const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getMaxWheelRPMToKMH() const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getMaxSteeringAngle() const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getAxleDistance(int axle1, int axle2) const;
	float getTotalWheelLoad() const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool getABSActive() const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool getASRActive() const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	int getUpdateRate() const { return mUpdateRate; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getMass() const { return mMass; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FVector getInertiaScale() const { return mInertiaScale; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FVector getLocalInertiaTensor() const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FVector getCenterofMass() const { return mCenterofMass; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	int getRayCount() const { return mRays; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	ECollisionChannel getTraceChannel() const { return mTraceChannel; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	uint8 getIgnoreMask() const { return mIgnoreMask; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool getTraceComplex() const { return mTraceComplex; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	int getLateralRayCount() const { return mLateralRays; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FGearQueryType getQueryType() const { return mQueryType; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FGearQueryMobilityType getQueryMobilityType() const { return mQueryMobilityType; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getHandbrakePower() const { return mHandBrakePower; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getRollingResistanceCoeff() const { return mRollingResistanceCoeff; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getDrivetrainInertia() const { return mDrivetrainInertia; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getDrivetrainFriction() const { return mDrivetrainFriction; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getReverseBrakeSpeed() const { return mReverseBrakeSpeed; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool getAllowReverse() const { return mAllowReverse; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool getDetectGround() const { return mDetectGround; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getOptimumSlipWeight() const { return mOptimumSlipWeight; }
	float getOptimumSlipState() const { return mOptimumSlipState; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getHardContactScale() const { return mHardContactScale; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool getRelaxationDownforce() const { return mRelaxationDownforce; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool getTirePenetration() const { return mTirePenetration; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getABS() const { return 0.01f * mABS; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getMinAbsSpeed() const { return mMinAbsSpeed; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getASR() const { return 0.01f * mASR; }
	FBodyInstance* getBody() const { return mBody; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UFGearSkeletalMeshComponent* getMesh() const { return mMesh; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UFGearAnimInstance* getAnim() const { return mAnimation; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UFGearEngine* getEngine() const { return mEngine; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UFGearTransmission* getTransmission() const { return mTransmission; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UFGearStandardInput* getStandardInput() const { return mStandardInput; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UFGearAeroDynamics* getAeroDynamics() const { return mAeroDynamics; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UFGearAxle* getAxle(int i) const { return mAxleList[i]; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FVector getPosition() const { return GetActorLocation(); }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FQuat getRotation() const { return GetActorRotation().Quaternion(); }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FVector getForwardDir() const { return GetActorForwardVector(); }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FVector getUpDir() const { return GetActorUpVector(); }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FVector getMyVelocity() const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FVector getVelocityDir() const { return getMyVelocity().GetSafeNormal(); }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	int getWheelCount() const { return 2 * mAxleList.Num(); }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	int getAxleCount() const { return mAxleList.Num(); }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getForwardSpeed() const { return FGearUtility::cm2meter * GetActorTransform().InverseTransformVector(getMyVelocity()).X; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getRightSpeed() const { return FGearUtility::cm2meter * GetActorTransform().InverseTransformVector(getMyVelocity()).Y; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getKMHSpeed() const { return getForwardSpeed() * FGearUtility::ms2kmh; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getVelocitySize() const { return getMyVelocity().Size() * FGearUtility::cm2meter; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getMassPerWheel() const { return mBody->GetMassOverride() / getWheelCount(); }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getSimCoeff() const { return mSimCoeff; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool isHandbrakeOn() const { return mHandbrakeOn; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool getESPActive() const { return mEspActive; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool isOverSteering() const { return mOverSteering; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool isUnderSteering() const { return mUnderSteering; }
	bool isSubstepping() const { return mSubstepping; }
	float getStickyTireState() const { return mStickyTireState; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setUpdateRate(int i) { mUpdateRate = i; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setRayCount(int ct) { mRays = ct; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setLateralRayCount(int ct) { mLateralRays = ct; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setTraceChannel(ECollisionChannel l) { mTraceChannel = l; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setIgnoreMask(uint8 f) { mIgnoreMask = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setTraceComplex(bool b) { mTraceComplex = b; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setQueryType(FGearQueryType t) { mQueryType = t; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setQueryMobilityType(FGearQueryMobilityType t) { mQueryMobilityType = t; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setHandbrakePower(float f) { mHandBrakePower = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setRollingResistanceCoeff(float f) { mRollingResistanceCoeff = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setDrivetrainInertia(float f) { mDrivetrainInertia = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setDrivetrainFriction(float f) { mDrivetrainFriction = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setReactionForceScale(float f) { mReactionScale = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setStickyTireSpeed(float f) { mStickyTireSpeed = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setStickyTireState(float f) { mStickyTireState = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setOptimumSlipWeight(float f) { mOptimumSlipWeight = FMath::Clamp(f, 0.0f, 1.0f); }
	void setOptimumSlipState(float f) { mOptimumSlipState = FMath::Clamp(f, 0.0f, 1.0f); }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setReverseBrake(float f) { mReverseBrakeSpeed = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setAllowReverse(bool allow) { mAllowReverse = allow; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setDetectGround(bool b) { mDetectGround = b; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setHardContactScale(float f) { mHardContactScale = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setRelaxationDownforce(bool b) { mRelaxationDownforce = b; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setTirePenetration(bool b) { mTirePenetration = b; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setABS(float f) { mABS = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setMinAbsSpeed(float f) { mMinAbsSpeed = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setASR(float f) { mASR = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setUnderSteerAssist(float f) { mUnderSteerAssist = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setMinUnderSteerAngle(float f) { mMinUnderSteerAngle = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setOverSteerAssist(float f) { mOverSteerAssist = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setMinOverSteerAngle(float f) { mMinOverSteerAngle = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setFrontAntiRollPower(float f) { mFrontAntiRollPower = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setRearAntiRollPower(float f) { mRearAntiRollPower = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setUpdateInputs(bool b) { mUpdateInputs = b; }

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FGearSleepSignature onSleepEvent;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FGearWakeSignature onWakeEvent;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FGearInputSignature onOverrideInput;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void myFixedUpdate(int i, float dt, float dts);
	void updateESP();
	void updateAntiRollBar();
	void handleHardContacts(float dt);
	void updateStats(float dt);
	float getCurrentSteering() const;
	void recalcRays();
	void initializeAxles(TArray<USceneComponent*> components, int index);

	void prepareTelemetry();
	UFUNCTION(Reliable, Client)
	void clientPrepareTelemetry();

	//replicated reset functions
	void localReset(FVector position, FQuat rotation);
	UFUNCTION(Reliable, NetMulticast)
	void clientReset(FVector position, FQuat rotation);
	UFUNCTION(Reliable, Server)
	void serverReset(FVector position, FQuat rotation);

	bool updateSleeping(float dt);
	void handleSleep();
	void handleWakeUp();
	UFUNCTION(Reliable, Server)
	void serverSleep();
	UFUNCTION(Reliable, Server)
	void serverWakeUp();
	UFUNCTION()
	void onWake(UPrimitiveComponent* component, FName bone);
	UFUNCTION()
	void onRepInSleep();

	FVector2D drawTelemetryVehicle(UCanvas* Canvas, UFont* Font, FLinearColor textColor, FFontRenderInfo renderInfo, FVector2D p);
	FVector2D drawTelemetryStats(UCanvas* Canvas, UFont* Font, FLinearColor textColor, FFontRenderInfo renderInfo, FVector2D p);
	FVector2D drawTelemetryWheels(UCanvas* Canvas, UFont* Font, FLinearColor textColor, FFontRenderInfo renderInfo, FVector2D p);
	FVector2D drawTelemetryGauge(UCanvas* Canvas, UFont* Font, FLinearColor textColor, FFontRenderInfo renderInfo, FVector2D p);

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "FGearVehicle", DisplayName = "Engine")
	UFGearEngine* mEngine;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "FGearVehicle", DisplayName = "Transmission")
	UFGearTransmission* mTransmission;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "FGearVehicle", DisplayName = "Standard Input")
	UFGearStandardInput* mStandardInput;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "FGearVehicle", DisplayName = "Aero Dynamics")
	UFGearAeroDynamics* mAeroDynamics;
	UPROPERTY(NoClear)
	UFGearOrbitCamera* mOrbitCamera;
	UPROPERTY(NoClear)
	UFGearArcadeAssists* mArcadeAssists;
	UPROPERTY(NoClear)
	UFGearSuspensionConstraint* mSuspensionConstraint;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "FGearVehicle", DisplayName = "Front Axle")
	UFGearAxle* mAxle0;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "FGearVehicle", DisplayName = "Front Wheel Options")
	UFGearWheelOptions* mWheelOptions0;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "FGearVehicle", DisplayName = "Front Wheel Left")
	UFGearWheel* mWheelLeft0;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "FGearVehicle", DisplayName = "Front Wheel Right")
	UFGearWheel* mWheelRight0;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "FGearVehicle", DisplayName = "Rear Axle")
	UFGearAxle* mAxle1;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "FGearVehicle", DisplayName = "Rear Wheel Options")
	UFGearWheelOptions* mWheelOptions1;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "FGearVehicle", DisplayName = "Rear Wheel Left")
	UFGearWheel* mWheelLeft1;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "FGearVehicle", DisplayName = "Rear Wheel Right")
	UFGearWheel* mWheelRight1;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "FGearVehicle", DisplayName = "Replication")
	UFGearReplication* mReplication;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "FGearVehicle", DisplayName = "Mesh")
	UFGearSkeletalMeshComponent* mMesh;
	UPROPERTY()
	UFGearAnimInstance* mAnimation;

	/** This parameter sets the update frequency of the vehicles so even if the frame rate is 60hz and the update rate is 300hz then the system makes 5 sub steps to make sure you will get a 300hz update rate. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|General", DisplayName = "Update Rate(Hz)", meta = (ClampMin = "100", ClampMax = "1000", UIMin = "100", UIMax = "1000"))
	int mUpdateRate;
	/** This is the mass of the vehicle body in kilograms and this value is set as the mass of the rigidbody component. Keep it close to realistic values. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|General", DisplayName = "Mass")
	float mMass;
	/** This value is a direct multiplier to the rigidbody inertia. Higher values result slower accelerations and more conservation of momentum. Experiment with it as you like. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|General", DisplayName = "Inertia Scale")
	FVector mInertiaScale;
	/** You can use multiple colliders with various sizes to achieve the desired effect which gives you the freedom to position center of mass as you like and usually you do not need to alter this setting. Center of mass has a dramatic effect on the handling of the vehicle so you can further tweak it with this parameter. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|General", DisplayName = "Center of Mass")
	FVector mCenterofMass;
	/** When active disables physics engine gravity and uses the custom gravity vector to apply a custom acceleration. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|General", DisplayName = "Use Custom Gravity")
	bool mCustomGravity;
	/** Gravitational acceleration vector to be used when custom gravity is active. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|General", DisplayName = "Gravity", meta = (EditCondition = "mCustomGravity"))
	FVector mGravity;
	/** Line tracing uses regular raycasting approach. Sphere casting uses a sphere collider to make a sweep test. Both approaches have up and down sides. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|Tracing", DisplayName = "Query Type")
	FGearQueryType mQueryType;
	/** This represents the number of rays that are casted for each wheel. If you need a 3d wheel effect you can increase this value. Keep in mind that the higher number of raycasts, the worse performance you get. For a racing game its better to keep its value at 1. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|Tracing", DisplayName = "Ray Count", meta = (ClampMin = "1", ClampMax = "49", UIMin = "1", UIMax = "49"))
	int mRays;
	/** If you need a wheel with real width than setting this value to 2 or 3 will multiply the Ray Count value and the origins of raycasts are offset to left and right according to the wheel width. For a racing game its better to keep its value at 1. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|Tracing", DisplayName = "Lateral Ray Count", meta = (ClampMin = "1", ClampMax = "3", UIMin = "1", UIMax = "3"))
	int mLateralRays;
	/** Select to query against static or dynamic or both. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|Tracing", DisplayName = "Query Mobility Type")
	FGearQueryMobilityType mQueryMobilityType;
	/** This is the collision channel parameter of LineTraceSingleByChannel function. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|Tracing", DisplayName = "Query Trace Channel")
	TEnumAsByte<ECollisionChannel> mTraceChannel;
	/** Extra filtering parameter of type FMaskFilter. This filter allows us to refine queries (channel, object) with an additional level of ignore by tagging entire classes of objects. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|Tracing", DisplayName = "Query Ignore Mask")
	uint8 mIgnoreMask;
	/** Whether we should trace against complex(mesh) collision. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|Tracing", DisplayName = "Trace Complex")
	bool mTraceComplex;
	/** Static mesh asset to be used for convex sweep queries. By default it is set to a simple cylinder mesh that is included in the plugin. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|Tracing", DisplayName = "Convex Cast Shape")
	UStaticMesh* mConvexAsset;
	/** The convex shape is scaled based on wheel radius and width. This value defines which axis will be scaled with the width of the wheel. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|Tracing", DisplayName = "Convex Shape Up")
	FGearUpDirection mConvexUp;
	/** The convex shape's local orientation is defined with this value. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|Tracing", DisplayName = "Convex Shape Rotation")
	FRotator mConvexRotation;
	/** Can be used for debugging purposes, this lets you render the convex shape that is used for wheel queries. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|Tracing", DisplayName = "Convex Shape Visible")
	bool mConvexVisible;
	UPROPERTY()
	UStaticMesh* mDefaultCylinderAsset;

	/** If set the wheels will detect the velocity of the ground and take it into account when calculating velocity. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|Simulation", DisplayName = "Detect Ground Velocity")
	bool mDetectGround;
	/** Suppose you are climbing a ramp in 1st gear and you release the throttle and wait for the car to slow down and stop. Since there is no engine stalling the car will start going backwards as if you have pressed the clutch if this value is set. If not set the wheels will resist going backwards */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|Simulation", DisplayName = "Allow Wheel Rotation Against Engine")
	bool mAllowReverse;
	/** It is not always possible to keep the suspension compression within limits. If the suspension is compressed beyond its limits then there are two possibilities. If this is checked the tires will penetrate the ground else the tires will stay on ground but can penetrate the fenders. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|Simulation", DisplayName = "Suspension Tire Penetration")
	bool mTirePenetration;
	/** If this is checked the relaxation damper can generate forces that pulls the vehicle towards the ground. This is mostly unnoticable at low values but if you provide a large value to relexation damper then you can observe that the vehicle will tend to stick to the ground even on bumpy surfaces. This is unrealistic but can be useful in certain cases where you need stable vehicles on rough terrain. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|Simulation", DisplayName = "Suspension Relaxation Downforce")
	bool mRelaxationDownforce;
	/** If set the suspensions will generate an extra force after being fully compressed. This extra force will try to prevent the suspension from compressing more then its limit. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|Simulation", DisplayName = "Suspension Hard Contact Scale", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float mHardContactScale;
	/** Normally brake powers are set per axle but if you want the rear wheels to lock quicker with handbrake you can increase this value. This has little to no effect on actual braking power but wheels lock more easily. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|Simulation", DisplayName = "HandBrake Power", meta = (ClampMin = "1.0", ClampMax = "100.0", UIMin = "1.0", UIMax = "100.0"))
	float mHandBrakePower;
	/** If the vehicles intended moving direction does not match its velocity the vehicle will automatically brake when the current speed is above this value. For ex. the vehicle is in first gear but it is moving backwards, when you press the throttle it will engage the brakes for you. The same goes for the reverse gear. This will be disabled when zero. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|Simulation", DisplayName = "Reverse Brake Speed")
	float mReverseBrakeSpeed;
	/** This is the inherent friction factor of wheels. Applied friction force increases linearly as the wheels rotate faster. You may adjust it to limit the maximum speed but be aware that it also decreases the acceleration of the vehicle */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|Simulation", DisplayName = "Rolling Resistance Coefficient")
	float mRollingResistanceCoeff;
	/** This used when calculating the acceleration/deceleration of the wheels based on applied torque. Higher inertia will result slower accelerations. This has an effect similar to wheel mass values. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|Simulation", DisplayName = "DrivetrainInertia")
	float mDrivetrainInertia;
	/** Wheels decelerate based on effects like engine braking, rolling resistance and aerodynamic drag. This is another factor that resists the wheel rotation. Unlike rolling resistance this is a constant factor. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|Simulation", DisplayName = "DrivetrainFriction")
	float mDrivetrainFriction;
	/** Pacejka tire model does not work well at low speeds. When the velocity gets too small the calculations get problematic so tires can slide slowly on a slope even if the vehicle is fully stopped. This is an experimental hacky solution to solve this issue. When the vehicles velocity is below this speed(kmh) the tires will switch to sticky state. If you keep this value at 0 then it is disabled. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|Simulation", DisplayName = "Sticky Tire Max Speed", meta = (ClampMin = "0.0", ClampMax = "5.0", UIMin = "0.0", UIMax = "5.0"))
	float mStickyTireSpeed;
	/** Tires generate friction forces to move the vehicle and suspensions generate forces to keep the vehicle away from ground. If you want the generated forces to be applied back to dynamic objects in your world then set this variable to 1. Sometimes generated forces are too large for objects with low mass so you can use a smaller value instead. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|Simulation", DisplayName = "Wheel Reaction Force Scale", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float mReactionScale;
	/** Experimental feature, use with caution. Optimum slip is the value at which the tire generates the greatest friction force. This basically adjusts the slip ratio to prevent it from going below the optimum value. The result is that the engine rpm and the wheel speeds will be at perfect match but having a constant slip value can effect the overall lateral traction too. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|Simulation", DisplayName = "Optimum Slip Weight", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float mOptimumSlipWeight;

	UPROPERTY(EditAnywhere, Category = "FGearVehicle|Telemetry", DisplayName = "Telemetry")
	FGearTelemetryOptions mTelemetry;
	/** The antilock braking system helps to prevent wheels from locking up when braking */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|DriveAssists", DisplayName = "ABS", meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
	float mABS;
	/** The antilock braking system will not be active below this speed. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|DriveAssists", DisplayName = "Minumum ABS Speed", meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
	float mMinAbsSpeed;
	/** Traction control assist helps to prevent loss of traction of driven wheels when accelerating. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|DriveAssists", DisplayName = "ASR", meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
	float mASR;
	/** Steering assist tries to steer towards the direction that the vehicle body travels. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|DriveAssists", DisplayName = "Under Steer Assist", meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
	float mUnderSteerAssist;
	/** If the angle between the steering direction and the vehicles velocity direction is above this value, under steer assist is activated. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|DriveAssists", DisplayName = "Min Under Steer Angle", meta = (ClampMin = "1.0", ClampMax = "90.0", UIMin = "1.0", UIMax = "90.0"))
	float mMinUnderSteerAngle;
	/** Helps to prevent oversteering by using brakes like an esp system. This only works for 4 wheel vehicles */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|DriveAssists", DisplayName = "Over Steer Assist", meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
	float mOverSteerAssist;
	/** If the angle between the steering direction and the vehicles velocity direction is above this value, over steer assist is activated */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|DriveAssists", DisplayName = "Min Over Steer Angle", meta = (ClampMin = "1.0", ClampMax = "90.0", UIMin = "1.0", UIMax = "90.0"))
	float mMinOverSteerAngle;
	/** Anti roll bars help to reduce body roll during fast cornering. Front antirollbars help to reduce oversteering. This only works for 4 wheel vehicles */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|DriveAssists", DisplayName = "Front Anti Roll Power", meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
	float mFrontAntiRollPower;
	/** Rear anti rollbars help to reduce understeering. This only works for 4 wheel vehicles. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|DriveAssists", DisplayName = "Rear Anti Roll Power", meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
	float mRearAntiRollPower;

	/** The minimum linear velocity(km/h) threshold required for the vehicle to enter sleep mode */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|Optimization", DisplayName = "Sleep Linear Threshold")
	float mSleepLinearThreshold;
	/** The minimum angular velocity(deg/s) threshold required for the vehicle to enter sleep mode. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|Optimization", DisplayName = "Sleep Angular Threshold")
	float mSleepAngularThreshold;
	/** The time needed to fall asleep in seconds. */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|Optimization", DisplayName = "Sleep Waiting Time")
	float mSleepWaitingTime;
	/** Filter the wake source. One or more wake sources can be used together */
	UPROPERTY(EditAnywhere, Category = "FGearVehicle|Optimization", meta = (Bitmask, BitmaskEnum = "/Script/FGearPlugin.FGearWakeUpSource"), DisplayName = "Wake Up Sources")
	uint8 mWakeUpSources;
	UPROPERTY(Transient, ReplicatedUsing = onRepInSleep)
	bool mInSleep;
	float mSleepTime;
	FVector mLastLinearVelocity;
	FVector mLastAngularVelocity;
	
#if ENGINE_MAJOR_VERSION == 5
	FFGearAsyncCallback* mAsyncCallback;
#endif

	FBodyInstance* mBody;
	TArray<UFGearAxle*> mAxleList;
	TMap<FName, FTransform> mBoneTransforms;
	TArray<FVector> mWheelForces;
	TArray<FVector> mWheelTorques;
	FMatrix mLocalIIT;
	FVector mNetForce;
	FVector mNetTorque;
	FVector mNetImpulse;
	FVector mNetImpulseTorque;

	bool mHandbrakeOn;
	bool mEspActive;
	bool mUnderSteering;
	bool mOverSteering;
	bool mSubstepping;
	bool mUpdateInputs;
	float mStickyTireState;
	float mOptimumSlipState;
	float mBrakeInput;

	//update
	float mSimCoeff;
	float mTime;

	//statistics
	float mZero100Time;
	float mZero200Time;
	float m100ZeroDistance;
	bool mGot100Result;
	bool mGot200Result;
	bool mGotZeroResult;
};