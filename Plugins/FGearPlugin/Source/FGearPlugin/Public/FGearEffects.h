// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FGearTelemetryDrawer.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "FGearEffects.generated.h"

class AFGearVehicle;
class UFGearWheel;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGearBrakeSignature, bool, state);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGearExhaustSignature);

UCLASS(meta = (BlueprintSpawnableComponent), ClassGroup = (FGear))
class FGEARPLUGIN_API UFGearEffects : public UFGearTelemetryDrawer
{
	GENERATED_BODY()
public:
	UFGearEffects();

	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void BeginPlay() override;
	virtual void OnRegister() override;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FGearBrakeSignature OnBrakeStateChanged;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FGearExhaustSignature OnBackFire;

	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setStopEffectsWhenSleeping(bool stopEffects, bool stopEngineSound);
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setBaseVolume(float v) { mBaseVolume = v; updateVolumes(); }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getBaseVolume() const { return mBaseVolume; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setEngineVolume(float v) { mEngineVolume = v; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getEngineVolume() const { return mEngineVolume; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setSkidVolume(float v) { mSkidVolume = v; updateVolumes(); }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getSkidVolume() const { return mSkidVolume; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setEnableSkidmarks(bool e) { mEnableSkidmarks = e; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool getEnableSkidmarks() const { return mEnableSkidmarks; }
	UFUNCTION(BlueprintPure, Category = "FGear")
	bool getStopEffectsWhenSleeping() const { return mStopEffectsWhenSleeping; }
	UFUNCTION(BlueprintPure, Category = "FGear")
	bool getStopEngineSoundWhenSleeping() const { return mStopEngineSoundWhenSleeping; }

protected:
	virtual void onBrakeStateChanged(bool state) {}
	virtual void onBackfire() {}

private:
	void physicsSurfaceUpdate();
	void effectUpdate(float dt);
	void backfireUpdate(float dt);
	void checkSkidmarks();
	UAudioComponent* initAudio(USoundWave* sound, float basePitch, float pitchScale, float innerRadius, float falloffDistance, bool forceCreate = false);

	UFUNCTION()
	void onWake(AFGearVehicle* vehicle);
	UFUNCTION()
	void onSleep(AFGearVehicle* vehicle);
private:
	void updateVolumes();

	UPROPERTY(EditAnywhere, Category = "FGearEffects", DisplayName = "Stop Effects When Sleeping")
	bool mStopEffectsWhenSleeping;
	UPROPERTY(EditAnywhere, Category = "FGearEffects", DisplayName = "Stop Engine Sound When Sleeping")
	bool mStopEngineSoundWhenSleeping;
	UPROPERTY(EditAnywhere, Category = "FGearEffects", DisplayName = "Long Slip Threshold")
	float mLongSlipThreshold;
	UPROPERTY(EditAnywhere, Category = "FGearEffects", DisplayName = "Lat Slip Threshold")
	float mLatSlipThreshold;
	UPROPERTY(EditAnywhere, Category = "FGearEffects", DisplayName = "Enable Tire Smoke")
	bool mEnableTireSmoke;

	/** Enable or disable skidmarks. You must use FGearPhysicalMaterial to your surface in order to see marks */
	UPROPERTY(EditAnywhere, Category = "FGearEffects|Skidmark", DisplayName = "Enable Skidmark")
	bool mEnableSkidmarks;
	/** Skidmark material */
	UPROPERTY(EditAnywhere, Category = "FGearEffects|Skidmark", DisplayName = "Skidmark Material", meta = (EditCondition = "mEnableSkidmarks"))
	UMaterialInterface* mSkidmarkMaterial;
	/** Max number of marks total for everyone together */
	UPROPERTY(EditAnywhere, Category = "FGearEffects|Skidmark", DisplayName = "Skidmark Max Marks", meta = (EditCondition = "mEnableSkidmarks"))
	int mMaxMarks;
	/** Width of the skidmarks. Should match the width of the wheels. If it is zero, it automatically takes the wheel width from wheel options */
	UPROPERTY(EditAnywhere, Category = "FGearEffects|Skidmark", DisplayName = "Skidmark Mark Width", meta = (EditCondition = "mEnableSkidmarks"))
	float mMarkWidth;
	/** Distance above surface in meters */
	UPROPERTY(EditAnywhere, Category = "FGearEffects|Skidmark", DisplayName = "Skidmark Ground Offset", meta = (EditCondition = "mEnableSkidmarks"))
	float mGroundOffset;
	/** Distance between skid texture sections in meters. If it is zero, it is calculated as four times the width. Higher = better performance, less smooth */
	UPROPERTY(EditAnywhere, Category = "FGearEffects|Skidmark", DisplayName = "Skidmark Min Distance", meta = (EditCondition = "mEnableSkidmarks"))
	float mMinDistance;
	UPROPERTY(NoClear)
	TArray<class AFGearSkidmark*> mSmarks;
	TArray<int> mSkids;

	UPROPERTY(EditAnywhere, Category = "FGearEffects|SFX", DisplayName = "Base Volume")
	float mBaseVolume;
	UPROPERTY(EditAnywhere, Category = "FGearEffects|SFX", DisplayName = "Engine Volume")
	float mEngineVolume;
	UPROPERTY(EditAnywhere, Category = "FGearEffects|SFX", DisplayName = "Skid Volume")
	float mSkidVolume;

	UPROPERTY(EditAnywhere, Category = "FGearEffects|SFX", DisplayName = "Engine High Sound Wave")
	USoundWave* mEngineHigh;
	UPROPERTY(EditAnywhere, Category = "FGearEffects|SFX", DisplayName = "Engine High Base Pitch")
	float mEngineHighBasePitch;
	/** Formula: sound pitch = base pitch + pitch scale * rpm ratio */
	UPROPERTY(EditAnywhere, Category = "FGearEffects|SFX", DisplayName = "Engine High Pitch Scale")
	float mEngineHighPitchScale;
	UPROPERTY(EditAnywhere, Category = "FGearEffects|SFX", DisplayName = "Engine High Inner Radius")
	float mEngineHighInnerRadius;
	UPROPERTY(EditAnywhere, Category = "FGearEffects|SFX", DisplayName = "Engine High Falloff Distance")
	float mEngineHighFalloffDistance;
	UPROPERTY(NoClear)
	UAudioComponent* mEngineHighComponent;

	UPROPERTY(EditAnywhere, Category = "FGearEffects|SFX", DisplayName = "Engine Low Sound Wave")
	USoundWave* mEngineLow;
	UPROPERTY(EditAnywhere, Category = "FGearEffects|SFX", DisplayName = "Engine Low Base Pitch")
	float mEngineLowBasePitch;
	/** Formula: sound pitch = base pitch + pitch scale * rpm ratio */
	UPROPERTY(EditAnywhere, Category = "FGearEffects|SFX", DisplayName = "Engine Low Pitch Scale")
	float mEngineLowPitchScale;
	UPROPERTY(EditAnywhere, Category = "FGearEffects|SFX", DisplayName = "Engine Low Inner Radius")
	float mEngineLowInnerRadius;
	UPROPERTY(EditAnywhere, Category = "FGearEffects|SFX", DisplayName = "Engine Low Falloff Distance")
	float mEngineLowFalloffDistance;
	UPROPERTY(NoClear)
	UAudioComponent* mEngineLowComponent;

	UPROPERTY(EditAnywhere, Category = "FGearEffects|SFX", DisplayName = "Enable Skid Sound")
	bool mEnableSkidclip;
	UPROPERTY(EditAnywhere, Category = "FGearEffects|SFX", DisplayName = "Skid Sound Inner Radius", meta = (EditCondition = "mEnableSkidclip"))
	float mSkidclipInnerRadius;
	UPROPERTY(EditAnywhere, Category = "FGearEffects|SFX", DisplayName = "Skid Sound Falloff Distance", meta = (EditCondition = "mEnableSkidclip"))
	float mSkidclipFalloffDistance;
	UPROPERTY(NoClear)
	UAudioComponent* mSkidSoundComponent;

	UPROPERTY(EditAnywhere, Category = "FGearEffects|Backfire", DisplayName = "Minumum Interval")
	float mBackfireInterval;
	UPROPERTY(EditAnywhere, Category = "FGearEffects|Backfire", DisplayName = "Minimum Rpm Delta")
	float mBackfireLimit;
	UPROPERTY(EditAnywhere, Category = "FGearEffects|Backfire", DisplayName = "Fire Rpm Delta Ratio(%)", meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "1.0", UIMax = "100.0"))
	float mFireRpmRatio;
	UPROPERTY(EditAnywhere, Category = "FGearEffects|Backfire", DisplayName = "Fire Gear Change Ratio(%)", meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "1.0", UIMax = "100.0"))
	float mFireGearChange;
	UPROPERTY(EditAnywhere, Category = "FGearEffects|Backfire", DisplayName = "Fire Limiter Ratio(%)", meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "1.0", UIMax = "100.0"))
	float mFireLimiter;

	/** Default physical material */
	UPROPERTY(EditAnywhere, Category = "FGearEffects|DefaultSurface", DisplayName = "Default Physical Material")
	class UFGearPhysicalMaterial* mDefaultPhysicalMaterial;

	UPROPERTY(NoClear)
	TArray<UNiagaraComponent*> mSmokes;

	AFGearVehicle* mVehicle;
	TArray<UFGearWheel*> mWheels;

	//backfire helpers
	float mLastRpm;
	float mLastFire;

	bool mPrevBraking;
	bool mSleepEventsRegistered;
};