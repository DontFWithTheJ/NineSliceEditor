#include "Details/MarginCustomization.h"

#include "UI/SNineSliceEditor.h"

#include "Delegates/Delegate.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Editor.h"
#include "Fonts/SlateFontInfo.h"
#include "Framework/Application/SlateApplication.h"
#include "IDetailChildrenBuilder.h"
#include "InputCoreTypes.h"
#include "Layout/Margin.h"
#include "Math/NumericLimits.h"
#include "Misc/Attribute.h"
#include "PropertyHandle.h"
#include "ScopedTransaction.h"
#include "Styling/SlateBrush.h"
#include "UObject/UnrealType.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Input/NumericTypeInterface.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/SBoxPanel.h"

TSharedRef<IPropertyTypeCustomization> FMarginCustomization::MakeInstance()
{
    return MakeShareable(
        new FMarginCustomization()
    );
}

FMarginCustomization::FMarginCustomization()
{
}

void FMarginCustomization::CustomizeHeader(
    TSharedRef<IPropertyHandle> InStructPropertyHandle,
    FDetailWidgetRow& HeaderRow,
    IPropertyTypeCustomizationUtils& StructCustomizationUtils
)
{
    StructPropertyHandle =
        InStructPropertyHandle;

    const FString& UVSpaceString =
        StructPropertyHandle->GetProperty()->GetMetaData(
            TEXT("UVSpace")
        );

    bIsMarginUsingUVSpace =
        UVSpaceString.Len() > 0 &&
        UVSpaceString == TEXT("true");

    NumericInterface =
        MakeShareable(
            new TDefaultNumericTypeInterface<float>
        );

    uint32 NumChildren = 0;

    StructPropertyHandle->GetNumChildren(
        NumChildren
    );

    ChildPropertyHandles.Reset();

    for (
        uint32 ChildIndex = 0;
        ChildIndex < NumChildren;
        ++ChildIndex
    )
    {
        TSharedPtr<IPropertyHandle> ChildHandle =
            StructPropertyHandle->GetChildHandle(
                ChildIndex
            );

        if (ChildHandle.IsValid())
        {
            ChildPropertyHandles.Add(
                ChildHandle.ToSharedRef()
            );
        }
    }

    HeaderRow.IsEnabled(
        TAttribute<bool>(
            InStructPropertyHandle,
            &IPropertyHandle::IsEditable
        )
    );

    TSharedPtr<SHorizontalBox> HorizontalBox;

    HeaderRow
        .NameContent()
        [
            StructPropertyHandle->CreatePropertyNameWidget()
        ]
        .ValueContent()
        .MinDesiredWidth(
            IsSlateBrushMargin()
                ? 340.0f
                : 250.0f
        )
        .MaxDesiredWidth(
            IsSlateBrushMargin()
                ? 340.0f
                : 250.0f
        )
        [
            SAssignNew(
                HorizontalBox,
                SHorizontalBox
            )
        ];

    HorizontalBox->AddSlot()
    .FillWidth(1.0f)
    [
        MakePropertyWidget()
    ];

    if (IsSlateBrushMargin())
    {
        HorizontalBox->AddSlot()
        .AutoWidth()
        .Padding(
            6.0f,
            0.0f,
            0.0f,
            0.0f
        )
        .VAlign(VAlign_Center)
        [
            SNew(SButton)
            .Text(
                FText::FromString(
                    TEXT("Edit")
                )
            )
            .OnClicked(
                FOnClicked::CreateSP(
                    this,
                    &FMarginCustomization::OnEditClicked
                )
            )
        ];
    }
}

void FMarginCustomization::CustomizeChildren(
    TSharedRef<IPropertyHandle> InStructPropertyHandle,
    IDetailChildrenBuilder& StructBuilder,
    IPropertyTypeCustomizationUtils& StructCustomizationUtils
)
{
    const uint32 NumChildren =
        ChildPropertyHandles.Num();

    for (
        uint32 ChildIndex = 0;
        ChildIndex < NumChildren;
        ++ChildIndex
    )
    {
        TSharedRef<IPropertyHandle> ChildHandle =
            ChildPropertyHandles[ChildIndex];

        IDetailPropertyRow& PropertyRow =
            StructBuilder.AddProperty(
                ChildHandle
            );

        PropertyRow
            .IsEnabled(
                TAttribute<bool>(
                    ChildHandle,
                    &IPropertyHandle::IsEditable
                )
            )
            .CustomWidget()
            .NameContent()
            [
                ChildHandle->CreatePropertyNameWidget(
                    ChildHandle->GetPropertyDisplayName()
                )
            ]
            .ValueContent()
            [
                MakeChildPropertyWidget(
                    ChildIndex
                )
            ];
    }
}

