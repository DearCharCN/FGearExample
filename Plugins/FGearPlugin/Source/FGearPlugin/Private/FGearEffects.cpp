// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearEffects.h"
#include "FGearVehicle.h"
#include "FGearAxle.h"
#include "FGearWheel.h"
#include "FGearEngine.h"
#include "FGearTransmission.h"
#include "FGearSkidmark.h"
#include "FGearPhysicalMaterial.h"
#include "Engine/World.h"
#include "Sound/SoundWave.h"
#include "FGearSkeletalMeshComponent.h"

UFGearEffects::UFGearEffects() :
	UFGearTelemetryDrawer(),

	mStopEffectsWhenSleeping(false),
	mStopEngineSoundWhenSleeping(false),
	mLongSlipThreshold(0.2f),
	mLatSlipThreshold(0.2f),
	mEnableTireSmoke(true),
	mEnableSkidmarks(true),
	mMaxMarks(128),
	mMarkWidth(0.0f),
	mGroundOffset(0.01f),
	mMinDistance(0.0f),

	mBaseVolume(1.0f),
	mEngineVolume(0.3f),
	mSkidVolume(0.2f),
	
	mEngineHighBasePitch(1.0f),
	mEngineHighPitchScale(1.0f),
	mEngineHighInnerRadius(1000.0f),
	mEngineHighFalloffDistance(1000.0f),

	mEngineLowBasePitch(1.0f),
	mEngineLowPitchScale(1.0f),
	mEngineLowInnerRadius(1000.0f),
	mEngineLowFalloffDistance(1000.0f),

	mEnableSkidclip(true),
	mSkidclipInnerRadius(1000.0f),
	mSkidclipFalloffDistance(2000.0f),

	mBackfireInterval(0.1f),
	mBackfireLimit(1000.0f),

	mFireRpmRatio(25.0f),
	mFireGearChange(80.0f),
	mFireLimiter(50.0f),

	mDefaultPhysicalMaterial(nullptr),

	mLastRpm(0.0f),
	mLastFire(0.0f),
	mPrevBraking(false),

	mSleepEventsRegistered(false)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PostPhysics;
}

void UFGearEffects::BeginPlay()
{
	Super::BeginPlay();

	mVehicle = Cast<AFGearVehicle>(GetOwner());
	if (mVehicle == nullptr) return;

	setStopEffectsWhenSleeping(mStopEffectsWhenSleeping, mStopEngineSoundWhenSleeping);
	
	int wheels = mVehicle->getAxleCount() * 2;
	mWheels.Init(0, wheels);
	for (int i = 0; i < mVehicle->getAxleCount(); i++)
	{
		mWheels[i * 2] = mVehicle->getAxle(i)->getLeftWheel();
		mWheels[i * 2 + 1] = mVehicle->getAxle(i)->getRightWheel();
	}

	//smokes
	mSmokes.Init(nullptr, wheels);
	for (int i = 0; i < wheels; i++)
	{
		mSmokes[i] = NewObject<UNiagaraComponent>(this, UNiagaraComponent::StaticClass());
		if (mSmokes[i])
		{
			mSmokes[i]->RegisterComponent();
			mSmokes[i]->SetRelativeLocation(FVector::ZeroVector);
			mSmokes[i]->AttachToComponent(this, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
			mSmokes[i]->bAutoActivate = false;
		}
	}

	//sounds
	mEngineHighComponent = initAudio(mEngineHigh, mEngineHighBasePitch, mEngineHighPitchScale, mEngineHighInnerRadius, mEngineHighFalloffDistance);
	mEngineLowComponent = initAudio(mEngineLow, mEngineLowBasePitch, mEngineLowPitchScale, mEngineLowInnerRadius, mEngineLowFalloffDistance);
	if (mEnableSkidclip)
	{
		mSkidSoundComponent = initAudio(nullptr, 0.0f, 0.0f, mSkidclipInnerRadius, mSkidclipFalloffDistance, true);
		mSkidSoundComponent->SetVolumeMultiplier(mBaseVolume * mSkidVolume);
	}
}

void UFGearEffects::OnRegister()
{
	Super::OnRegister();

	if (mSkidSoundComponent != nullptr)
	{
		mSkidSoundComponent->AttenuationOverrides.AttenuationShapeExtents.X = mSkidclipInnerRadius;
		mSkidSoundComponent->AttenuationOverrides.FalloffDistance = mSkidclipFalloffDistance;
		mSkidSoundComponent->SetVolumeMultiplier(mBaseVolume * mSkidVolume);
	}
	if (mEngineLowComponent != nullptr)
	{
		if (mEngineLow != nullptr) mEngineLowComponent->SetSound((USoundBase*)mEngineLow);
		mEngineLowComponent->AttenuationOverrides.AttenuationShapeExtents.X = mEngineLowInnerRadius;
		mEngineLowComponent->AttenuationOverrides.FalloffDistance = mEngineLowFalloffDistance;
	}
	if (mEngineHighComponent != nullptr)
	{
		if (mEngineLow != nullptr) mEngineHighComponent->SetSound((USoundBase*)mEngineHigh);
		mEngineHighComponent->AttenuationOverrides.AttenuationShapeExtents.X = mEngineHighInnerRadius;
		mEngineHighComponent->AttenuationOverrides.FalloffDistance = mEngineHighFalloffDistance;
	}
}

void UFGearEffects::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	for (int i = 0; i < mSmarks.Num(); i++)
	{
		if (mSmarks[i] != nullptr && mSmarks[i]->IsValidLowLevel())
		{
			mSmarks[i]->Destroy();
			mSmarks[i] = nullptr;
		}
	}
	
	setStopEffectsWhenSleeping(false, false);

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

void UFGearEffects::setStopEffectsWhenSleeping(bool stopEffects, bool stopEngineSound)
{
	mStopEffectsWhenSleeping = stopEffects;
	mStopEngineSoundWhenSleeping = stopEngineSound;

	if ((stopEffects || stopEngineSound) && !mSleepEventsRegistered)
	{
		mVehicle->onWakeEvent.AddDynamic(this, &UFGearEffects::onWake);
		mVehicle->onSleepEvent.AddDynamic(this, &UFGearEffects::onSleep);
		mSleepEventsRegistered = true;
	}
	else if ((!stopEffects && !stopEngineSound) && mSleepEventsRegistered)
	{
		mVehicle->onWakeEvent.RemoveDynamic(this, &UFGearEffects::onWake);
		mVehicle->onSleepEvent.RemoveDynamic(this, &UFGearEffects::onSleep);
		mSleepEventsRegistered = false;
	}
}

void UFGearEffects::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (mVehicle == nullptr) return;
	if (mVehicle->getBody() == nullptr) return;

	physicsSurfaceUpdate();
	effectUpdate(DeltaTime);
	backfireUpdate(DeltaTime);
}

