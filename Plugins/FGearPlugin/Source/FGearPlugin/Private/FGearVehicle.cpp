// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearVehicle.h"
#include "FGearAnimInstance.h"
#include "FGearEngine.h"
#include "FGearAeroDynamics.h"
#include "FGearTransmission.h"
#include "FGearAxle.h"
#include "FGearWheelOptions.h"
#include "FGearWheel.h"
#include "FGearSkeletalMeshComponent.h"
#include "FGearStandardInput.h"
#include "FGearOrbitCamera.h"
#include "FGearArcadeAssists.h"
#include "FGearSuspensionConstraint.h"
#include "FGearReplication.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#if WITH_EDITOR
#include "Runtime/SlateCore/Public/Input/Reply.h"
#endif
#if ENGINE_MAJOR_VERSION == 5
#include "Engine/OverlapResult.h"
#include "Physics/Experimental/PhysScene_Chaos.h"
#endif

AFGearVehicle::AFGearVehicle() :
	APawn(),
	mUpdateRate(250),
	mMass(1000.0f),
	mInertiaScale(FVector::OneVector),
	mCenterofMass(FVector::ZeroVector),
	mCustomGravity(false),
	mGravity(FVector(0.0f, 0.0f, -980.0f)),
	mQueryType(FGearQueryType::Line),
	mRays(1),
	mLateralRays(1),
	mQueryMobilityType(FGearQueryMobilityType::Any),
	mTraceChannel(ECC_Visibility),
	mIgnoreMask(0),
	mTraceComplex(false),
	mConvexAsset(nullptr),
	mConvexUp(FGearUpDirection::Y_UP),
	mConvexRotation(0.0f, 0.0f, 0.0f),
	mConvexVisible(false),
	mDefaultCylinderAsset(nullptr),
	mDetectGround(false),
	mAllowReverse(true),
	mTirePenetration(true),
	mRelaxationDownforce(false),
	mHardContactScale(0.0f),
	mHandBrakePower(1.0f),
	mReverseBrakeSpeed(1.0f),
	mRollingResistanceCoeff(1.0f),
	mDrivetrainInertia(1.0f),
	mDrivetrainFriction(50.0f),
	mStickyTireSpeed(0.0f),
	mReactionScale(0.0f),
	mOptimumSlipWeight(0.0f),
	mABS(0.0f),
	mMinAbsSpeed(10.0f),
	mASR(0.0f),
	mUnderSteerAssist(0.0f),
	mMinUnderSteerAngle(10.0f),
	mOverSteerAssist(0.0f),
	mMinOverSteerAngle(20.0f),
	mFrontAntiRollPower(0.0f),
	mRearAntiRollPower(0.0f),
	mSleepLinearThreshold(0.0f),
	mSleepAngularThreshold(0.0f),
	mSleepWaitingTime(1.0f),
	mWakeUpSources(1U),
	mInSleep(false),
	mSleepTime(0.0f),

#if ENGINE_MAJOR_VERSION == 5
	mAsyncCallback(nullptr),
#endif
	mBody(nullptr),
	mNetForce(FVector::ZeroVector),
	mNetTorque(FVector::ZeroVector),
	mNetImpulse(FVector::ZeroVector),
	mNetImpulseTorque(FVector::ZeroVector),
	mUpdateInputs(true),
	mBrakeInput(0.0f),
	mTime(0.0f),
	mZero100Time(0.0f),
	mZero200Time(0.0f),
	m100ZeroDistance(0.0f),
	mGotZeroResult(true)
{
 	PrimaryActorTick.bCanEverTick = true;

	//store substep setting
	mSubstepping = UPhysicsSettings::Get()->bSubstepping;

	//convex casting assets
	static ConstructorHelpers::FObjectFinder<UStaticMesh> asset(TEXT("/FGearPlugin/Mesh/Cylinder.Cylinder"));
	if (asset.Succeeded()) mDefaultCylinderAsset = asset.Object;
	mConvexAsset = mDefaultCylinderAsset;

	mMesh = CreateDefaultSubobject<UFGearSkeletalMeshComponent>(TEXT("Mesh Component"));
	SetRootComponent(mMesh);
	mMesh->OnComponentWake.AddDynamic(this, &AFGearVehicle::onWake);

	mEngine = CreateDefaultSubobject<UFGearEngine>(TEXT("Engine"));
	mEngine->SetupAttachment(RootComponent);

	mTransmission = CreateDefaultSubobject<UFGearTransmission>(TEXT("Transmission"));
	mTransmission->SetupAttachment(RootComponent);

	mStandardInput = CreateDefaultSubobject<UFGearStandardInput>(TEXT("Standard Input"));
	mStandardInput->SetupAttachment(RootComponent);

	mAeroDynamics = CreateDefaultSubobject<UFGearAeroDynamics>(TEXT("Aero Dynamics"));
	mAeroDynamics->SetupAttachment(RootComponent);

	//Axle 0
	mAxle0 = CreateDefaultSubobject<UFGearAxle>(TEXT("Front Axle"));
	mAxle0->SetupAttachment(RootComponent);

	mWheelOptions0 = CreateDefaultSubobject<UFGearWheelOptions>(TEXT("Front Wheel Options"));
	mWheelOptions0->SetupAttachment(mAxle0);

	mWheelLeft0 = CreateDefaultSubobject<UFGearWheel>(TEXT("Front Wheel Left"));
	mWheelLeft0->SetupAttachment(mAxle0);
	mWheelLeft0->setWheelSide(FGearWheelSide::WS_LEFT);

	mWheelRight0 = CreateDefaultSubobject<UFGearWheel>(TEXT("Front Wheel Right"));
	mWheelRight0->SetupAttachment(mAxle0);
	mWheelRight0->setWheelSide(FGearWheelSide::WS_RIGHT);

	//Axle 1
	mAxle1 = CreateDefaultSubobject<UFGearAxle>(TEXT("Rear Axle"));
	mAxle1->SetupAttachment(RootComponent);

	mWheelOptions1 = CreateDefaultSubobject<UFGearWheelOptions>(TEXT("Rear Wheel Options"));
	mWheelOptions1->SetupAttachment(mAxle1);

	mWheelLeft1 = CreateDefaultSubobject<UFGearWheel>(TEXT("Rear Wheel Left"));
	mWheelLeft1->SetupAttachment(mAxle1);
	mWheelLeft1->setWheelSide(FGearWheelSide::WS_LEFT);

	mWheelRight1 = CreateDefaultSubobject<UFGearWheel>(TEXT("Rear Wheel Right"));
	mWheelRight1->SetupAttachment(mAxle1);
	mWheelRight1->setWheelSide(FGearWheelSide::WS_RIGHT);

	//replication
	bNetLoadOnClient = false;
	mReplication = CreateDefaultSubobject<UFGearReplication>(TEXT("Replication"));
	mReplication->SetupAttachment(RootComponent);
}

void AFGearVehicle::BeginPlay()
{
	Super::BeginPlay();

	if (mMesh == nullptr) return;
	mBody = mMesh->GetBodyInstance();
	if (mBody == nullptr) return;

	//keep all bodies active
	mMesh->SetAllBodiesSimulatePhysics(true);

	//set mass
	setMass(mMass);

	//set center of mass offset
	setCenterOfMassOffset(mCenterofMass);
	
	//set inertia tensor
	setInertiaScale(mInertiaScale);

	//set gravity state
	mBody->SetEnableGravity(!mCustomGravity);

	//this is available since 5.3 and it's required for 5.4+
	//kinematic objects do not calculate its velocity if this is off
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 2
	mBody->SetUpdateKinematicFromSimulation(true);
#endif

	mAnimation = Cast<UFGearAnimInstance>(mMesh->GetAnimInstance());
	if (mAnimation != nullptr) mAnimation->init(mAxleList.Num());

	mWheelForces.Init(FVector::ZeroVector, 2 * mAxleList.Num());
	mWheelTorques.Init(FVector::ZeroVector, 2 * mAxleList.Num());

	TArray<FName> boneNames;
	mMesh->GetBoneNames(boneNames);
	for (int i = 0; i < boneNames.Num(); i++)
	{
		int boneIndex = mMesh->GetBoneIndex(boneNames[i]);
		FTransform boneTrans = mMesh->GetBoneTransform(boneIndex, FTransform::Identity);
		boneTrans.SetScale3D(FVector::OneVector);
		mBoneTransforms.Add(boneNames[i], boneTrans);

		if (i == 0 && !boneTrans.GetLocation().IsZero())
		{
			UE_LOG(LogTemp, Warning, TEXT("Root bone must be located in origin of the vehicle (%s)"), *GetName());
		}
	}

	mEngine->init(this);
	for (int i = 0; i < mAxleList.Num(); i++)
	{
		mAxleList[i]->init(this, i);
	}
	mTransmission->init(this);
	mStandardInput->init(this);
	mAeroDynamics->init(this);
	mArcadeAssists = FindComponentByClass<UFGearArcadeAssists>();
	if (mArcadeAssists) mArcadeAssists->init(this);
	mSuspensionConstraint = FindComponentByClass<UFGearSuspensionConstraint>();
	if (mSuspensionConstraint) mSuspensionConstraint->init(this);
	mReplication->init(this);

#if ENGINE_MAJOR_VERSION == 5
	if (isSubstepping())
	{
		mAsyncCallback = GetWorld()->GetPhysicsScene()->GetSolver()->CreateAndRegisterSimCallbackObject_External<FFGearAsyncCallback>();
		mAsyncCallback->init(this);
	}
#else
	mMesh->init(this);
#endif
}