bool FMarginCustomization::IsSlateBrushMargin() const
{
    if (!StructPropertyHandle.IsValid())
    {
        return false;
    }

    TSharedPtr<IPropertyHandle> ParentHandle =
        StructPropertyHandle->GetParentHandle();

    if (!ParentHandle.IsValid())
    {
        return false;
    }

    FStructProperty* ParentStructProperty =
        CastField<FStructProperty>(
            ParentHandle->GetProperty()
        );

    if (!ParentStructProperty)
    {
        return false;
    }

    return ParentStructProperty->Struct ==
        FSlateBrush::StaticStruct();
}

TSharedRef<SEditableTextBox>
FMarginCustomization::MakePropertyWidget()
{
    return
        SAssignNew(
            MarginEditableTextBox,
            SEditableTextBox
        )
        .Text(
            this,
            &FMarginCustomization::GetMarginText
        )
        .ToolTipText(
            FText::FromString(
                TEXT("Margin values")
            )
        )
        .OnTextCommitted(
            this,
            &FMarginCustomization::OnMarginTextCommitted
        )
        .Font(
            IDetailLayoutBuilder::GetDetailFont()
        )
        .SelectAllTextWhenFocused(true)
        .SelectAllTextOnCommit(true)
        .ClearKeyboardFocusOnCommit(false);
}

TSharedRef<SWidget>
FMarginCustomization::MakeChildPropertyWidget(
    int32 PropertyIndex
) const
{
    return
        SNew(SNumericEntryBox<float>)
        .Value(
            this,
            &FMarginCustomization::OnGetValue,
            PropertyIndex
        )
        .Font(
            IDetailLayoutBuilder::GetDetailFont()
        )
        .UndeterminedString(
            FText::FromString(
                TEXT("Multiple Values")
            )
        )
        .OnValueCommitted(
            const_cast<FMarginCustomization*>(this),
            &FMarginCustomization::OnValueCommitted,
            PropertyIndex
        )
        .OnValueChanged(
            const_cast<FMarginCustomization*>(this),
            &FMarginCustomization::OnValueChanged,
            PropertyIndex
        )
        .OnBeginSliderMovement(
            const_cast<FMarginCustomization*>(this),
            &FMarginCustomization::OnBeginSliderMovement
        )
        .OnEndSliderMovement(
            const_cast<FMarginCustomization*>(this),
            &FMarginCustomization::OnEndSliderMovement
        )
        .LabelVAlign(VAlign_Center)
        .AllowSpin(
            bIsMarginUsingUVSpace
        )
        .MinValue(
            bIsMarginUsingUVSpace
                ? 0.0f
                : TNumericLimits<float>::Lowest()
        )
        .MaxValue(
            bIsMarginUsingUVSpace
                ? 1.0f
                : TNumericLimits<float>::Max()
        )
        .MinSliderValue(
            bIsMarginUsingUVSpace
                ? 0.0f
                : TNumericLimits<float>::Lowest()
        )
        .MaxSliderValue(
            bIsMarginUsingUVSpace
                ? 1.0f
                : TNumericLimits<float>::Max()
        )
        .TypeInterface(
            NumericInterface
        );
}

FText FMarginCustomization::GetMarginText() const
{
    return FText::FromString(
        GetMarginTextFromProperties()
    );
}

