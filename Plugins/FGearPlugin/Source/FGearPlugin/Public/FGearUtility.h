// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Engine/Canvas.h"
#include <limits>

struct FBodyInstance;

#define stringify(name) #name

class FGearUtility
{
public:
	static bool substepping;
	static constexpr float rads2Rpm = 60.0f / (2.0f * PI);
	static constexpr float rpm2Rads = (2.0f * PI) / 60.0f;
	static constexpr float ms2kmh = 3.6f;
	static constexpr float kmh2ms = 0.277778f;
	static constexpr float cms2kmh = 0.036f;
	static constexpr float kw2hp = 1.34102f;
	static constexpr float cm2meter = 0.01f;
	static constexpr float meter2cm = 100.0f;
	static constexpr float epsilon = std::numeric_limits<float>::epsilon();

	static float SignedAngle(FVector vec1, FVector vec2, FVector nAxis);
	static FQuat Euler(float x, float y, float z);
	static float Angle(FVector vec1, FVector vec2);
	static float LerpAngle(float s, float t, float d);
	template<class T> static T LerpUnclamped(const T& A, const T& B, const float& Alpha);
	static void AddTorqueAtPosition(class AFGearVehicle* vehicle, FVector position, FVector torque);
	static void RotateQuat(FQuat& rot, FVector dv);

	static FTransform getWorldTransform(const FBodyInstance* body);
	static FVector getWorldVelocityAtPoint(const FBodyInstance* body, const FVector& location);
	static FVector getWorldVelocity(const FBodyInstance* body);
	static FVector getWorldAngularVelocity(const FBodyInstance* body);
	static FVector getCenterOfMass(const FBodyInstance* body);

	static uint8 floatToByte(float f);
	static float byteToFloat(uint8 u);

	static void drawLine(UCanvas* Canvas, float x1, float y1, float x2, float y2, FLinearColor color, bool shadowed = true);
	static void drawRect(UCanvas* Canvas, float x1, float y1, float x2, float y2, FLinearColor color, bool borders, bool filled);

	template<typename T> static FString EnumToString(const FString& enumName, const T value);

	template<class T> static T Sign(const T A)
	{
		return (A >= (T)0) ? (T)1 : (T)-1;
	}
};

template<class T>
inline T FGearUtility::LerpUnclamped(const T & A, const T & B, const float & Alpha)
{
	return (T)(A + Alpha * (B - A));
}

template<typename T>
inline FString FGearUtility::EnumToString(const FString & enumName, const T value)
{
	const UEnum* enm =
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
	FindFirstObjectSafe<UEnum>(*enumName);
#else
	FindObject<UEnum>(ANY_PACKAGE, *enumName);
#endif
	return *(enm ? enm->GetNameStringByIndex(static_cast<uint8>(value)) : "Unknown");
}