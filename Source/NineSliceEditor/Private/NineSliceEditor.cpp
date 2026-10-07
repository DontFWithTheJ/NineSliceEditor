#include "NineSliceEditor.h"

#include "Details/MarginCustomization.h"

#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"

void FNineSliceEditor::StartupModule()
{
    FPropertyEditorModule& PropertyEditorModule =
        FModuleManager::LoadModuleChecked<FPropertyEditorModule>(
            "PropertyEditor"
        );

    PropertyEditorModule.UnregisterCustomPropertyTypeLayout(
        TEXT("Margin")
    );

    PropertyEditorModule.RegisterCustomPropertyTypeLayout(
        TEXT("Margin"),
        FOnGetPropertyTypeCustomizationInstance::CreateStatic(
            &FMarginCustomization::MakeInstance
        )
    );

    PropertyEditorModule.NotifyCustomizationModuleChanged();
}

void FNineSliceEditor::ShutdownModule()
{
    if (!FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
    {
        return;
    }

    FPropertyEditorModule& PropertyEditorModule =
        FModuleManager::GetModuleChecked<FPropertyEditorModule>(
            "PropertyEditor"
        );

    PropertyEditorModule.UnregisterCustomPropertyTypeLayout(
        TEXT("Margin")
    );

    PropertyEditorModule.NotifyCustomizationModuleChanged();
}

IMPLEMENT_MODULE(FNineSliceEditor, NineSliceEditor)