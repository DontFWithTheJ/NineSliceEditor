#include "UI/SNineSliceEditor.h"

#include "Brushes/SlateRoundedBoxBrush.h"
#include "Engine/Texture2D.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"
#include "Misc/ConfigCacheIni.h"
#include "PropertyHandle.h"
#include "Rendering/DrawElements.h"
#include "ScopedTransaction.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "UI/SNineSlicePreview.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SWindow.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
    TWeakPtr<SWindow> ActiveNineSliceEditorWindow;

    const TCHAR* NineSliceColorConfigSection =
        TEXT("NineSliceEditor");

    const TCHAR* BackgroundColorConfigKey =
        TEXT("BackgroundColorPreset");

    const TCHAR* GuideColorConfigKey =
        TEXT("GuideColorPreset");

    const TCHAR* CanvasColorConfigKey =
        TEXT("CanvasColorPreset");

    const TCHAR* CloseWindowOnApplyConfigKey =
        TEXT("CloseWindowOnApply");

    const TCHAR* PreviewPanelVisibleConfigKey =
        TEXT("PreviewPanelVisible");

    FLinearColor OffsetColor(
        const FLinearColor& InColor,
        const float Delta
    )
    {
        return FLinearColor(
            FMath::Clamp(
                InColor.R + Delta,
                0.0f,
                1.0f
            ),
            FMath::Clamp(
                InColor.G + Delta,
                0.0f,
                1.0f
            ),
            FMath::Clamp(
                InColor.B + Delta,
                0.0f,
                1.0f
            ),
            InColor.A
        );
    }
}

void SNineSliceEditor::OpenEditorWindow(
    const FSlateBrush& InBrush,
    const TSharedPtr<IPropertyHandle>& InLeftHandle,
    const TSharedPtr<IPropertyHandle>& InTopHandle,
    const TSharedPtr<IPropertyHandle>& InRightHandle,
    const TSharedPtr<IPropertyHandle>& InBottomHandle,
    const TSharedPtr<SWindow>& InParentWindow
)
{
    if (
        TSharedPtr<SWindow> ExistingWindow =
            ActiveNineSliceEditorWindow.Pin()
    )
    {
        if (ExistingWindow->IsWindowMinimized())
        {
            ExistingWindow->Restore();
        }

        ExistingWindow->BringToFront(
            true
        );

        return;
    }

    TSharedRef<SNineSliceEditor> Editor =
        SNew(SNineSliceEditor)
        .Brush(
            InBrush
        )
        .LeftHandle(
            InLeftHandle
        )
        .TopHandle(
            InTopHandle
        )
        .RightHandle(
            InRightHandle
        )
        .BottomHandle(
            InBottomHandle
        );

    const FWindowStyle& WindowStyle =
        FAppStyle::Get().GetWidgetStyle<FWindowStyle>(
            TEXT("Window")
        );

    TSharedRef<SWindow> Window =
        SNew(SWindow)
        .Title(
            FText::FromString(
                TEXT("Nine-Slice Editor")
            )
        )
        .ClientSize(
            FVector2D(
                1320.0f,
                800.0f
            )
        )
        .SizingRule(
            ESizingRule::UserSized
        )
        .Style(
            &WindowStyle
        )
        .SupportsMinimize(
            true
        )
        .SupportsMaximize(
            true
        )
        .HasCloseButton(
            true
        )
        .UseOSWindowBorder(
            false
        )
        .CreateTitleBar(
            true
        )
        .FocusWhenFirstShown(
            true
        )
        .SaneWindowPlacement(
            true
        )
        [
            Editor
        ];

    ActiveNineSliceEditorWindow =
        Window;

    Window->SetOnWindowClosed(
        FOnWindowClosed::CreateLambda(
            [](
                const TSharedRef<SWindow>& ClosedWindow
            )
            {
                ActiveNineSliceEditorWindow.Reset();
            }
        )
    );

    if (InParentWindow.IsValid())
    {
        FSlateApplication::Get().AddWindowAsNativeChild(
            Window,
            InParentWindow.ToSharedRef(),
            true
        );
    }
    else
    {
        FSlateApplication::Get().AddWindow(
            Window,
            true
        );
    }
}