void AFGearVehicle::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
#if ENGINE_MAJOR_VERSION == 5
	if (mAsyncCallback)
	{
		GetWorld()->GetPhysicsScene()->GetSolver()->UnregisterAndFreeSimCallbackObject_External(mAsyncCallback);
		mAsyncCallback = nullptr;
	}
#endif
	if (mMesh) mMesh->OnComponentWake.RemoveDynamic(this, &AFGearVehicle::onWake);
}

#if ENGINE_MAJOR_VERSION == 5
void FFGearAsyncCallback::OnPreSimulate_Internal()
{
	if (mVehicle)
	{
		mVehicle->substepTick(GetDeltaTime_Internal());
		//fire substep event
		if (mVehicle->getMesh()->OnSubstepTick.IsBound())
		{
			mVehicle->getMesh()->OnSubstepTick.Broadcast(GetDeltaTime_Internal());
		}
	}
}
#endif

void AFGearVehicle::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	mAxleList.Empty();
	initializeAxles(RootComponent->GetAttachChildren(), 0);
}

void AFGearVehicle::OnConstruction(const FTransform & Transform)
{
	Super::OnConstruction(Transform);
	if (mAxleList.Num() == 0) return;

	TArray< USceneComponent*> sceneComps;
	for (int i = 0; i < BlueprintCreatedComponents.Num(); i++)
	{
		sceneComps.Add(Cast<USceneComponent>(BlueprintCreatedComponents[i]));
	}
	initializeAxles(sceneComps, 2);
}

//Initialize function for set UFGearWheel(s) and UFGearWheelOptions references to axle component.
//@index refers to start axle index 
void AFGearVehicle::initializeAxles(TArray<USceneComponent*> components, int index)
{
	for (USceneComponent* comp : components)
	{
		if (comp == nullptr) continue;
		//Find axle component
		if (comp->GetClass() == UFGearAxle::StaticClass())
		{
			//Get components attached to the axle. It should contains UFGearWheelOptions and two UFGearWheel
			TArray<USceneComponent*> childs = comp->GetAttachChildren();
			if (childs.Num() == 3)
			{
				//Clear components
				UFGearWheelOptions* wo = nullptr;
				UFGearWheel* l = nullptr;
				UFGearWheel* r = nullptr;

				//Scan childs and set references
				for (USceneComponent* child : childs)
				{
					if (!child) continue;
					if (child->GetClass() == UFGearWheelOptions::StaticClass())
					{
						wo = Cast<UFGearWheelOptions>(child);
					}
					else if (child->GetClass() == UFGearWheel::StaticClass())
					{
						UFGearWheel* w = Cast<UFGearWheel>(child);
						//Separate wheel components by FGearWheelSide
						if (l == nullptr && w->getWheelSide() == FGearWheelSide::WS_LEFT) l = w;
						else if (r == nullptr && w->getWheelSide() == FGearWheelSide::WS_RIGHT) r = w;
					}
				}

				//Set reference of the components to axle component if scan complete
				if (wo != nullptr && l != nullptr && r != nullptr)
				{
					UFGearAxle* axle = Cast<UFGearAxle>(comp);
					axle->setWheelOptions(wo);
					axle->setLeftWheel(l);
					axle->setRightWheel(r);

					if (mAxleList.IsValidIndex(index))
					{
						//Set axle to list and init if already exist
						mAxleList[index] = axle;
						axle->init(this, index);
					}
					else
					{
						//Create one if does not exist
						mAxleList.Add(axle);
					}
					//Increase index for next axle
					index++;
				}
			}
		}
	}
}

FTransform AFGearVehicle::getBoneTransform(FName nm) const
{
	if (!mBoneTransforms.Contains(nm)) return FTransform::Identity;
	return mBoneTransforms[nm];
}

UStaticMesh* AFGearVehicle::getConvexAsset()
{
	if (!mConvexAsset) mConvexAsset = mDefaultCylinderAsset;
	return mConvexAsset;
}

void AFGearVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	mStandardInput->setupPlayerInputComponent(PlayerInputComponent);
}

void AFGearVehicle::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (!IsPlayerControlled()) return;

	mStandardInput->initInputMapping(NewController);
	prepareTelemetry();
	if (GetLocalRole() == ENetRole::ROLE_Authority) clientPrepareTelemetry();
}

void AFGearVehicle::UnPossessed()
{
	if (const APlayerController* pc = GetWorld()->GetFirstPlayerController())
	{
		if (AHUD* hud = pc->GetHUD()) hud->RemovePostRenderedActor(this);
	}
	Super::UnPossessed();
}

void AFGearVehicle::OnRep_Controller()
{
	Super::OnRep_Controller();
	if (Controller) mStandardInput->initInputMapping(Controller);
}

void AFGearVehicle::setMass(float mass)
{
	mMass = mass;
	if (mBody != nullptr)
	{
		mBody->SetMassOverride(mMass);
		mBody->UpdateMassProperties();
	}
}

void AFGearVehicle::setCenterOfMassOffset(FVector com)
{
	mCenterofMass = com;
	if (mBody != nullptr)
	{
		mBody->COMNudge = FGearUtility::meter2cm * (mCenterofMass / mBody->Scale3D);
		mBody->UpdateMassProperties();
	}
}

void AFGearVehicle::setInertiaScale(FVector iscale)
{
	mInertiaScale = iscale;
	if (mBody == nullptr) return;
	FVector inertia = mInertiaScale;
	FPhysicsCommand::ExecuteWrite(mBody->GetPhysicsActorHandle(), [&](FPhysicsActorHandle& Actor)
	{
		if (FPhysicsInterface::IsRigidBody(Actor))
		{
			inertia = mInertiaScale * FPhysicsInterface::GetLocalInertiaTensor_AssumesLocked(Actor);
			FPhysicsInterface::SetMassSpaceInertiaTensor_AssumesLocked(Actor, inertia);
		}
	});
	//store local inertia tensor matrix
	mLocalIIT.M[0][0] = 1.0f / inertia.X;
	mLocalIIT.M[1][1] = 1.0f / inertia.Y;
	mLocalIIT.M[2][2] = 1.0f / inertia.Z;
	mLocalIIT.M[3][3] = 1.0f;
}

void AFGearVehicle::localReset(FVector position, FQuat rotation)
{
	if (mBody == nullptr) return;

	mLastLinearVelocity = mLastAngularVelocity = FVector::ZeroVector;

	SetActorLocation(position, false, nullptr, ETeleportType::TeleportPhysics);
	SetActorRotation(rotation, ETeleportType::TeleportPhysics);
	mBody->SetLinearVelocity(FVector::ZeroVector, false);
	mBody->SetAngularVelocityInRadians(FVector::ZeroVector, false);

	mEngine->resetRpm();
	mTransmission->setCurrentGear(0, true);
	mReplication->reset(position, rotation);

	//reset all wheels
	for (int i = 0; i < mAxleList.Num(); i++)
	{
		mAxleList[i]->getLeftWheel()->reset();
		mAxleList[i]->getRightWheel()->reset();
	}
}

void AFGearVehicle::reset(FVector position, FRotator rotation)
{
	if (GetLocalRole() < ENetRole::ROLE_Authority)
	{
		serverReset(position, rotation.Quaternion());
	}
	else localReset(position, rotation.Quaternion());
}

void AFGearVehicle::clientReset_Implementation(FVector position, FQuat rotation)
{
	localReset(position, rotation);
}

void AFGearVehicle::serverReset_Implementation(FVector position, FQuat rotation)
{
	clientReset(position, rotation);
}

void AFGearVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//update replication
	if (GetNetMode() != ENetMode::NM_Standalone && mReplication->isEnabled())
	{
		mReplication->myUpdate(DeltaTime);
	}
	//update inputs in local game
	else if (mUpdateInputs)
	{
		mStandardInput->myUpdate(DeltaTime, false);
	}

	//update sleeping status
	if (updateSleeping(DeltaTime)) return;

	//call substepTick directly if not substepping or
	//in case this is a kinematic body(ROLE_SimulatedProxy)
	if (!mSubstepping || !mBody->bSimulatePhysics) substepTick(DeltaTime);

	if (mTelemetry.mShowStats) updateStats(DeltaTime);

	//update wheels visual
	for (int i = 0; i < mAxleList.Num(); i++)
	{
		mAxleList[i]->getLeftWheel()->visualsUpdate(DeltaTime);
		mAxleList[i]->getRightWheel()->visualsUpdate(DeltaTime);
	}
}

//called from AFGearVehicle::Tick if mSubstepping is false
//called from FGearSkeletalMeshComponent::SubstepTick for substepping
void AFGearVehicle::substepTick(float dt)
{
	if (mBody == nullptr || getIsSleeping()) return;
	
	//brake when throttle against drive force
	if (mReverseBrakeSpeed > 0.0f)
	{
		float curSpeed = getMaxWheelKMHSpeed(false, false);
		int curGear = mTransmission->getCurGear();
		if (curGear != 0  && FMath::Abs(curSpeed) > mReverseBrakeSpeed && FGearUtility::Sign(curGear) != FGearUtility::Sign(curSpeed))
		{
			if (mEngine->getThrottle() > 0.0f && mBrakeInput == 0.0f)
			{
				setBraking(1.0f, mHandbrakeOn);
			}
		}
	}

	//custom gravity
	if (mCustomGravity)
	{
		addForce(mGravity * mMass);
	}

	//reset wheel forces & torques
	for (int i = 0; i < mAxleList.Num(); i++)
	{
		mWheelForces[i * 2] = FVector::ZeroVector;
		mWheelForces[i * 2 + 1] = FVector::ZeroVector;
		mWheelTorques[i * 2] = FVector::ZeroVector;
		mWheelTorques[i * 2 + 1] = FVector::ZeroVector;
	}

	//check sticky tire condition, see Wheel.stickyTireUpdate()
	mStickyTireState = 1.0f;
	if (mStickyTireSpeed > 0.0f)
	{
		float angularVelocity = FGearUtility::getWorldAngularVelocity(mBody).Size();
		float lateralVelocity = FMath::Abs(getForwardSpeed()) + FMath::Abs(getRightSpeed());
		float totalSpeed = (lateralVelocity + angularVelocity) * FGearUtility::ms2kmh;
		mStickyTireState = FMath::Clamp(totalSpeed / mStickyTireSpeed, 0.0f, 1.0f);
	}

	//reset optimum slip state
	mOptimumSlipState = mOptimumSlipWeight;
	
	//cruise control needs to be updated before components
	if (mArcadeAssists) mArcadeAssists->updateCruiseControl(dt);

	//update @ mUpdateRate Hz
	{
		mTime += dt;
		float dts = 1.0f / mUpdateRate;
		//no sub steps in stick state
		if (mStickyTireState < 1.0f) dts = dt;
		mSimCoeff = dts / dt;
		int i = 0;
		while (mTime >= dts)
		{
			mTime -= dts;
			myFixedUpdate(i++, dt, dts);
		}
	}

	//skip the rest for kinematic body
	if (!mBody->bSimulatePhysics) return;

	//apply wheel forces
	for (int i = 0; i < mAxleList.Num(); i++)
	{
		addForceAtPosition(FGearUtility::meter2cm * mWheelForces[2 * i], mAxleList[i]->getLeftWheel()->getHubPosition());
		addForceAtPosition(FGearUtility::meter2cm * mWheelForces[2 * i + 1], mAxleList[i]->getRightWheel()->getHubPosition());

		FGearUtility::AddTorqueAtPosition(this, mAxleList[i]->getLeftWheel()->getHubPosition(), FGearUtility::meter2cm * mWheelTorques[2 * i]);
		FGearUtility::AddTorqueAtPosition(this, mAxleList[i]->getRightWheel()->getHubPosition(), FGearUtility::meter2cm * mWheelTorques[2 * i + 1]);

		//optional force to be applied back to dynamic colliders
		if (mReactionScale > 0.0f)
		{
			FHitResult hitL = mAxleList[i]->getLeftWheel()->getRayHit();
			FHitResult hitR = mAxleList[i]->getRightWheel()->getRayHit();

			if (hitL.GetComponent())
			{
				FBodyInstance* hitBodyL = hitL.GetComponent()->GetBodyInstance();
				if (hitBodyL != nullptr && hitBodyL->IsNonKinematic())
				{
					hitBodyL->AddForceAtPosition(-mReactionScale * FGearUtility::meter2cm * mWheelForces[2 * i], hitL.ImpactPoint, !mSubstepping);
				}
			}

			if (hitR.GetComponent())
			{
				FBodyInstance* hitBodyR = hitR.GetComponent()->GetBodyInstance();
				if (hitBodyR != nullptr && hitBodyR->IsNonKinematic())
				{
					hitBodyR->AddForceAtPosition(-mReactionScale * FGearUtility::meter2cm * mWheelForces[2 * i + 1], hitR.ImpactPoint, !mSubstepping);
				}
			}
		}
	}

	//handle hard contacts
	handleHardContacts(dt);

	//update assists
	updateESP();
	updateAntiRollBar();
	mAeroDynamics->myFixedUpdate();
	if (mArcadeAssists) mArcadeAssists->substepTick(dt);
	if (mSuspensionConstraint) mSuspensionConstraint->substepTick();

	//apply net force/torque
#if ENGINE_MAJOR_VERSION == 5
	if (!IsInGameThread())
	{
		if (FPhysicsActorHandle handle = mBody->ActorHandle)
		{
			if (Chaos::FRigidBodyHandle_Internal* rigidHandle = handle->GetPhysicsThreadAPI())
			{
				if (ensure(rigidHandle))
				{
					rigidHandle->AddForce(mNetForce, false);
					rigidHandle->AddTorque(mNetTorque, false);
					rigidHandle->SetLinearImpulse(rigidHandle->LinearImpulse() + mNetImpulse, false);
					rigidHandle->SetAngularImpulse(rigidHandle->AngularImpulse() + mNetImpulseTorque, false);
				}
			}
		}
	}
	else
	{
		mBody->AddForce(mNetForce, !mSubstepping);
		mBody->AddTorqueInRadians(mNetTorque, !mSubstepping);
		mBody->AddImpulse(mNetImpulse, false);
		mBody->AddAngularImpulseInRadians(mNetImpulseTorque, false);
	}
#else
	mBody->AddForce(mNetForce, !mSubstepping);
	mBody->AddTorqueInRadians(mNetTorque, !mSubstepping);
	mBody->AddImpulse(mNetImpulse, false);
	mBody->AddAngularImpulseInRadians(mNetImpulseTorque, false);
#endif

	mNetForce = mNetTorque = FVector::ZeroVector;
	mNetImpulse = mNetImpulseTorque = FVector::ZeroVector;
}

void AFGearVehicle::myFixedUpdate(int i, float dt, float dts)
{
	//update engine/transmission with substeps
	mEngine->myFixedUpdate(dts);
	mTransmission->myFixedUpdate(dts);

	//pre update axles
	for (int j = 0; j < mAxleList.Num(); j++)
	{
		mAxleList[j]->preWheelupdate(i == 0);
	}

	//update wheels
	for (int j = 0; j < mAxleList.Num(); j++)
	{
		FVector force, torque;

		mAxleList[j]->getLeftWheel()->myFixedUpdate(dt, dts, i, force, torque);
		mWheelForces[j * 2] += mSimCoeff * force;
		mWheelTorques[j * 2] += mSimCoeff * torque;

		mAxleList[j]->getRightWheel()->myFixedUpdate(dt, dts, i, force, torque);
		mWheelForces[j * 2 + 1] += mSimCoeff * force;
		mWheelTorques[j * 2 + 1] += mSimCoeff * torque;
	}

	//postupdate axles
	for (int j = 0; j < mAxleList.Num(); j++)
	{
		mAxleList[j]->postWheelupdate();
	}

	//apply engine rpm feedback
	{
		//feedback torque is multiplied with clutch inside engine
		float feedbackTorque = 0.0f;
		for (int j = 0; j < mAxleList.Num(); j++)
		{
			float share = mAxleList[j]->getTorqueShare();
			feedbackTorque += share * mAxleList[j]->getFeedbackTorque();
		}
		float targetRpm = getFeedbackWheelRPM();
		float clutchScale = 0.01f * mTransmission->getClutchScale(); //artificial
		mEngine->setFeedback(dts, targetRpm, feedbackTorque * clutchScale);
	}
}

