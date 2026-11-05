// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FGearVehicle.h"
#include "FGearRewindReplay.generated.h"

UENUM()
enum class FGearRewindReplayState : uint8
{
	RS_PLAY 	UMETA(DisplayName = "Play"),
	RS_REWIND 	UMETA(DisplayName = "Rewind"),
	RS_REPLAY	UMETA(DisplayName = "Replay"),
	RS_PAUSE	UMETA(DisplayName = "Pause")
};

UCLASS(ClassGroup = (FGear), hidecategories = (Rendering, Replication, Input, Actor, Collision, LOD, Cooking))
class FGEARPLUGIN_API AFGearRewindReplay : public AActor
{
	GENERATED_BODY()

public:	
	AFGearRewindReplay();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "FGear")
	void play();
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void pause();
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void replay(bool resetIndex);
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void rewind(bool resetIndex);

	UFUNCTION(BlueprintCallable, Category = "FGear")
	FGearRewindReplayState getState() { return mState; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setReplayFrames(int count);
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getProgressValue() const;

protected:
	void addState(float dt);
	void replayState(float dt);
	void rewindState(float dt);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FGearRewindReplay", DisplayName = "Vehicles")
	TArray<AFGearVehicle*> mVehicleList;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FGearRewindReplay", DisplayName = "Replay Frames", meta = (ClampMin = "1", ClampMax = "8196", UIMin = "1", UIMax = "8196"))
	int mStateSize;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FGearRewindReplay", DisplayName = "Pause on Rewind End")
	bool mRewindPause;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FGearRewindReplay", DisplayName = "Pause on Replay End")
	bool mReplayPause;

	//self state
	FGearRewindReplayState mState;
	TArray<FFGearVehicleState> mStateBuffer;
	int mPlayIndex;
	int mRewindIndex;
	bool mFirstTick;
};