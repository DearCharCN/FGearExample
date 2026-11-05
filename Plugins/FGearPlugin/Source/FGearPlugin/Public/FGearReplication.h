// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FGearTelemetryDrawer.h"
#include "FGearVehicleState.h"
#include "FGearReplication.generated.h"

class AFGearVehicle;
class UCurveFloat;

USTRUCT()
struct FGearSnapState
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY()
	float mTime;
	UPROPERTY()
	FVector mPosition;
	UPROPERTY()
	FQuat mRotation;
	UPROPERTY()
	FVector mVelocity;
	UPROPERTY()
	FVector mAngVelocity;

	FGearSnapState() 
		: mTime(0)
		, mPosition(FVector::ZeroVector)
		, mRotation(FQuat::Identity)
		, mVelocity(FVector::ZeroVector)
		, mAngVelocity(FVector::ZeroVector)
	{}
};

UCLASS()
class FGEARPLUGIN_API UFGearReplication : public UFGearTelemetryDrawer
{
	GENERATED_BODY()
	friend class FGearSerializer;
public:
	UFGearReplication();
	void init(AFGearVehicle* v);
	void myUpdate(float dt);
	void reset(FVector position, FQuat rotation);

	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool isEnabled() const { return mEnabled; }
	bool isServerAuth() const { return mServerAuthority; }
	float getMinCollisionBlendSpeed() const { return mMinCollisionBlendSpeed; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void resetCollisionTime(float thresholdTime = 0.5f) { if (mCollisionTime > thresholdTime) mCollisionTime = 0.0f; }

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual FVector2D drawTelemetry(UCanvas* Canvas, UFont* Font, FLinearColor textColor, FFontRenderInfo renderInfo, FVector2D p) override;

private:
	void updateInputs(float dt);
	void updateInterpolation(float dt);
	void sendInputs(float dt);
	void sendStates(float dt);
	void getDeadReckoning(FVector& pos, FQuat& rot, float dt);
	
	/** If set we can override common replication settings. */
	UPROPERTY(EditAnywhere, Category = "FGearReplication", DisplayName = "OverrideSettings")
	bool mOverrideSettings;

	/** If unchecked fgear replication will not have any effect. */
	UPROPERTY(EditAnywhere, Category = "FGearReplication", DisplayName = "Enabled", meta = (EditCondition = "mOverrideSettings"))
	bool mEnabled;
	bool mServerAuthority;

	/** Network send rate for inputs. */
	UPROPERTY(EditAnywhere, Category = "FGearReplication", DisplayName = "Input Sendrate", meta = (ClampMin = "0", ClampMax = "100", UIMin = "0", UIMax = "100", EditCondition = "mEnabled && mOverrideSettings"))
	uint8 mInputSendRate;
	
	/** Network send rate for transforms. */
	UPROPERTY(EditAnywhere, Category = "FGearReplication", DisplayName = "State Sendrate", meta = (ClampMin = "0", ClampMax = "100", UIMin = "0", UIMax = "100", EditCondition = "mEnabled && mOverrideSettings"))
	uint8 mStateSendRate;

	/** Defines how much blend between physics <-> network transform should happen based on positional error. This is only used when server authority is FALSE. */
	UPROPERTY(EditAnywhere, Category = "FGearReplication", DisplayName = "Position Error Curve", meta = (EditCondition = "mEnabled && mOverrideSettings"))
	UCurveFloat* mPositionError;

	/** Defines how much blend between physics <-> network transform should happen based on rotational error. This is only used when server authority is FALSE. */
	UPROPERTY(EditAnywhere, Category = "FGearReplication", DisplayName = "Rotation Error Curve", meta = (EditCondition = "mEnabled && mOverrideSettings"))
	UCurveFloat* mRotationError;

	/** Defines how much blend between physics <-> network transform should happen when a collision occurs. This is only used when server authority is FALSE. */
	UPROPERTY(EditAnywhere, Category = "FGearReplication", DisplayName = "Collision Blending Curve", meta = (EditCondition = "mEnabled && mOverrideSettings"))
	UCurveFloat* mCollisionError;

	/** Minumum contact speed for network <-> physics blending in KMH. This is only used when server authority is FALSE. */
	UPROPERTY(EditAnywhere, Category = "FGearReplication", DisplayName = "Min Collision Blend Speed", meta = (ClampMin = "0", ClampMax = "1000", UIMin = "0", UIMax = "1000", EditCondition = "mEnabled && mOverrideSettings"))
	float mMinCollisionBlendSpeed;

	/** When the network distance error is larger then this value(in meters) then we skip the local collision check and force a snap. This is only used when server authority is FALSE. */
	UPROPERTY(EditAnywhere, Category = "FGearReplication", DisplayName = "Force Snap Distance", meta = (EditCondition = "mEnabled && mOverrideSettings"))
	float mForceSnapDistance;

	//inputs from client to server
	UFUNCTION(Unreliable, Server)
	void serverReceiveInputs(uint64 pckt);

	//kinematic state from client to server
	UFUNCTION(Unreliable, Server)
	void serverReceiveState(float time, FVector_NetQuantize100 pos, FVector_NetQuantize100 rot, FVector_NetQuantize100 vel, FVector_NetQuantize100 avel);

	//kinematic state from server to client
	UFUNCTION(Unreliable, NetMulticast)
	void clientReceiveState(float time, FVector_NetQuantize100 pos, FVector_NetQuantize100 rot, FVector_NetQuantize100 vel, FVector_NetQuantize100 avel);

	//time
	float mLocalTime;
	float mInputSendTime;
	float mStateSendTime;

	//input
	UPROPERTY(Replicated)
	FFGearInputState mCurrentInput;

	//snapshot states
	FGearSnapState mCurrentSnap;

	//dead reckoning
	float mExtraTime;
	FVector mExtraPos, mExtraJPos, mLastPos;
	FQuat mExtraRot, mExtraJRot, mLastRot;
	FVector mV0, mAcc, mLastAcc, mLastV0;

	//collision blending
	float mCollisionTime;
	float mCollisionErrorTime;

	AFGearVehicle* mVehicle;
	int mTick;
	float mWrongGearTime;
};