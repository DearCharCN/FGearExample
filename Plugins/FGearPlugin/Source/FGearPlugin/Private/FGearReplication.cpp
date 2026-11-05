// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearReplication.h"
#include "FGearVehicle.h"
#include "FGearStandardInput.h"
#include "FGearTransmission.h"
#include "FGearSettings.h"
#include "Engine/World.h"
#include "Curves/CurveFloat.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameState.h"

UFGearReplication::UFGearReplication() :
	UFGearTelemetryDrawer(),
	mOverrideSettings(false),
	mEnabled(true),
	mServerAuthority(false),
	mInputSendRate(20),
	mStateSendRate(20),
	mPositionError(nullptr),
	mRotationError(nullptr),
	mCollisionError(nullptr),
	mMinCollisionBlendSpeed(10.0f),
	mForceSnapDistance(15.0f),
	mLocalTime(0.0f),
	mInputSendTime(0.0f),
	mStateSendTime(0.0f),
	mExtraTime(0.0f),
	mCollisionTime(1.0f),
	mCollisionErrorTime(1.0f),
	mVehicle(NULL),
	mTick(-1),
	mWrongGearTime(0.0f)
{
}

void UFGearReplication::init(AFGearVehicle* v)
{
	//check settings override
	const UFGearSettings* settings = GetDefault<UFGearSettings>();
	if (settings)
	{
		mServerAuthority = settings->mServerAuthority;
		if (!mOverrideSettings)
		{
			mEnabled = settings->mEnabled;
			mInputSendRate = settings->mInputSendRate;
			mStateSendRate = settings->mStateSendRate;
			mMinCollisionBlendSpeed = settings->mMinCollisionBlendSpeed;
			mForceSnapDistance = settings->mForceSnapDistance;
		}
	}

	SetIsReplicated(mEnabled);
	if (!mEnabled) return;

	mVehicle = v;
	setWindowHeight(110);

	//state replication stuff
	if (mStateSendRate > 0)
	{
		//disable pawn replication
		mVehicle->SetReplicateMovement(false);

		//clients are kinematic in server auth mode
		if (mServerAuthority)
		{
			bool simulate = mVehicle->GetLocalRole() == ENetRole::ROLE_Authority;
			mVehicle->getBody()->SetInstanceSimulatePhysics(simulate);
		}
	}

	//use initial values
	if (mVehicle->GetLocalRole() != ENetRole::ROLE_Authority)
	{
		reset(v->getPosition(), v->getRotation());
	}

	//do not leave curves empty
	if (mPositionError == nullptr) mPositionError = NewObject<UCurveFloat>();
	if (mPositionError->FloatCurve.GetNumKeys() <= 1)
	{
		mPositionError->FloatCurve.AddKey(0.0f, 0.0f);
		mPositionError->FloatCurve.AddKey(0.5f, 0.1f);
		mPositionError->FloatCurve.AddKey(1.0f, 1.0f);
	}

	//do not leave curves empty
	if (mRotationError == nullptr) mRotationError = NewObject<UCurveFloat>();
	if (mRotationError->FloatCurve.GetNumKeys() <= 1)
	{
		mRotationError->FloatCurve.AddKey(0.0f, 0.0f);
		mRotationError->FloatCurve.AddKey(5.0f, 0.1f);
		mRotationError->FloatCurve.AddKey(15.0f, 1.0f);
	}

	//read collision error time
	if (mCollisionError)
	{
		mCollisionTime = mCollisionError->FloatCurve.GetLastKey().Time;
		mCollisionErrorTime = mCollisionTime;
	}
	//do not leave curves empty
	else mCollisionError = NewObject<UCurveFloat>();
	if (mCollisionError->FloatCurve.GetNumKeys() <= 1)
	{
		mCollisionError->FloatCurve.AddKey(0.0f, 0.0f);
		mCollisionError->FloatCurve.AddKey(0.5f, 0.1f);
		mCollisionError->FloatCurve.AddKey(1.0f, 1.0f);
	}

	//initial time sync
	mLocalTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
}