void AFGearVehicle::saveState(FFGearVehicleState& state)
{
	//self state
	state.mVehicleTime = mTime;
	FTransform trans = FGearUtility::getWorldTransform(mBody);
	state.mPosition = trans.GetLocation();
	state.mRotation = trans.GetRotation();
	state.mVelocity = FGearUtility::getWorldVelocity(mBody);
	state.mAngularVelocity = FGearUtility::getWorldAngularVelocity(mBody);

	//component states
	mEngine->saveState(state);
	mTransmission->saveState(state);
	mStandardInput->saveState(state);

	//axle/wheel states
	for (int i = 0; i < mAxleList.Num(); i++)
	{
		mAxleList[i]->saveState(state);
	}
}

void AFGearVehicle::loadState(FFGearVehicleState state)
{
	//self state
	mTime = state.mVehicleTime;
	mBody->SetBodyTransform(FTransform(state.mRotation, state.mPosition), ETeleportType::ResetPhysics);
	mBody->SetLinearVelocity(state.mVelocity, false);
	mBody->SetAngularVelocityInRadians(state.mAngularVelocity, false);
	
	//component states
	mEngine->loadState(state);
	mTransmission->loadState(state);
	mStandardInput->loadState(state);
	for (int i = 0; i < mAxleList.Num(); i++)
	{
		mAxleList[i]->loadState(state);
	}
}

void AFGearVehicle::setBraking(float brakeInput, bool handbrake)
{
	//change in brake input can wakeup
	if (getIsSleeping() && canWakeByBraking() && ((brakeInput != mBrakeInput) || (mHandbrakeOn && !handbrake)))
	{
		wakeUp();
	}

	mBrakeInput = brakeInput;
	mHandbrakeOn = handbrake;

	for (int i = 0; i < mAxleList.Num(); i++)
	{
		bool handBrake = mHandbrakeOn && mAxleList[i]->hasHandBrake();
		mAxleList[i]->getLeftWheel()->setBraking(handBrake ? 1.0f : mBrakeInput);
		mAxleList[i]->getRightWheel()->setBraking(handBrake ? 1.0f : mBrakeInput);
	}
}

void AFGearVehicle::setSteering(float steerInput)
{
	//find prev. steering input, any change can wakeup
	if (getIsSleeping() && canWakeBySteering())
	{
		float prevInput = 0.0f;
		for (int i = 0; i < mAxleList.Num(); i++)
		{
			if (mAxleList[i]->hasSteering())
			{
				prevInput = mAxleList[i]->getSteeringInput();
			}
		}
		if (prevInput != steerInput) wakeUp();
	}

	for (int i = 0; i < mAxleList.Num(); i++)
	{
		if (mAxleList[i]->hasSteering())
		{
			mAxleList[i]->setTargetSteer(steerInput);
		}
	}
}

float AFGearVehicle::getNormalizedSteering() const
{
	for (int i = 0; i < mAxleList.Num(); i++)
	{
		if (mAxleList[i]->hasSteering())
		{
			return mAxleList[i]->getNormalizedSteering();
		}
	}
	return 0.0f;
}

//valid only for 4 wheel vehicles
float AFGearVehicle::getCurrentSteering() const
{
	for (int i = 0; i < mAxleList.Num(); i++)
	{
		if (mAxleList[i]->hasSteering())
		{
			return mAxleList[i]->getNormalizedSteering() * mAxleList[i]->getMaxSteerAngle();
		}
	}
	return 0.0f;
}

//valid only for 4 wheel vehicles
float AFGearVehicle::getSteerDeltaAngle(bool front) const
{
	if (getAxleCount() < 2) return 0.0f;
	UFGearAxle* frontAxle = getAxle(0);
	UFGearAxle* rearAxle = getAxle(1);

	FTransform trans = FGearUtility::getWorldTransform(mBody);
	FVector forward = trans.TransformVectorNoScale(FVector::ForwardVector);
	FQuat qs = FGearUtility::Euler(0.0f, getCurrentSteering(), 0.0f);
	FVector dir1 = qs * forward;
	dir1.Z = 0.0f;
	dir1.Normalize();
	FVector vel = FGearUtility::getWorldVelocityAtPoint(mBody, front ? frontAxle->getGlobalCenter() : rearAxle->getGlobalCenter()) * FGearUtility::cm2meter;
	FVector dir2 = vel.GetSafeNormal();
	dir2.Z = 0.0f;
	dir2.Normalize();
	FVector dir3 = forward;
	dir3.Z = 0.0f;
	dir3.Normalize();

	float angle = front ? FGearUtility::SignedAngle(dir2, dir1, FVector::UpVector) : FGearUtility::SignedAngle(dir2, dir3, FVector::UpVector);
	if (angle > 90.0f) return 90.0f;
	else if (angle < -90.0f) return -90.0f;
	return angle;
}

//designed only for 4 wheel vehicles
void AFGearVehicle::updateAntiRollBar()
{
	if (getAxleCount() != 2) return;

	if (mFrontAntiRollPower > 0.0f)
	{
		UFGearWheel* fl = getAxle(0)->getLeftWheel();
		UFGearWheel* fr = getAxle(0)->getRightWheel();

		//reset
		fl->setSpringCoeff(1.0f);
		fr->setSpringCoeff(1.0f);

		//calculate
		float flCompress = fl->getSuspensionCompressRatio();
		float frCompress = fr->getSuspensionCompressRatio();
		float frontAntiRoll = (flCompress - frCompress) * 0.01f * mFrontAntiRollPower;

		//apply
		fl->setSpringCoeff(1.0f + frontAntiRoll);
		fr->setSpringCoeff(1.0f - frontAntiRoll);
	}

	if (mRearAntiRollPower > 0.0f)
	{
		UFGearWheel* rl = getAxle(1)->getLeftWheel();
		UFGearWheel* rr = getAxle(1)->getRightWheel();

		//reset
		rl->setSpringCoeff(1.0f);
		rr->setSpringCoeff(1.0f);

		//calculate
		float rlCompress = rl->getSuspensionCompressRatio();
		float rrCompress = rr->getSuspensionCompressRatio();
		float rearAntiRoll = (rlCompress - rrCompress) * 0.01f * mRearAntiRollPower;

		//apply
		rl->setSpringCoeff(1.0f + rearAntiRoll);
		rr->setSpringCoeff(1.0f - rearAntiRoll);
	}
}

//designed only for 4 wheel vehicles
void AFGearVehicle::updateESP()
{
	mEspActive = false;
	mUnderSteering = false;
	mOverSteering = false;

	if (getAxleCount() != 2 || getTransmission()->getCurGear() < 0) return;

	UFGearWheel* fl = getAxle(0)->getLeftWheel();
	UFGearWheel* fr = getAxle(0)->getRightWheel();
	UFGearWheel* rl = getAxle(1)->getLeftWheel();
	UFGearWheel* rr = getAxle(1)->getRightWheel();

	float maxSteerAngle = getAxle(0)->getMaxSteerAngle();
	float speedCoeff = FMath::Min(1.0f, getVelocitySize() / 10.0f);

	//oversteer, brake front wheels
	if (mOverSteerAssist > 0.0f)
	{
		float angle = getSteerDeltaAngle(false);

		//turn left
		if (angle < -mMinOverSteerAngle)
		{
			float power = speedCoeff * (0.01f * mOverSteerAssist) * -angle / maxSteerAngle;
			fr->setBraking(FMath::Min(1.0f, power));
			mEspActive = true;
			mOverSteering = true;
		}
		//turn right
		if (angle > mMinOverSteerAngle)
		{
			float power = speedCoeff * (0.01f * mOverSteerAssist) * angle / maxSteerAngle;
			fl->setBraking(FMath::Min(1.0f, power));
			mEspActive = true;
			mOverSteering = true;
		}
	}

	//understeer, brake rear wheels
	//skip understeer when oversteer
	if (mUnderSteerAssist > 0.0f)
	{
		float angle = getSteerDeltaAngle(true);

		//turn left
		if (angle < -mMinUnderSteerAngle)
		{
			float power = speedCoeff * FMath::Min(1.0f, (0.01f * mUnderSteerAssist) * -angle / maxSteerAngle);
			rl->setBraking(FMath::Min(1.0f, power));
			mEspActive = true;
			mUnderSteering = true;
		}
		//turn right
		if (angle > mMinUnderSteerAngle)
		{
			float power = speedCoeff * FMath::Min(1.0f, (0.01f * mUnderSteerAssist) * angle / maxSteerAngle);
			rr->setBraking(FMath::Min(1.0f, power));
			mEspActive = true;
			mUnderSteering = true;
		}
	}
}

