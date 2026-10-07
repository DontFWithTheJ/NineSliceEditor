#pragma once

#include "CoreMinimal.h"
#include "Brushes/SlateColorBrush.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Styling/SlateBrush.h"
#include "Widgets/Input/NumericTypeInterface.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/SCompoundWidget.h"

class IPropertyHandle;
class SBox;
class SWindow;
class SNineSlicePreview;

class SNineSliceEditor : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SNineSliceEditor)
        : _Brush()
    {
    }

    SLATE_ARGUMENT(FSlateBrush, Brush)
    SLATE_ARGUMENT(TSharedPtr<IPropertyHandle>, LeftHandle)
    SLATE_ARGUMENT(TSharedPtr<IPropertyHandle>, TopHandle)
    SLATE_ARGUMENT(TSharedPtr<IPropertyHandle>, RightHandle)
    SLATE_ARGUMENT(TSharedPtr<IPropertyHandle>, BottomHandle)
    SLATE_END_ARGS()

    static void OpenEditorWindow(
        const FSlateBrush& InBrush,
        const TSharedPtr<IPropertyHandle>& InLeftHandle,
        const TSharedPtr<IPropertyHandle>& InTopHandle,
        const TSharedPtr<IPropertyHandle>& InRightHandle,
        const TSharedPtr<IPropertyHandle>& InBottomHandle,
        const TSharedPtr<SWindow>& InParentWindow
    );

    void Construct(
        const FArguments& InArgs
    );

protected:
    virtual FVector2D ComputeDesiredSize(
        float LayoutScaleMultiplier
    ) const override;

    virtual int32 OnPaint(
        const FPaintArgs& Args,
        const FGeometry& AllottedGeometry,
        const FSlateRect& MyCullingRect,
        FSlateWindowElementList& OutDrawElements,
        int32 LayerId,
        const FWidgetStyle& InWidgetStyle,
        bool bParentEnabled
    ) const override;

    virtual FReply OnMouseButtonDown(
        const FGeometry& MyGeometry,
        const FPointerEvent& MouseEvent
    ) override;

    virtual FReply OnMouseButtonUp(
        const FGeometry& MyGeometry,
        const FPointerEvent& MouseEvent
    ) override;

    virtual FReply OnMouseMove(
        const FGeometry& MyGeometry,
        const FPointerEvent& MouseEvent
    ) override;

    virtual FReply OnMouseWheel(
        const FGeometry& MyGeometry,
        const FPointerEvent& MouseEvent
    ) override;

    virtual TOptional<EMouseCursor::Type> GetCursor() const override;