void UFGearReplication::myUpdate(float dt)
{
	//update time and tick
	float serverTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	mLocalTime += dt;
	mLocalTime += (serverTime - mLocalTime) * dt;
	mTick++;

	//input replication
	if (mInputSendRate > 0)
	{
		//update inputs
		updateInputs(dt);

		//local player send inputs
		sendInputs(dt);
	}

	//state replication
	if (mStateSendRate > 0)
	{
		//clients update interpolation
		updateInterpolation(dt);
		
		//server sends states
		sendStates(dt);
	}
}

void UFGearReplication::reset(FVector position, FQuat rotation)
{
	mCurrentSnap.mTime = mLocalTime + 0.1f; //pause for an extra 100ms
	mCurrentSnap.mPosition = mExtraPos;
	mCurrentSnap.mRotation = mExtraRot;
	mCurrentSnap.mVelocity = FVector::ZeroVector;
	mCurrentSnap.mAngVelocity = FVector::ZeroVector;

	mExtraTime = 0.0f;
	mExtraPos = position;
	mExtraRot = rotation;
	mExtraJPos = mExtraPos;
	mExtraJRot = mExtraRot;
	mLastPos = mExtraPos;
	mLastRot = mExtraRot;
	mV0 = mLastV0 = FVector::ZeroVector;
	mAcc = mLastAcc = FVector::ZeroVector;
}

void UFGearReplication::updateInputs(float dt)
{
	//local player
	if (mVehicle->IsLocallyControlled())
	{
		//standard input update
		mVehicle->getStandardInput()->myUpdate(dt, false);

		//store state
		mCurrentInput.mTick = mTick;
		mCurrentInput.mThrottle = mVehicle->getStandardInput()->getThrottleRawInput();
		mCurrentInput.mBrake = mVehicle->getStandardInput()->getBrakeRawInput();
		mCurrentInput.mSteer = mVehicle->getStandardInput()->getSteerRawInput();
		mCurrentInput.mClutch = mVehicle->getStandardInput()->getClutchRawInput();
		mCurrentInput.mGear = mVehicle->getTransmission()->getCurGear(); //current gear
		mCurrentInput.mHandbrake = mVehicle->getStandardInput()->getHandbrakeInput();
	}
	//server & others in client
	else
	{
		//set & apply inputs
		FFGearInputState s = mCurrentInput;
		mVehicle->getStandardInput()->setInputs(s.mThrottle, s.mBrake, s.mSteer, s.mClutch, 0, s.mHandbrake);
		mVehicle->getStandardInput()->myUpdate(dt, true); //we manually provide inputs so "skipReading" must be true
		//also enforce current gear if auto change is off
		if (!mVehicle->getTransmission()->isAutoChange())
		{
			mVehicle->getTransmission()->setCurrentGear(s.mGear, true);
		}
	}
}

void UFGearReplication::updateInterpolation(float dt)
{
	if (mServerAuthority)
	{
		//interpolate non authorative in server auth mode
		if (mVehicle->GetLocalRole() == ENetRole::ROLE_Authority) return;

		//update transform
		FVector pos; FQuat rot;
		getDeadReckoning(pos, rot, dt);
		mVehicle->SetActorLocationAndRotation(pos, rot, false, nullptr, ETeleportType::None);
	}
	else if (!mVehicle->IsLocallyControlled())
	{
		//physics transform
		FTransform t = FGearUtility::getWorldTransform(mVehicle->getBody());
		FVector pos1 = t.GetLocation();
		FQuat rot1 = t.GetRotation();

		//extrapolated transform
		FVector pos; FQuat rot;
		getDeadReckoning(pos, rot, dt);

		//calc. error scale if there is an ongoing collision
		float errorScale = 1.0f;
		if (mCollisionTime < mCollisionErrorTime)
		{
			errorScale = mCollisionError->GetFloatValue(mCollisionTime);
			mCollisionTime += dt;
		}
		
		//calc. transform error
		float locationError = errorScale * FVector::Distance(pos1, pos) * FGearUtility::cm2meter;
		float rotationError = errorScale * FMath::RadiansToDegrees(rot1.AngularDistance(rot));

		//lerp between physics <-> extrapolated
		float positionWeight = mPositionError->GetFloatValue(locationError);
		float rotationWeight = mRotationError->GetFloatValue(rotationError);

		//apply
		pos = FMath::Lerp(pos1, pos, positionWeight);
		rot = FMath::Lerp(rot1, rot, rotationWeight);
		bool doSweep = locationError < mForceSnapDistance;
		mVehicle->SetActorLocationAndRotation(pos, rot, doSweep, nullptr, ETeleportType::TeleportPhysics);
		
		//fix velocity.Z
		FVector vel = FGearUtility::getWorldVelocity(mVehicle->getBody());
		vel.Z = 0.0f;
		mVehicle->getBody()->SetLinearVelocity(vel, false);
	}
}