//check each wheels hit surface physics material
//change wheel friction acc. to surface friction value
//apply wheel drag acc. to surface roughness value
void UFGearEffects::physicsSurfaceUpdate()
{
	for (int i = 0; i < mWheels.Num(); i++)
	{
		UFGearWheel* w = mWheels[i];
		w->setFrictionFactor(1.0f);
		w->setAdditionalDrag(0.0f);

		FHitResult hit = w->getRayHit();
		if (hit.PhysMaterial == nullptr) continue;
		if (hit.PhysMaterial->GetClass() != UFGearPhysicalMaterial::StaticClass()) continue;

		UFGearPhysicalMaterial* physicalMaterial = Cast<UFGearPhysicalMaterial>(hit.PhysMaterial);
		w->setFrictionFactor(physicalMaterial->mFriction);
		w->setAdditionalDrag(physicalMaterial->mRoughness);
	}
}

//update skid effectss, engine sounds etc.
void UFGearEffects::effectUpdate(float dt)
{
	int skidCount = 0;
	float maxSlip = 0.0f;
	USoundWave* skidSound = nullptr;

	checkSkidmarks();
	for (int i = 0; i < mWheels.Num(); i++)
	{
		UFGearWheel* w = mWheels[i];

		//longitudinal slip
		float lngSlip = FMath::Abs(w->getSlipRatio()) * FMath::Min(1.0f, FMath::Abs(w->getLongitudinalSlip()));
		//lateral slip
		float latSlip = FMath::Abs(w->getSlipAngle()) * FMath::Min(1.0f, FMath::Abs(w->getLateralSlip()));

		//check conditions
		FHitResult hit = w->getRayHit();

		UFGearPhysicalMaterial* physicalMaterial = nullptr;
		if (hit.PhysMaterial != nullptr && hit.PhysMaterial->GetClass() == UFGearPhysicalMaterial::StaticClass()) physicalMaterial = Cast<UFGearPhysicalMaterial>(hit.PhysMaterial);
		else physicalMaterial = mDefaultPhysicalMaterial;

		bool cs = w->hasContact() && (lngSlip > mLongSlipThreshold || latSlip > mLatSlipThreshold) && physicalMaterial != nullptr;

		//skid marks
		if (cs && mEnableSkidmarks && physicalMaterial->mReceiveSkidmark && mSmarks[i] != nullptr)
		{
			mSkids[i] = mSmarks[i]->addSkidMark(hit.ImpactPoint, hit.ImpactNormal * -1.0f, w->getSuspensionCompressRatio(), mSkids[i]);
		}
		else if (mSkids.IsValidIndex(i)) mSkids[i] = -1;

		//smokes
		if (mEnableTireSmoke && cs && physicalMaterial->mSmokeParticle != nullptr)
		{
			if (mSmokes[i]->GetAsset() == nullptr || mSmokes[i]->GetAsset() != physicalMaterial->mSmokeParticle)
				mSmokes[i]->SetAsset(physicalMaterial->mSmokeParticle);
			mSmokes[i]->Activate();
			mSmokes[i]->SetWorldLocation(hit.ImpactPoint);
		}
		else
		{
			mSmokes[i]->Deactivate();
		}

		//skid clip
		if (cs && mEnableSkidclip && physicalMaterial->mSkidClip != nullptr)
		{
			skidCount++;
			maxSlip = FMath::Max(maxSlip, FMath::Max(lngSlip, latSlip));
			skidSound = physicalMaterial->mSkidClip;
		}
	}

	//skid clip
	if (skidCount > 0)
	{
#if ENGINE_MAJOR_VERSION == 5
		if (mSkidSoundComponent->Sound.Get() != skidSound)
#else
		if (mSkidSoundComponent->Sound != skidSound)
#endif
		{
			mSkidSoundComponent->Stop();
			skidSound->bLooping = true;
			mSkidSoundComponent->SetSound(skidSound);
		}
		if (!mSkidSoundComponent->IsPlaying()) mSkidSoundComponent->Play();

		mSkidSoundComponent->SetPitchMultiplier(FMath::Min(0.75f, FMath::Max(0.4f, 0.1f + maxSlip)));
	}
	else
	{
		if (mSkidSoundComponent != nullptr && mSkidSoundComponent->IsPlaying())
		{
			mSkidSoundComponent->Stop();
		}
	}

	//set engine sound pitch/volme acc. to throttle & rpm
	if (mEngineHighComponent != nullptr)
	{
		float throttle = mVehicle->getEngine()->getThrottle();
		float vol = mBaseVolume * mEngineVolume * (0.75f + 0.25f * throttle);
		if (!mVehicle->getEngine()->isRunning()) vol = 0.0f;
		mEngineHighComponent->SetVolumeMultiplier(vol);
		mEngineHighComponent->SetPitchMultiplier(mEngineHighBasePitch + (mEngineHighPitchScale * mVehicle->getEngine()->getRpmRatio()));
	}
	if (mEngineLowComponent != nullptr)
	{
		float throttle = mVehicle->getEngine()->getThrottle();
		float vol = mBaseVolume * mEngineVolume * (1.0f - 0.25f * throttle);
		if (!mVehicle->getEngine()->isRunning()) vol = 0.0f;
		mEngineLowComponent->SetVolumeMultiplier(vol);
		mEngineLowComponent->SetPitchMultiplier(mEngineLowBasePitch + (mEngineLowPitchScale * mVehicle->getEngine()->getRpmRatio()));
	}

	//braking
	bool braking = mVehicle->getAxle(0)->getLeftWheel()->getBraking() > 0.0f;
	if (braking != mPrevBraking)
	{
		if (OnBrakeStateChanged.IsBound())	OnBrakeStateChanged.Broadcast(braking);
		onBrakeStateChanged(braking);
	}
	mPrevBraking = braking;
}