void AFGearVehicle::handleHardContacts(float dt)
{
	if (mHardContactScale <= 0.0f) return;

	//calc. world inverse inertia
	FQuat worldRot = FGearUtility::getWorldTransform(mBody).GetRotation();
	FMatrix rotMat = FRotationMatrix::Make(worldRot);
	FMatrix worldIIT = (rotMat * mLocalIIT) * rotMat.GetTransposed();
	
	float biasFactor = 0.2f; //penetration resolve
	float invDt = 1.0f / dt;

	//calc. & apply required impulse to stop penetrations
	for (int i = 0; i < mAxleList.Num(); i++)
	{
		float pLeft = mAxleList[i]->getLeftWheel()->getSuspensionCompressedLength() * FGearUtility::meter2cm;
		float pRight = mAxleList[i]->getRightWheel()->getSuspensionCompressedLength() * FGearUtility::meter2cm;
		//left
		if (pLeft < 0.0f)
		{
			FHitResult hit = mAxleList[i]->getLeftWheel()->getRayHit();
			FVector relPt = hit.ImpactPoint - FGearUtility::getCenterOfMass(mBody);
			float kNormal = (1.0f / mMass) + FVector::DotProduct(FVector::CrossProduct(relPt, hit.ImpactNormal), worldIIT.TransformVector(FVector::CrossProduct(relPt, hit.ImpactNormal)));
			float massNormal = 1.0f / kNormal;
			float bias = -biasFactor * invDt * FMath::Min(0.0f, pLeft);
			FVector relVel = FGearUtility::getWorldVelocityAtPoint(mBody, hit.ImpactPoint);
			float vn = FVector::DotProduct(relVel, hit.ImpactNormal);
			float dPn = mHardContactScale * massNormal * (-vn + bias);
			dPn = FMath::Max(dPn, 0.0f);
			FVector pn = dPn * hit.ImpactNormal;
			addImpulseAtPosition(pn, hit.ImpactPoint);
		}
		//right
		if (pRight < 0.0f)
		{
			FHitResult hit = mAxleList[i]->getRightWheel()->getRayHit();
			FVector relPt = hit.ImpactPoint - FGearUtility::getCenterOfMass(mBody);
			float kNormal = (1.0f / mMass) + FVector::DotProduct(FVector::CrossProduct(relPt, hit.ImpactNormal), worldIIT.TransformVector(FVector::CrossProduct(relPt, hit.ImpactNormal)));
			float massNormal = 1.0f / kNormal;
			float bias = -biasFactor * invDt * FMath::Min(0.0f, pRight);
			FVector relVel = FGearUtility::getWorldVelocityAtPoint(mBody, hit.ImpactPoint);
			float vn = FVector::DotProduct(relVel, hit.ImpactNormal);
			float dPn = mHardContactScale * massNormal * (-vn + bias);
			dPn = FMath::Max(dPn, 0.0f);
			FVector pn = dPn * hit.ImpactNormal;
			addImpulseAtPosition(pn, hit.ImpactPoint);
		}
	}
}

void AFGearVehicle::updateStats(float dt)
{
	//reset stats if stopped or going backwards
	if (getKMHSpeed() <= 0.0f)
	{
		mZero100Time = 0.0f;
		mZero200Time = 0.0f;
		mGot100Result = false;
		mGot200Result = false;
		return;
	}

	//0-100
	if (!mGot100Result && mTransmission->getCurGear() > -1)
	{
		if (getKMHSpeed() < 100.0f) mZero100Time += dt;
		else mGot100Result = true;
	}

	//0-200
	if (!mGot200Result && mTransmission->getCurGear() > -1)
	{
		if (getKMHSpeed() < 200.0f) mZero200Time += dt;
		else mGot200Result = true;
	}

	//reset braking stat
	if (getKMHSpeed() >= 100.0f)
	{
		m100ZeroDistance = 0.0f;
		mGotZeroResult = false;
	}

	//100-0 braking
	if (!mGotZeroResult)
	{
		if (getKMHSpeed() < 100.0f && getKMHSpeed() > 0.0f)
		{
			m100ZeroDistance += getKMHSpeed() * FGearUtility::kmh2ms * dt;
		}
		else if (getKMHSpeed() <= 0.0f) mGotZeroResult = true;
	}
}

//get velocity from wheels or directly from rigidbody
//if the vehicle is on a moving platform mDetectGround should be set
FVector AFGearVehicle::getMyVelocity() const
{
	int axleNum = mAxleList.Num();
	if (!mDetectGround || axleNum == 0) return FGearUtility::getWorldVelocity(mBody);

	FVector vel = FVector::ZeroVector;
	for (int i = 0; i < axleNum; i++)
	{
		vel += mAxleList[i]->getLeftWheel()->getGroundVelocity();
		vel += mAxleList[i]->getRightWheel()->getGroundVelocity();
	}
	//zero axle case not checked
	return vel / (2.0f * axleNum);
}

//return the speed of fastest wheel
float AFGearVehicle::getMaxWheelKMHSpeed(bool onlyPowered, bool absValue) const
{
	float maxSpeed = 0.0f;
	float sign = 1.0f;
	for (int i = 0; i < mAxleList.Num(); i++)
	{
		if (!onlyPowered || mAxleList[i]->getTorqueShare() > 0.0f)
		{
			float cspeed = mAxleList[i]->getKMHSpeed();
			if (maxSpeed < FMath::Abs(cspeed))
			{
				maxSpeed = FMath::Abs(cspeed);
				if (!absValue) sign = FGearUtility::Sign(cspeed);
			}
		}
	}
	return sign * maxSpeed;
}

//return the speed of slowest wheel (check each wheel)
float AFGearVehicle::getMinWheelKMHSpeed(bool onlyPowered) const
{
	float speed = FLT_MAX;
	for (int i = 0; i < mAxleList.Num(); i++)
	{
		if (!onlyPowered || mAxleList[i]->getTorqueShare() > 0.0f)
		{
			speed = FMath::Min(speed, FMath::Abs(mAxleList[i]->getLeftWheel()->getKMHSpeed()));
			speed = FMath::Min(speed, FMath::Abs(mAxleList[i]->getRightWheel()->getKMHSpeed()));
		}
	}
	return speed;
}

//used for engine feedback
float AFGearVehicle::getFeedbackWheelRPM() const
{
	float finalRpm = 0.0f;
	float sign = 1.0f;
	for (int j = 0; j < mAxleList.Num(); j++)
	{
		if (mAxleList[j]->getTorqueShare() > 0.0f)
		{
			float crpm = mAxleList[j]->getRPM();
			if (finalRpm < FMath::Abs(crpm))
			{
				finalRpm = FMath::Abs(crpm);
				sign = FGearUtility::Sign(crpm);
			}
		}
	}
	return sign * finalRpm * mTransmission->getTransmissionRatio();
}

//used for engine braking
float AFGearVehicle::getWheelSpeedLimitKMH() const
{
	float limit = 0.0f;
	for (int i = 0; i < mAxleList.Num(); i++)
	{
		float leftSpeed = FMath::Abs(mAxleList[i]->getMaxRpm()) * mAxleList[i]->getLeftWheel()->getWheelRPMToKMH();
		float rightSpeed = FMath::Abs(mAxleList[i]->getMaxRpm()) * mAxleList[i]->getRightWheel()->getWheelRPMToKMH();
		if (leftSpeed > limit) limit = leftSpeed;
		if (rightSpeed > limit) limit = rightSpeed;
	}
	return limit * FMath::Sign(mTransmission->getTransmissionRatio());
}

//used for max speed calculations
float AFGearVehicle::getMaxWheelRPMToKMH() const
{
	float maxRatio = 0.0f;

	if (getAxleCount() != 0)
	{
		for (int i = 0; i < mAxleList.Num(); i++)
		{
			if (mAxleList[i]->getTorqueShare() > 0.0f)
			{
				maxRatio = FMath::Max(maxRatio, mAxleList[i]->getWheelRPMToKMH());
			}
		}
	}
	//can be called in editor time
	//so we need an alternative solution
	else
	{
		//get component list
		TArray<UActorComponent*> componentList;
		GetComponents(componentList);

		//iterate components and get axle/wheeloptions
		for (UActorComponent* comp : componentList)
		{
			if (comp && comp->GetClass() == UFGearWheelOptions::StaticClass())
			{
				UFGearWheelOptions* opts = Cast<UFGearWheelOptions>(comp);
				UFGearAxle* axle = Cast<UFGearAxle>(opts->GetAttachParent());
				if (axle && axle->getTorqueShare() > 0.0f)
				{
					float ratio = 2.0f * PI * opts->getRadius() * 60.0f / 1000.0f; //WheelRPMToKMH
					maxRatio = FMath::Max(maxRatio, ratio);
				}
			}
		}
	}
	
	return maxRatio;
}

