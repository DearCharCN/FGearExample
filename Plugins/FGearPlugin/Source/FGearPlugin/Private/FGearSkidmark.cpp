// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearSkidmark.h"
#include "FGearUtility.h"

AFGearSkidmark::AFGearSkidmark() :
	AActor(),
	markIndex(0),
	vertIndex(0)
{
	PrimaryActorTick.bCanEverTick = true;

	marksMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("SkidmarkMesh"));
	marksMesh->bReceiveMobileCSMShadows = false;
	marksMesh->bCastDynamicShadow = false;
	marksMesh->SetCastShadow(false);
	SetRootComponent(marksMesh);
}

void AFGearSkidmark::setMaterial(UMaterialInterface * m)
{
	marksMesh->SetMaterial(0, m);
}

int AFGearSkidmark::addSkidMark(FVector pos, FVector normal, float intensity, int lastIndex)
{
	if (intensity > 1.0f) intensity = 1.0f;
	else if (intensity < 0.0f) return -1.0f;

	if (lastIndex > 0)
	{
		float sqrDistance = (pos - skidmarks[lastIndex].pos).SizeSquared();
		if (sqrDistance < mMinSQRdistance) return lastIndex;
	}

	skidmarks[markIndex].pos = pos + normal * mGroundOffset;
	skidmarks[markIndex].normal = normal;
	skidmarks[markIndex].intensity = (uint8)(intensity * 255.0f);
	skidmarks[markIndex].lastIndex = lastIndex;

	if (lastIndex != -1)
	{
		FVector dir = (skidmarks[markIndex].pos - skidmarks[lastIndex].pos);
		FVector xDir = FVector::CrossProduct(dir, normal).GetSafeNormal();

		skidmarks[markIndex].posL = skidmarks[markIndex].pos + xDir * mMarkWidth * 0.5f;
		skidmarks[markIndex].posR = skidmarks[markIndex].pos - xDir * mMarkWidth * 0.5f;
		skidmarks[markIndex].tangent = FProcMeshTangent(FVector(xDir.X, xDir.Y, xDir.Z), true);

		if (skidmarks[lastIndex].lastIndex == -1)
		{
			skidmarks[lastIndex].tangent = skidmarks[markIndex].tangent;
			skidmarks[lastIndex].posL = skidmarks[markIndex].pos + xDir * mMarkWidth * 0.5f;
			skidmarks[lastIndex].posR = skidmarks[markIndex].pos - xDir * mMarkWidth * 0.5f;
		}
	}

	updateSkidmarksMesh();

	int curIndex = markIndex;
	// Update circular index
	markIndex = ++markIndex % mMaxMarks;

	return curIndex;
}

void AFGearSkidmark::init(int maxMarks, float markWidth, float groundOffset, float minDistance)
{
	mMaxMarks = maxMarks;
	mMarkWidth = markWidth * FGearUtility::meter2cm;
	mGroundOffset = groundOffset * -1 * FGearUtility::meter2cm;
	mMinSQRdistance = (minDistance * minDistance) * FGearUtility::meter2cm;
	// Generate a fixed array of skidmarks
	skidmarks.Init(FGearMarkSection(), mMaxMarks);
}

void AFGearSkidmark::updateSkidmarksMesh()
{
	// Nothing to connect to yet
	if (skidmarks[markIndex].lastIndex == -1) return;

	if (vertices.Num() < mMaxMarks * 4)
	{
		for (int i = 0;i < 4;i++)
		{
			vertices.Add(FVector::ZeroVector);
			normals.Add(FVector::ZeroVector);
			tangents.Add(FProcMeshTangent());
			colors.Add(FLinearColor());
			uvs.Add(FVector2D::ZeroVector);
			triangles.Add(0);
		}
		triangles.Add(0);
		triangles.Add(0);
	}

	vertices[vertIndex * 4 + 0] = skidmarks[skidmarks[markIndex].lastIndex].posL;
	vertices[vertIndex * 4 + 1] = skidmarks[skidmarks[markIndex].lastIndex].posR;
	vertices[vertIndex * 4 + 2] = skidmarks[markIndex].posL;
	vertices[vertIndex * 4 + 3] = skidmarks[markIndex].posR;

	normals[vertIndex * 4 + 0] = skidmarks[skidmarks[markIndex].lastIndex].normal;
	normals[vertIndex * 4 + 1] = skidmarks[skidmarks[markIndex].lastIndex].normal;
	normals[vertIndex * 4 + 2] = skidmarks[markIndex].normal;
	normals[vertIndex * 4 + 3] = skidmarks[markIndex].normal;

	tangents[vertIndex * 4 + 0] = skidmarks[skidmarks[markIndex].lastIndex].tangent;
	tangents[vertIndex * 4 + 1] = skidmarks[skidmarks[markIndex].lastIndex].tangent;
	tangents[vertIndex * 4 + 2] = skidmarks[markIndex].tangent;
	tangents[vertIndex * 4 + 3] = skidmarks[markIndex].tangent;

	colors[vertIndex * 4 + 0] = FLinearColor(0, 0, 0, skidmarks[skidmarks[markIndex].lastIndex].intensity);
	colors[vertIndex * 4 + 1] = FLinearColor(0, 0, 0, skidmarks[skidmarks[markIndex].lastIndex].intensity);
	colors[vertIndex * 4 + 2] = FLinearColor(0, 0, 0, skidmarks[markIndex].intensity);
	colors[vertIndex * 4 + 3] = FLinearColor(0, 0, 0, skidmarks[markIndex].intensity);

	uvs[vertIndex * 4 + 0] = FVector2D(0, 0);
	uvs[vertIndex * 4 + 1] = FVector2D(1, 0);
	uvs[vertIndex * 4 + 2] = FVector2D(0, 1);
	uvs[vertIndex * 4 + 3] = FVector2D(1, 1);

	triangles[vertIndex * 6 + 0] = vertIndex * 4 + 0;
	triangles[vertIndex * 6 + 2] = vertIndex * 4 + 1;
	triangles[vertIndex * 6 + 1] = vertIndex * 4 + 2;

	triangles[vertIndex * 6 + 3] = vertIndex * 4 + 2;
	triangles[vertIndex * 6 + 5] = vertIndex * 4 + 1;
	triangles[vertIndex * 6 + 4] = vertIndex * 4 + 3;

	marksMesh->CreateMeshSection_LinearColor(0, vertices, triangles, normals, uvs, colors, tangents, false);

	vertIndex = ++vertIndex % mMaxMarks;
}