void FMarginCustomization::OnMarginTextCommitted(
    const FText& InText,
    ETextCommit::Type InCommitType
)
{
    FString InString =
        InText.ToString();

    bool bError = false;

    if (InCommitType != ETextCommit::OnCleared)
    {
        TArray<float> PropertyValues;

        while (
            InString.Len() > 0 &&
            !bError
        )
        {
            FString LeftString;

            const bool bSuccess =
                InString.Split(
                    TEXT(","),
                    &LeftString,
                    &InString
                );

            if (
                !InString.IsEmpty() &&
                (
                    (bSuccess && !LeftString.IsEmpty()) ||
                    !bSuccess
                )
            )
            {
                if (!bSuccess)
                {
                    LeftString =
                        InString;

                    InString.Empty();
                }

                LeftString.TrimStartAndEndInline();

                float Value = 0.0f;

                TOptional<float> NumericValue =
                    NumericInterface->FromString(
                        LeftString,
                        Value
                    );

                if (NumericValue.IsSet())
                {
                    Value =
                        NumericValue.GetValue();

                    PropertyValues.Add(
                        bIsMarginUsingUVSpace
                            ? FMath::Clamp(
                                Value,
                                0.0f,
                                1.0f
                            )
                            : FMath::Max(
                                Value,
                                0.0f
                            )
                    );
                }
                else
                {
                    bError = true;
                }
            }
            else
            {
                bError = true;
            }
        }

        if (!bError)
        {
            FMargin NewMargin;

            if (PropertyValues.Num() == 1)
            {
                NewMargin =
                    FMargin(
                        PropertyValues[0]
                    );
            }
            else if (PropertyValues.Num() == 2)
            {
                NewMargin =
                    FMargin(
                        PropertyValues[0],
                        PropertyValues[1]
                    );
            }
            else if (PropertyValues.Num() == 4)
            {
                NewMargin.Left =
                    PropertyValues[0];

                NewMargin.Top =
                    PropertyValues[1];

                NewMargin.Right =
                    PropertyValues[2];

                NewMargin.Bottom =
                    PropertyValues[3];
            }
            else
            {
                bError = true;
            }

            if (!bError)
            {
                if (bIsMarginUsingUVSpace)
                {
                    if (
                        NewMargin.Left +
                        NewMargin.Right >
                        1.0f
                    )
                    {
                        NewMargin.Left =
                            1.0f -
                            NewMargin.Right;
                    }

                    if (
                        NewMargin.Top +
                        NewMargin.Bottom >
                        1.0f
                    )
                    {
                        NewMargin.Top =
                            1.0f -
                            NewMargin.Bottom;
                    }
                }

                FScopedTransaction Transaction(
                    FText::Format(
                        FText::FromString(
                            TEXT("Edit {0}")
                        ),
                        StructPropertyHandle->GetPropertyDisplayName()
                    )
                );

                if (ChildPropertyHandles.Num() >= 4)
                {
                    ChildPropertyHandles[0]->SetValue(
                        NewMargin.Left
                    );

                    ChildPropertyHandles[1]->SetValue(
                        NewMargin.Top
                    );

                    ChildPropertyHandles[2]->SetValue(
                        NewMargin.Right
                    );

                    ChildPropertyHandles[3]->SetValue(
                        NewMargin.Bottom
                    );
                }

                if (MarginEditableTextBox.IsValid())
                {
                    MarginEditableTextBox->SetError(
                        FString()
                    );
                }
            }
        }

        if (
            bError &&
            MarginEditableTextBox.IsValid()
        )
        {
            MarginEditableTextBox->SetError(
                FText::FromString(
                    TEXT(
                        "Valid Margin formats are:\n"
                        "Uniform Margin; eg. 0.5\n"
                        "Horizontal / Vertical Margins; eg. 2, 3\n"
                        "Left / Top / Right / Bottom Margins; eg. 0.2, 1, 1.5, 3"
                    )
                )
            );
        }
    }
}

FString FMarginCustomization::GetMarginTextFromProperties() const
{
    FString MarginText;

    float PropertyValues[4];

    bool bMultipleValues =
        false;

    for (
        int32 PropertyIndex = 0;
        PropertyIndex < 4 &&
        !bMultipleValues;
        ++PropertyIndex
    )
    {
        if (
            ChildPropertyHandles.Num() <=
            PropertyIndex
        )
        {
            return FString();
        }

        const FPropertyAccess::Result Result =
            ChildPropertyHandles[PropertyIndex]->GetValue(
                PropertyValues[PropertyIndex]
            );

        if (
            Result ==
            FPropertyAccess::MultipleValues
        )
        {
            bMultipleValues = true;
        }
    }

    if (bMultipleValues)
    {
        MarginText =
            FText::FromString(
                TEXT("Multiple Values")
            ).ToString();
    }
    else
    {
        if (
            PropertyValues[0] ==
            PropertyValues[1] &&
            PropertyValues[1] ==
            PropertyValues[2] &&
            PropertyValues[2] ==
            PropertyValues[3]
        )
        {
            MarginText =
                FString::SanitizeFloat(
                    PropertyValues[0]
                );
        }
        else if (
            PropertyValues[0] ==
            PropertyValues[2] &&
            PropertyValues[1] ==
            PropertyValues[3]
        )
        {
            MarginText =
                FString::SanitizeFloat(
                    PropertyValues[0]
                ) +
                FString(TEXT(", ")) +
                FString::SanitizeFloat(
                    PropertyValues[1]
                );
        }
        else
        {
            MarginText =
                FString::SanitizeFloat(
                    PropertyValues[0]
                ) +
                FString(TEXT(", ")) +
                FString::SanitizeFloat(
                    PropertyValues[1]
                ) +
                FString(TEXT(", ")) +
                FString::SanitizeFloat(
                    PropertyValues[2]
                ) +
                FString(TEXT(", ")) +
                FString::SanitizeFloat(
                    PropertyValues[3]
                );
        }
    }

    return MarginText;
}