void SNineSliceEditor::Construct(
    const FArguments& InArgs
)
{
    Brush =
        InArgs._Brush;

    PreviewPanelBrush =
        MakeUnique<FSlateRoundedBoxBrush>(
            PreviewPanelColor,
            10.0f
        );

    ZoomControlBrush =
        MakeUnique<FSlateRoundedBoxBrush>(
            ZoomControlColor,
            10.0f
        );

    EditorButtonNormalBrush =
        MakeUnique<FSlateRoundedBoxBrush>(
            PreviewCloseButtonColor,
            7.0f
        );

    EditorButtonHoveredBrush =
        MakeUnique<FSlateRoundedBoxBrush>(
            PreviewCloseButtonColor,
            7.0f
        );

    EditorButtonPressedBrush =
        MakeUnique<FSlateRoundedBoxBrush>(
            PreviewCloseButtonColor,
            7.0f
        );

    ColorDropdownItemBrush =
        MakeUnique<FSlateRoundedBoxBrush>(
            DropdownItemColor,
            5.0f
        );

    LeftHandle =
        InArgs._LeftHandle;

    TopHandle =
        InArgs._TopHandle;

    RightHandle =
        InArgs._RightHandle;

    BottomHandle =
        InArgs._BottomHandle;

    ValueUnitOptions.Reset();

    ValueUnitOptions.Add(
        MakeShared<EValueUnit>(
            EValueUnit::Pixels
        )
    );

    ValueUnitOptions.Add(
        MakeShared<EValueUnit>(
            EValueUnit::Normalized
        )
    );

    ValueUnitOptions.Add(
        MakeShared<EValueUnit>(
            EValueUnit::Percentage
        )
    );

    SelectedValueUnit =
        ValueUnitOptions[0];

    ValueUnit =
        EValueUnit::Pixels;

    auto AddColorPreset =
        [](
            TArray<TSharedPtr<FColorPreset>>& Options,
            const TCHAR* Name,
            const FLinearColor& Color,
            const FLinearColor& TextColor
        )
        {
            TSharedPtr<FColorPreset> Preset =
                MakeShared<FColorPreset>();

            Preset->Name =
                FText::FromString(
                    Name
                );

            Preset->Color =
                Color;

            Preset->TextColor =
                TextColor;

            Options.Add(
                Preset
            );

            return Preset;
        };

    BackgroundColorOptions.Reset();
    GuideColorOptions.Reset();
    CanvasColorOptions.Reset();

    AddColorPreset(
        BackgroundColorOptions,
        TEXT("Unreal Dark"),
        FLinearColor(
            0.055f,
            0.060f,
            0.070f,
            1.0f
        ),
        FLinearColor::White
    );

    AddColorPreset(
        BackgroundColorOptions,
        TEXT("Deep Blue"),
        FLinearColor(
            0.035f,
            0.075f,
            0.16f,
            1.0f
        ),
        FLinearColor::White
    );

    AddColorPreset(
        BackgroundColorOptions,
        TEXT("Deep Teal"),
        FLinearColor(
            0.035f,
            0.12f,
            0.11f,
            1.0f
        ),
        FLinearColor::White
    );

    AddColorPreset(
        BackgroundColorOptions,
        TEXT("Deep Purple"),
        FLinearColor(
            0.10f,
            0.045f,
            0.14f,
            1.0f
        ),
        FLinearColor::White
    );

    AddColorPreset(
        BackgroundColorOptions,
        TEXT("Forest"),
        FLinearColor(
            0.035f,
            0.11f,
            0.065f,
            1.0f
        ),
        FLinearColor::White
    );

    AddColorPreset(
        BackgroundColorOptions,
        TEXT("Warm Brown"),
        FLinearColor(
            0.14f,
            0.085f,
            0.045f,
            1.0f
        ),
        FLinearColor::White
    );

    AddColorPreset(
        BackgroundColorOptions,
        TEXT("Cool Light"),
        FLinearColor(
            0.74f,
            0.80f,
            0.88f,
            1.0f
        ),
        FLinearColor::Black
    );

    AddColorPreset(
        BackgroundColorOptions,
        TEXT("Warm Light"),
        FLinearColor(
            0.88f,
            0.84f,
            0.76f,
            1.0f
        ),
        FLinearColor::Black
    );

    AddColorPreset(
        GuideColorOptions,
        TEXT("Red"),
        FLinearColor(
            1.0f,
            0.0f,
            0.0f,
            1.0f
        ),
        FLinearColor::Black
    );

    AddColorPreset(
        GuideColorOptions,
        TEXT("Blue"),
        FLinearColor(
            0.10f,
            0.50f,
            1.0f,
            1.0f
        ),
        FLinearColor::Black
    );

    AddColorPreset(
        GuideColorOptions,
        TEXT("Cyan"),
        FLinearColor(
            0.0f,
            0.88f,
            1.0f,
            1.0f
        ),
        FLinearColor::Black
    );

    AddColorPreset(
        GuideColorOptions,
        TEXT("Green"),
        FLinearColor(
            0.10f,
            1.0f,
            0.25f,
            1.0f
        ),
        FLinearColor::Black
    );

    AddColorPreset(
        GuideColorOptions,
        TEXT("Yellow"),
        FLinearColor(
            1.0f,
            0.85f,
            0.05f,
            1.0f
        ),
        FLinearColor::Black
    );

    AddColorPreset(
        GuideColorOptions,
        TEXT("Orange"),
        FLinearColor(
            1.0f,
            0.42f,
            0.04f,
            1.0f
        ),
        FLinearColor::Black
    );

    AddColorPreset(
        GuideColorOptions,
        TEXT("White"),
        FLinearColor::White,
        FLinearColor::Black
    );

    AddColorPreset(
        GuideColorOptions,
        TEXT("Black"),
        FLinearColor::Black,
        FLinearColor::White
    );

    AddColorPreset(
        CanvasColorOptions,
        TEXT("White"),
        FLinearColor(
            0.94f,
            0.94f,
            0.94f,
            1.0f
        ),
        FLinearColor::Black
    );

    AddColorPreset(
        CanvasColorOptions,
        TEXT("Cool White"),
        FLinearColor(
            0.88f,
            0.91f,
            0.96f,
            1.0f
        ),
        FLinearColor::Black
    );

    AddColorPreset(
        CanvasColorOptions,
        TEXT("Warm White"),
        FLinearColor(
            0.94f,
            0.91f,
            0.85f,
            1.0f
        ),
        FLinearColor::Black
    );

    AddColorPreset(
        CanvasColorOptions,
        TEXT("Light Gray"),
        FLinearColor(
            0.72f,
            0.74f,
            0.78f,
            1.0f
        ),
        FLinearColor::Black
    );

    AddColorPreset(
        CanvasColorOptions,
        TEXT("Soft Blue"),
        FLinearColor(
            0.62f,
            0.76f,
            0.90f,
            1.0f
        ),
        FLinearColor::Black
    );

    AddColorPreset(
        CanvasColorOptions,
        TEXT("Soft Green"),
        FLinearColor(
            0.66f,
            0.82f,
            0.70f,
            1.0f
        ),
        FLinearColor::Black
    );

    AddColorPreset(
        CanvasColorOptions,
        TEXT("Dark Gray"),
        FLinearColor(
            0.12f,
            0.13f,
            0.15f,
            1.0f
        ),
        FLinearColor::White
    );

    AddColorPreset(
        CanvasColorOptions,
        TEXT("Dark Blue"),
        FLinearColor(
            0.07f,
            0.14f,
            0.25f,
            1.0f
        ),
        FLinearColor::White
    );

    SelectedBackgroundColor =
        LoadColorPreset(
            EColorSelector::Background,
            BackgroundColorOptions
        );

    SelectedGuideColor =
        LoadColorPreset(
            EColorSelector::Guide,
            GuideColorOptions
        );

    SelectedCanvasColor =
        LoadColorPreset(
            EColorSelector::Canvas,
            CanvasColorOptions
        );

    LoadCloseWindowOnApplySetting();
    LoadPreviewPanelVisibleSetting();

    PreviewPanelWidth =
        bPreviewPanelVisible
            ? PreviewPanelExpandedWidth
            : 0.0f;

    ApplyColorPreset(
        EColorSelector::Background,
        SelectedBackgroundColor
    );

    ApplyColorPreset(
        EColorSelector::Guide,
        SelectedGuideColor
    );

    ApplyColorPreset(
        EColorSelector::Canvas,
        SelectedCanvasColor
    );

    NumericInterface =
        MakeShared<
            TDefaultNumericTypeInterface<float>
        >();

    NumericInterface->SetMinFractionalDigits(
        TAttribute<TOptional<int32>>(
            TOptional<int32>(
                GetMinFractionalDigits()
            )
        )
    );

    NumericInterface->SetMaxFractionalDigits(
        TAttribute<TOptional<int32>>(
            TOptional<int32>(
                GetMaxFractionalDigits()
            )
        )
    );

    ReadPropertyValues();

    SAssignNew(
        PreviewWidget,
        SNineSlicePreview
    )
    .Brush(
        Brush
    )
    .SourceWidth(
        GetSourceWidth()
    )
    .SourceHeight(
        GetSourceHeight()
    )
    .LeftMargin(
        LeftMargin
    )
    .TopMargin(
        TopMargin
    )
    .RightMargin(
        RightMargin
    )
    .BottomMargin(
        BottomMargin
    )
    .AnimationSpeed(
        PreviewStretchAnimationSpeed
    )
    .BaseMaxSize(
        PreviewBaseMaxSize
    )
    .StretchMaxScale(
        PreviewStretchMaxScale
    )
    .CardGap(
        PreviewCardGap
    )
    .CardPadding(
        PreviewCardPadding
    )
    .LabelHeight(
        PreviewLabelHeight
    )
    .CardColor(
        PreviewCardColor
    )
    .TextColor(
        PreviewTextColor
    );

    const FSlateFontInfo BoldFont =
        FCoreStyle::Get().GetFontStyle(
            TEXT("BoldFont")
        );

    const FLinearColor ApplyTextColor(
        1.0f,
        0.12f,
        0.12f,
        1.0f
    );

    const FLinearColor ApplyButtonColor(
        0.32f,
        0.08f,
        0.08f,
        1.0f
    );

    const FLinearColor ApplyButtonHoveredColor(
        0.46f,
        0.10f,
        0.10f,
        1.0f
    );

    const FLinearColor ApplyButtonPressedColor(
        0.24f,
        0.05f,
        0.05f,
        1.0f
    );

    TSharedRef<SWidget> PreviewPanel =
        SNew(SBorder)
        .BorderImage(
            PreviewPanelBrush.Get()
        )
        .Padding(
            10.0f
        )
        .Clipping(
            EWidgetClipping::ClipToBounds
        )
        [
            SNew(SVerticalBox)

            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SHorizontalBox)

                + SHorizontalBox::Slot()
                .FillWidth(
                    1.0f
                )
                .VAlign(
                    VAlign_Center
                )
                [
                    SNew(STextBlock)
                    .Text(
                        FText::FromString(
                            TEXT("9-Slice Preview")
                        )
                    )
                    .Font(
                        BoldFont
                    )
                    .ColorAndOpacity(
                        this,
                        &SNineSliceEditor::GetPreviewPanelTextColor
                    )
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(
                    VAlign_Center
                )
                [
                    SNew(SButton)
                    .ButtonStyle(
                        &EditorButtonStyle
                    )
                    .ForegroundColor(
                        this,
                        &SNineSliceEditor::GetPreviewCloseButtonTextColor
                    )
                    .ContentPadding(
                        FMargin(
                            7.0f,
                            2.0f
                        )
                    )
                    .IsFocusable(
                        false
                    )
                    [
                        SNew(SBox)
                        .HAlign(
                            HAlign_Center
                        )
                        .VAlign(
                            VAlign_Center
                        )
                        [
                            SNew(STextBlock)
                            .Text(
                                FText::FromString(
                                    TEXT("X")
                                )
                            )
                            .Font(
                                BoldFont
                            )
                            .ColorAndOpacity(
                                this,
                                &SNineSliceEditor::GetPreviewCloseButtonTextColor
                            )
                        ]
                    ]
                    .OnClicked(
                        this,
                        &SNineSliceEditor::OnPreviewToggleClicked
                    )
                ]
            ]

            + SVerticalBox::Slot()
            .FillHeight(
                1.0f
            )
            .HAlign(
                HAlign_Fill
            )
            .VAlign(
                VAlign_Fill
            )
            .Padding(
                0.0f,
                8.0f,
                0.0f,
                0.0f
            )
            [
                PreviewWidget.ToSharedRef()
            ]
        ];

    SAssignNew(
        PreviewPanelContainer,
        SBox
    )
    .WidthOverride(
        PreviewPanelWidth
    )
    .HAlign(
        HAlign_Fill
    )
    .VAlign(
        VAlign_Fill
    )
    .Clipping(
        EWidgetClipping::ClipToBounds
    )
    [
        PreviewPanel
    ];

    auto MakeColorSelector =
        [this, BoldFont](
            const FText& Label,
            TArray<TSharedPtr<FColorPreset>>& Options,
            TSharedPtr<FColorPreset>& Selected,
            EColorSelector Selector
        )
        -> TSharedRef<SWidget>
        {
            return
                SNew(SHorizontalBox)

                + SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(
                    VAlign_Center
                )
                .Padding(
                    0.0f,
                    0.0f,
                    6.0f,
                    0.0f
                )
                [
                    SNew(STextBlock)
                    .Text(
                        Label
                    )
                    .Font(
                        BoldFont
                    )
                    .ColorAndOpacity(
                        this,
                        &SNineSliceEditor::GetEditorTextColor
                    )
                ]

                + SHorizontalBox::Slot()
                .FillWidth(
                    1.0f
                )
                [
                    SNew(SComboBox<TSharedPtr<FColorPreset>>)
                    .ComboBoxStyle(
                        &EditorComboBoxStyle
                    )
                    .ForegroundColor(
                        this,
                        &SNineSliceEditor::GetEditorTextColor
                    )
                    .ContentPadding(
                        FMargin(
                            8.0f,
                            4.0f
                        )
                    )
                    .OptionsSource(
                        &Options
                    )
                    .InitiallySelectedItem(
                        Selected
                    )
                    .OnGenerateWidget_Lambda(
                        [BoldFont](
                            TSharedPtr<FColorPreset> Item
                        ) -> TSharedRef<SWidget>
                        {
                            if (!Item.IsValid())
                            {
                                return SNew(STextBlock);
                            }

                            return
                                SNew(SBox)
                                .HeightOverride(
                                    28.0f
                                )
                                .WidthOverride(
                                    220.0f
                                )
                                [
                                    SNew(SOverlay)

                                    + SOverlay::Slot()
                                    [
                                        SNew(SColorBlock)
                                        .Color(
                                            Item->Color
                                        )
                                    ]

                                    + SOverlay::Slot()
                                    .VAlign(
                                        VAlign_Center
                                    )
                                    .Padding(
                                        10.0f,
                                        0.0f
                                    )
                                    [
                                        SNew(STextBlock)
                                        .Text(
                                            Item->Name
                                        )
                                        .Font(
                                            BoldFont
                                        )
                                        .ColorAndOpacity(
                                            Item->TextColor
                                        )
                                    ]
                                ];
                        }
                    )
                    .OnSelectionChanged_Lambda(
                        [this, Selector](
                            TSharedPtr<FColorPreset> NewPreset,
                            ESelectInfo::Type SelectInfo
                        )
                        {
                            if (!NewPreset.IsValid())
                            {
                                return;
                            }

                            ApplyColorPreset(
                                Selector,
                                NewPreset
                            );

                            SaveColorPreset(
                                Selector,
                                NewPreset
                            );

                            Invalidate(
                                EInvalidateWidgetReason::LayoutAndVolatility
                            );
                        }
                    )
                    [
                        SNew(SHorizontalBox)

                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        .VAlign(
                            VAlign_Center
                        )
                        .Padding(
                            0.0f,
                            0.0f,
                            6.0f,
                            0.0f
                        )
                        [
                            SNew(SColorBlock)
                            .Color_Lambda(
                                [this, Selector]()
                                {
                                    switch (Selector)
                                    {
                                        case EColorSelector::Background:
                                        {
                                            return SelectedBackgroundColor.IsValid()
                                                ? SelectedBackgroundColor->Color
                                                : FLinearColor::White;
                                        }

                                        case EColorSelector::Guide:
                                        {
                                            return SelectedGuideColor.IsValid()
                                                ? SelectedGuideColor->Color
                                                : FLinearColor::White;
                                        }

                                        case EColorSelector::Canvas:
                                        {
                                            return SelectedCanvasColor.IsValid()
                                                ? SelectedCanvasColor->Color
                                                : FLinearColor::White;
                                        }

                                        default:
                                        {
                                            return FLinearColor::White;
                                        }
                                    }
                                }
                            )
                            .Size(
                                FVector2D(
                                    18.0f,
                                    18.0f
                                )
                            )
                        ]

                        + SHorizontalBox::Slot()
                        .FillWidth(
                            1.0f
                        )
                        .VAlign(
                            VAlign_Center
                        )
                        [
                            SNew(STextBlock)
                            .Text_Lambda(
                                [this, Selector]()
                                {
                                    return GetSelectedColorPresetText(
                                        Selector
                                    );
                                }
                            )
                            .Font(
                                BoldFont
                            )
                            .ColorAndOpacity(
                                this,
                                &SNineSliceEditor::GetEditorTextColor
                            )
                        ]
                    ]
                ];
        };

    ChildSlot
    [
        SNew(SBox)
        .Padding(
            12.0f
        )
        .Clipping(
            EWidgetClipping::ClipToBounds
        )
        [
            SNew(SVerticalBox)

            + SVerticalBox::Slot()
            .FillHeight(
                1.0f
            )
            [
                SAssignNew(
                    WorkspaceContainer,
                    SBox
                )
                .Clipping(
                    EWidgetClipping::ClipToBounds
                )
                [
                    SNew(SOverlay)

                    + SOverlay::Slot()
                    [
                        SNew(SBox)
                    ]

                    + SOverlay::Slot()
                    .HAlign(
                        HAlign_Left
                    )
                    .VAlign(
                        VAlign_Bottom
                    )
                    .Padding(
                        0.0f,
                        0.0f,
                        0.0f,
                        8.0f
                    )
                    [
                        SNew(SBox)
                        .WidthOverride(
                            TAttribute<FOptionalSize>::CreateLambda(
                                [this]()
                                {
                                    return FOptionalSize(
                                        FMath::Max(
                                            GetCanvasViewportWidth(),
                                            0.0f
                                        )
                                    );
                                }
                            )
                        )
                        .HAlign(
                            HAlign_Center
                        )
                        [
                            SNew(SHorizontalBox)

                            + SHorizontalBox::Slot()
                            .AutoWidth()
                            [
                                SNew(SBox)
                                .WidthOverride(
                                    460.0f
                                )
                                [
                                    SNew(SBorder)
                                    .BorderImage(
                                        ZoomControlBrush.Get()
                                    )
                                    .Padding(
                                        8.0f,
                                        5.0f
                                    )
                                    [
                                        SNew(SHorizontalBox)

                                        + SHorizontalBox::Slot()
                                        .AutoWidth()
                                        .VAlign(
                                            VAlign_Center
                                        )
                                        [
                                            SNew(SButton)
                                            .ButtonStyle(
                                                &EditorButtonStyle
                                            )
                                            .ForegroundColor(
                                                this,
                                                &SNineSliceEditor::GetZoomControlButtonTextColor
                                            )
                                            .ContentPadding(
                                                FMargin(
                                                    7.0f,
                                                    1.0f
                                                )
                                            )
                                            .IsFocusable(
                                                false
                                            )
                                            .OnClicked(
                                                this,
                                                &SNineSliceEditor::OnZoomMinusClicked
                                            )
                                            [
                                                SNew(STextBlock)
                                                .Text(
                                                    FText::FromString(
                                                        TEXT("-")
                                                    )
                                                )
                                                .Font(
                                                    BoldFont
                                                )
                                                .ColorAndOpacity(
                                                    this,
                                                    &SNineSliceEditor::GetZoomControlButtonTextColor
                                                )
                                            ]
                                        ]

                                        + SHorizontalBox::Slot()
                                        .FillWidth(
                                            1.0f
                                        )
                                        .VAlign(
                                            VAlign_Center
                                        )
                                        .Padding(
                                            8.0f,
                                            0.0f
                                        )
                                        [
                                            SNew(SSlider)
                                            .Value(
                                                this,
                                                &SNineSliceEditor::GetZoomSliderValue
                                            )
                                            .MinValue(
                                                0.0f
                                            )
                                            .MaxValue(
                                                1.0f
                                            )
                                            .StepSize(
                                                0.01f
                                            )
                                            .SliderBarColor(
                                                this,
                                                &SNineSliceEditor::GetZoomSliderBarColor
                                            )
                                            .SliderHandleColor(
                                                this,
                                                &SNineSliceEditor::GetZoomSliderHandleColor
                                            )
                                            .OnValueChanged(
                                                this,
                                                &SNineSliceEditor::OnZoomSliderChanged
                                            )
                                        ]

                                        + SHorizontalBox::Slot()
                                        .AutoWidth()
                                        .VAlign(
                                            VAlign_Center
                                        )
                                        [
                                            SNew(SButton)
                                            .ButtonStyle(
                                                &EditorButtonStyle
                                            )
                                            .ForegroundColor(
                                                this,
                                                &SNineSliceEditor::GetZoomControlButtonTextColor
                                            )
                                            .ContentPadding(
                                                FMargin(
                                                    7.0f,
                                                    1.0f
                                                )
                                            )
                                            .IsFocusable(
                                                false
                                            )
                                            .OnClicked(
                                                this,
                                                &SNineSliceEditor::OnZoomPlusClicked
                                            )
                                            [
                                                SNew(STextBlock)
                                                .Text(
                                                    FText::FromString(
                                                        TEXT("+")
                                                    )
                                                )
                                                .Font(
                                                    BoldFont
                                                )
                                                .ColorAndOpacity(
                                                    this,
                                                    &SNineSliceEditor::GetZoomControlButtonTextColor
                                                )
                                            ]
                                        ]

                                        + SHorizontalBox::Slot()
                                        .AutoWidth()
                                        .VAlign(
                                            VAlign_Center
                                        )
                                        .Padding(
                                            8.0f,
                                            0.0f
                                        )
                                        [
                                            SNew(STextBlock)
                                            .Text(
                                                this,
                                                &SNineSliceEditor::GetZoomText
                                            )
                                            .Font(
                                                BoldFont
                                            )
                                            .MinDesiredWidth(
                                                48.0f
                                            )
                                            .Justification(
                                                ETextJustify::Center
                                            )
                                            .ColorAndOpacity(
                                                this,
                                                &SNineSliceEditor::GetZoomControlTextColor
                                            )
                                        ]

                                        + SHorizontalBox::Slot()
                                        .AutoWidth()
                                        .VAlign(
                                            VAlign_Center
                                        )
                                        [
                                            SNew(SButton)
                                            .ButtonStyle(
                                                &EditorButtonStyle
                                            )
                                            .ForegroundColor(
                                                this,
                                                &SNineSliceEditor::GetZoomControlButtonTextColor
                                            )
                                            .ContentPadding(
                                                FMargin(
                                                    8.0f,
                                                    1.0f
                                                )
                                            )
                                            .IsFocusable(
                                                false
                                            )
                                            .OnClicked(
                                                this,
                                                &SNineSliceEditor::OnZoomFitClicked
                                            )
                                            [
                                                SNew(STextBlock)
                                                .Text(
                                                    FText::FromString(
                                                        TEXT("Fit")
                                                    )
                                                )
                                                .Font(
                                                    BoldFont
                                                )
                                                .ColorAndOpacity(
                                                    this,
                                                    &SNineSliceEditor::GetZoomControlButtonTextColor
                                                )
                                            ]
                                        ]
                                    ]
                                ]
                            ]

                            + SHorizontalBox::Slot()
                            .AutoWidth()
                            .VAlign(
                                VAlign_Center
                            )
                            .Padding(
                                12.0f,
                                0.0f,
                                0.0f,
                                0.0f
                            )
                            [
                                SNew(SBox)
                                .WidthOverride(
                                    170.0f
                                )
                                [
                                    SNew(STextBlock)
                                    .Text(
                                        FText::FromString(
                                            TEXT("Mouse Wheel - Zoom\nRMB - Move image")
                                        )
                                    )
                                    .Font(
                                        BoldFont
                                    )
                                    .AutoWrapText(
                                        true
                                    )
                                    .WrapTextAt(
                                        170.0f
                                    )
                                    .ColorAndOpacity(
                                        this,
                                        &SNineSliceEditor::GetEditorTextColor
                                    )
                                ]
                            ]
                        ]
                    ]

                    + SOverlay::Slot()
                    .HAlign(
                        HAlign_Right
                    )
                    .VAlign(
                        VAlign_Fill
                    )
                    [
                        PreviewPanelContainer.ToSharedRef()
                    ]
                ]
            ]

            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SHorizontalBox)

                + SHorizontalBox::Slot()
                .FillWidth(
                    1.0f
                )
                .Padding(
                    0.0f,
                    8.0f,
                    6.0f,
                    8.0f
                )
                [
                    SNew(SHorizontalBox)

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .VAlign(
                        VAlign_Center
                    )
                    .Padding(
                        0.0f,
                        0.0f,
                        6.0f,
                        0.0f
                    )
                    [
                        SNew(STextBlock)
                        .Text(
                            FText::FromString(
                                TEXT("Left")
                            )
                        )
                        .Font(
                            BoldFont
                        )
                        .ColorAndOpacity(
                            this,
                            &SNineSliceEditor::GetEditorTextColor
                        )
                    ]

                    + SHorizontalBox::Slot()
                    .FillWidth(
                        1.0f
                    )
                    [
                        SNew(SNumericEntryBox<float>)
                        .Value(
                            this,
                            &SNineSliceEditor::GetLeftValue
                        )
                        .MinValue(
                            0.0f
                        )
                        .MaxValue_Lambda(
                            [this]()
                            {
                                return TOptional<float>(
                                    GetDisplayValueMax(
                                        FMath::Max(
                                            GetSourceWidth() -
                                            RightMargin -
                                            1.0f,
                                            0.0f
                                        ),
                                        GetSourceWidth()
                                    )
                                );
                            }
                        )
                        .MinSliderValue(
                            0.0f
                        )
                        .MaxSliderValue_Lambda(
                            [this]()
                            {
                                return TOptional<float>(
                                    GetDisplayValueMax(
                                        FMath::Max(
                                            GetSourceWidth() -
                                            RightMargin -
                                            1.0f,
                                            0.0f
                                        ),
                                        GetSourceWidth()
                                    )
                                );
                            }
                        )
                        .Delta_Lambda(
                            [this]()
                            {
                                return GetDisplayDelta();
                            }
                        )
                        .MinFractionalDigits_Lambda(
                            [this]()
                            {
                                return TOptional<int32>(
                                    GetMinFractionalDigits()
                                );
                            }
                        )
                        .MaxFractionalDigits_Lambda(
                            [this]()
                            {
                                return TOptional<int32>(
                                    GetMaxFractionalDigits()
                                );
                            }
                        )
                        .AllowSpin(
                            true
                        )
                        .AllowWheel(
                            false
                        )
                        .TypeInterface(
                            NumericInterface
                        )
                        .OnValueChanged(
                            this,
                            &SNineSliceEditor::OnLeftValueChanged
                        )
                        .OnValueCommitted(
                            this,
                            &SNineSliceEditor::OnLeftValueCommitted
                        )
                    ]
                ]

                + SHorizontalBox::Slot()
                .FillWidth(
                    1.0f
                )
                .Padding(
                    6.0f,
                    8.0f
                )
                [
                    SNew(SHorizontalBox)

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .VAlign(
                        VAlign_Center
                    )
                    .Padding(
                        0.0f,
                        0.0f,
                        6.0f,
                        0.0f
                    )
                    [
                        SNew(STextBlock)
                        .Text(
                            FText::FromString(
                                TEXT("Top")
                            )
                        )
                        .Font(
                            BoldFont
                        )
                        .ColorAndOpacity(
                            this,
                            &SNineSliceEditor::GetEditorTextColor
                        )
                    ]

                    + SHorizontalBox::Slot()
                    .FillWidth(
                        1.0f
                    )
                    [
                        SNew(SNumericEntryBox<float>)
                        .Value(
                            this,
                            &SNineSliceEditor::GetTopValue
                        )
                        .MinValue(
                            0.0f
                        )
                        .MaxValue_Lambda(
                            [this]()
                            {
                                return TOptional<float>(
                                    GetDisplayValueMax(
                                        FMath::Max(
                                            GetSourceHeight() -
                                            BottomMargin -
                                            1.0f,
                                            0.0f
                                        ),
                                        GetSourceHeight()
                                    )
                                );
                            }
                        )
                        .MinSliderValue(
                            0.0f
                        )
                        .MaxSliderValue_Lambda(
                            [this]()
                            {
                                return TOptional<float>(
                                    GetDisplayValueMax(
                                        FMath::Max(
                                            GetSourceHeight() -
                                            BottomMargin -
                                            1.0f,
                                            0.0f
                                        ),
                                        GetSourceHeight()
                                    )
                                );
                            }
                        )
                        .Delta_Lambda(
                            [this]()
                            {
                                return GetDisplayDelta();
                            }
                        )
                        .MinFractionalDigits_Lambda(
                            [this]()
                            {
                                return TOptional<int32>(
                                    GetMinFractionalDigits()
                                );
                            }
                        )
                        .MaxFractionalDigits_Lambda(
                            [this]()
                            {
                                return TOptional<int32>(
                                    GetMaxFractionalDigits()
                                );
                            }
                        )
                        .AllowSpin(
                            true
                        )
                        .AllowWheel(
                            false
                        )
                        .TypeInterface(
                            NumericInterface
                        )
                        .OnValueChanged(
                            this,
                            &SNineSliceEditor::OnTopValueChanged
                        )
                        .OnValueCommitted(
                            this,
                            &SNineSliceEditor::OnTopValueCommitted
                        )
                    ]
                ]

                + SHorizontalBox::Slot()
                .FillWidth(
                    1.0f
                )
                .Padding(
                    6.0f,
                    8.0f
                )
                [
                    SNew(SHorizontalBox)

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .VAlign(
                        VAlign_Center
                    )
                    .Padding(
                        0.0f,
                        0.0f,
                        6.0f,
                        0.0f
                    )
                    [
                        SNew(STextBlock)
                        .Text(
                            FText::FromString(
                                TEXT("Right")
                            )
                        )
                        .Font(
                            BoldFont
                        )
                        .ColorAndOpacity(
                            this,
                            &SNineSliceEditor::GetEditorTextColor
                        )
                    ]

                    + SHorizontalBox::Slot()
                    .FillWidth(
                        1.0f
                    )
                    [
                        SNew(SNumericEntryBox<float>)
                        .Value(
                            this,
                            &SNineSliceEditor::GetRightValue
                        )
                        .MinValue(
                            0.0f
                        )
                        .MaxValue_Lambda(
                            [this]()
                            {
                                return TOptional<float>(
                                    GetDisplayValueMax(
                                        FMath::Max(
                                            GetSourceWidth() -
                                            LeftMargin -
                                            1.0f,
                                            0.0f
                                        ),
                                        GetSourceWidth()
                                    )
                                );
                            }
                        )
                        .MinSliderValue(
                            0.0f
                        )
                        .MaxSliderValue_Lambda(
                            [this]()
                            {
                                return TOptional<float>(
                                    GetDisplayValueMax(
                                        FMath::Max(
                                            GetSourceWidth() -
                                            LeftMargin -
                                            1.0f,
                                            0.0f
                                        ),
                                        GetSourceWidth()
                                    )
                                );
                            }
                        )
                        .Delta_Lambda(
                            [this]()
                            {
                                return GetDisplayDelta();
                            }
                        )
                        .MinFractionalDigits_Lambda(
                            [this]()
                            {
                                return TOptional<int32>(
                                    GetMinFractionalDigits()
                                );
                            }
                        )
                        .MaxFractionalDigits_Lambda(
                            [this]()
                            {
                                return TOptional<int32>(
                                    GetMaxFractionalDigits()
                                );
                            }
                        )
                        .AllowSpin(
                            true
                        )
                        .AllowWheel(
                            false
                        )
                        .TypeInterface(
                            NumericInterface
                        )
                        .OnValueChanged(
                            this,
                            &SNineSliceEditor::OnRightValueChanged
                        )
                        .OnValueCommitted(
                            this,
                            &SNineSliceEditor::OnRightValueCommitted
                        )
                    ]
                ]

                + SHorizontalBox::Slot()
                .FillWidth(
                    1.0f
                )
                .Padding(
                    6.0f,
                    8.0f,
                    0.0f,
                    8.0f
                )
                [
                    SNew(SHorizontalBox)

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .VAlign(
                        VAlign_Center
                    )
                    .Padding(
                        0.0f,
                        0.0f,
                        6.0f,
                        0.0f
                    )
                    [
                        SNew(STextBlock)
                        .Text(
                            FText::FromString(
                                TEXT("Bottom")
                            )
                        )
                        .Font(
                            BoldFont
                        )
                        .ColorAndOpacity(
                            this,
                            &SNineSliceEditor::GetEditorTextColor
                        )
                    ]

                    + SHorizontalBox::Slot()
                    .FillWidth(
                        1.0f
                    )
                    [
                        SNew(SNumericEntryBox<float>)
                        .Value(
                            this,
                            &SNineSliceEditor::GetBottomValue
                        )
                        .MinValue(
                            0.0f
                        )
                        .MaxValue_Lambda(
                            [this]()
                            {
                                return TOptional<float>(
                                    GetDisplayValueMax(
                                        FMath::Max(
                                            GetSourceHeight() -
                                            TopMargin -
                                            1.0f,
                                            0.0f
                                        ),
                                        GetSourceHeight()
                                    )
                                );
                            }
                        )
                        .MinSliderValue(
                            0.0f
                        )
                        .MaxSliderValue_Lambda(
                            [this]()
                            {
                                return TOptional<float>(
                                    GetDisplayValueMax(
                                        FMath::Max(
                                            GetSourceHeight() -
                                            TopMargin -
                                            1.0f,
                                            0.0f
                                        ),
                                        GetSourceHeight()
                                    )
                                );
                            }
                        )
                        .Delta_Lambda(
                            [this]()
                            {
                                return GetDisplayDelta();
                            }
                        )
                        .MinFractionalDigits_Lambda(
                            [this]()
                            {
                                return TOptional<int32>(
                                    GetMinFractionalDigits()
                                );
                            }
                        )
                        .MaxFractionalDigits_Lambda(
                            [this]()
                            {
                                return TOptional<int32>(
                                    GetMaxFractionalDigits()
                                );
                            }
                        )
                        .AllowSpin(
                            true
                        )
                        .AllowWheel(
                            false
                        )
                        .TypeInterface(
                            NumericInterface
                        )
                        .OnValueChanged(
                            this,
                            &SNineSliceEditor::OnBottomValueChanged
                        )
                        .OnValueCommitted(
                            this,
                            &SNineSliceEditor::OnBottomValueCommitted
                        )
                    ]
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(
                    12.0f,
                    8.0f,
                    6.0f,
                    8.0f
                )
                .VAlign(
                    VAlign_Center
                )
                [
                    SNew(STextBlock)
                    .Text(
                        FText::FromString(
                            TEXT("Units")
                        )
                    )
                    .Font(
                        BoldFont
                    )
                    .ColorAndOpacity(
                        this,
                        &SNineSliceEditor::GetEditorTextColor
                    )
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(
                    0.0f,
                    8.0f,
                    6.0f,
                    8.0f
                )
                [
                    SNew(SBox)
                    .WidthOverride(
                        110.0f
                    )
                    [
                        SNew(SComboBox<TSharedPtr<EValueUnit>>)
                        .ComboBoxStyle(
                            &EditorComboBoxStyle
                        )
                        .ForegroundColor(
                            this,
                            &SNineSliceEditor::GetEditorTextColor
                        )
                        .ContentPadding(
                            FMargin(
                                8.0f,
                                4.0f
                            )
                        )
                        .OptionsSource(
                            &ValueUnitOptions
                        )
                        .InitiallySelectedItem(
                            SelectedValueUnit
                        )
                        .OnGenerateWidget_Lambda(
                            [this, BoldFont](
                                TSharedPtr<EValueUnit> Item
                            ) -> TSharedRef<SWidget>
                            {
                                return
                                    SNew(SBorder)
                                    .BorderImage(
                                        ColorDropdownItemBrush.Get()
                                    )
                                    .Padding(
                                        FMargin(
                                            6.0f,
                                            4.0f
                                        )
                                    )
                                    [
                                        SNew(STextBlock)
                                        .Text(
                                            GetValueUnitText(
                                                Item
                                            )
                                        )
                                        .Font(
                                            BoldFont
                                        )
                                        .ColorAndOpacity(
                                            this,
                                            &SNineSliceEditor::GetEditorTextColor
                                        )
                                    ];
                            }
                        )
                        .OnSelectionChanged(
                            this,
                            &SNineSliceEditor::OnValueUnitChanged
                        )
                        [
                            SNew(STextBlock)
                            .Text(
                                this,
                                &SNineSliceEditor::GetSelectedValueUnitText
                            )
                            .Font(
                                BoldFont
                            )
                            .ColorAndOpacity(
                                this,
                                &SNineSliceEditor::GetEditorTextColor
                            )
                        ]
                    ]
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(
                    10.0f,
                    8.0f,
                    0.0f,
                    8.0f
                )
                [
                    SNew(SButton)
                    .ButtonStyle(
                        &EditorButtonStyle
                    )
                    .ForegroundColor(
                        this,
                        &SNineSliceEditor::GetZoomControlButtonTextColor
                    )
                    .ContentPadding(
                        FMargin(
                            10.0f,
                            4.0f
                        )
                    )
                    [
                        SNew(STextBlock)
                        .Text(
                            this,
                            &SNineSliceEditor::GetPreviewToggleText
                        )
                        .Font(
                            BoldFont
                        )
                        .ColorAndOpacity(
                            this,
                            &SNineSliceEditor::GetZoomControlButtonTextColor
                        )
                    ]
                    .OnClicked(
                        this,
                        &SNineSliceEditor::OnPreviewToggleClicked
                    )
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(
                    10.0f,
                    8.0f,
                    0.0f,
                    8.0f
                )
                [
                    SNew(SButton)
                    .ButtonStyle(
                        &EditorButtonStyle
                    )
                    .ForegroundColor(
                        this,
                        &SNineSliceEditor::GetZoomControlButtonTextColor
                    )
                    .ContentPadding(
                        FMargin(
                            10.0f,
                            4.0f
                        )
                    )
                    [
                        SNew(STextBlock)
                        .Text(
                            FText::FromString(
                                TEXT("Cancel")
                            )
                        )
                        .Font(
                            BoldFont
                        )
                        .ColorAndOpacity(
                            this,
                            &SNineSliceEditor::GetZoomControlButtonTextColor
                        )
                    ]
                    .OnClicked(
                        this,
                        &SNineSliceEditor::OnCancelClicked
                    )
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(
                    6.0f,
                    8.0f,
                    0.0f,
                    8.0f
                )
                [
                    SNew(SButton)
                    .ButtonStyle(
                        &EditorButtonStyle
                    )
                    .ButtonColorAndOpacity(
                        ApplyButtonColor
                    )
                    .ForegroundColor(
                        ApplyTextColor
                    )
                    .ContentPadding(
                        FMargin(
                            10.0f,
                            4.0f
                        )
                    )
                    [
                        SNew(STextBlock)
                        .Text(
                            FText::FromString(
                                TEXT("Apply")
                            )
                        )
                        .Font(
                            BoldFont
                        )
                        .ColorAndOpacity(
                            ApplyTextColor
                        )
                    ]
                    .OnClicked(
                        this,
                        &SNineSliceEditor::OnApplyClicked
                    )
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(
                    VAlign_Center
                )
                .Padding(
                    12.0f,
                    8.0f,
                    0.0f,
                    8.0f
                )
                [
                    SNew(SCheckBox)
                    .IsChecked(
                        this,
                        &SNineSliceEditor::GetCloseWindowOnApplyState
                    )
                    .OnCheckStateChanged(
                        this,
                        &SNineSliceEditor::OnCloseWindowOnApplyChanged
                    )
                    [
                        SNew(STextBlock)
                        .Text(
                            FText::FromString(
                                TEXT("Close window on Apply")
                            )
                        )
                        .Font(
                            BoldFont
                        )
                        .ColorAndOpacity(
                            this,
                            &SNineSliceEditor::GetEditorTextColor
                        )
                    ]
                ]
            ]

            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SHorizontalBox)

                + SHorizontalBox::Slot()
                .FillWidth(
                    1.0f
                )
                .Padding(
                    0.0f,
                    0.0f,
                    6.0f,
                    8.0f
                )
                [
                    MakeColorSelector(
                        FText::FromString(
                            TEXT("Background")
                        ),
                        BackgroundColorOptions,
                        SelectedBackgroundColor,
                        EColorSelector::Background
                    )
                ]

                + SHorizontalBox::Slot()
                .FillWidth(
                    1.0f
                )
                .Padding(
                    6.0f,
                    0.0f,
                    6.0f,
                    8.0f
                )
                [
                    MakeColorSelector(
                        FText::FromString(
                            TEXT("Guides")
                        ),
                        GuideColorOptions,
                        SelectedGuideColor,
                        EColorSelector::Guide
                    )
                ]

                + SHorizontalBox::Slot()
                .FillWidth(
                    1.0f
                )
                .Padding(
                    6.0f,
                    0.0f,
                    0.0f,
                    8.0f
                )
                [
                    MakeColorSelector(
                        FText::FromString(
                            TEXT("Canvas")
                        ),
                        CanvasColorOptions,
                        SelectedCanvasColor,
                        EColorSelector::Canvas
                    )
                ]
            ]
        ]
    ];

    UpdateThemeColors();

    RegisterActiveTimer(
        0.0f,
        FWidgetActiveTimerDelegate::CreateSP(
            this,
            &SNineSliceEditor::TickPreviewPanelAnimation
        )
    );
}

FVector2D SNineSliceEditor::ComputeDesiredSize(
    float LayoutScaleMultiplier
) const
{
    return FVector2D(
        1320.0f,
        800.0f
    );
}

bool SNineSliceEditor::HasResource() const
{
    return Brush.GetResourceObject() != nullptr;
}

float SNineSliceEditor::GetSourceWidth() const
{
    if (
        const UTexture2D* Texture =
            Cast<UTexture2D>(
                Brush.GetResourceObject()
            )
    )
    {
        return static_cast<float>(
            Texture->GetSizeX()
        );
    }

    if (Brush.ImageSize.X > 0.0f)
    {
        return Brush.ImageSize.X;
    }

    return 256.0f;
}

float SNineSliceEditor::GetSourceHeight() const
{
    if (
        const UTexture2D* Texture =
            Cast<UTexture2D>(
                Brush.GetResourceObject()
            )
    )
    {
        return static_cast<float>(
            Texture->GetSizeY()
        );
    }

    if (Brush.ImageSize.Y > 0.0f)
    {
        return Brush.ImageSize.Y;
    }

    return 256.0f;
}

SNineSliceEditor::FPreviewRect
SNineSliceEditor::CalculateCanvasViewportRect(
    const FVector2D& AvailableSize
) const
{
    const FVector2D WorkspaceSize =
        WorkspaceContainer.IsValid()
            ? WorkspaceContainer
                ->GetCachedGeometry()
                .GetLocalSize()
            : AvailableSize;

    const float CanvasWidth =
        FMath::Max(
            WorkspaceSize.X -
            PreviewPanelWidth -
            CanvasViewportHorizontalPadding * 2.0f,
            1.0f
        );

    const float CanvasHeight =
        FMath::Max(
            WorkspaceSize.Y -
            CanvasViewportTopPadding -
            CanvasViewportBottomPadding,
            1.0f
        );

    return {
        FVector2D(
            CanvasViewportHorizontalPadding,
            CanvasViewportTopPadding
        ),
        FVector2D(
            CanvasWidth,
            CanvasHeight
        )
    };
}

SNineSliceEditor::FPreviewRect
SNineSliceEditor::CalculatePreviewRect(
    const FVector2D& AvailableSize
) const
{
    const FPreviewRect CanvasRect =
        CalculateCanvasViewportRect(
            AvailableSize
        );

    const float SourceWidth =
        GetSourceWidth();

    const float SourceHeight =
        GetSourceHeight();

    if (
        SourceWidth <= 0.0f ||
        SourceHeight <= 0.0f ||
        CanvasRect.Size.X <= 0.0f ||
        CanvasRect.Size.Y <= 0.0f
    )
    {
        return {};
    }

    const float SafeWidth =
        FMath::Max(
            CanvasRect.Size.X -
            HandleLength,
            1.0f
        );

    const float SafeHeight =
        FMath::Max(
            CanvasRect.Size.Y -
            HandleLength,
            1.0f
        );

    const float Scale =
        FMath::Min(
            FMath::Min(
                MaxPreviewWidth /
                    SourceWidth,
                MaxPreviewHeight /
                    SourceHeight
            ),
            FMath::Min(
                SafeWidth /
                    SourceWidth,
                SafeHeight /
                    SourceHeight
            )
        );

    const FVector2D BaseSize(
        SourceWidth *
            Scale,
        SourceHeight *
            Scale
    );

    const FVector2D ZoomedSize =
        BaseSize *
        CanvasZoom;

    const FVector2D CanvasCenter =
        CanvasRect.Position +
        CanvasRect.Size *
        0.5f;

    float MaxPanX =
        FMath::Max(
            (
                ZoomedSize.X -
                CanvasRect.Size.X
            ) *
            0.5f,
            0.0f
        );

    float MaxPanY =
        FMath::Max(
            (
                ZoomedSize.Y -
                CanvasRect.Size.Y
            ) *
            0.5f,
            0.0f
        );

    if (CanvasZoom > 1.0f)
    {
        MaxPanX +=
            CanvasPanOverscroll;

        MaxPanY +=
            CanvasPanOverscroll;
    }

    const FVector2D EffectivePan(
        FMath::Clamp(
            CanvasPanOffset.X,
            -MaxPanX,
            MaxPanX
        ),
        FMath::Clamp(
            CanvasPanOffset.Y,
            -MaxPanY,
            MaxPanY
        )
    );

    return {
        CanvasCenter +
            EffectivePan -
            ZoomedSize * 0.5f,
        ZoomedSize
    };
}

float SNineSliceEditor::GetCanvasViewportWidth() const
{
    const FVector2D WorkspaceSize =
        WorkspaceContainer.IsValid()
            ? WorkspaceContainer
                ->GetCachedGeometry()
                .GetLocalSize()
            : GetCachedGeometry().GetLocalSize();

    return FMath::Max(
        WorkspaceSize.X -
        PreviewPanelWidth -
        CanvasViewportHorizontalPadding * 2.0f,
        1.0f
    );
}

void SNineSliceEditor::ClampCanvasPan(
    const FVector2D& AvailableSize
)
{
    const FPreviewRect CanvasRect =
        CalculateCanvasViewportRect(
            AvailableSize
        );

    const float SourceWidth =
        GetSourceWidth();

    const float SourceHeight =
        GetSourceHeight();

    if (
        SourceWidth <= 0.0f ||
        SourceHeight <= 0.0f
    )
    {
        CanvasPanOffset =
            FVector2D::ZeroVector;

        return;
    }

    const float SafeWidth =
        FMath::Max(
            CanvasRect.Size.X -
            HandleLength,
            1.0f
        );

    const float SafeHeight =
        FMath::Max(
            CanvasRect.Size.Y -
            HandleLength,
            1.0f
        );

    const float Scale =
        FMath::Min(
            FMath::Min(
                MaxPreviewWidth /
                    SourceWidth,
                MaxPreviewHeight /
                    SourceHeight
            ),
            FMath::Min(
                SafeWidth /
                    SourceWidth,
                SafeHeight /
                    SourceHeight
            )
        );

    const FVector2D ZoomedSize(
        SourceWidth *
            Scale *
            CanvasZoom,
        SourceHeight *
            Scale *
            CanvasZoom
    );

    float MaxPanX =
        FMath::Max(
            (
                ZoomedSize.X -
                CanvasRect.Size.X
            ) *
            0.5f,
            0.0f
        );

    float MaxPanY =
        FMath::Max(
            (
                ZoomedSize.Y -
                CanvasRect.Size.Y
            ) *
            0.5f,
            0.0f
        );

    if (CanvasZoom > 1.0f)
    {
        MaxPanX +=
            CanvasPanOverscroll;

        MaxPanY +=
            CanvasPanOverscroll;
    }

    CanvasPanOffset.X =
        FMath::Clamp(
            CanvasPanOffset.X,
            -MaxPanX,
            MaxPanX
        );

    CanvasPanOffset.Y =
        FMath::Clamp(
            CanvasPanOffset.Y,
            -MaxPanY,
            MaxPanY
        );
}

void SNineSliceEditor::SetCanvasZoom(
    float NewZoom
)
{
    CanvasZoom =
        FMath::Clamp(
            NewZoom,
            CanvasZoomMin,
            CanvasZoomMax
        );

    if (CanvasZoom <= 1.0f)
    {
        CanvasPanOffset =
            FVector2D::ZeroVector;
    }

    ClampCanvasPan(
        GetCachedGeometry().GetLocalSize()
    );

    Invalidate(
        EInvalidateWidgetReason::Paint
    );
}

void SNineSliceEditor::SetCanvasZoomAtPosition(
    float NewZoom,
    const FVector2D& FocusPosition,
    const FVector2D& AvailableSize
)
{
    const FPreviewRect OldPreviewRect =
        CalculatePreviewRect(
            AvailableSize
        );

    const bool bFocusInsideImage =
        FocusPosition.X >=
            OldPreviewRect.Position.X &&
        FocusPosition.X <=
            OldPreviewRect.Position.X +
            OldPreviewRect.Size.X &&
        FocusPosition.Y >=
            OldPreviewRect.Position.Y &&
        FocusPosition.Y <=
            OldPreviewRect.Position.Y +
            OldPreviewRect.Size.Y;

    const float TargetZoom =
        FMath::Clamp(
            NewZoom,
            CanvasZoomMin,
            CanvasZoomMax
        );

    if (
        FMath::IsNearlyEqual(
            CanvasZoom,
            TargetZoom,
            0.0001f
        )
    )
    {
        return;
    }

    if (!bFocusInsideImage)
    {
        SetCanvasZoom(
            TargetZoom
        );

        return;
    }

    const FVector2D LocalImagePosition =
        FocusPosition -
        OldPreviewRect.Position;

    const FVector2D NormalizedPosition(
        OldPreviewRect.Size.X > 0.0f
            ? LocalImagePosition.X /
                OldPreviewRect.Size.X
            : 0.5f,

        OldPreviewRect.Size.Y > 0.0f
            ? LocalImagePosition.Y /
                OldPreviewRect.Size.Y
            : 0.5f
    );

    CanvasZoom =
        TargetZoom;

    const FPreviewRect CanvasRect =
        CalculateCanvasViewportRect(
            AvailableSize
        );

    const float SourceWidth =
        GetSourceWidth();

    const float SourceHeight =
        GetSourceHeight();

    const float SafeWidth =
        FMath::Max(
            CanvasRect.Size.X -
            HandleLength,
            1.0f
        );

    const float SafeHeight =
        FMath::Max(
            CanvasRect.Size.Y -
            HandleLength,
            1.0f
        );

    const float Scale =
        FMath::Min(
            FMath::Min(
                MaxPreviewWidth /
                    SourceWidth,
                MaxPreviewHeight /
                    SourceHeight
            ),
            FMath::Min(
                SafeWidth /
                    SourceWidth,
                SafeHeight /
                    SourceHeight
            )
        );

    const FVector2D NewSize(
        SourceWidth *
            Scale *
            CanvasZoom,

        SourceHeight *
            Scale *
            CanvasZoom
    );

    const FVector2D NewPosition =
        FocusPosition -
        NewSize *
        NormalizedPosition;

    const FVector2D CanvasCenter =
        CanvasRect.Position +
        CanvasRect.Size *
        0.5f;

    CanvasPanOffset =
        NewPosition +
        NewSize *
        0.5f -
        CanvasCenter;

    ClampCanvasPan(
        AvailableSize
    );

    Invalidate(
        EInvalidateWidgetReason::Paint
    );
}

void SNineSliceEditor::ResetCanvasZoom()
{
    CanvasZoom =
        1.0f;

    CanvasPanOffset =
        FVector2D::ZeroVector;

    ClampCanvasPan(
        GetCachedGeometry().GetLocalSize()
    );

    Invalidate(
        EInvalidateWidgetReason::Paint
    );
}

void SNineSliceEditor::ReadPropertyValues()
{
    LeftMargin = 0.0f;
    TopMargin = 0.0f;
    RightMargin = 0.0f;
    BottomMargin = 0.0f;

    if (LeftHandle.IsValid())
    {
        float Value = 0.0f;

        if (
            LeftHandle->GetValue(Value) ==
            FPropertyAccess::Success
        )
        {
            LeftMargin =
                Value *
                GetSourceWidth();
        }
    }

    if (TopHandle.IsValid())
    {
        float Value = 0.0f;

        if (
            TopHandle->GetValue(Value) ==
            FPropertyAccess::Success
        )
        {
            TopMargin =
                Value *
                GetSourceHeight();
        }
    }

    if (RightHandle.IsValid())
    {
        float Value = 0.0f;

        if (
            RightHandle->GetValue(Value) ==
            FPropertyAccess::Success
        )
        {
            RightMargin =
                Value *
                GetSourceWidth();
        }
    }

    if (BottomHandle.IsValid())
    {
        float Value = 0.0f;

        if (
            BottomHandle->GetValue(Value) ==
            FPropertyAccess::Success
        )
        {
            BottomMargin =
                Value *
                GetSourceHeight();
        }
    }

    ClampMargins();
}

void SNineSliceEditor::ApplyMargins()
{
    const float SourceWidth =
        GetSourceWidth();

    const float SourceHeight =
        GetSourceHeight();

    if (
        SourceWidth <= 0.0f ||
        SourceHeight <= 0.0f
    )
    {
        return;
    }

    FScopedTransaction Transaction(
        FText::FromString(
            TEXT("Edit 9-Slice Margins")
        )
    );

    if (LeftHandle.IsValid())
    {
        LeftHandle->SetValue(
            LeftMargin /
            SourceWidth
        );
    }

    if (TopHandle.IsValid())
    {
        TopHandle->SetValue(
            TopMargin /
            SourceHeight
        );
    }

    if (RightHandle.IsValid())
    {
        RightHandle->SetValue(
            RightMargin /
            SourceWidth
        );
    }

    if (BottomHandle.IsValid())
    {
        BottomHandle->SetValue(
            BottomMargin /
            SourceHeight
        );
    }
}

void SNineSliceEditor::ClampMargins()
{
    const float SourceWidth =
        GetSourceWidth();

    const float SourceHeight =
        GetSourceHeight();

    const float MaximumHorizontalMargins =
        FMath::Max(
            SourceWidth - 1.0f,
            0.0f
        );

    const float MaximumVerticalMargins =
        FMath::Max(
            SourceHeight - 1.0f,
            0.0f
        );

    LeftMargin =
        FMath::Clamp(
            LeftMargin,
            0.0f,
            SourceWidth
        );

    RightMargin =
        FMath::Clamp(
            RightMargin,
            0.0f,
            SourceWidth
        );

    TopMargin =
        FMath::Clamp(
            TopMargin,
            0.0f,
            SourceHeight
        );

    BottomMargin =
        FMath::Clamp(
            BottomMargin,
            0.0f,
            SourceHeight
        );

    if (
        LeftMargin +
        RightMargin >
        MaximumHorizontalMargins
    )
    {
        if (ActiveHandle == EHandle::Left)
        {
            LeftMargin =
                FMath::Max(
                    MaximumHorizontalMargins -
                    RightMargin,
                    0.0f
                );
        }
        else
        {
            RightMargin =
                FMath::Max(
                    MaximumHorizontalMargins -
                    LeftMargin,
                    0.0f
                );
        }
    }

    if (
        TopMargin +
        BottomMargin >
        MaximumVerticalMargins
    )
    {
        if (ActiveHandle == EHandle::Top)
        {
            TopMargin =
                FMath::Max(
                    MaximumVerticalMargins -
                    BottomMargin,
                    0.0f
                );
        }
        else
        {
            BottomMargin =
                FMath::Max(
                    MaximumVerticalMargins -
                    TopMargin,
                    0.0f
                );
        }
    }
}

float SNineSliceEditor::PixelToDisplay(
    float PixelValue,
    float SourceSize
) const
{
    if (SourceSize <= 0.0f)
    {
        return 0.0f;
    }

    switch (ValueUnit)
    {
        case EValueUnit::Normalized:
        {
            return PixelValue /
                SourceSize;
        }

        case EValueUnit::Percentage:
        {
            return PixelValue /
                SourceSize *
                100.0f;
        }

        default:
        {
            return PixelValue;
        }
    }
}

float SNineSliceEditor::DisplayToPixel(
    float DisplayValue,
    float SourceSize
) const
{
    switch (ValueUnit)
    {
        case EValueUnit::Normalized:
        {
            return DisplayValue *
                SourceSize;
        }

        case EValueUnit::Percentage:
        {
            return DisplayValue *
                SourceSize /
                100.0f;
        }

        default:
        {
            return DisplayValue;
        }
    }
}

float SNineSliceEditor::GetDisplayDelta() const
{
    switch (ValueUnit)
    {
        case EValueUnit::Normalized:
        {
            return 0.01f;
        }

        case EValueUnit::Percentage:
        {
            return 1.0f;
        }

        default:
        {
            return 1.0f;
        }
    }
}

float SNineSliceEditor::GetDisplayValueMax(
    float PixelMax,
    float SourceSize
) const
{
    if (SourceSize <= 0.0f)
    {
        return 0.0f;
    }

    const float ClampedPixelMax =
        FMath::Max(
            PixelMax,
            0.0f
        );

    switch (ValueUnit)
    {
        case EValueUnit::Normalized:
        {
            return ClampedPixelMax /
                SourceSize;
        }

        case EValueUnit::Percentage:
        {
            return ClampedPixelMax /
                SourceSize *
                100.0f;
        }

        default:
        {
            return ClampedPixelMax;
        }
    }
}

int32 SNineSliceEditor::GetMaxFractionalDigits() const
{
    switch (ValueUnit)
    {
        case EValueUnit::Normalized:
        {
            return 4;
        }

        case EValueUnit::Percentage:
        {
            return 1;
        }

        default:
        {
            return 0;
        }
    }
}

int32 SNineSliceEditor::GetMinFractionalDigits() const
{
    return 0;
}

FText SNineSliceEditor::GetValueUnitText(
    TSharedPtr<EValueUnit> Unit
) const
{
    if (!Unit.IsValid())
    {
        return FText::GetEmpty();
    }

    switch (*Unit)
    {
        case EValueUnit::Pixels:
        {
            return FText::FromString(
                TEXT("Pixels")
            );
        }

        case EValueUnit::Normalized:
        {
            return FText::FromString(
                TEXT("0 - 1")
            );
        }

        case EValueUnit::Percentage:
        {
            return FText::FromString(
                TEXT("0 - 100%")
            );
        }

        default:
        {
            return FText::GetEmpty();
        }
    }
}

FText SNineSliceEditor::GetSelectedValueUnitText() const
{
    switch (ValueUnit)
    {
        case EValueUnit::Pixels:
        {
            return FText::FromString(
                TEXT("Pixels")
            );
        }

        case EValueUnit::Normalized:
        {
            return FText::FromString(
                TEXT("0 - 1")
            );
        }

        case EValueUnit::Percentage:
        {
            return FText::FromString(
                TEXT("0 - 100%")
            );
        }

        default:
        {
            return FText::GetEmpty();
        }
    }
}

void SNineSliceEditor::OnValueUnitChanged(
    TSharedPtr<EValueUnit> NewUnit,
    ESelectInfo::Type SelectInfo
)
{
    if (!NewUnit.IsValid())
    {
        return;
    }

    ValueUnit =
        *NewUnit;

    SelectedValueUnit =
        NewUnit;

    if (NumericInterface.IsValid())
    {
        NumericInterface->SetMinFractionalDigits(
            TAttribute<TOptional<int32>>(
                TOptional<int32>(
                    GetMinFractionalDigits()
                )
            )
        );

        NumericInterface->SetMaxFractionalDigits(
            TAttribute<TOptional<int32>>(
                TOptional<int32>(
                    GetMaxFractionalDigits()
                )
            )
        );
    }

    Invalidate(
        EInvalidateWidgetReason::LayoutAndVolatility
    );
}

FText SNineSliceEditor::GetPreviewToggleText() const
{
    return FText::FromString(
        bPreviewPanelVisible
            ? TEXT("Hide Preview")
            : TEXT("Show Preview")
    );
}

FReply SNineSliceEditor::OnPreviewToggleClicked()
{
    bPreviewPanelVisible =
        !bPreviewPanelVisible;

    SavePreviewPanelVisibleSetting();

    Invalidate(
        EInvalidateWidgetReason::LayoutAndVolatility
    );

    return FReply::Handled();
}

EActiveTimerReturnType
SNineSliceEditor::TickPreviewPanelAnimation(
    double InCurrentTime,
    float InDeltaTime
)
{
    if (!PreviewPanelContainer.IsValid())
    {
        return EActiveTimerReturnType::Continue;
    }

    const float TargetWidth =
        bPreviewPanelVisible
            ? PreviewPanelExpandedWidth
            : 0.0f;

    PreviewPanelWidth =
        FMath::FInterpConstantTo(
            PreviewPanelWidth,
            TargetWidth,
            InDeltaTime,
            PreviewPanelAnimationSpeed
        );

    if (
        FMath::IsNearlyEqual(
            PreviewPanelWidth,
            TargetWidth,
            0.01f
        )
    )
    {
        PreviewPanelWidth =
            TargetWidth;
    }

    PreviewPanelContainer->SetWidthOverride(
        FOptionalSize(
            PreviewPanelWidth
        )
    );

    ClampCanvasPan(
        GetCachedGeometry().GetLocalSize()
    );

    Invalidate(
        EInvalidateWidgetReason::LayoutAndVolatility
    );

    return EActiveTimerReturnType::Continue;
}

void SNineSliceEditor::UpdatePreviewWidget()
{
    if (!PreviewWidget.IsValid())
    {
        return;
    }

    PreviewWidget->SetBrush(
        Brush,
        GetSourceWidth(),
        GetSourceHeight()
    );

    PreviewWidget->SetMargins(
        LeftMargin,
        TopMargin,
        RightMargin,
        BottomMargin
    );
}

void SNineSliceEditor::UpdateThemeColors()
{
    const float Luminance =
        BackgroundColor.R * 0.2126f +
        BackgroundColor.G * 0.7152f +
        BackgroundColor.B * 0.0722f;

    const bool bLightBackground =
        Luminance >= 0.45f;

    EditorTextColor =
        bLightBackground
            ? FLinearColor::Black
            : FLinearColor::White;

    if (bLightBackground)
    {
        PreviewPanelColor =
            OffsetColor(
                BackgroundColor,
                -0.16f
            );

        PreviewCloseButtonColor =
            OffsetColor(
                BackgroundColor,
                -0.28f
            );

        ZoomSliderBarColor =
            OffsetColor(
                BackgroundColor,
                -0.34f
            );

        ZoomSliderHandleColor =
            FLinearColor::Black;

        DropdownItemColor =
            OffsetColor(
                BackgroundColor,
                -0.12f
            );
    }
    else
    {
        PreviewPanelColor =
            OffsetColor(
                BackgroundColor,
                0.07f
            );

        PreviewCloseButtonColor =
            OffsetColor(
                BackgroundColor,
                0.16f
            );

        ZoomSliderBarColor =
            OffsetColor(
                BackgroundColor,
                0.18f
            );

        ZoomSliderHandleColor =
            FLinearColor(
                0.90f,
                0.90f,
                0.92f,
                1.0f
            );

        DropdownItemColor =
            OffsetColor(
                BackgroundColor,
                0.11f
            );
    }

    PreviewPanelTextColor =
        EditorTextColor;

    PreviewCloseButtonTextColor =
        EditorTextColor;

    ZoomControlColor =
        PreviewPanelColor;

    ZoomControlTextColor =
        PreviewPanelTextColor;

    ZoomControlButtonColor =
        PreviewCloseButtonColor;

    ZoomControlButtonTextColor =
        PreviewCloseButtonTextColor;

    DropdownItemTextColor =
        EditorTextColor;

    const FLinearColor ButtonHoverColor =
        bLightBackground
            ? OffsetColor(
                BackgroundColor,
                -0.08f
            )
            : OffsetColor(
                BackgroundColor,
                0.22f
            );

    const FLinearColor ButtonPressedColor =
        bLightBackground
            ? OffsetColor(
                BackgroundColor,
                -0.34f
            )
            : OffsetColor(
                BackgroundColor,
                0.10f
            );

    const FLinearColor PanelOutlineColor =
        bLightBackground
            ? OffsetColor(
                BackgroundColor,
                -0.32f
            )
            : OffsetColor(
                BackgroundColor,
                0.18f
            );

    const FLinearColor ButtonOutlineColor =
        bLightBackground
            ? OffsetColor(
                BackgroundColor,
                -0.40f
            )
            : OffsetColor(
                BackgroundColor,
                0.24f
            );

    if (PreviewPanelBrush.IsValid())
    {
        PreviewPanelBrush->TintColor =
            FSlateColor(
                PreviewPanelColor
            );

        PreviewPanelBrush->OutlineSettings =
            FSlateBrushOutlineSettings(
                10.0f,
                FSlateColor(
                    PanelOutlineColor
                ),
                1.5f
            );
    }

    if (ZoomControlBrush.IsValid())
    {
        ZoomControlBrush->TintColor =
            FSlateColor(
                ZoomControlColor
            );

        ZoomControlBrush->OutlineSettings =
            FSlateBrushOutlineSettings(
                10.0f,
                FSlateColor(
                    PanelOutlineColor
                ),
                1.5f
            );
    }

    if (EditorButtonNormalBrush.IsValid())
    {
        EditorButtonNormalBrush->TintColor =
            FSlateColor(
                PreviewCloseButtonColor
            );

        EditorButtonNormalBrush->OutlineSettings =
            FSlateBrushOutlineSettings(
                7.0f,
                FSlateColor(
                    ButtonOutlineColor
                ),
                1.5f
            );
    }

    if (EditorButtonHoveredBrush.IsValid())
    {
        EditorButtonHoveredBrush->TintColor =
            FSlateColor(
                ButtonHoverColor
            );

        EditorButtonHoveredBrush->OutlineSettings =
            FSlateBrushOutlineSettings(
                7.0f,
                FSlateColor(
                    ButtonOutlineColor
                ),
                1.5f
            );
    }

    if (EditorButtonPressedBrush.IsValid())
    {
        EditorButtonPressedBrush->TintColor =
            FSlateColor(
                ButtonPressedColor
            );

        EditorButtonPressedBrush->OutlineSettings =
            FSlateBrushOutlineSettings(
                7.0f,
                FSlateColor(
                    ButtonOutlineColor
                ),
                1.5f
            );
    }

    if (ColorDropdownItemBrush.IsValid())
    {
        ColorDropdownItemBrush->TintColor =
            FSlateColor(
                DropdownItemColor
            );

        ColorDropdownItemBrush->OutlineSettings =
            FSlateBrushOutlineSettings(
                5.0f,
                FSlateColor(
                    PanelOutlineColor
                ),
                1.0f
            );
    }

    EditorButtonStyle =
        FButtonStyle();

    EditorButtonStyle.Normal =
        *EditorButtonNormalBrush;

    EditorButtonStyle.Hovered =
        *EditorButtonHoveredBrush;

    EditorButtonStyle.Pressed =
        *EditorButtonPressedBrush;

    EditorButtonStyle.Disabled =
        *EditorButtonNormalBrush;

    EditorButtonStyle.NormalForeground =
        FSlateColor(
            ZoomControlButtonTextColor
        );

    EditorButtonStyle.HoveredForeground =
        FSlateColor(
            ZoomControlButtonTextColor
        );

    EditorButtonStyle.PressedForeground =
        FSlateColor(
            ZoomControlButtonTextColor
        );

    EditorButtonStyle.DisabledForeground =
        FSlateColor(
            ZoomControlButtonTextColor
        );

    EditorButtonStyle.NormalPadding =
        FMargin(
            7.0f,
            4.0f
        );

    EditorButtonStyle.PressedPadding =
        FMargin(
            7.0f,
            4.0f
        );

    EditorComboBoxStyle =
        FAppStyle::Get().GetWidgetStyle<FComboBoxStyle>(
            TEXT("ComboBox")
        );

    EditorComboBoxStyle.ComboButtonStyle.ButtonStyle =
        EditorButtonStyle;

    EditorComboBoxStyle.ComboButtonStyle.ButtonStyle.NormalForeground =
        FSlateColor(
            EditorTextColor
        );

    EditorComboBoxStyle.ComboButtonStyle.ButtonStyle.HoveredForeground =
        FSlateColor(
            EditorTextColor
        );

    EditorComboBoxStyle.ComboButtonStyle.ButtonStyle.PressedForeground =
        FSlateColor(
            EditorTextColor
        );

    EditorComboBoxStyle.ComboButtonStyle.ButtonStyle.DisabledForeground =
        FSlateColor(
            EditorTextColor
        );

    EditorComboBoxStyle.ComboButtonStyle.ContentPadding =
        FMargin(
            8.0f,
            4.0f
        );

    EditorComboBoxStyle.ContentPadding =
        FMargin(
            8.0f,
            4.0f
        );

    EditorComboBoxStyle.MenuRowPadding =
        FMargin(
            2.0f,
            2.0f
        );

    if (PreviewWidget.IsValid())
    {
        PreviewWidget->SetColors(
            PreviewCardColor,
            PreviewTextColor
        );
    }

    Invalidate(
        EInvalidateWidgetReason::LayoutAndVolatility
    );
}

FSlateColor SNineSliceEditor::GetEditorTextColor() const
{
    return FSlateColor(
        EditorTextColor
    );
}

FSlateColor SNineSliceEditor::GetPreviewPanelTextColor() const
{
    return FSlateColor(
        PreviewPanelTextColor
    );
}

FSlateColor SNineSliceEditor::GetPreviewCloseButtonColor() const
{
    return FSlateColor(
        PreviewCloseButtonColor
    );
}

FSlateColor SNineSliceEditor::GetPreviewCloseButtonTextColor() const
{
    return FSlateColor(
        PreviewCloseButtonTextColor
    );
}

FSlateColor SNineSliceEditor::GetZoomControlTextColor() const
{
    return FSlateColor(
        ZoomControlTextColor
    );
}

FSlateColor SNineSliceEditor::GetZoomControlButtonColor() const
{
    return FSlateColor(
        ZoomControlButtonColor
    );
}

FSlateColor SNineSliceEditor::GetZoomControlButtonTextColor() const
{
    return FSlateColor(
        ZoomControlButtonTextColor
    );
}

FSlateColor SNineSliceEditor::GetZoomSliderBarColor() const
{
    return FSlateColor(
        ZoomSliderBarColor
    );
}

FSlateColor SNineSliceEditor::GetZoomSliderHandleColor() const
{
    return FSlateColor(
        ZoomSliderHandleColor
    );
}

void SNineSliceEditor::ApplyColorPreset(
    EColorSelector Selector,
    const TSharedPtr<FColorPreset>& Preset
)
{
    if (!Preset.IsValid())
    {
        return;
    }

    switch (Selector)
    {
        case EColorSelector::Background:
        {
            BackgroundColor =
                Preset->Color;

            UpdateThemeColors();

            Invalidate(
                EInvalidateWidgetReason::Paint
            );

            break;
        }

        case EColorSelector::Guide:
        {
            GuideColor =
                Preset->Color;

            HandleColor =
                Preset->Color;

            HandleTextColor =
                Preset->TextColor;

            Invalidate(
                EInvalidateWidgetReason::Paint
            );

            break;
        }

        case EColorSelector::Canvas:
        {
            PreviewCardColor =
                Preset->Color;

            PreviewTextColor =
                Preset->TextColor;

            if (PreviewWidget.IsValid())
            {
                PreviewWidget->SetColors(
                    PreviewCardColor,
                    PreviewTextColor
                );
            }

            Invalidate(
                EInvalidateWidgetReason::Paint
            );

            break;
        }

        default:
        {
            break;
        }
    }
}

void SNineSliceEditor::SaveColorPreset(
    EColorSelector Selector,
    const TSharedPtr<FColorPreset>& Preset
)
{
    if (
        !Preset.IsValid() ||
        !GConfig
    )
    {
        return;
    }

    const TCHAR* Key = nullptr;

    switch (Selector)
    {
        case EColorSelector::Background:
        {
            Key =
                BackgroundColorConfigKey;

            break;
        }

        case EColorSelector::Guide:
        {
            Key =
                GuideColorConfigKey;

            break;
        }

        case EColorSelector::Canvas:
        {
            Key =
                CanvasColorConfigKey;

            break;
        }

        default:
        {
            return;
        }
    }

    GConfig->SetString(
        NineSliceColorConfigSection,
        Key,
        *Preset->Name.ToString(),
        GEditorPerProjectIni
    );

    GConfig->Flush(
        false,
        GEditorPerProjectIni
    );
}

TSharedPtr<SNineSliceEditor::FColorPreset>
SNineSliceEditor::LoadColorPreset(
    EColorSelector Selector,
    const TArray<TSharedPtr<FColorPreset>>& Options
) const
{
    if (Options.Num() == 0)
    {
        return nullptr;
    }

    if (!GConfig)
    {
        return Options[0];
    }

    const TCHAR* Key = nullptr;

    switch (Selector)
    {
        case EColorSelector::Background:
        {
            Key =
                BackgroundColorConfigKey;

            break;
        }

        case EColorSelector::Guide:
        {
            Key =
                GuideColorConfigKey;

            break;
        }

        case EColorSelector::Canvas:
        {
            Key =
                CanvasColorConfigKey;

            break;
        }

        default:
        {
            return Options[0];
        }
    }

    FString SavedPresetName;

    if (
        !GConfig->GetString(
            NineSliceColorConfigSection,
            Key,
            SavedPresetName,
            GEditorPerProjectIni
        )
    )
    {
        return Options[0];
    }

    for (
        const TSharedPtr<FColorPreset>& Preset :
        Options
    )
    {
        if (
            Preset.IsValid() &&
            Preset->Name.ToString() ==
            SavedPresetName
        )
        {
            return Preset;
        }
    }

    return Options[0];
}

void SNineSliceEditor::LoadCloseWindowOnApplySetting()
{
    bCloseWindowOnApply =
        true;

    if (!GConfig)
    {
        return;
    }

    GConfig->GetBool(
        NineSliceColorConfigSection,
        CloseWindowOnApplyConfigKey,
        bCloseWindowOnApply,
        GEditorPerProjectIni
    );
}

void SNineSliceEditor::SaveCloseWindowOnApplySetting()
{
    if (!GConfig)
    {
        return;
    }

    GConfig->SetBool(
        NineSliceColorConfigSection,
        CloseWindowOnApplyConfigKey,
        bCloseWindowOnApply,
        GEditorPerProjectIni
    );

    GConfig->Flush(
        false,
        GEditorPerProjectIni
    );
}

void SNineSliceEditor::LoadPreviewPanelVisibleSetting()
{
    bPreviewPanelVisible =
        true;

    if (!GConfig)
    {
        return;
    }

    GConfig->GetBool(
        NineSliceColorConfigSection,
        PreviewPanelVisibleConfigKey,
        bPreviewPanelVisible,
        GEditorPerProjectIni
    );
}

void SNineSliceEditor::SavePreviewPanelVisibleSetting()
{
    if (!GConfig)
    {
        return;
    }

    GConfig->SetBool(
        NineSliceColorConfigSection,
        PreviewPanelVisibleConfigKey,
        bPreviewPanelVisible,
        GEditorPerProjectIni
    );

    GConfig->Flush(
        false,
        GEditorPerProjectIni
    );
}

void SNineSliceEditor::OnCloseWindowOnApplyChanged(
    ECheckBoxState NewState
)
{
    bCloseWindowOnApply =
        NewState ==
        ECheckBoxState::Checked;

    SaveCloseWindowOnApplySetting();
}

ECheckBoxState
SNineSliceEditor::GetCloseWindowOnApplyState() const
{
    return bCloseWindowOnApply
        ? ECheckBoxState::Checked
        : ECheckBoxState::Unchecked;
}

float SNineSliceEditor::GetZoomSliderValue() const
{
    const float Range =
        CanvasZoomMax -
        CanvasZoomMin;

    if (Range <= 0.0f)
    {
        return 0.0f;
    }

    return FMath::Clamp(
        (
            CanvasZoom -
            CanvasZoomMin
        ) /
        Range,
        0.0f,
        1.0f
    );
}

void SNineSliceEditor::OnZoomSliderChanged(
    float NewValue
)
{
    const FPreviewRect CanvasRect =
        CalculateCanvasViewportRect(
            GetCachedGeometry().GetLocalSize()
        );

    const FVector2D CanvasCenter =
        CanvasRect.Position +
        CanvasRect.Size *
        0.5f;

    const float NewZoom =
        FMath::Lerp(
            CanvasZoomMin,
            CanvasZoomMax,
            FMath::Clamp(
                NewValue,
                0.0f,
                1.0f
            )
        );

    SetCanvasZoomAtPosition(
        NewZoom,
        CanvasCenter,
        GetCachedGeometry().GetLocalSize()
    );
}

FText SNineSliceEditor::GetZoomText() const
{
    return FText::FromString(
        FString::Printf(
            TEXT("%.0f%%"),
            CanvasZoom *
            100.0f
        )
    );
}

FReply SNineSliceEditor::OnZoomMinusClicked()
{
    const FPreviewRect CanvasRect =
        CalculateCanvasViewportRect(
            GetCachedGeometry().GetLocalSize()
        );

    const FVector2D CanvasCenter =
        CanvasRect.Position +
        CanvasRect.Size *
        0.5f;

    SetCanvasZoomAtPosition(
        CanvasZoom -
        CanvasZoomButtonStep,
        CanvasCenter,
        GetCachedGeometry().GetLocalSize()
    );

    return FReply::Handled();
}

FReply SNineSliceEditor::OnZoomPlusClicked()
{
    const FPreviewRect CanvasRect =
        CalculateCanvasViewportRect(
            GetCachedGeometry().GetLocalSize()
        );

    const FVector2D CanvasCenter =
        CanvasRect.Position +
        CanvasRect.Size *
        0.5f;

    SetCanvasZoomAtPosition(
        CanvasZoom +
        CanvasZoomButtonStep,
        CanvasCenter,
        GetCachedGeometry().GetLocalSize()
    );

    return FReply::Handled();
}

FReply SNineSliceEditor::OnZoomFitClicked()
{
    ResetCanvasZoom();

    return FReply::Handled();
}

FText SNineSliceEditor::GetSelectedColorPresetText(
    EColorSelector Selector
) const
{
    const TSharedPtr<FColorPreset>* SelectedPreset =
        nullptr;

    switch (Selector)
    {
        case EColorSelector::Background:
        {
            SelectedPreset =
                &SelectedBackgroundColor;

            break;
        }

        case EColorSelector::Guide:
        {
            SelectedPreset =
                &SelectedGuideColor;

            break;
        }

        case EColorSelector::Canvas:
        {
            SelectedPreset =
                &SelectedCanvasColor;

            break;
        }

        default:
        {
            break;
        }
    }

    if (
        SelectedPreset == nullptr ||
        !SelectedPreset->IsValid()
    )
    {
        return FText::GetEmpty();
    }

    return (*SelectedPreset)->Name;
}

int32 SNineSliceEditor::OnPaint(
    const FPaintArgs& Args,
    const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect,
    FSlateWindowElementList& OutDrawElements,
    int32 LayerId,
    const FWidgetStyle& InWidgetStyle,
    bool bParentEnabled
) const
{
    const FSlateBrush* WhiteBrush =
        FAppStyle::Get().GetBrush(
            TEXT("WhiteBrush")
        );

    FSlateDrawElement::MakeBox(
        OutDrawElements,
        LayerId,
        AllottedGeometry.ToPaintGeometry(),
        WhiteBrush,
        ESlateDrawEffect::None,
        BackgroundColor
    );

    if (HasResource())
    {
        const FVector2D LocalSize =
            AllottedGeometry.GetLocalSize();

        const FPreviewRect CanvasRect =
            CalculateCanvasViewportRect(
                LocalSize
            );

        const FPreviewRect PreviewRect =
            CalculatePreviewRect(
                LocalSize
            );

        const float SourceWidth =
            GetSourceWidth();

        const float SourceHeight =
            GetSourceHeight();

        if (
            CanvasRect.Size.X > 0.0f &&
            CanvasRect.Size.Y > 0.0f &&
            PreviewRect.Size.X > 0.0f &&
            PreviewRect.Size.Y > 0.0f &&
            SourceWidth > 0.0f &&
            SourceHeight > 0.0f
        )
        {
            const FPaintGeometry CanvasPaintGeometry =
                AllottedGeometry.ToPaintGeometry(
                    CanvasRect.Size,
                    FSlateLayoutTransform(
                        CanvasRect.Position
                    )
                );

            OutDrawElements.PushClip(
                FSlateClippingZone(
                    CanvasPaintGeometry
                )
            );

            const int32 PreviewLayer =
                LayerId + 1;

            const int32 GuideLayer =
                LayerId + 2;

            const float LeftGuideX =
                PreviewRect.Position.X +
                (
                    LeftMargin /
                    SourceWidth
                ) *
                PreviewRect.Size.X;

            const float RightGuideX =
                PreviewRect.Position.X +
                PreviewRect.Size.X -
                (
                    RightMargin /
                    SourceWidth
                ) *
                PreviewRect.Size.X;

            const float TopGuideY =
                PreviewRect.Position.Y +
                (
                    TopMargin /
                    SourceHeight
                ) *
                PreviewRect.Size.Y;

            const float BottomGuideY =
                PreviewRect.Position.Y +
                PreviewRect.Size.Y -
                (
                    BottomMargin /
                    SourceHeight
                ) *
                PreviewRect.Size.Y;

            FSlateBrush MainBrush =
                Brush;

            MainBrush.DrawAs =
                ESlateBrushDrawType::Image;

            MainBrush.Tiling =
                ESlateBrushTileType::NoTile;

            MainBrush.Mirroring =
                ESlateBrushMirrorType::NoMirror;

            MainBrush.Margin =
                FMargin(
                    0.0f
                );

            FSlateDrawElement::MakeBox(
                OutDrawElements,
                PreviewLayer,
                AllottedGeometry.ToPaintGeometry(
                    PreviewRect.Size,
                    FSlateLayoutTransform(
                        PreviewRect.Position
                    )
                ),
                &MainBrush,
                ESlateDrawEffect::None,
                FLinearColor::White
            );

            FSlateColorBrush GuideBrush(
                GuideColor
            );

            FSlateRoundedBoxBrush HorizontalHandleBrush(
                HandleColor,
                HandleSize *
                0.5f,
                FVector2f(
                    HandleLength,
                    HandleSize
                )
            );

            FSlateRoundedBoxBrush VerticalHandleBrush(
                HandleColor,
                HandleSize *
                0.5f,
                FVector2f(
                    HandleSize,
                    HandleLength
                )
            );

            FSlateFontInfo HandleFont =
                FCoreStyle::Get().GetFontStyle(
                    TEXT("BoldFont")
                );

            HandleFont.Size =
                10;

            const TSharedRef<FSlateFontMeasure> FontMeasure =
                FSlateApplication::Get()
                .GetRenderer()
                ->GetFontMeasureService();

            const auto DrawHandle =
                [&](
                    const FVector2D& Center,
                    bool bHorizontal,
                    const TCHAR* Label
                )
                {
                    const FVector2D HandleDimensions =
                        bHorizontal
                            ? FVector2D(
                                HandleLength,
                                HandleSize
                            )
                            : FVector2D(
                                HandleSize,
                                HandleLength
                            );

                    const FVector2D HandlePosition =
                        Center -
                        HandleDimensions *
                        0.5f;

                    FSlateDrawElement::MakeBox(
                        OutDrawElements,
                        GuideLayer + 1,
                        AllottedGeometry.ToPaintGeometry(
                            HandleDimensions,
                            FSlateLayoutTransform(
                                HandlePosition
                            )
                        ),
                        bHorizontal
                            ? static_cast<const FSlateBrush*>(
                                &HorizontalHandleBrush
                            )
                            : static_cast<const FSlateBrush*>(
                                &VerticalHandleBrush
                            ),
                        ESlateDrawEffect::None,
                        HandleColor
                    );

                    const FText LabelText =
                        FText::FromString(
                            FString(
                                Label
                            )
                        );

                    const FVector2f MeasuredSize =
                        FontMeasure->Measure(
                            LabelText,
                            HandleFont,
                            1.0f
                        );

                    const FVector2D TextSize(
                        MeasuredSize.X,
                        MeasuredSize.Y
                    );

                    const FVector2D TextPosition =
                        Center -
                        TextSize *
                        0.5f;

                    FSlateDrawElement::MakeText(
                        OutDrawElements,
                        GuideLayer + 2,
                        AllottedGeometry.ToPaintGeometry(
                            TextSize,
                            FSlateLayoutTransform(
                                TextPosition
                            )
                        ),
                        LabelText,
                        HandleFont,
                        ESlateDrawEffect::None,
                        HandleTextColor
                    );
                };

            FSlateDrawElement::MakeBox(
                OutDrawElements,
                GuideLayer,
                AllottedGeometry.ToPaintGeometry(
                    FVector2D(
                        GuideThickness,
                        PreviewRect.Size.Y
                    ),
                    FSlateLayoutTransform(
                        FVector2D(
                            LeftGuideX -
                            GuideThickness *
                            0.5f,
                            PreviewRect.Position.Y
                        )
                    )
                ),
                &GuideBrush,
                ESlateDrawEffect::None,
                GuideColor
            );

            FSlateDrawElement::MakeBox(
                OutDrawElements,
                GuideLayer,
                AllottedGeometry.ToPaintGeometry(
                    FVector2D(
                        GuideThickness,
                        PreviewRect.Size.Y
                    ),
                    FSlateLayoutTransform(
                        FVector2D(
                            RightGuideX -
                            GuideThickness *
                            0.5f,
                            PreviewRect.Position.Y
                        )
                    )
                ),
                &GuideBrush,
                ESlateDrawEffect::None,
                GuideColor
            );

            FSlateDrawElement::MakeBox(
                OutDrawElements,
                GuideLayer,
                AllottedGeometry.ToPaintGeometry(
                    FVector2D(
                        PreviewRect.Size.X,
                        GuideThickness
                    ),
                    FSlateLayoutTransform(
                        FVector2D(
                            PreviewRect.Position.X,
                            TopGuideY -
                            GuideThickness *
                            0.5f
                        )
                    )
                ),
                &GuideBrush,
                ESlateDrawEffect::None,
                GuideColor
            );

            FSlateDrawElement::MakeBox(
                OutDrawElements,
                GuideLayer,
                AllottedGeometry.ToPaintGeometry(
                    FVector2D(
                        PreviewRect.Size.X,
                        GuideThickness
                    ),
                    FSlateLayoutTransform(
                        FVector2D(
                            PreviewRect.Position.X,
                            BottomGuideY -
                            GuideThickness *
                            0.5f
                        )
                    )
                ),
                &GuideBrush,
                ESlateDrawEffect::None,
                GuideColor
            );

            DrawHandle(
                FVector2D(
                    LeftGuideX,
                    PreviewRect.Position.Y +
                    PreviewRect.Size.Y *
                    0.5f
                ),
                false,
                TEXT("L")
            );

            DrawHandle(
                FVector2D(
                    RightGuideX,
                    PreviewRect.Position.Y +
                    PreviewRect.Size.Y *
                    0.5f
                ),
                false,
                TEXT("R")
            );

            DrawHandle(
                FVector2D(
                    PreviewRect.Position.X +
                    PreviewRect.Size.X *
                    0.5f,
                    TopGuideY
                ),
                true,
                TEXT("T")
            );

            DrawHandle(
                FVector2D(
                    PreviewRect.Position.X +
                    PreviewRect.Size.X *
                    0.5f,
                    BottomGuideY
                ),
                true,
                TEXT("B")
            );

            OutDrawElements.PopClip();
        }
    }

    return SCompoundWidget::OnPaint(
        Args,
        AllottedGeometry,
        MyCullingRect,
        OutDrawElements,
        LayerId + 10,
        InWidgetStyle,
        bParentEnabled
    );
}

SNineSliceEditor::EHandle
SNineSliceEditor::HitTestHandle(
    const FVector2D& LocalPosition,
    const FGeometry& Geometry
) const
{
    if (!HasResource())
    {
        return EHandle::None;
    }

    const FPreviewRect CanvasRect =
        CalculateCanvasViewportRect(
            Geometry.GetLocalSize()
        );

    const bool bInsideCanvas =
        LocalPosition.X >=
            CanvasRect.Position.X &&
        LocalPosition.X <=
            CanvasRect.Position.X +
            CanvasRect.Size.X &&
        LocalPosition.Y >=
            CanvasRect.Position.Y &&
        LocalPosition.Y <=
            CanvasRect.Position.Y +
            CanvasRect.Size.Y;

    if (!bInsideCanvas)
    {
        return EHandle::None;
    }

    const FPreviewRect PreviewRect =
        CalculatePreviewRect(
            Geometry.GetLocalSize()
        );

    const float SourceWidth =
        GetSourceWidth();

    const float SourceHeight =
        GetSourceHeight();

    if (
        SourceWidth <= 0.0f ||
        SourceHeight <= 0.0f ||
        PreviewRect.Size.X <= 0.0f ||
        PreviewRect.Size.Y <= 0.0f
    )
    {
        return EHandle::None;
    }

    const float LeftX =
        PreviewRect.Position.X +
        (
            LeftMargin /
            SourceWidth
        ) *
        PreviewRect.Size.X;

    const float RightX =
        PreviewRect.Position.X +
        PreviewRect.Size.X -
        (
            RightMargin /
            SourceWidth
        ) *
        PreviewRect.Size.X;

    const float TopY =
        PreviewRect.Position.Y +
        (
            TopMargin /
            SourceHeight
        ) *
        PreviewRect.Size.Y;

    const float BottomY =
        PreviewRect.Position.Y +
        PreviewRect.Size.Y -
        (
            BottomMargin /
            SourceHeight
        ) *
        PreviewRect.Size.Y;

    const bool bInsideVerticalRange =
        LocalPosition.Y >=
            PreviewRect.Position.Y -
            HandleHitDistance &&
        LocalPosition.Y <=
            PreviewRect.Position.Y +
            PreviewRect.Size.Y +
            HandleHitDistance;

    const bool bInsideHorizontalRange =
        LocalPosition.X >=
            PreviewRect.Position.X -
            HandleHitDistance &&
        LocalPosition.X <=
            PreviewRect.Position.X +
            PreviewRect.Size.X +
            HandleHitDistance;

    if (
        bInsideVerticalRange &&
        FMath::Abs(
            LocalPosition.X -
            LeftX
        ) <= HandleHitDistance
    )
    {
        return EHandle::Left;
    }

    if (
        bInsideVerticalRange &&
        FMath::Abs(
            LocalPosition.X -
            RightX
        ) <= HandleHitDistance
    )
    {
        return EHandle::Right;
    }

    if (
        bInsideHorizontalRange &&
        FMath::Abs(
            LocalPosition.Y -
            TopY
        ) <= HandleHitDistance
    )
    {
        return EHandle::Top;
    }

    if (
        bInsideHorizontalRange &&
        FMath::Abs(
            LocalPosition.Y -
            BottomY
        ) <= HandleHitDistance
    )
    {
        return EHandle::Bottom;
    }

    return EHandle::None;
}

void SNineSliceEditor::UpdateHoveredHandle(
    const FVector2D& LocalPosition,
    const FGeometry& Geometry
)
{
    HoveredHandle =
        HitTestHandle(
            LocalPosition,
            Geometry
        );

    Invalidate(
        EInvalidateWidgetReason::Paint
    );
}

void SNineSliceEditor::UpdateMarginsFromMouse(
    const FVector2D& LocalPosition,
    const FPreviewRect& PreviewRect
)
{
    const float SourceWidth =
        GetSourceWidth();

    const float SourceHeight =
        GetSourceHeight();

    if (
        SourceWidth <= 0.0f ||
        SourceHeight <= 0.0f ||
        PreviewRect.Size.X <= 0.0f ||
        PreviewRect.Size.Y <= 0.0f
    )
    {
        return;
    }

    const float ScaleX =
        PreviewRect.Size.X /
        SourceWidth;

    const float ScaleY =
        PreviewRect.Size.Y /
        SourceHeight;

    if (
        ScaleX <= 0.0f ||
        ScaleY <= 0.0f
    )
    {
        return;
    }

    const FVector2D Delta =
        LocalPosition -
        DragStartPosition;

    switch (ActiveHandle)
    {
        case EHandle::Left:
        {
            LeftMargin =
                DragStartLeft +
                Delta.X /
                ScaleX;

            break;
        }

        case EHandle::Top:
        {
            TopMargin =
                DragStartTop +
                Delta.Y /
                ScaleY;

            break;
        }

        case EHandle::Right:
        {
            RightMargin =
                DragStartRight -
                Delta.X /
                ScaleX;

            break;
        }

        case EHandle::Bottom:
        {
            BottomMargin =
                DragStartBottom -
                Delta.Y /
                ScaleY;

            break;
        }

        default:
        {
            break;
        }
    }

    LeftMargin =
        FMath::RoundToFloat(
            LeftMargin
        );

    TopMargin =
        FMath::RoundToFloat(
            TopMargin
        );

    RightMargin =
        FMath::RoundToFloat(
            RightMargin
        );

    BottomMargin =
        FMath::RoundToFloat(
            BottomMargin
        );

    ClampMargins();

    UpdatePreviewWidget();

    Invalidate(
        EInvalidateWidgetReason::Paint
    );
}

FReply SNineSliceEditor::OnMouseButtonDown(
    const FGeometry& MyGeometry,
    const FPointerEvent& MouseEvent
)
{
    if (
        MouseEvent.GetEffectingButton() ==
        EKeys::RightMouseButton
    )
    {
        const FVector2D LocalPosition =
            MyGeometry.AbsoluteToLocal(
                MouseEvent.GetScreenSpacePosition()
            );

        const FPreviewRect CanvasRect =
            CalculateCanvasViewportRect(
                MyGeometry.GetLocalSize()
            );

        const bool bInsideCanvas =
            LocalPosition.X >=
                CanvasRect.Position.X &&
            LocalPosition.X <=
                CanvasRect.Position.X +
                CanvasRect.Size.X &&
            LocalPosition.Y >=
                CanvasRect.Position.Y &&
            LocalPosition.Y <=
                CanvasRect.Position.Y +
                CanvasRect.Size.Y;

        if (bInsideCanvas)
        {
            bPanningCanvas =
                true;

            PanStartMousePosition =
                LocalPosition;

            PanStartOffset =
                CanvasPanOffset;

            return FReply::Handled()
                .CaptureMouse(
                    AsShared()
                );
        }
    }

    if (
        MouseEvent.GetEffectingButton() !=
        EKeys::LeftMouseButton
    )
    {
        return SCompoundWidget::OnMouseButtonDown(
            MyGeometry,
            MouseEvent
        );
    }

    const FVector2D LocalPosition =
        MyGeometry.AbsoluteToLocal(
            MouseEvent.GetScreenSpacePosition()
        );

    const EHandle Handle =
        HitTestHandle(
            LocalPosition,
            MyGeometry
        );

    if (Handle == EHandle::None)
    {
        return SCompoundWidget::OnMouseButtonDown(
            MyGeometry,
            MouseEvent
        );
    }

    ActiveHandle =
        Handle;

    DragStartPosition =
        LocalPosition;

    DragStartLeft =
        LeftMargin;

    DragStartTop =
        TopMargin;

    DragStartRight =
        RightMargin;

    DragStartBottom =
        BottomMargin;

    Invalidate(
        EInvalidateWidgetReason::Paint
    );

    return FReply::Handled()
        .CaptureMouse(
            AsShared()
        );
}

FReply SNineSliceEditor::OnMouseButtonUp(
    const FGeometry& MyGeometry,
    const FPointerEvent& MouseEvent
)
{
    if (
        MouseEvent.GetEffectingButton() ==
        EKeys::RightMouseButton
    )
    {
        if (bPanningCanvas)
        {
            bPanningCanvas =
                false;

            return FReply::Handled()
                .ReleaseMouseCapture();
        }
    }

    if (
        MouseEvent.GetEffectingButton() !=
        EKeys::LeftMouseButton
    )
    {
        return SCompoundWidget::OnMouseButtonUp(
            MyGeometry,
            MouseEvent
        );
    }

    if (
        ActiveHandle ==
        EHandle::None
    )
    {
        return SCompoundWidget::OnMouseButtonUp(
            MyGeometry,
            MouseEvent
        );
    }

    ActiveHandle =
        EHandle::None;

    Invalidate(
        EInvalidateWidgetReason::Paint
    );

    return FReply::Handled()
        .ReleaseMouseCapture();
}

FReply SNineSliceEditor::OnMouseMove(
    const FGeometry& MyGeometry,
    const FPointerEvent& MouseEvent
)
{
    const FVector2D LocalPosition =
        MyGeometry.AbsoluteToLocal(
            MouseEvent.GetScreenSpacePosition()
        );

    if (bPanningCanvas)
    {
        const FVector2D Delta =
            LocalPosition -
            PanStartMousePosition;

        CanvasPanOffset =
            PanStartOffset +
            Delta;

        ClampCanvasPan(
            MyGeometry.GetLocalSize()
        );

        Invalidate(
            EInvalidateWidgetReason::Paint
        );

        return FReply::Handled();
    }

    if (
        ActiveHandle !=
        EHandle::None
    )
    {
        const FPreviewRect PreviewRect =
            CalculatePreviewRect(
                MyGeometry.GetLocalSize()
            );

        UpdateMarginsFromMouse(
            LocalPosition,
            PreviewRect
        );

        return FReply::Handled();
    }

    UpdateHoveredHandle(
        LocalPosition,
        MyGeometry
    );

    return SCompoundWidget::OnMouseMove(
        MyGeometry,
        MouseEvent
    );
}

FReply SNineSliceEditor::OnMouseWheel(
    const FGeometry& MyGeometry,
    const FPointerEvent& MouseEvent
)
{
    const FVector2D LocalPosition =
        MyGeometry.AbsoluteToLocal(
            MouseEvent.GetScreenSpacePosition()
        );

    const FPreviewRect CanvasRect =
        CalculateCanvasViewportRect(
            MyGeometry.GetLocalSize()
        );

    const bool bInsideCanvas =
        LocalPosition.X >=
            CanvasRect.Position.X &&
        LocalPosition.X <=
            CanvasRect.Position.X +
            CanvasRect.Size.X &&
        LocalPosition.Y >=
            CanvasRect.Position.Y &&
        LocalPosition.Y <=
            CanvasRect.Position.Y +
            CanvasRect.Size.Y;

    if (!bInsideCanvas)
    {
        return SCompoundWidget::OnMouseWheel(
            MyGeometry,
            MouseEvent
        );
    }

    const float WheelDelta =
        MouseEvent.GetWheelDelta();

    if (
        FMath::IsNearlyZero(
            WheelDelta
        )
    )
    {
        return FReply::Unhandled();
    }

    const float ZoomFactor =
        FMath::Pow(
            CanvasZoomWheelFactor,
            WheelDelta
        );

    SetCanvasZoomAtPosition(
        CanvasZoom *
        ZoomFactor,
        LocalPosition,
        MyGeometry.GetLocalSize()
    );

    return FReply::Handled();
}

TOptional<EMouseCursor::Type>
SNineSliceEditor::GetCursor() const
{
    if (bPanningCanvas)
    {
        return EMouseCursor::GrabHandClosed;
    }

    switch (HoveredHandle)
    {
        case EHandle::Left:
        case EHandle::Right:
        {
            return EMouseCursor::ResizeLeftRight;
        }

        case EHandle::Top:
        case EHandle::Bottom:
        {
            return EMouseCursor::ResizeUpDown;
        }

        default:
        {
            break;
        }
    }

    if (CanvasZoom > 1.0f)
    {
        return EMouseCursor::GrabHand;
    }

    return TOptional<EMouseCursor::Type>();
}

TOptional<float>
SNineSliceEditor::GetLeftValue() const
{
    return PixelToDisplay(
        LeftMargin,
        GetSourceWidth()
    );
}

TOptional<float>
SNineSliceEditor::GetTopValue() const
{
    return PixelToDisplay(
        TopMargin,
        GetSourceHeight()
    );
}

TOptional<float>
SNineSliceEditor::GetRightValue() const
{
    return PixelToDisplay(
        RightMargin,
        GetSourceWidth()
    );
}

TOptional<float>
SNineSliceEditor::GetBottomValue() const
{
    return PixelToDisplay(
        BottomMargin,
        GetSourceHeight()
    );
}

void SNineSliceEditor::SetLeftValue(
    float NewValue
)
{
    LeftMargin =
        DisplayToPixel(
            NewValue,
            GetSourceWidth()
        );

    ActiveHandle =
        EHandle::Left;

    ClampMargins();

    ActiveHandle =
        EHandle::None;

    UpdatePreviewWidget();

    Invalidate(
        EInvalidateWidgetReason::Paint
    );
}

void SNineSliceEditor::SetTopValue(
    float NewValue
)
{
    TopMargin =
        DisplayToPixel(
            NewValue,
            GetSourceHeight()
        );

    ActiveHandle =
        EHandle::Top;

    ClampMargins();

    ActiveHandle =
        EHandle::None;

    UpdatePreviewWidget();

    Invalidate(
        EInvalidateWidgetReason::Paint
    );
}

void SNineSliceEditor::SetRightValue(
    float NewValue
)
{
    RightMargin =
        DisplayToPixel(
            NewValue,
            GetSourceWidth()
        );

    ActiveHandle =
        EHandle::Right;

    ClampMargins();

    ActiveHandle =
        EHandle::None;

    UpdatePreviewWidget();

    Invalidate(
        EInvalidateWidgetReason::Paint
    );
}

void SNineSliceEditor::SetBottomValue(
    float NewValue
)
{
    BottomMargin =
        DisplayToPixel(
            NewValue,
            GetSourceHeight()
        );

    ActiveHandle =
        EHandle::Bottom;

    ClampMargins();

    ActiveHandle =
        EHandle::None;

    UpdatePreviewWidget();

    Invalidate(
        EInvalidateWidgetReason::Paint
    );
}

void SNineSliceEditor::OnLeftValueChanged(
    float NewValue
)
{
    SetLeftValue(
        NewValue
    );
}

void SNineSliceEditor::OnTopValueChanged(
    float NewValue
)
{
    SetTopValue(
        NewValue
    );
}

void SNineSliceEditor::OnRightValueChanged(
    float NewValue
)
{
    SetRightValue(
        NewValue
    );
}

void SNineSliceEditor::OnBottomValueChanged(
    float NewValue
)
{
    SetBottomValue(
        NewValue
    );
}

void SNineSliceEditor::OnLeftValueCommitted(
    float NewValue,
    ETextCommit::Type CommitType
)
{
    SetLeftValue(
        NewValue
    );
}

void SNineSliceEditor::OnTopValueCommitted(
    float NewValue,
    ETextCommit::Type CommitType
)
{
    SetTopValue(
        NewValue
    );
}

void SNineSliceEditor::OnRightValueCommitted(
    float NewValue,
    ETextCommit::Type CommitType
)
{
    SetRightValue(
        NewValue
    );
}

void SNineSliceEditor::OnBottomValueCommitted(
    float NewValue,
    ETextCommit::Type CommitType
)
{
    SetBottomValue(
        NewValue
    );
}

FReply SNineSliceEditor::OnApplyClicked()
{
    ApplyMargins();
    SaveCloseWindowOnApplySetting();
    SavePreviewPanelVisibleSetting();

    if (bCloseWindowOnApply)
    {
        TSharedPtr<SWindow> Window =
            FSlateApplication::Get().FindWidgetWindow(
                AsShared()
            );

        if (Window.IsValid())
        {
            Window->RequestDestroyWindow();
        }
    }

    return FReply::Handled();
}

FReply SNineSliceEditor::OnCancelClicked()
{
    TSharedPtr<SWindow> Window =
        FSlateApplication::Get().FindWidgetWindow(
            AsShared()
        );

    if (Window.IsValid())
    {
        Window->RequestDestroyWindow();
    }

    return FReply::Handled();
}