//send inputs to server
void UFGearReplication::sendInputs(float dt)
{
	if (mVehicle->GetLocalRole() == ENetRole::ROLE_AutonomousProxy)
	{
		mInputSendTime += dt;
		float dts = 1.0f / mInputSendRate;
		if (dts < dt) dts = dt; //no more than frame rate
		if (mInputSendTime >= dts)
		{
			mInputSendTime -= dts;
			serverReceiveInputs(mCurrentInput.compress());
		}
	}
}

//store last input
void UFGearReplication::serverReceiveInputs_Implementation(uint64 pckt)
{
	mCurrentInput.uncompress(pckt);
}

//send kinematic state to clients with server auth mode
//send kinematic state to server with client auth mode
void UFGearReplication::sendStates(float dt)
{
	if ((mServerAuthority && mVehicle->GetLocalRole() == ENetRole::ROLE_Authority) || (!mServerAuthority && mVehicle->IsLocallyControlled()))
	{
		mStateSendTime += dt;
		float dts = 1.0f / mStateSendRate;
		if (dts < dt) dts = dt; //no more than frame rate
		if (mStateSendTime >= dts)
		{
			mStateSendTime -= dts;
			//transform & velocities
			FTransform trans = FGearUtility::getWorldTransform(mVehicle->getBody());
			FVector pos = trans.GetLocation();
			FQuat rot = trans.GetRotation();
			FVector vel = FGearUtility::getWorldVelocity(mVehicle->getBody());
			FVector avel = FGearUtility::getWorldAngularVelocity(mVehicle->getBody());
			//send
			if (mServerAuthority)
			{
				clientReceiveState(mLocalTime, pos, rot.Rotator().Euler(), vel, avel);
			}
			else
			{
				serverReceiveState(mLocalTime, pos, rot.Rotator().Euler(), vel, avel);
			}
		}
	}
}

void UFGearReplication::serverReceiveState_Implementation(float time, FVector_NetQuantize100 pos, FVector_NetQuantize100 rot, FVector_NetQuantize100 vel, FVector_NetQuantize100 avel)
{
	clientReceiveState(time, pos, rot, vel, avel);
}

void UFGearReplication::clientReceiveState_Implementation(float time, FVector_NetQuantize100 pos, FVector_NetQuantize100 rot, FVector_NetQuantize100 vel, FVector_NetQuantize100 avel)
{
	//only clients with server auth
	if (mServerAuthority)
	{
		if (GetNetMode() < ENetMode::NM_Client) return;
	}
	//only non players
	else if (mVehicle == nullptr || mVehicle->IsLocallyControlled()) return;

	//buffer snap data, discard late packets
	float snapTime = time - mCurrentSnap.mTime;
	if (snapTime > 0.0f)
	{
		const FQuat rotq(FRotator(rot.Y, rot.Z, rot.X));
		//need last velocity
		FVector lastVel = mCurrentSnap.mVelocity;
		//get snapshot
		mCurrentSnap.mTime = time;
		mCurrentSnap.mPosition = pos;
		mCurrentSnap.mRotation = rotq;
		mCurrentSnap.mVelocity = vel;
		mCurrentSnap.mAngVelocity = avel;
		//extrapolation
		mExtraTime = 0.0f;
		mExtraPos = mLastPos;
		mExtraRot = mLastRot;
		mV0 = mLastV0;
		//jitter fix
		float lag = mLocalTime - time;
		static const float lagLimit = 0.1f;
		//limit lag for vertical movement, acceleration and rotation, allow negative lag with server-auth for smooth rotations
		float limitedLag = FMath::Clamp(lag, mServerAuthority ? -lagLimit : 0.0f, lagLimit);
		FVector deltaPos(lag * vel.X, lag * vel.Y, limitedLag * vel.Z);
		mExtraJPos = pos + deltaPos;
		mExtraJRot = rotq;
		FGearUtility::RotateQuat(mExtraJRot, limitedLag * avel);
		//use previous to calculate acc.
		mAcc = (vel - lastVel) / FMath::Clamp(snapTime, 0.0f, lagLimit);
		//scale using previous acc.
		float accScale = FVector::DotProduct(mAcc.GetSafeNormal(), mLastAcc.GetSafeNormal());
		mLastAcc = mAcc;
		mAcc = FMath::Max(0.0f, accScale) * mAcc;
		mAcc.Z = 0.0f;
	}
}

