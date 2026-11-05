// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearUtility.h"
#include "FGearVehicle.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsEngine/BodyInstance.h"
#if ENGINE_MAJOR_VERSION == 5
#include "PhysicsProxy/SingleParticlePhysicsProxy.h"
#endif

float FGearUtility::SignedAngle(FVector vec1, FVector vec2, FVector nAxis)
{
	float Angle = FMath::Acos(FVector::DotProduct(vec1.GetSafeNormal(), vec2.GetSafeNormal()));
	FVector cross = FVector::CrossProduct(vec1, vec2);
	if (FVector::DotProduct(nAxis, cross) < 0.0f) Angle = -Angle;
	return FMath::RadiansToDegrees(Angle);
}

FQuat FGearUtility::Euler(float x, float y, float z)
{
	return FQuat::MakeFromEuler(FVector(z, x, y));
}

float FGearUtility::Angle(FVector vec1, FVector vec2)
{
	float Angle = 0.0f;
	FVector nAxis;
	FQuat BetweenQuat = FQuat::FindBetweenVectors(vec1, vec2);
	BetweenQuat.ToAxisAndAngle(nAxis, Angle);
	return FMath::RadiansToDegrees(Angle);
}

float FGearUtility::LerpAngle(float s, float t, float d)
{
	return FMath::Lerp(FRotator(0, s, 0), FRotator(0, t, 0), d).Yaw;
}

static void OrthoNormalize(FVector& Normal, FVector& Tangent)
{
	Normal = Normal.GetSafeNormal();
	Tangent = Tangent - (Normal * FVector::DotProduct(Tangent, Normal));
	Tangent = Tangent.GetSafeNormal();
}

void FGearUtility::AddTorqueAtPosition(AFGearVehicle* vehicle, FVector position, FVector torque)
{
	if (torque.SizeSquared() < epsilon) return;
	FVector torqueAxis = torque.GetSafeNormal();
	FVector ortho = FVector::RightVector;

	//prevent torqueAxis and ortho from pointing in the same direction
	if ((torqueAxis - ortho).SizeSquared() < epsilon) ortho = FVector::UpVector;

	//calculate force
	OrthoNormalize(torqueAxis, ortho);
	FVector force = FVector::CrossProduct(0.5f * torque, ortho);
	vehicle->addForceAtPosition(force, position + meter2cm * ortho);
	vehicle->addForceAtPosition(-force, position - meter2cm * ortho);
}

void FGearUtility::RotateQuat(FQuat& rot, FVector dv)
{
	FQuat q(dv.X, dv.Y, dv.Z, 0.0f);
	q *= rot;
	rot.X += 0.5f * q.X;
	rot.Y += 0.5f * q.Y;
	rot.Z += 0.5f * q.Z;
	rot.W += 0.5f * q.W;
	rot.Normalize();
}

FTransform FGearUtility::getWorldTransform(const FBodyInstance* body)
{
#if ENGINE_MAJOR_VERSION == 5
	if (FPhysicsActorHandle actorHandle = body->ActorHandle)
	{
		if (!IsInGameThread())
		{
			if (Chaos::FRigidBodyHandle_Internal* rigidHandle = actorHandle->GetPhysicsThreadAPI())
			{
				if (ensure(rigidHandle)) return FTransform(rigidHandle->R(), rigidHandle->X());
			}
		}
		else
		{
			Chaos::FRigidBodyHandle_External& rigidHandle = actorHandle->GetGameThreadAPI();
			return FTransform(rigidHandle.R(), rigidHandle.X());
		}
	}
#endif
	return body->GetUnrealWorldTransform();
}

FVector FGearUtility::getWorldVelocityAtPoint(const FBodyInstance* body, const FVector& location)
{
#if ENGINE_MAJOR_VERSION == 5
	if (FPhysicsActorHandle actorHandle = body->ActorHandle)
	{
		FVector diff = location - getCenterOfMass(body);
		if (!IsInGameThread())
		{
			if (Chaos::FRigidBodyHandle_Internal* rigidHandle = actorHandle->GetPhysicsThreadAPI())
			{
				if (ensure(rigidHandle)) return rigidHandle->V() - Chaos::FVec3::CrossProduct(diff, rigidHandle->W());
			}
		}
		else
		{
			Chaos::FRigidBodyHandle_External& rigidHandle = actorHandle->GetGameThreadAPI();
			return rigidHandle.V() - Chaos::FVec3::CrossProduct(diff, rigidHandle.W());
		}
	}
#endif
	return body->GetUnrealWorldVelocityAtPoint(location);
}