//fire backfire event if the conditions are met
void UFGearEffects::backfireUpdate(float dt)
{
	if (!mVehicle->getEngine()->isRunning()) return;
	float rpm = mVehicle->getEngine()->getRpm();
	float idle = mVehicle->getEngine()->getIdleRpm();
	float delta = rpm - mLastRpm;
	if (delta == 0.0f || rpm < 1.02f * idle) return; //1.02 -> see engine idle rpm limit
	float speed = delta / dt;
	mLastRpm = rpm;

	bool trigger = speed < -mBackfireLimit && FMath::RandRange(0.0f, 100.0f) < mFireRpmRatio;
	bool throttle = mVehicle->getEngine()->getThrottle() < 0.9f;
	bool gear = mVehicle->getTransmission()->isChanging() && mVehicle->getTransmission()->getCurGear() != 0 && FMath::RandRange(0.0f, 100.0f) < mFireGearChange;
	bool limit = mVehicle->getEngine()->isLimiterOn() && FMath::RandRange(0.0f, 100.0f) < mFireLimiter;
	trigger &= throttle;
	trigger |= gear;
	trigger |= limit;

	if (mLastFire <= 0.0f)
	{
		if (trigger)
		{
			if (OnBackFire.IsBound()) OnBackFire.Broadcast();
			onBackfire();
		}
		mLastFire = FMath::RandRange(0.75f * mBackfireInterval, 1.25f * mBackfireInterval);
	}
	else if (mLastFire > 0.0f) mLastFire -= dt;
}