private:
    enum class EHandle : uint8
    {
        None,
        Left,
        Top,
        Right,
        Bottom
    };

    enum class EValueUnit : uint8
    {
        Pixels,
        Normalized,
        Percentage
    };

    enum class EColorSelector : uint8
    {
        Background,
        Guide,
        Canvas
    };

    struct FPreviewRect
    {
        FVector2D Position = FVector2D::ZeroVector;
        FVector2D Size = FVector2D::ZeroVector;
    };

    struct FColorPreset
    {
        FText Name;

        FLinearColor Color =
            FLinearColor::White;

        FLinearColor TextColor =
            FLinearColor::Black;
    };

    FSlateBrush Brush;

    TUniquePtr<FSlateRoundedBoxBrush> PreviewPanelBrush;
    TUniquePtr<FSlateRoundedBoxBrush> ZoomControlBrush;

    TUniquePtr<FSlateRoundedBoxBrush> EditorButtonNormalBrush;
    TUniquePtr<FSlateRoundedBoxBrush> EditorButtonHoveredBrush;
    TUniquePtr<FSlateRoundedBoxBrush> EditorButtonPressedBrush;
    TUniquePtr<FSlateRoundedBoxBrush> ColorDropdownItemBrush;

    FButtonStyle EditorButtonStyle;
    FComboBoxStyle EditorComboBoxStyle;

    TSharedPtr<IPropertyHandle> LeftHandle;
    TSharedPtr<IPropertyHandle> TopHandle;
    TSharedPtr<IPropertyHandle> RightHandle;
    TSharedPtr<IPropertyHandle> BottomHandle;

    TSharedPtr<SNineSlicePreview> PreviewWidget;
    TSharedPtr<SBox> WorkspaceContainer;
    TSharedPtr<SBox> PreviewPanelContainer;

    TArray<TSharedPtr<EValueUnit>> ValueUnitOptions;
    TSharedPtr<EValueUnit> SelectedValueUnit;
    TSharedPtr<INumericTypeInterface<float>> NumericInterface;

    TArray<TSharedPtr<FColorPreset>> BackgroundColorOptions;
    TArray<TSharedPtr<FColorPreset>> GuideColorOptions;
    TArray<TSharedPtr<FColorPreset>> CanvasColorOptions;

    TSharedPtr<FColorPreset> SelectedBackgroundColor;
    TSharedPtr<FColorPreset> SelectedGuideColor;
    TSharedPtr<FColorPreset> SelectedCanvasColor;

    EValueUnit ValueUnit =
        EValueUnit::Pixels;

    float LeftMargin = 0.0f;
    float TopMargin = 0.0f;
    float RightMargin = 0.0f;
    float BottomMargin = 0.0f;

    float DragStartLeft = 0.0f;
    float DragStartTop = 0.0f;
    float DragStartRight = 0.0f;
    float DragStartBottom = 0.0f;

    FVector2D DragStartPosition =
        FVector2D::ZeroVector;

    EHandle HoveredHandle =
        EHandle::None;

    EHandle ActiveHandle =
        EHandle::None;

    float MaxPreviewWidth =
        800.0f;

    float MaxPreviewHeight =
        500.0f;

    float GuideThickness =
        1.0f;

    float HandleSize =
        12.0f;

    float HandleLength =
        28.0f;

    float HandleHitDistance =
        12.0f;

    bool bPreviewPanelVisible =
        true;

    float PreviewPanelWidth =
        420.0f;

    float PreviewPanelExpandedWidth =
        420.0f;

    float PreviewPanelAnimationSpeed =
        800.0f;

    float PreviewStretchAnimationSpeed =
        0.7f;

    float PreviewBaseMaxSize =
        0.0f;

    float PreviewStretchMaxScale =
        3.0f;

    float PreviewCardGap =
        6.0f;

    float PreviewCardPadding =
        8.0f;

    float PreviewLabelHeight =
        24.0f;

    bool bCloseWindowOnApply =
        true;

    float CanvasZoom =
        1.0f;

    float CanvasZoomMin =
        0.25f;

    float CanvasZoomMax =
        4.0f;

    float CanvasZoomButtonStep =
        0.25f;

    float CanvasZoomWheelFactor =
        1.15f;

    float CanvasViewportHorizontalPadding =
        8.0f;

    float CanvasViewportTopPadding =
        8.0f;

    float CanvasViewportBottomPadding =
        72.0f;

    float CanvasPanOverscroll =
        80.0f;

    FVector2D CanvasPanOffset =
        FVector2D::ZeroVector;

    bool bPanningCanvas =
        false;

    FVector2D PanStartMousePosition =
        FVector2D::ZeroVector;

    FVector2D PanStartOffset =
        FVector2D::ZeroVector;

    FLinearColor BackgroundColor =
        FLinearColor(
            0.05f,
            0.06f,
            0.07f,
            1.0f
        );

    FLinearColor EditorTextColor =
        FLinearColor::White;

    FLinearColor GuideColor =
        FLinearColor(
            1.0f,
            0.0f,
            0.0f,
            1.0f
        );

    FLinearColor HandleColor =
        FLinearColor(
            1.0f,
            0.0f,
            0.0f,
            1.0f
        );

    FLinearColor HandleTextColor =
        FLinearColor::Black;

    FLinearColor PreviewPanelColor =
        FLinearColor(
            0.12f,
            0.13f,
            0.15f,
            1.0f
        );

    FLinearColor PreviewPanelTextColor =
        FLinearColor::White;

    FLinearColor PreviewCloseButtonColor =
        FLinearColor(
            0.22f,
            0.23f,
            0.25f,
            1.0f
        );

    FLinearColor PreviewCloseButtonTextColor =
        FLinearColor::White;

    FLinearColor PreviewCardColor =
        FLinearColor(
            0.92f,
            0.92f,
            0.92f,
            1.0f
        );

    FLinearColor PreviewTextColor =
        FLinearColor(
            0.02f,
            0.02f,
            0.02f,
            1.0f
        );

    FLinearColor ZoomControlColor =
        FLinearColor(
            0.12f,
            0.13f,
            0.15f,
            1.0f
        );

    FLinearColor ZoomControlTextColor =
        FLinearColor::White;

    FLinearColor ZoomControlButtonColor =
        FLinearColor(
            0.22f,
            0.23f,
            0.25f,
            1.0f
        );

    FLinearColor ZoomControlButtonTextColor =
        FLinearColor::White;

    FLinearColor ZoomSliderBarColor =
        FLinearColor(
            0.28f,
            0.29f,
            0.31f,
            1.0f
        );

    FLinearColor ZoomSliderHandleColor =
        FLinearColor(
            0.90f,
            0.90f,
            0.90f,
            1.0f
        );

    FLinearColor DropdownItemColor =
        FLinearColor(
            0.14f,
            0.15f,
            0.17f,
            1.0f
        );

    FLinearColor DropdownItemTextColor =
        FLinearColor::White;

    float GetSourceWidth() const;
    float GetSourceHeight() const;

    bool HasResource() const;

    FPreviewRect CalculateCanvasViewportRect(
        const FVector2D& AvailableSize
    ) const;

    FPreviewRect CalculatePreviewRect(
        const FVector2D& AvailableSize
    ) const;

    float GetCanvasViewportWidth() const;

    void ClampCanvasPan(
        const FVector2D& AvailableSize
    );

    void SetCanvasZoom(
        float NewZoom
    );

    void SetCanvasZoomAtPosition(
        float NewZoom,
        const FVector2D& FocusPosition,
        const FVector2D& AvailableSize
    );

    void ResetCanvasZoom();

    void ReadPropertyValues();

    void ApplyMargins();

    void ClampMargins();

    void UpdateMarginsFromMouse(
        const FVector2D& LocalPosition,
        const FPreviewRect& PreviewRect
    );

    void UpdateHoveredHandle(
        const FVector2D& LocalPosition,
        const FGeometry& Geometry
    );

    EHandle HitTestHandle(
        const FVector2D& LocalPosition,
        const FGeometry& Geometry
    ) const;

    float PixelToDisplay(
        float PixelValue,
        float SourceSize
    ) const;

    float DisplayToPixel(
        float DisplayValue,
        float SourceSize
    ) const;

    float GetDisplayDelta() const;

    float GetDisplayValueMax(
        float PixelMax,
        float SourceSize
    ) const;

    int32 GetMaxFractionalDigits() const;
    int32 GetMinFractionalDigits() const;

    FText GetValueUnitText(
        TSharedPtr<EValueUnit> Unit
    ) const;

    FText GetSelectedValueUnitText() const;

    void OnValueUnitChanged(
        TSharedPtr<EValueUnit> NewUnit,
        ESelectInfo::Type SelectInfo
    );

    FText GetPreviewToggleText() const;

    FReply OnPreviewToggleClicked();

    EActiveTimerReturnType TickPreviewPanelAnimation(
        double InCurrentTime,
        float InDeltaTime
    );

    void UpdatePreviewWidget();

    void UpdateThemeColors();

    FSlateColor GetEditorTextColor() const;
    FSlateColor GetPreviewPanelTextColor() const;
    FSlateColor GetPreviewCloseButtonColor() const;
    FSlateColor GetPreviewCloseButtonTextColor() const;
    FSlateColor GetZoomControlTextColor() const;
    FSlateColor GetZoomControlButtonColor() const;
    FSlateColor GetZoomControlButtonTextColor() const;
    FSlateColor GetZoomSliderBarColor() const;
    FSlateColor GetZoomSliderHandleColor() const;

    void ApplyColorPreset(
        EColorSelector Selector,
        const TSharedPtr<FColorPreset>& Preset
    );

    void SaveColorPreset(
        EColorSelector Selector,
        const TSharedPtr<FColorPreset>& Preset
    );

    TSharedPtr<FColorPreset> LoadColorPreset(
        EColorSelector Selector,
        const TArray<TSharedPtr<FColorPreset>>& Options
    ) const;

    FText GetSelectedColorPresetText(
        EColorSelector Selector
    ) const;

    void SaveCloseWindowOnApplySetting();
    void LoadCloseWindowOnApplySetting();

    void SavePreviewPanelVisibleSetting();
    void LoadPreviewPanelVisibleSetting();

    void OnCloseWindowOnApplyChanged(
        ECheckBoxState NewState
    );

    ECheckBoxState GetCloseWindowOnApplyState() const;

    float GetZoomSliderValue() const;

    void OnZoomSliderChanged(
        float NewValue
    );

    FText GetZoomText() const;

    FReply OnZoomMinusClicked();
    FReply OnZoomPlusClicked();
    FReply OnZoomFitClicked();

    FReply OnApplyClicked();
    FReply OnCancelClicked();

    TOptional<float> GetLeftValue() const;
    TOptional<float> GetTopValue() const;
    TOptional<float> GetRightValue() const;
    TOptional<float> GetBottomValue() const;

    void OnLeftValueCommitted(
        float NewValue,
        ETextCommit::Type CommitType
    );

    void OnTopValueCommitted(
        float NewValue,
        ETextCommit::Type CommitType
    );

    void OnRightValueCommitted(
        float NewValue,
        ETextCommit::Type CommitType
    );

    void OnBottomValueCommitted(
        float NewValue,
        ETextCommit::Type CommitType
    );

    void OnLeftValueChanged(
        float NewValue
    );

    void OnTopValueChanged(
        float NewValue
    );

    void OnRightValueChanged(
        float NewValue
    );

    void OnBottomValueChanged(
        float NewValue
    );

    void SetLeftValue(
        float NewValue
    );

    void SetTopValue(
        float NewValue
    );

    void SetRightValue(
        float NewValue
    );

    void SetBottomValue(
        float NewValue
    );
};