FVector FGearUtility::getWorldVelocity(const FBodyInstance* body)
{
#if ENGINE_MAJOR_VERSION == 5
	if (FPhysicsActorHandle actorHandle = body->ActorHandle)
	{
		if (!IsInGameThread())
		{
			if (Chaos::FRigidBodyHandle_Internal* rigidHandle = actorHandle->GetPhysicsThreadAPI())
			{
				if (ensure(rigidHandle)) return rigidHandle->V();
			}
		}
		else
		{
			Chaos::FRigidBodyHandle_External& rigidHandle = actorHandle->GetGameThreadAPI();
			return rigidHandle.V();
		}
	}
#endif
	return body->GetUnrealWorldVelocity();
}

FVector FGearUtility::getWorldAngularVelocity(const FBodyInstance* body)
{
#if ENGINE_MAJOR_VERSION == 5
	if (FPhysicsActorHandle actorHandle = body->ActorHandle)
	{
		if (!IsInGameThread())
		{
			if (Chaos::FRigidBodyHandle_Internal* rigidHandle = actorHandle->GetPhysicsThreadAPI())
			{
				if (ensure(rigidHandle)) return rigidHandle->W();
			}
		}
		else
		{
			Chaos::FRigidBodyHandle_External& rigidHandle = actorHandle->GetGameThreadAPI();
			return rigidHandle.W();
		}
	}
#endif
	return body->GetUnrealWorldAngularVelocityInRadians();
}

FVector FGearUtility::getCenterOfMass(const FBodyInstance* body)
{
#if ENGINE_MAJOR_VERSION == 5
	if (FPhysicsActorHandle actorHandle = body->ActorHandle)
	{
		if (!IsInGameThread())
		{
			if (Chaos::FRigidBodyHandle_Internal* rigidHandle = actorHandle->GetPhysicsThreadAPI())
			{
				if (ensure(rigidHandle)) return rigidHandle->X() + rigidHandle->R() * rigidHandle->CenterOfMass();
			}
		}
		else
		{
			Chaos::FRigidBodyHandle_External& rigidHandle = actorHandle->GetGameThreadAPI();
			return rigidHandle.X() + rigidHandle.R() * rigidHandle.CenterOfMass();
		}
	}
#endif
	return body->GetCOMPosition();
}

//[-1, 1] to uint8
uint8 FGearUtility::floatToByte(float f)
{
	return (uint8)FMath::Clamp((f + 1.0f) * 127.0f, 0.0f, 255.0f);
}

//uint8 to [-1, 1]
float FGearUtility::byteToFloat(uint8 u)
{
	return ((float)u / 127.0f) - 1.0f;
}

void FGearUtility::drawLine(UCanvas* Canvas, float x1, float y1, float x2, float y2, FLinearColor color, bool shadowed)
{
	if (shadowed)
	{
		FCanvasLineItem LineItem(FVector2D(x1 + 1, y1 + 1), FVector2D(x2 + 1, y2 + 1));
		LineItem.SetColor(FLinearColor::Black);
		LineItem.LineThickness = 1;
		Canvas->DrawItem(LineItem);
	}

	FCanvasLineItem LineItem(FVector2D(x1, y1), FVector2D(x2, y2));
	LineItem.SetColor(color);
	LineItem.LineThickness = 1;
	Canvas->DrawItem(LineItem);
}

void FGearUtility::drawRect(UCanvas* Canvas, float x1, float y1, float x2, float y2, FLinearColor color, bool borders, bool filled)
{
	float w = FMath::Abs(x1 - x2);
	float h = FMath::Abs(y1 - y2);

	if (borders)
	{
		FCanvasBoxItem boxItem(FVector2D(x1, y1), FVector2D(w, h));
		boxItem.SetColor(color);
		Canvas->DrawItem(boxItem);
	}

	if (filled)
	{
		if (w > h)
		{
			float dx = (x1 < x2 ? 0.5f : -0.5f) * h;
			FCanvasLineItem LineItem(FVector2D(x1 + dx, 0.5f * (y1 + y2)), FVector2D(x2 - dx, 0.5f * (y1 + y2)));
			LineItem.SetColor(color);
			LineItem.LineThickness = h;
			Canvas->DrawItem(LineItem);
		}
		else
		{
			float dw = (y1 < y2 ? 0.5f : -0.5f) * w;
			FCanvasLineItem LineItem(FVector2D(0.5f * (x1 + x2), y1 + dw), FVector2D(0.5f * (x1 + x2), y2 - dw));
			LineItem.SetColor(color);
			LineItem.LineThickness = w;
			Canvas->DrawItem(LineItem);
		}
	}
}