TOptional<float> FMarginCustomization::OnGetValue(
    int32 PropertyIndex
) const
{
    if (
        !ChildPropertyHandles.IsValidIndex(
            PropertyIndex
        )
    )
    {
        return TOptional<float>();
    }

    float FloatValue = 0.0f;

    if (
        ChildPropertyHandles[PropertyIndex]->GetValue(
            FloatValue
        ) ==
        FPropertyAccess::Success
    )
    {
        return TOptional<float>(
            FloatValue
        );
    }

    return TOptional<float>();
}

void FMarginCustomization::OnBeginSliderMovement()
{
    bIsUsingSlider = true;

    if (GEditor)
    {
        GEditor->BeginTransaction(
            FText::Format(
                FText::FromString(
                    TEXT("Edit {0}")
                ),
                StructPropertyHandle->GetPropertyDisplayName()
            )
        );
    }
}

void FMarginCustomization::OnEndSliderMovement(
    float NewValue
)
{
    bIsUsingSlider = false;

    if (GEditor)
    {
        GEditor->EndTransaction();
    }
}

void FMarginCustomization::OnValueCommitted(
    float NewValue,
    ETextCommit::Type CommitType,
    int32 PropertyIndex
)
{
    if (
        ChildPropertyHandles.IsValidIndex(
            PropertyIndex
        )
    )
    {
        ChildPropertyHandles[PropertyIndex]->SetValue(
            NewValue
        );
    }
}

void FMarginCustomization::OnValueChanged(
    float NewValue,
    int32 PropertyIndex
)
{
    if (
        bIsUsingSlider &&
        ChildPropertyHandles.IsValidIndex(
            PropertyIndex
        )
    )
    {
        const EPropertyValueSetFlags::Type Flags =
            EPropertyValueSetFlags::InteractiveChange;

        ChildPropertyHandles[PropertyIndex]->SetValue(
            NewValue,
            Flags
        );
    }
}

FReply FMarginCustomization::OnEditClicked()
{
    if (!StructPropertyHandle.IsValid())
    {
        return FReply::Handled();
    }

    TSharedPtr<IPropertyHandle> BrushHandle =
        StructPropertyHandle->GetParentHandle();

    if (!BrushHandle.IsValid())
    {
        return FReply::Handled();
    }

    TArray<const void*> RawData;

    BrushHandle->AccessRawData(
        RawData
    );

    if (
        RawData.Num() == 0 ||
        RawData[0] == nullptr
    )
    {
        return FReply::Handled();
    }

    const FSlateBrush* Brush =
        static_cast<const FSlateBrush*>(
            RawData[0]
        );

    if (!Brush)
    {
        return FReply::Handled();
    }

    TSharedPtr<IPropertyHandle> LeftHandle =
        StructPropertyHandle->GetChildHandle(
            TEXT("Left")
        );

    TSharedPtr<IPropertyHandle> TopHandle =
        StructPropertyHandle->GetChildHandle(
            TEXT("Top")
        );

    TSharedPtr<IPropertyHandle> RightHandle =
        StructPropertyHandle->GetChildHandle(
            TEXT("Right")
        );

    TSharedPtr<IPropertyHandle> BottomHandle =
        StructPropertyHandle->GetChildHandle(
            TEXT("Bottom")
        );

    if (
        !LeftHandle.IsValid() ||
        !TopHandle.IsValid() ||
        !RightHandle.IsValid() ||
        !BottomHandle.IsValid()
    )
    {
        return FReply::Handled();
    }

    TSharedPtr<SWindow> ParentWindow =
        FSlateApplication::Get()
        .GetActiveTopLevelRegularWindow();

    SNineSliceEditor::OpenEditorWindow(
        *Brush,
        LeftHandle,
        TopHandle,
        RightHandle,
        BottomHandle,
        ParentWindow
    );

    return FReply::Handled();
}