// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class AFGearVehicle;
class FJsonValue;

class FGearSerializer
{
public:
	/** Serialize vehicle and attached component properties and save */
	static bool saveSettings(AFGearVehicle* v, FString path);
	/** Deserialize file and set vehicle and attached component properties */
	static bool loadSettings(AFGearVehicle* v, FString path);

#if WITH_EDITOR
	/** Get file path for save/load */
	static FString getFilePath(bool isSave);
#endif

private:
	/** Convert vector to FJsonValue array */
	static TArray<TSharedPtr<FJsonValue>> fromVectorToArray(FVector v);
	/** Convert FJsonValue array to vector */
	static FVector fromArrayToVector(TArray<TSharedPtr<FJsonValue>> arr);
	/** Convert rotator to FJsonValue array */
	static TArray<TSharedPtr<FJsonValue>> fromRotatorToArray(FRotator r);
	/** Convert FJsonValue array to rotator */
	static FRotator fromArrayToRotator(TArray<TSharedPtr<FJsonValue>> arr);
	/** Convert struct to serialized string */
	static FString fromStructToString(const UStruct* StructDefinition, const void* Struct);
	/** Convert TArray to FJsonValue array */
	static TArray<TSharedPtr<FJsonValue>> fromTArrayToJsonArray(TArray<float> src);
	/** Convert FJsonValue array to TArray */
	static TArray<float> fromJsonArrayToFloatArray(TArray<TSharedPtr<FJsonValue>> src);
	/** Get object path */
	static FORCEINLINE FString getObjPath(const UObject* obj);
	/** Load and get object */
	template <typename ObjClass>
	static FORCEINLINE ObjClass* loadObjFromPath(const FString& path);
};

template<typename ObjClass>
inline ObjClass* FGearSerializer::loadObjFromPath(const FString& path)
{
	if (path == FString("")) return NULL;
	return Cast<ObjClass>(StaticLoadObject(ObjClass::StaticClass(), NULL, *path));
}