//used in VehicleInput->limit steering angle
float AFGearVehicle::getMaxSteeringAngle() const
{
	float maxSteer = 0.0f;
	for (int i = 0; i < mAxleList.Num(); i++)
	{
		maxSteer = FMath::Max(maxSteer, mAxleList[i]->getMaxSteerAngle());
	}
	return maxSteer;
}

//used in ackerman calculations
float AFGearVehicle::getAxleDistance(int axle1, int axle2) const
{
	return FMath::Abs(mAxleList[axle1]->getLocalCenter().X - mAxleList[axle2]->getLocalCenter().X);
}

//used for hard contact simulation of suspensions
float AFGearVehicle::getTotalWheelLoad() const
{
	float loadSum = 0.0f;
	for (int i = 0; i < mAxleList.Num(); i++)
	{
		loadSum += mAxleList[i]->getLeftWheel()->getCurrentLoad();
		loadSum += mAxleList[i]->getRightWheel()->getCurrentLoad();
	}
	return loadSum;
}

//if any wheel has active abs then return true
bool AFGearVehicle::getABSActive() const
{
	for (int i = 0; i < mAxleList.Num(); i++)
	{
		if (mAxleList[i]->getLeftWheel()->isABSActive() || mAxleList[i]->getRightWheel()->isABSActive()) return true;
	}
	return false;
}

//if any wheel has active asr then return true
bool AFGearVehicle::getASRActive() const
{
	for (int i = 0; i < mAxleList.Num(); i++)
	{
		if (mAxleList[i]->getLeftWheel()->isASRActive() || mAxleList[i]->getRightWheel()->isASRActive()) return true;
	}
	return false;
}

FVector AFGearVehicle::getLocalInertiaTensor() const
{
	return FVector(1.0f / mLocalIIT.M[0][0], 1.0f / mLocalIIT.M[1][1], 1.0f / mLocalIIT.M[2][2]);
}

void AFGearVehicle::addForce(FVector f)
{
	mNetForce += f;
}

void AFGearVehicle::addRelativeForce(FVector f)
{
	addForce(getRotation() * f);
}

void AFGearVehicle::addForceAtPosition(FVector f, FVector p)
{
	FVector COM = FGearUtility::getCenterOfMass(mBody);
	FVector t = FVector::CrossProduct(p - COM, f);
	addForce(f);
	addTorque(t);
}

void AFGearVehicle::addTorque(FVector t)
{
	mNetTorque += t;
}

void AFGearVehicle::addImpulse(FVector i)
{
	mNetImpulse += i;
}

void AFGearVehicle::addImpulseAtPosition(FVector i, FVector p)
{
	FVector COM = FGearUtility::getCenterOfMass(mBody);
	FVector it = FVector::CrossProduct(p - COM, i);
	addImpulse(i);
	addImpulsiveTorque(it);
}

void AFGearVehicle::addImpulsiveTorque(FVector it)
{
	mNetImpulseTorque += it;
}

void AFGearVehicle::PostRenderFor(class APlayerController* PC, class UCanvas* Canvas, FVector CameraPosition, FVector CameraDir)
{
	Super::PostRenderFor(PC, Canvas, CameraPosition, CameraDir);
	if (!mTelemetry.mShow) return;
	if (GEngine == nullptr) return;

	UFont* Font = GEngine->GetSmallFont();
	FFontRenderInfo renderInfo;
	renderInfo.bEnableShadow = true;

	FVector2D p = FVector2D::ZeroVector;
	if (mTelemetry.mShowVehicle) p = drawTelemetryVehicle(Canvas, Font, mTelemetry.mTextColor, renderInfo, p);
	if (mTelemetry.mShowStats) drawTelemetryStats(Canvas, Font, mTelemetry.mTextColor, renderInfo, FVector2D(Canvas->SizeX - 105, Canvas->SizeY - 150));
	if (mTelemetry.mShowGauge) drawTelemetryGauge(Canvas, Font, mTelemetry.mTextColor, renderInfo, FVector2D(Canvas->SizeX - 105, Canvas->SizeY - 55));
	if (mTelemetry.mShowWheels) drawTelemetryWheels(Canvas, Font, mTelemetry.mTextColor, renderInfo, FVector2D(Canvas->SizeX - 855, 0));
	if (mTelemetry.mShowAeroDynamics) p = mAeroDynamics->drawTelemetry(Canvas, Font, mTelemetry.mTextColor, renderInfo, p);
	if (mTelemetry.mShowInput) p = mStandardInput->drawTelemetry(Canvas, Font, mTelemetry.mTextColor, renderInfo, p);
	if (mTelemetry.mShowEngine) p = mEngine->drawTelemetry(Canvas, Font, mTelemetry.mTextColor, renderInfo, p);
	if (mTelemetry.mShowTransmission) p = mTransmission->drawTelemetry(Canvas, Font, mTelemetry.mTextColor, renderInfo, p);
	if (mTelemetry.mShowReplication) p = mReplication->drawTelemetry(Canvas, Font, mTelemetry.mTextColor, renderInfo, p);
	if (mTelemetry.mShowAxles)
	{
		for (int i = 0; i < mAxleList.Num(); i++)
		{
			p = mAxleList[i]->drawTelemetry(Canvas, Font, mTelemetry.mTextColor, renderInfo, p);
		}
	}
	if (mTelemetry.mShowGizmos)
	{
		//center of mass
		FVector COM = FGearUtility::getCenterOfMass(mBody);
		DrawDebugSphere(GetWorld(), COM, 10.f, 32, FColor::Yellow, false, -1.0f, SDPG_MAX);
		// wheels
		for (int i = 0; i < mAxleList.Num(); i++)
		{
			mAxleList[i]->getLeftWheel()->drawGizmo(mTelemetry.mTextColor.ToFColor(true));
			mAxleList[i]->getRightWheel()->drawGizmo(mTelemetry.mTextColor.ToFColor(true));
		}
	}
}

void AFGearVehicle::PostInitProperties()
{
	Super::PostInitProperties();
	recalcRays();
}

//trigger a hit event on replication component so that we can blend network & physics transform for a while
void AFGearVehicle::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	//for simulated proxies, client authoritative only
	if (GetLocalRole() == ENetRole::ROLE_SimulatedProxy && mReplication->isEnabled() && !mReplication->isServerAuth())
	{
		//only with another vehicle
		if (AFGearVehicle* otherVehicle = Cast<AFGearVehicle>(Other))
		{
			//check relative speed vs threshold
			float relativeSpeed = (getMyVelocity() - otherVehicle->getMyVelocity()).Size() * FGearUtility::cms2kmh;
			if (relativeSpeed > mReplication->getMinCollisionBlendSpeed())
			{
				mReplication->resetCollisionTime();
				if (mTelemetry.mPrintCollisionLogs)
				{
					UE_LOG(LogTemp, Warning, TEXT("NotifyHit (%s)<->(%s) : %.1f/%.1f"), *GetName(), *Other->GetName(), relativeSpeed, mReplication->getMinCollisionBlendSpeed());
				}
			}
		}
	}

	//check wake up upon any hit event
	handleWakeUp();
}

#if WITH_EDITOR
void AFGearVehicle::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.MemberProperty)
	{
		FString propName = PropertyChangedEvent.MemberProperty->GetNameCPP();
		if (propName == "mMass") setMass(mMass);
		if (propName == "mCenterofMass") setCenterOfMassOffset(mCenterofMass);
		if (propName == "mInertiaScale") setInertiaScale(mInertiaScale);
		recalcRays();
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

bool AFGearVehicle::CanEditChange(const FProperty* Property) const
{
	const bool parentVal = Super::CanEditChange(Property);
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(AFGearVehicle, mRays)) return mQueryType == FGearQueryType::Line;
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(AFGearVehicle, mLateralRays)) return mQueryType == FGearQueryType::Line;
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(AFGearVehicle, mConvexAsset)) return mQueryType == FGearQueryType::Convex;
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(AFGearVehicle, mConvexUp)) return mQueryType == FGearQueryType::Convex;
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(AFGearVehicle, mConvexRotation)) return mQueryType == FGearQueryType::Convex;
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(AFGearVehicle, mConvexVisible)) return mQueryType == FGearQueryType::Convex;
	return parentVal;
}

