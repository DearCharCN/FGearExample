// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "FGearAnimInstance.generated.h"

USTRUCT(BlueprintType)
struct FGearBoneTransform
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "FGear", DisplayName = "Position")
	FVector position = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "FGear", DisplayName = "Rotation")
	FRotator rotation = FRotator::ZeroRotator;
};

USTRUCT(BlueprintType)
struct FGearBoneAxle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "FGear", DisplayName = "Left Wheel")
	FGearBoneTransform left;
	UPROPERTY(BlueprintReadOnly, Category = "FGear", DisplayName = "Right Wheel")
	FGearBoneTransform right;
};

UCLASS(transient, Blueprintable, hideCategories = AnimInstance, BlueprintType)
class FGEARPLUGIN_API UFGearAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UFGearAnimInstance();

	UPROPERTY(BlueprintReadOnly, Category = "FGear", DisplayName = "Axles")
	TArray<FGearBoneAxle> axles;

	void init(int num);
	void update(int axle, bool isLeft, FVector p, FRotator r);
};