void UFGearReplication::getDeadReckoning(FVector& pos, FQuat& rot, float dt)
{
	//increment time
	mExtraTime += dt;

	//normalised time
	float sendTime = 1.0f / mStateSendRate;
	float normTime = FMath::Clamp(mExtraTime / sendTime, 0.0f, 1.0f);

	//position
	{
		//find acceleration delta
		FVector accDx = 0.5f * mAcc * mExtraTime * mExtraTime;

		//velocity blending
		FVector Vb = mV0 + normTime * (mCurrentSnap.mVelocity - mV0);

		//projecting from where we were
		FVector Pt = mExtraPos + mExtraTime * Vb + accDx;

		//projecting from last known
		FVector Pt1 = mExtraJPos + mExtraTime * mCurrentSnap.mVelocity + accDx;

		//combined pos
		pos = Pt + normTime * (Pt1 - Pt);

		//calc. V0
		mLastV0 = (pos - mLastPos) / dt;
		mLastPos = pos;
	}

	//rotation
	{
		//impact of angular velocity
		FQuat Rdt = FQuat::Identity;
		FGearUtility::RotateQuat(Rdt, mExtraTime * mCurrentSnap.mAngVelocity);

		//rotated from where we were
		FQuat Rt = Rdt * mExtraRot;

		//rotated from last known
		FQuat Rt1 = Rdt * mExtraJRot;

		//combined rot
		rot = FQuat::Slerp(Rt, Rt1, normTime);
		mLastRot = rot;
	}
}

void UFGearReplication::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UFGearReplication, mCurrentInput, COND_SkipOwner);
}

FVector2D UFGearReplication::drawTelemetry(UCanvas* Canvas, UFont* Font, FLinearColor textColor, FFontRenderInfo renderInfo, FVector2D p)
{
	p = Super::drawTelemetry(Canvas, Font, textColor, renderInfo, p);
	Canvas->SetLinearDrawColor(textColor);
	Canvas->DrawText(Font, "Replication", p.X, p.Y, 1.0f, 1.0f, renderInfo);
	FGearUtility::drawLine(Canvas, p.X, p.Y + 18, p.X + 32, p.Y + 18, textColor);
	FString netStr = "Standalone";
	if (GetNetMode() == ENetMode::NM_DedicatedServer) netStr = "DedicatedServer";
	else if (GetNetMode() == ENetMode::NM_ListenServer) netStr = "ListenServer";
	else if (GetNetMode() == ENetMode::NM_Client) netStr = "Client";
	Canvas->DrawText(Font, "Net Mode:" + netStr, p.X, p.Y + 20, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, "Enabled:" + UKismetStringLibrary::Conv_BoolToString(mEnabled), p.X, p.Y + 35, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, "Server Authoritative:" + UKismetStringLibrary::Conv_BoolToString(mServerAuthority), p.X, p.Y + 50, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("Local Time: %.1f"), mLocalTime), p.X, p.Y + 65, 1.0f, 1.0f, renderInfo);
	p.Y += getWindowHeight();
	return p;
}