FReply AFGearVehicle::onSaveClicked()
{
	FString path = FGearSerializer::getFilePath(true);
	if (!path.IsEmpty()) FGearSerializer::saveSettings(this, path);
	return FReply::Handled();
}

FReply AFGearVehicle::onLoadClicked()
{
	FString path = FGearSerializer::getFilePath(false);
	if (!path.IsEmpty()) FGearSerializer::loadSettings(this, path);
	return FReply::Handled();
}
#endif

void AFGearVehicle::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
	if (mOrbitCamera != nullptr && mOrbitCamera->IsActive())
	{
		mOrbitCamera->update(DeltaTime, OutResult);
	}
	else Super::CalcCamera(DeltaTime, OutResult);
}

void AFGearVehicle::recalcRays()
{
	if (mRays % 2 == 0)	mRays -= 1;
}

//-----------------------------------------------------------------------SLEEP CODE BEGIN
void AFGearVehicle::sleep()
{
	if (HasAuthority() || (mReplication->isEnabled() && !mReplication->isServerAuth()))
	{
		handleSleep();
	}
	else if (IsLocallyControlled()) serverSleep();
}

void AFGearVehicle::wakeUp()
{
	if (HasAuthority() || (mReplication->isEnabled() && !mReplication->isServerAuth()))
	{
		handleWakeUp();
	}
	else if (IsLocallyControlled()) serverWakeUp();
}

bool AFGearVehicle::updateSleeping(float dt)
{
	//only work on server if running server-auth
	if (mReplication->isEnabled() && mReplication->isServerAuth() && !HasAuthority())
	{
		return getIsSleeping();
	}

	if (mBody && mMesh && mMesh->IsAnyRigidBodyAwake())
	{
		if (mBody->GetUnrealWorldVelocity().Size() * FGearUtility::cms2kmh < mSleepLinearThreshold &&
			mBody->GetUnrealWorldAngularVelocityInRadians().Size() < FMath::DegreesToRadians(mSleepAngularThreshold))
		{
			mSleepTime += dt;
			if (mSleepTime >= mSleepWaitingTime) handleSleep();
		}
		else
		{
			mSleepTime = 0.0f;
		}
	}
	return getIsSleeping();
}

void AFGearVehicle::handleSleep()
{
	//only work on server if running server-auth
	if (mReplication->isEnabled() && mReplication->isServerAuth() && !HasAuthority())
	{
		return;
	}

	if (!mBody || getIsSleeping()) return;

	//skip sleep when in contact with another simulated actor
	FCollisionObjectQueryParams objectParams(FCollisionObjectQueryParams::AllDynamicObjects);
	FCollisionQueryParams queryParams; queryParams.AddIgnoredActor(this);
	FBox bounds = mBody->GetBodyBounds();
	FCollisionShape boxShape = FCollisionShape::MakeBox(bounds.GetExtent());
	TArray<FOverlapResult> results;
	if (GetWorld()->OverlapMultiByObjectType(results, bounds.GetCenter(), FQuat::Identity, objectParams, boxShape, queryParams))
	{
		mSleepTime = 0.0f;
		return;
	}

	mInSleep = true;
	mSleepTime = mSleepWaitingTime;
	if (onSleepEvent.IsBound()) onSleepEvent.Broadcast(this);

	if (mBody->bSimulatePhysics)
	{
		mLastLinearVelocity = mBody->GetUnrealWorldVelocity();
		mLastAngularVelocity = mBody->GetUnrealWorldAngularVelocityInRadians();
		mBody->SetLinearVelocity(FVector::ZeroVector, false, false);
		mBody->SetAngularVelocityInRadians(FVector::ZeroVector, false, false);
		if (mMesh) mMesh->PutAllRigidBodiesToSleep();
	}
}

void AFGearVehicle::handleWakeUp()
{
	//only work on server if running server-auth
	if (mReplication->isEnabled() && mReplication->isServerAuth() && !HasAuthority())
	{
		return;
	}

	if (!mBody || !getIsSleeping()) return;

	mInSleep = false;
	mSleepTime = 0.0f;
	if (onWakeEvent.IsBound()) onWakeEvent.Broadcast(this);

	if (mBody->bSimulatePhysics)
	{
		if (mMesh && !mMesh->IsAnyRigidBodyAwake()) mMesh->WakeAllRigidBodies();
		mBody->SetLinearVelocity(mLastLinearVelocity, false);
		mBody->SetAngularVelocityInRadians(mLastAngularVelocity, false);
	}
}

void AFGearVehicle::serverSleep_Implementation()
{
	handleSleep();
}

void AFGearVehicle::serverWakeUp_Implementation()
{
	handleWakeUp();
}

void AFGearVehicle::onWake(UPrimitiveComponent* component, FName bone)
{
	handleWakeUp();
}

void AFGearVehicle::onRepInSleep()
{
	if (mInSleep && onSleepEvent.IsBound()) onSleepEvent.Broadcast(this);
	else if (!mInSleep && onWakeEvent.IsBound()) onWakeEvent.Broadcast(this);

	if (mInSleep && mBody && mBody->bSimulatePhysics)
	{
		mBody->SetLinearVelocity(FVector::ZeroVector, false, false);
		mBody->SetAngularVelocityInRadians(FVector::ZeroVector, false, false);
		if (mMesh) mMesh->PutAllRigidBodiesToSleep();
	}
}
//-----------------------------------------------------------------------SLEEP CODE END

void AFGearVehicle::prepareTelemetry()
{
	if (!IsLocallyControlled()) return;

	APlayerController* pc = Cast<APlayerController>(GetController());
	if (pc == nullptr) return;

	AHUD* hud = pc->GetHUD();
	if (hud == nullptr) return;

	hud->bShowOverlays = mTelemetry.mShow;
	if (mTelemetry.mShow) hud->AddPostRenderedActor(this);
	else hud->RemovePostRenderedActor(this);
}


void AFGearVehicle::clientPrepareTelemetry_Implementation()
{
	prepareTelemetry();
}

FVector2D AFGearVehicle::drawTelemetryVehicle(UCanvas* Canvas, UFont* Font, FLinearColor textColor, FFontRenderInfo renderInfo, FVector2D p)
{
	Canvas->SetLinearDrawColor(textColor);
	Canvas->DrawText(Font, "Vehicle", p.X, p.Y, 1.0f, 1.0f, renderInfo);
	FGearUtility::drawLine(Canvas, p.X, p.Y + 18, p.X + 43, p.Y + 18, textColor);
	Canvas->DrawText(Font, FString::Printf(TEXT("Mass: %.0f kg"), mMass), p.X, p.Y + 20, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("Update Rate: %d hz"), mUpdateRate), p.X, p.Y + 35, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("Total Raycasts: %d"), (mLateralRays * mRays)), p.X, p.Y + 50, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("ABS: %%%.0f"), mABS), p.X, p.Y + 65, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("ASR: %%%.0f"), mASR), p.X, p.Y + 80, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("Steering Assist: %%%.0f"), mStandardInput->getSteeringAssist()), p.X, p.Y + 95, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("Oversteer Assist: %%%.0f"), mOverSteerAssist), p.X, p.Y + 110, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("Understeer Assist: %%%.0f"), mUnderSteerAssist), p.X, p.Y + 125, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("Front Antiroll: %%%.0f"), mFrontAntiRollPower), p.X, p.Y + 140, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("Rear Antiroll: %%%.0f"), mRearAntiRollPower), p.X, p.Y + 155, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, "Sleeping: " + UKismetStringLibrary::Conv_BoolToString(mInSleep), p.X, p.Y + 170, 1.0f, 1.0f, renderInfo);
	p.Y += 200;

	return p;
}

FVector2D AFGearVehicle::drawTelemetryStats(UCanvas* Canvas, UFont* Font, FLinearColor textColor, FFontRenderInfo renderInfo, FVector2D p)
{
	Canvas->SetLinearDrawColor(textColor);
	Canvas->DrawText(Font, "Statistics", p.X, p.Y, 1.0f, 1.0f, renderInfo);
	FGearUtility::drawLine(Canvas, p.X, p.Y + 18, p.X + 58, p.Y + 18, textColor);
	Canvas->DrawText(Font, FString::Printf(TEXT("Speed: %.1f kmh"), getKMHSpeed()), p.X, p.Y + 20, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("0-100: %.1f s"), mZero100Time), p.X, p.Y + 35, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("0-200: %.1f s"), mZero200Time), p.X, p.Y + 50, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("100-0: %.1f m"), m100ZeroDistance), p.X, p.Y + 65, 1.0f, 1.0f, renderInfo);
	p.Y += 200;
	return p;
}