void UFGearEffects::checkSkidmarks()
{
	//skid marks
	if (mEnableSkidmarks && mSkids.Num() == 0 && mWheels.Num() > 0)
	{
		mSkids.Init(0, mWheels.Num());
		mSmarks.Init(nullptr, mWheels.Num());
		for (int i = 0; i < mWheels.Num(); i++)
		{
			mSmarks[i] = GetWorld()->SpawnActor<AFGearSkidmark>();
			if (mSmarks[i] != nullptr)
			{
				const float width = mMarkWidth > 0.0f ? mMarkWidth : mWheels[i]->getAxle()->getWheelOptions()->getWidth();
				const float dist = mMinDistance > 0.0f ? mMinDistance : width * 4.0f;
				mSmarks[i]->init(mMaxMarks, width, mGroundOffset, dist);
				mSmarks[i]->setMaterial(mSkidmarkMaterial);
			}
		}
	}
}

UAudioComponent* UFGearEffects::initAudio(USoundWave * sound, float basePitch, float pitchScale, float innerRadius, float falloffDistance, bool forceCreate)
{
	if (sound == nullptr && !forceCreate) return nullptr;

	UAudioComponent* comp = NewObject<UAudioComponent>(this, UAudioComponent::StaticClass());
	if (comp)
	{
		comp->RegisterComponent();
		comp->AttachToComponent(this, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
		if (sound != nullptr)
		{
			sound->bLooping = true;
			comp->SetSound(sound);
		}
		comp->bOverrideAttenuation = true;
		comp->PitchModulationMin = basePitch;
		comp->PitchModulationMax = basePitch + pitchScale;

		FSoundAttenuationSettings AttenuationSettings;
		AttenuationSettings.bAttenuate = true;
		AttenuationSettings.bSpatialize = true;
		AttenuationSettings.DistanceAlgorithm = EAttenuationDistanceModel::Linear;
		AttenuationSettings.AttenuationShape = EAttenuationShape::Sphere;
		AttenuationSettings.AttenuationShapeExtents.X = innerRadius;
		AttenuationSettings.FalloffDistance = falloffDistance;
		comp->AdjustAttenuation(AttenuationSettings);
		comp->Play();
	}
	return comp;
}

void UFGearEffects::onWake(AFGearVehicle* vehicle)
{
	if (mStopEffectsWhenSleeping)
	{
		if (mSkidSoundComponent && mSkidSoundComponent->bIsPaused) mSkidSoundComponent->SetPaused(false);
		for (int i = 0; i < mSmokes.Num(); i++)
		{
			if (mSmokes[i]) mSmokes[i]->SetVisibility(true);
		}
	}

	if (mStopEngineSoundWhenSleeping)
	{
		if (mEngineHighComponent && mEngineHighComponent->bIsPaused) mEngineHighComponent->SetPaused(false);
		if (mEngineLowComponent && mEngineLowComponent->bIsPaused) mEngineLowComponent->SetPaused(false);
	}
}

void UFGearEffects::onSleep(AFGearVehicle* vehicle)
{
	if (mStopEffectsWhenSleeping)
	{
		if (mSkidSoundComponent && mSkidSoundComponent->IsPlaying()) mSkidSoundComponent->SetPaused(true);
		for (int i = 0; i < mSmokes.Num(); i++)
		{
			if (mSmokes[i]) mSmokes[i]->SetVisibility(false);
		}
	}

	if (mStopEngineSoundWhenSleeping)
	{
		if (mEngineHighComponent && mEngineHighComponent->IsPlaying()) mEngineHighComponent->SetPaused(true);
		if (mEngineLowComponent && mEngineLowComponent->IsPlaying()) mEngineLowComponent->SetPaused(true);
	}
}

void UFGearEffects::updateVolumes()
{
	if (mEnableSkidclip && mSkidSoundComponent) mSkidSoundComponent->SetVolumeMultiplier(mBaseVolume * mSkidVolume);
}