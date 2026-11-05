// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearPlugin.h"

#if WITH_EDITOR
#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"
#include "FGearCustomPanel.h"
#endif

#define LOCTEXT_NAMESPACE "FFGearPluginModule"

void FFGearPluginModule::StartupModule()
{
#if WITH_EDITOR
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked< FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.RegisterCustomClassLayout("FGearVehicle", FOnGetDetailCustomizationInstance::CreateStatic(&FGearCustomPanel::MakeInstance));
	PropertyEditorModule.RegisterCustomClassLayout("FGearEngine", FOnGetDetailCustomizationInstance::CreateStatic(&FGearCustomPanel::MakeInstance));
	PropertyEditorModule.RegisterCustomClassLayout("FGearWheelOptions", FOnGetDetailCustomizationInstance::CreateStatic(&FGearCustomPanel::MakeInstance));
	PropertyEditorModule.RegisterCustomClassLayout("FGearTransmission", FOnGetDetailCustomizationInstance::CreateStatic(&FGearCustomPanel::MakeInstance));
	// hide unnecessary categories
	PropertyEditorModule.RegisterCustomClassLayout("FGearSkeletalMeshComponent", FOnGetDetailCustomizationInstance::CreateStatic(&FGearCustomPanel::MakeInstance));
	PropertyEditorModule.RegisterCustomClassLayout("FGearStandardInput", FOnGetDetailCustomizationInstance::CreateStatic(&FGearCustomPanel::MakeInstance));
	PropertyEditorModule.RegisterCustomClassLayout("FGearAeroDynamics", FOnGetDetailCustomizationInstance::CreateStatic(&FGearCustomPanel::MakeInstance));
	PropertyEditorModule.RegisterCustomClassLayout("FGearAxle", FOnGetDetailCustomizationInstance::CreateStatic(&FGearCustomPanel::MakeInstance));
	PropertyEditorModule.RegisterCustomClassLayout("FGearWheel", FOnGetDetailCustomizationInstance::CreateStatic(&FGearCustomPanel::MakeInstance));
	PropertyEditorModule.RegisterCustomClassLayout("FGearReplication", FOnGetDetailCustomizationInstance::CreateStatic(&FGearCustomPanel::MakeInstance));
#endif
}

void FFGearPluginModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFGearPluginModule, FGearPlugin)