FVector2D AFGearVehicle::drawTelemetryWheels(UCanvas* Canvas, UFont* Font, FLinearColor textColor, FFontRenderInfo renderInfo, FVector2D p)
{
	Canvas->SetLinearDrawColor(textColor);
	Canvas->DrawText(Font, "Wheels", p.X, p.Y, 1.0f, 1.0f, renderInfo);
	FGearUtility::drawLine(Canvas, p.X, p.Y + 18, p.X + 43, p.Y + 18, textColor);
	Canvas->DrawText(Font, "Speed", p.X + 23, p.Y + 20, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, "Torque", p.X + 70, p.Y + 20, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, "Load", p.X + 120, p.Y + 20, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, "RideHeight", p.X + 160, p.Y + 20, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, "SteerAngle", p.X + 240, p.Y + 20, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, "TireFriction(F/R)", p.X + 310, p.Y + 20, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, "ContactFriction", p.X + 415, p.Y + 20, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, "LngForce", p.X + 515, p.Y + 20, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, "LngSlip", p.X + 580, p.Y + 20, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, "SlipRatio", p.X + 630, p.Y + 20, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, "LatForce", p.X + 690, p.Y + 20, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, "LatSlip", p.X + 750, p.Y + 20, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, "SlipAngle", p.X + 800, p.Y + 20, 1.0f, 1.0f, renderInfo);
	p.Y += 20;
	for (int i = 0; i < mAxleList.Num(); i++)
	{
		p = mAxleList[i]->getLeftWheel()->drawTelemetry(Canvas, Font, textColor, renderInfo, p);
		p = mAxleList[i]->getRightWheel()->drawTelemetry(Canvas, Font, textColor, renderInfo, p);
		p.Y += 5;
	}
	return p;
}

FVector2D AFGearVehicle::drawTelemetryGauge(UCanvas* Canvas, UFont* Font, FLinearColor textColor, FFontRenderInfo renderInfo, FVector2D p)
{
	Canvas->SetLinearDrawColor(textColor);

	//rpm
	FGearUtility::drawRect(Canvas, p.X, p.Y + 20, p.X + 90, p.Y + 35, textColor, true, false);
	FGearUtility::drawRect(Canvas, p.X, p.Y + 20, p.X + 90 * mEngine->getRpmRatio(), p.Y + 35, textColor, false, true);

	//gear text
	FString gearStr = "N";
	if (mTransmission->getCurGear() > 0)
	{
		gearStr = FString::SanitizeFloat(mTransmission->getCurGear(), 0);
	}
	else if (mTransmission->getCurGear() < 0) gearStr = "R";
	Canvas->DrawText(Font, gearStr, p.X + 80, p.Y - 2, 1.5f, 1.5f, renderInfo);

	//speed text
	Canvas->DrawText(Font, FString::Printf(TEXT("%.0f"), getMaxWheelKMHSpeed(true, true)), p.X, p.Y - 10.0f, 2.0f, 2.0f, renderInfo);

	//clutch
	FGearUtility::drawRect(Canvas, p.X, p.Y + 38, p.X + 90, p.Y + 43, textColor, true, false);
	FGearUtility::drawRect(Canvas, p.X, p.Y + 38, p.X + 90 * (1.0f - mTransmission->getClutchState()), p.Y + 43, FLinearColor::Blue, false, true);

	//steering bar
	float hw = 0.5f * Canvas->SizeX;
	float steer = getNormalizedSteering() * 100.0f;
	FGearUtility::drawLine(Canvas, hw - 100.0f, p.Y + 35, hw + 100.0f, p.Y + 35, textColor);
	FGearUtility::drawLine(Canvas, hw, p.Y + 30, hw, p.Y + 40, textColor);
	FGearUtility::drawLine(Canvas, hw + steer, p.Y + 30, hw + steer, p.Y + 40, FLinearColor::Red);

	//traction
	{
		float flgas = mEngine->getThrottle() * getAxle(0)->getTorqueShare();
		float frgas = mEngine->getThrottle() * getAxle(0)->getTorqueShare();
		float rlgas = mEngine->getThrottle() * getAxle(1)->getTorqueShare();
		float rrgas = mEngine->getThrottle() * getAxle(1)->getTorqueShare();

		float flbrake = getAxle(0)->getLeftWheel()->getBraking();
		float frbrake = getAxle(0)->getRightWheel()->getBraking();
		float rlbrake = getAxle(1)->getLeftWheel()->getBraking();
		float rrbrake = getAxle(1)->getRightWheel()->getBraking();

		//box
		FGearUtility::drawRect(Canvas, p.X - 80, p.Y - 40, p.X - 60, p.Y - 0, textColor, true, false);
		FGearUtility::drawRect(Canvas, p.X - 50, p.Y - 40, p.X - 30, p.Y - 0, textColor, true, false);
		FGearUtility::drawRect(Canvas, p.X - 80, p.Y + 10, p.X - 60, p.Y + 50, textColor, true, false);
		FGearUtility::drawRect(Canvas, p.X - 50, p.Y + 10, p.X - 30, p.Y + 50, textColor, true, false);

		//throttle
		FGearUtility::drawRect(Canvas, p.X - 80, p.Y - 0, p.X - 60, p.Y - 40 * flgas, FLinearColor::Green, false, true);
		FGearUtility::drawRect(Canvas, p.X - 50, p.Y - 0, p.X - 30, p.Y - 40 * frgas, FLinearColor::Green, false, true);
		FGearUtility::drawRect(Canvas, p.X - 80, p.Y + 50, p.X - 60, p.Y + 50 - 40 * rlgas, FLinearColor::Green, false, true);
		FGearUtility::drawRect(Canvas, p.X - 50, p.Y + 50, p.X - 30, p.Y + 50 - 40 * rrgas, FLinearColor::Green, false, true);

		//brake
		FGearUtility::drawRect(Canvas, p.X - 80, p.Y - 40, p.X - 60, p.Y - 40 + (flbrake * 40), FLinearColor::Red, false, true);
		FGearUtility::drawRect(Canvas, p.X - 50, p.Y - 40, p.X - 30, p.Y - 40 + (frbrake * 40), FLinearColor::Red, false, true);
		FGearUtility::drawRect(Canvas, p.X - 80, p.Y + 10, p.X - 60, p.Y + 10 + (rlbrake * 40), FLinearColor::Red, false, true);
		FGearUtility::drawRect(Canvas, p.X - 50, p.Y + 10, p.X - 30, p.Y + 10 + (rrbrake * 40), FLinearColor::Red, false, true);
	}

	//suspension
	{
		float fls = getAxle(0)->getLeftWheel()->getSuspensionCompressRatio();
		float frs = getAxle(0)->getRightWheel()->getSuspensionCompressRatio();
		float rls = getAxle(1)->getLeftWheel()->getSuspensionCompressRatio();
		float rrs = getAxle(1)->getRightWheel()->getSuspensionCompressRatio();

		//box
		FGearUtility::drawRect(Canvas, p.X - 150, p.Y - 40, p.X - 130, p.Y - 0, textColor, true, false);
		FGearUtility::drawRect(Canvas, p.X - 120, p.Y - 40, p.X - 100, p.Y - 0, textColor, true, false);
		FGearUtility::drawRect(Canvas, p.X - 150, p.Y + 10, p.X - 130, p.Y + 50, textColor, true, false);
		FGearUtility::drawRect(Canvas, p.X - 120, p.Y + 10, p.X - 100, p.Y + 50, textColor, true, false);

		//ratio
		FGearUtility::drawRect(Canvas, p.X - 150, p.Y - 0, p.X - 130, p.Y - 40 * fls, FLinearColor(fls, 1.0f - fls, 0.0f), false, true);
		FGearUtility::drawRect(Canvas, p.X - 120, p.Y - 0, p.X - 100, p.Y - 40 * frs, FLinearColor(frs, 1.0f - frs, 0.0f), false, true);
		FGearUtility::drawRect(Canvas, p.X - 150, p.Y + 50, p.X - 130, p.Y + 50 - 40 * rls, FLinearColor(rls, 1.0f - rls, 0.0f), false, true);
		FGearUtility::drawRect(Canvas, p.X - 120, p.Y + 50, p.X - 100, p.Y + 50 - 40 * rrs, FLinearColor(rrs, 1.0f - rrs, 0.0f), false, true);
	}

	return p;
}

void AFGearVehicle::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFGearVehicle, mInSleep);
}

void AFGearVehicle::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);
	DOREPLIFETIME_ACTIVE_OVERRIDE(AFGearVehicle, mInSleep, mReplication->isEnabled() && mReplication->isServerAuth());
}