// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "FGearSkidmark.generated.h"

USTRUCT()
struct FGearMarkSection
{
	GENERATED_BODY()

	FVector pos;
	FVector normal;
	FProcMeshTangent tangent;
	FVector posL;
	FVector posR;
	uint8 intensity;
	int lastIndex;

	FGearMarkSection()
	{
		pos = FVector::ZeroVector;
		normal = FVector::ZeroVector;
		posL = FVector::ZeroVector;
		posR = FVector::ZeroVector;
		intensity = 0;
		lastIndex = 0;
	}
};

UCLASS(NotPlaceable)
class FGEARPLUGIN_API AFGearSkidmark : public AActor
{
	GENERATED_BODY()
public:
	AFGearSkidmark();
	void init(int maxMarks, float markWidth, float groundOffset, float minDistance);
	void setMaterial(UMaterialInterface* m);
	int addSkidMark(FVector pos, FVector normal, float intensity, int lastIndex);

private:
	int mMaxMarks;
	float mMarkWidth;
	float mGroundOffset;
	float mMinSQRdistance;

	int markIndex;
	int vertIndex;
	TArray<FGearMarkSection> skidmarks;
	UProceduralMeshComponent* marksMesh;

	TArray<FVector> vertices;
	TArray<FVector> normals;
	TArray<FProcMeshTangent> tangents;
	TArray<FLinearColor> colors;
	TArray<FVector2D> uvs;
	TArray<int32> triangles;

	void updateSkidmarksMesh();
};