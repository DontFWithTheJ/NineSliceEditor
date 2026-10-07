#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"
#include "Widgets/SCompoundWidget.h"

class SOverlay;
class SSlider;

class SNineSlicePreview : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SNineSlicePreview)
    {
    }

    SLATE_ARGUMENT(FSlateBrush, Brush)
    SLATE_ARGUMENT(float, SourceWidth)
    SLATE_ARGUMENT(float, SourceHeight)

    SLATE_ARGUMENT(float, LeftMargin)
    SLATE_ARGUMENT(float, TopMargin)
    SLATE_ARGUMENT(float, RightMargin)
    SLATE_ARGUMENT(float, BottomMargin)

    SLATE_ARGUMENT(float, AnimationSpeed)
    SLATE_ARGUMENT(float, BaseMaxSize)
    SLATE_ARGUMENT(float, StretchMaxScale)

    SLATE_ARGUMENT(float, CardGap)
    SLATE_ARGUMENT(float, CardPadding)
    SLATE_ARGUMENT(float, LabelHeight)

    SLATE_ARGUMENT(FLinearColor, CardColor)
    SLATE_ARGUMENT(FLinearColor, TextColor)

    SLATE_END_ARGS()

    void Construct(
        const FArguments& InArgs
    );

    void SetBrush(
        const FSlateBrush& InBrush,
        float InSourceWidth,
        float InSourceHeight
    );

    void SetMargins(
        float InLeft,
        float InTop,
        float InRight,
        float InBottom
    );

    void SetColors(
        const FLinearColor& InCardColor,
        const FLinearColor& InTextColor
    );

protected:
    virtual FVector2D ComputeDesiredSize(
        float LayoutScaleMultiplier
    ) const override;

    virtual void Tick(
        const FGeometry& AllottedGeometry,
        const double InCurrentTime,
        const float InDeltaTime
    ) override;

    virtual int32 OnPaint(
        const FPaintArgs& Args,
        const FGeometry& AllottedGeometry,
        const FSlateRect& MyCullingRect,
        FSlateWindowElementList& OutDrawElements,
        int32 LayerId,
        const FWidgetStyle& InWidgetStyle,
        bool bParentEnabled
    ) const override;

    virtual FReply OnPreviewMouseButtonDown(
        const FGeometry& MyGeometry,
        const FPointerEvent& MouseEvent
    ) override;

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
    enum class EStretchMode : uint8
    {
        Horizontal,
        Vertical,
        Both
    };

    struct FPreviewRect
    {
        FVector2D Position = FVector2D::ZeroVector;
        FVector2D Size = FVector2D::ZeroVector;
    };

    struct FSliceData
    {
        FVector2D Position = FVector2D::ZeroVector;
        FVector2D Size = FVector2D::ZeroVector;
        FBox2D UV = FBox2D(
            FVector2D::ZeroVector,
            FVector2D::ZeroVector
        );
    };

    static constexpr int32 CardCount = 3;

    FSlateBrush Brush;

    float SourceWidth = 1.0f;
    float SourceHeight = 1.0f;

    float LeftMargin = 0.0f;
    float TopMargin = 0.0f;
    float RightMargin = 0.0f;
    float BottomMargin = 0.0f;

    float AnimationSpeed = 0.35f;
    float BaseMaxSize = 0.0f;
    float StretchMaxScale = 3.0f;

    float CardGap = 6.0f;
    float CardPadding = 8.0f;
    float LabelHeight = 24.0f;

    FLinearColor CardColor = FLinearColor::White;

    FLinearColor TextColor = FLinearColor(
        0.03f,
        0.03f,
        0.04f,
        1.0f
    );

    float AnimationPhase = 0.0f;
    float AnimationScale = 0.0f;

    float CardScaleMin = 1.0f;
    float CardScaleMax = 3.0f;

    float HeaderHeight = 40.0f;

    TArray<float> CardScales;
    TArray<FVector2D> CardPanOffsets;
    TArray<FVector2D> PanStartMousePositions;
    TArray<FVector2D> PanStartOffsets;

    TArray<TSharedPtr<SOverlay>> CardWidgets;
    TArray<TSharedPtr<SSlider>> CardScaleSliders;

    int32 ActivePanCard = INDEX_NONE;

    FPreviewRect GetCardWidgetRect(
        int32 CardIndex,
        const FGeometry& ParentGeometry
    ) const;

    FPreviewRect GetCardViewportRect(
        int32 CardIndex,
        const FGeometry& ParentGeometry
    ) const;

    FPreviewRect CalculateCardImageRect(
        int32 CardIndex,
        const FVector2D& ViewportSize
    ) const;

    FVector2D CalculateCardImageSize(
        int32 CardIndex,
        const FVector2D& ViewportSize
    ) const;

    FVector2D CalculateMaximumAnimatedImageSize(
        int32 CardIndex,
        const FVector2D& ViewportSize
    ) const;

    float CalculateAnimationMaximum(
        int32 CardIndex
    ) const;

    float CalculateBaseScale(
        int32 CardIndex,
        const FVector2D& ViewportSize
    ) const;

    FVector2D GetSourceSize() const;

    FMargin GetEffectiveMargin() const;

    void GetSourceMargins(
        FVector2D& OutTopLeft,
        FVector2D& OutBottomRight
    ) const;

    void BuildSlices(
        int32 CardIndex,
        const FVector2D& ViewportSize,
        TArray<FSliceData, TInlineAllocator<9>>& OutSlices
    ) const;

    void DrawSlice(
        const FSliceData& Slice,
        const FGeometry& AllottedGeometry,
        const FVector2D& ViewportPosition,
        FSlateWindowElementList& OutDrawElements,
        int32 LayerId,
        const FWidgetStyle& InWidgetStyle
    ) const;

    void ClampCardPan(
        int32 CardIndex,
        const FVector2D& ViewportSize
    );

    void SetCardScaleAtPosition(
        int32 CardIndex,
        float NewScale,
        const FVector2D& FocusPosition,
        const FVector2D& ViewportSize
    );

    int32 HitTestCard(
        const FVector2D& LocalPosition,
        const FGeometry& Geometry
    ) const;

    float GetCardScaleSliderValue(
        int32 CardIndex
    ) const;

    void OnCardScaleChanged(
        float NewValue,
        int32 CardIndex
    );

    FText GetCardScaleText(
        int32 CardIndex
    ) const;

    void ResetCardView(
        int32 CardIndex
    );

    void LoadCardScaleSettings();

    void SaveCardScaleSettings(
        int32 CardIndex
    );

    void DrawCardImage(
        int32 CardIndex,
        const FGeometry& AllottedGeometry,
        const FPreviewRect& ViewportRect,
        FSlateWindowElementList& OutDrawElements,
        int32 LayerId,
        const FWidgetStyle& InWidgetStyle
    ) const;
};