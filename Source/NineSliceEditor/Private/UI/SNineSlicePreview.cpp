#include "UI/SNineSlicePreview.h"

#include "Brushes/SlateColorBrush.h"
#include "Input/Events.h"
#include "Layout/Clipping.h"
#include "Misc/ConfigCacheIni.h"
#include "Rendering/DrawElements.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"

namespace
{
const TCHAR* CardScaleConfigKeys[] =
{
    TEXT("PreviewCardScaleHorizontalV10"),
    TEXT("PreviewCardScaleVerticalV10"),
    TEXT("PreviewCardScaleBothV10")
};

const TCHAR* CardTitles[] =
{
    TEXT("Horizontal"),
    TEXT("Vertical"),
    TEXT("Both")
};

constexpr float AnimationViewportRatio = 0.78f;
}

void SNineSlicePreview::Construct(
    const FArguments& InArgs
)
{
    Brush = InArgs._Brush;

    SourceWidth =
        FMath::Max(
            InArgs._SourceWidth,
            1.0f
        );

    SourceHeight =
        FMath::Max(
            InArgs._SourceHeight,
            1.0f
        );

    LeftMargin =
        FMath::Max(
            InArgs._LeftMargin,
            0.0f
        );

    TopMargin =
        FMath::Max(
            InArgs._TopMargin,
            0.0f
        );

    RightMargin =
        FMath::Max(
            InArgs._RightMargin,
            0.0f
        );

    BottomMargin =
        FMath::Max(
            InArgs._BottomMargin,
            0.0f
        );

    AnimationSpeed =
        FMath::Max(
            InArgs._AnimationSpeed,
            0.0f
        ) * 0.28f;

    BaseMaxSize =
        FMath::Max(
            InArgs._BaseMaxSize,
            0.0f
        );

    StretchMaxScale =
        FMath::Max(
            InArgs._StretchMaxScale,
            1.0f
        );

    CardGap =
        FMath::Max(
            InArgs._CardGap,
            0.0f
        );

    CardPadding =
        FMath::Max(
            InArgs._CardPadding,
            0.0f
        );

    LabelHeight =
        FMath::Max(
            InArgs._LabelHeight,
            12.0f
        );

    CardColor = InArgs._CardColor;
    TextColor = InArgs._TextColor;

    CardScaleMin = 1.0f;

    CardScaleMax =
        FMath::Max(
            3.0f,
            StretchMaxScale
        );

    HeaderHeight =
        FMath::Max(
            LabelHeight + 14.0f,
            34.0f
        );

    AnimationPhase = 0.0f;
    AnimationScale = 0.0f;

    CardScales.Init(
        1.0f,
        CardCount
    );

    CardPanOffsets.Init(
        FVector2D::ZeroVector,
        CardCount
    );

    PanStartMousePositions.Init(
        FVector2D::ZeroVector,
        CardCount
    );

    PanStartOffsets.Init(
        FVector2D::ZeroVector,
        CardCount
    );

    LoadCardScaleSettings();

    CardWidgets.Empty();
    CardScaleSliders.Empty();

    const FSlateFontInfo BoldFont =
        FCoreStyle::Get().GetFontStyle(
            TEXT("BoldFont")
        );

    TSharedRef<SVerticalBox> CardList =
        SNew(SVerticalBox);

    for (
        int32 CardIndex = 0;
        CardIndex < CardCount;
        ++CardIndex
    )
    {
        const int32 CurrentCardIndex =
            CardIndex;

        TSharedPtr<SSlider> ScaleSlider;

        TSharedPtr<SOverlay> CardWidget =
            SNew(SOverlay);

        ScaleSlider =
            SNew(SSlider)
            .Value_Lambda(
                [this, CurrentCardIndex]()
                {
                    return GetCardScaleSliderValue(
                        CurrentCardIndex
                    );
                }
            )
            .MinValue(0.0f)
            .MaxValue(1.0f)
            .StepSize(0.01f)
            .SliderBarColor(
                FSlateColor(TextColor)
            )
            .SliderHandleColor(
                FSlateColor(TextColor)
            )
            .OnValueChanged(
                this,
                &SNineSlicePreview::OnCardScaleChanged,
                CurrentCardIndex
            );

        CardScaleSliders.Add(
            ScaleSlider
        );

        TSharedRef<SBox> HeaderBox =
            SNew(SBox)
            .HeightOverride(
                HeaderHeight
            )
            [
                SNew(SHorizontalBox)

                + SHorizontalBox::Slot()
                .FillWidth(1.0f)
                .VAlign(VAlign_Center)
                [
                    SNew(STextBlock)
                    .Text(
                        FText::FromString(
                            CardTitles[CardIndex]
                        )
                    )
                    .Font(
                        BoldFont
                    )
                    .ColorAndOpacity_Lambda(
                        [this]()
                        {
                            return TextColor;
                        }
                    )
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                .Padding(
                    8.0f,
                    0.0f
                )
                [
                    SNew(SBox)
                    .WidthOverride(
                        140.0f
                    )
                    [
                        ScaleSlider.ToSharedRef()
                    ]
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                [
                    SNew(STextBlock)
                    .Font(
                        BoldFont
                    )
                    .MinDesiredWidth(
                        44.0f
                    )
                    .Justification(
                        ETextJustify::Right
                    )
                    .Text(
                        this,
                        &SNineSlicePreview::GetCardScaleText,
                        CurrentCardIndex
                    )
                    .ColorAndOpacity_Lambda(
                        [this]()
                        {
                            return TextColor;
                        }
                    )
                ]
            ];

        CardWidget->AddSlot()
        .HAlign(HAlign_Fill)
        .VAlign(VAlign_Top)
        .Padding(CardPadding)
        [
            HeaderBox
        ];

        CardWidgets.Add(
            CardWidget
        );

        CardList->AddSlot()
        .FillHeight(1.0f)
        .Padding(
            0.0f,
            CardIndex == 0
                ? 0.0f
                : CardGap,
            0.0f,
            0.0f
        )
        [
            CardWidget.ToSharedRef()
        ];
    }

    ChildSlot[
        CardList
    ];
}

void SNineSlicePreview::SetBrush(
    const FSlateBrush& InBrush,
    const float InSourceWidth,
    const float InSourceHeight
)
{
    Brush = InBrush;

    SourceWidth =
        FMath::Max(
            InSourceWidth,
            1.0f
        );

    SourceHeight =
        FMath::Max(
            InSourceHeight,
            1.0f
        );

    Invalidate(
        EInvalidateWidgetReason::Paint
    );
}

void SNineSlicePreview::SetMargins(
    const float InLeft,
    const float InTop,
    const float InRight,
    const float InBottom
)
{
    LeftMargin =
        FMath::Max(
            InLeft,
            0.0f
        );

    TopMargin =
        FMath::Max(
            InTop,
            0.0f
        );

    RightMargin =
        FMath::Max(
            InRight,
            0.0f
        );

    BottomMargin =
        FMath::Max(
            InBottom,
            0.0f
        );

    Invalidate(
        EInvalidateWidgetReason::Paint
    );
}

void SNineSlicePreview::SetColors(
    const FLinearColor& InCardColor,
    const FLinearColor& InTextColor
)
{
    CardColor = InCardColor;
    TextColor = InTextColor;

    const FSlateColor SliderColor(
        TextColor
    );

    for (
        const TSharedPtr<SSlider>& Slider :
        CardScaleSliders
    )
    {
        if (!Slider.IsValid())
        {
            continue;
        }

        Slider->SetSliderBarColor(
            SliderColor
        );

        Slider->SetSliderHandleColor(
            SliderColor
        );

        Slider->Invalidate(
            EInvalidateWidgetReason::Paint
        );
    }

    Invalidate(
        EInvalidateWidgetReason::Paint
    );
}

FVector2D SNineSlicePreview::ComputeDesiredSize(
    const float LayoutScaleMultiplier
) const
{
    return FVector2D(
        420.0f,
        560.0f
    );
}

void SNineSlicePreview::Tick(
    const FGeometry& AllottedGeometry,
    const double InCurrentTime,
    const float InDeltaTime
)
{
    if (AnimationSpeed > 0.0f)
    {
        AnimationPhase +=
            InDeltaTime *
            AnimationSpeed;

        AnimationPhase =
            FMath::Fmod(
                AnimationPhase,
                1.0f
            );

        AnimationScale =
            0.5f -
            0.5f *
            FMath::Cos(
                AnimationPhase *
                2.0f *
                PI
            );
    }
    else
    {
        AnimationScale = 0.0f;
    }

    for (
        int32 CardIndex = 0;
        CardIndex < CardCount;
        ++CardIndex
    )
    {
        const FPreviewRect ViewportRect =
            GetCardViewportRect(
                CardIndex,
                AllottedGeometry
            );

        ClampCardPan(
            CardIndex,
            ViewportRect.Size
        );
    }

    Invalidate(
        EInvalidateWidgetReason::Paint
    );
}

int32 SNineSlicePreview::OnPaint(
    const FPaintArgs& Args,
    const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect,
    FSlateWindowElementList& OutDrawElements,
    const int32 LayerId,
    const FWidgetStyle& InWidgetStyle,
    const bool bParentEnabled
) const
{
    int32 CurrentLayer =
        LayerId;

    const FSlateColorBrush CardBackgroundBrush(
        CardColor
    );

    for (
        int32 CardIndex = 0;
        CardIndex < CardCount;
        ++CardIndex
    )
    {
        const FPreviewRect CardRect =
            GetCardWidgetRect(
                CardIndex,
                AllottedGeometry
            );

        const FPaintGeometry CardGeometry =
            AllottedGeometry.ToPaintGeometry(
                CardRect.Size,
                FSlateLayoutTransform(
                    CardRect.Position
                )
            );

        FSlateDrawElement::MakeBox(
            OutDrawElements,
            CurrentLayer,
            CardGeometry,
            &CardBackgroundBrush,
            ESlateDrawEffect::None,
            CardColor
        );

        const FPreviewRect ViewportRect =
            GetCardViewportRect(
                CardIndex,
                AllottedGeometry
            );

        const FVector2D AbsoluteTopLeft =
            AllottedGeometry.LocalToAbsolute(
                ViewportRect.Position
            );

        const FVector2D AbsoluteBottomRight =
            AllottedGeometry.LocalToAbsolute(
                ViewportRect.Position +
                ViewportRect.Size
            );

        const FSlateRect AbsoluteViewportRect(
            AbsoluteTopLeft.X,
            AbsoluteTopLeft.Y,
            AbsoluteBottomRight.X,
            AbsoluteBottomRight.Y
        );

        OutDrawElements.PushClip(
            FSlateClippingZone(
                AbsoluteViewportRect
            )
        );

        DrawCardImage(
            CardIndex,
            AllottedGeometry,
            ViewportRect,
            OutDrawElements,
            CurrentLayer + 1,
            InWidgetStyle
        );

        OutDrawElements.PopClip();

        CurrentLayer += 10;
    }

    return SCompoundWidget::OnPaint(
        Args,
        AllottedGeometry,
        MyCullingRect,
        OutDrawElements,
        CurrentLayer,
        InWidgetStyle,
        bParentEnabled
    );
}

FReply SNineSlicePreview::OnPreviewMouseButtonDown(
    const FGeometry& MyGeometry,
    const FPointerEvent& MouseEvent
)
{
    if (
        MouseEvent.GetEffectingButton() !=
        EKeys::RightMouseButton
    )
    {
        return SCompoundWidget::OnPreviewMouseButtonDown(
            MyGeometry,
            MouseEvent
        );
    }

    const FVector2D LocalPosition =
        MyGeometry.AbsoluteToLocal(
            MouseEvent.GetScreenSpacePosition()
        );

    const int32 CardIndex =
        HitTestCard(
            LocalPosition,
            MyGeometry
        );

    if (CardIndex == INDEX_NONE)
    {
        return SCompoundWidget::OnPreviewMouseButtonDown(
            MyGeometry,
            MouseEvent
        );
    }

    const FPreviewRect ViewportRect =
        GetCardViewportRect(
            CardIndex,
            MyGeometry
        );

    const FBox2D ViewportBounds(
        ViewportRect.Position,
        ViewportRect.Position +
        ViewportRect.Size
    );

    if (
        !ViewportBounds.IsInside(
            LocalPosition
        )
    )
    {
        return SCompoundWidget::OnPreviewMouseButtonDown(
            MyGeometry,
            MouseEvent
        );
    }

    ActivePanCard =
        CardIndex;

    PanStartMousePositions[
        CardIndex
    ] =
        LocalPosition;

    PanStartOffsets[
        CardIndex
    ] =
        CardPanOffsets[
            CardIndex
        ];

    return FReply::Handled()
        .CaptureMouse(
            AsShared()
        );
}

FReply SNineSlicePreview::OnMouseButtonDown(
    const FGeometry& MyGeometry,
    const FPointerEvent& MouseEvent
)
{
    if (
        MouseEvent.GetEffectingButton() !=
        EKeys::RightMouseButton
    )
    {
        return SCompoundWidget::OnMouseButtonDown(
            MyGeometry,
            MouseEvent
        );
    }

    if (ActivePanCard != INDEX_NONE)
    {
        return FReply::Handled();
    }

    const FVector2D LocalPosition =
        MyGeometry.AbsoluteToLocal(
            MouseEvent.GetScreenSpacePosition()
        );

    const int32 CardIndex =
        HitTestCard(
            LocalPosition,
            MyGeometry
        );

    if (CardIndex == INDEX_NONE)
    {
        return SCompoundWidget::OnMouseButtonDown(
            MyGeometry,
            MouseEvent
        );
    }

    const FPreviewRect ViewportRect =
        GetCardViewportRect(
            CardIndex,
            MyGeometry
        );

    const FBox2D ViewportBounds(
        ViewportRect.Position,
        ViewportRect.Position +
        ViewportRect.Size
    );

    if (
        !ViewportBounds.IsInside(
            LocalPosition
        )
    )
    {
        return SCompoundWidget::OnMouseButtonDown(
            MyGeometry,
            MouseEvent
        );
    }

    ActivePanCard =
        CardIndex;

    PanStartMousePositions[
        CardIndex
    ] =
        LocalPosition;

    PanStartOffsets[
        CardIndex
    ] =
        CardPanOffsets[
            CardIndex
        ];

    return FReply::Handled()
        .CaptureMouse(
            AsShared()
        );
}

FReply SNineSlicePreview::OnMouseButtonUp(
    const FGeometry& MyGeometry,
    const FPointerEvent& MouseEvent
)
{
    if (
        MouseEvent.GetEffectingButton() !=
        EKeys::RightMouseButton
    )
    {
        return SCompoundWidget::OnMouseButtonUp(
            MyGeometry,
            MouseEvent
        );
    }

    if (ActivePanCard != INDEX_NONE)
    {
        ActivePanCard =
            INDEX_NONE;

        return FReply::Handled()
            .ReleaseMouseCapture();
    }

    return SCompoundWidget::OnMouseButtonUp(
        MyGeometry,
        MouseEvent
    );
}

FReply SNineSlicePreview::OnMouseMove(
    const FGeometry& MyGeometry,
    const FPointerEvent& MouseEvent
)
{
    if (
        ActivePanCard == INDEX_NONE ||
        !PanStartMousePositions.IsValidIndex(
            ActivePanCard
        ) ||
        !PanStartOffsets.IsValidIndex(
            ActivePanCard
        )
    )
    {
        return SCompoundWidget::OnMouseMove(
            MyGeometry,
            MouseEvent
        );
    }

    const FVector2D LocalPosition =
        MyGeometry.AbsoluteToLocal(
            MouseEvent.GetScreenSpacePosition()
        );

    const FVector2D Delta =
        LocalPosition -
        PanStartMousePositions[
            ActivePanCard
        ];

    CardPanOffsets[
        ActivePanCard
    ] =
        PanStartOffsets[
            ActivePanCard
        ] +
        Delta;

    const FPreviewRect ViewportRect =
        GetCardViewportRect(
            ActivePanCard,
            MyGeometry
        );

    ClampCardPan(
        ActivePanCard,
        ViewportRect.Size
    );

    Invalidate(
        EInvalidateWidgetReason::Paint
    );

    return FReply::Handled();
}

FReply SNineSlicePreview::OnMouseWheel(
    const FGeometry& MyGeometry,
    const FPointerEvent& MouseEvent
)
{
    const FVector2D LocalPosition =
        MyGeometry.AbsoluteToLocal(
            MouseEvent.GetScreenSpacePosition()
        );

    const int32 CardIndex =
        HitTestCard(
            LocalPosition,
            MyGeometry
        );

    if (
        !CardScales.IsValidIndex(
            CardIndex
        )
    )
    {
        return SCompoundWidget::OnMouseWheel(
            MyGeometry,
            MouseEvent
        );
    }

    const FPreviewRect ViewportRect =
        GetCardViewportRect(
            CardIndex,
            MyGeometry
        );

    const FBox2D ViewportBounds(
        ViewportRect.Position,
        ViewportRect.Position +
        ViewportRect.Size
    );

    if (
        !ViewportBounds.IsInside(
            LocalPosition
        )
    )
    {
        return SCompoundWidget::OnMouseWheel(
            MyGeometry,
            MouseEvent
        );
    }

    const float WheelDelta =
        MouseEvent.GetWheelDelta();

    const float ScaleMultiplier =
        WheelDelta > 0.0f
            ? 1.15f
            : 1.0f / 1.15f;

    const float CurrentScale =
        CardScales[
            CardIndex
        ];

    const float NewScale =
        FMath::Clamp(
            CurrentScale *
            ScaleMultiplier,
            CardScaleMin,
            CardScaleMax
        );

    SetCardScaleAtPosition(
        CardIndex,
        NewScale,
        LocalPosition -
        ViewportRect.Position,
        ViewportRect.Size
    );

    return FReply::Handled();
}

TOptional<EMouseCursor::Type>
SNineSlicePreview::GetCursor() const
{
    if (ActivePanCard != INDEX_NONE)
    {
        return EMouseCursor::GrabHand;
    }

    return EMouseCursor::Default;
}

SNineSlicePreview::FPreviewRect
SNineSlicePreview::GetCardWidgetRect(
    const int32 CardIndex,
    const FGeometry& ParentGeometry
) const
{
    FPreviewRect Result;

    if (
        CardIndex < 0 ||
        CardIndex >= CardCount
    )
    {
        return Result;
    }

    const FVector2D ParentSize =
        ParentGeometry.GetLocalSize();

    const float TotalGap =
        CardGap *
        static_cast<float>(
            CardCount - 1
        );

    const float CardHeight =
        FMath::Max(
            (
                ParentSize.Y -
                TotalGap
            ) /
            static_cast<float>(
                CardCount
            ),
            0.0f
        );

    Result.Position =
        FVector2D(
            0.0f,
            (
                CardHeight +
                CardGap
            ) *
            static_cast<float>(
                CardIndex
            )
        );

    Result.Size =
        FVector2D(
            ParentSize.X,
            CardHeight
        );

    return Result;
}

SNineSlicePreview::FPreviewRect
SNineSlicePreview::GetCardViewportRect(
    const int32 CardIndex,
    const FGeometry& ParentGeometry
) const
{
    const FPreviewRect CardRect =
        GetCardWidgetRect(
            CardIndex,
            ParentGeometry
        );

    FPreviewRect Result;

    Result.Position =
        CardRect.Position +
        FVector2D(
            CardPadding,
            HeaderHeight +
            CardPadding
        );

    Result.Size =
        FVector2D(
            FMath::Max(
                CardRect.Size.X -
                CardPadding * 2.0f,
                0.0f
            ),

            FMath::Max(
                CardRect.Size.Y -
                HeaderHeight -
                CardPadding * 2.0f,
                0.0f
            )
        );

    return Result;
}

FVector2D SNineSlicePreview::GetSourceSize() const
{
    return FVector2D(
        FMath::Max(
            SourceWidth,
            1.0f
        ),
        FMath::Max(
            SourceHeight,
            1.0f
        )
    );
}

FMargin SNineSlicePreview::GetEffectiveMargin() const
{
    const FVector2D SourceSize =
        GetSourceSize();

    float Left =
        LeftMargin;

    float Top =
        TopMargin;

    float Right =
        RightMargin;

    float Bottom =
        BottomMargin;

    if (
        Left > 1.0f ||
        Top > 1.0f ||
        Right > 1.0f ||
        Bottom > 1.0f
    )
    {
        Left /=
            SourceSize.X;

        Right /=
            SourceSize.X;

        Top /=
            SourceSize.Y;

        Bottom /=
            SourceSize.Y;
    }

    Left =
        FMath::Clamp(
            Left,
            0.0f,
            1.0f
        );

    Top =
        FMath::Clamp(
            Top,
            0.0f,
            1.0f
        );

    Right =
        FMath::Clamp(
            Right,
            0.0f,
            1.0f
        );

    Bottom =
        FMath::Clamp(
            Bottom,
            0.0f,
            1.0f
        );

    if (Left + Right >= 1.0f)
    {
        const float Sum =
            Left +
            Right;

        Left =
            Left /
            Sum *
            0.98f;

        Right =
            Right /
            Sum *
            0.98f;
    }

    if (Top + Bottom >= 1.0f)
    {
        const float Sum =
            Top +
            Bottom;

        Top =
            Top /
            Sum *
            0.98f;

        Bottom =
            Bottom /
            Sum *
            0.98f;
    }

    return FMargin(
        Left,
        Top,
        Right,
        Bottom
    );
}

void SNineSlicePreview::GetSourceMargins(
    FVector2D& OutTopLeft,
    FVector2D& OutBottomRight
) const
{
    const FVector2D SourceSize =
        GetSourceSize();

    const FMargin Margin =
        GetEffectiveMargin();

    OutTopLeft =
        FVector2D(
            SourceSize.X *
            Margin.Left,

            SourceSize.Y *
            Margin.Top
        );

    OutBottomRight =
        FVector2D(
            SourceSize.X *
            Margin.Right,

            SourceSize.Y *
            Margin.Bottom
        );
}

float SNineSlicePreview::CalculateAnimationMaximum(
    const int32 CardIndex
) const
{
    return FMath::Max(
        StretchMaxScale,
        1.0f
    );
}

float SNineSlicePreview::CalculateBaseScale(
    const int32 CardIndex,
    const FVector2D& ViewportSize
) const
{
    if (
        ViewportSize.X <= 0.0f ||
        ViewportSize.Y <= 0.0f
    )
    {
        return 1.0f;
    }

    const FVector2D SourceSize =
        GetSourceSize();

    FVector2D TopLeft;
    FVector2D BottomRight;

    GetSourceMargins(
        TopLeft,
        BottomRight
    );

    const float CenterWidth =
        FMath::Max(
            SourceSize.X -
            TopLeft.X -
            BottomRight.X,
            1.0f
        );

    const float CenterHeight =
        FMath::Max(
            SourceSize.Y -
            TopLeft.Y -
            BottomRight.Y,
            1.0f
        );

    const float MaximumCenterScale =
        CalculateAnimationMaximum(
            CardIndex
        );

    const EStretchMode StretchMode =
        CardIndex == 0
            ? EStretchMode::Horizontal
            : CardIndex == 1
                ? EStretchMode::Vertical
                : EStretchMode::Both;

    float MaximumWidth =
        SourceSize.X;

    float MaximumHeight =
        SourceSize.Y;

    if (
        StretchMode ==
            EStretchMode::Horizontal ||
        StretchMode ==
            EStretchMode::Both
    )
    {
        MaximumWidth =
            TopLeft.X +
            BottomRight.X +
            CenterWidth *
            MaximumCenterScale;
    }

    if (
        StretchMode ==
            EStretchMode::Vertical ||
        StretchMode ==
            EStretchMode::Both
    )
    {
        MaximumHeight =
            TopLeft.Y +
            BottomRight.Y +
            CenterHeight *
            MaximumCenterScale;
    }

    const float AvailableWidth =
        ViewportSize.X *
        AnimationViewportRatio;

    const float AvailableHeight =
        ViewportSize.Y *
        AnimationViewportRatio;

    float BaseScale =
        FMath::Min(
            AvailableWidth /
                FMath::Max(
                    MaximumWidth,
                    1.0f
                ),

            AvailableHeight /
                FMath::Max(
                    MaximumHeight,
                    1.0f
                )
        );

    if (BaseMaxSize > 0.0f)
    {
        const float NaturalMaximumDimension =
            FMath::Max(
                SourceSize.X,
                SourceSize.Y
            );

        BaseScale =
            FMath::Min(
                BaseScale,
                BaseMaxSize /
                FMath::Max(
                    NaturalMaximumDimension,
                    1.0f
                )
            );
    }

    return FMath::Max(
        BaseScale,
        KINDA_SMALL_NUMBER
    );
}

FVector2D SNineSlicePreview::CalculateMaximumAnimatedImageSize(
    const int32 CardIndex,
    const FVector2D& ViewportSize
) const
{
    if (
        ViewportSize.X <= 0.0f ||
        ViewportSize.Y <= 0.0f
    )
    {
        return FVector2D::ZeroVector;
    }

    const FVector2D SourceSize =
        GetSourceSize();

    FVector2D TopLeft;
    FVector2D BottomRight;

    GetSourceMargins(
        TopLeft,
        BottomRight
    );

    const float CenterWidth =
        FMath::Max(
            SourceSize.X -
            TopLeft.X -
            BottomRight.X,
            1.0f
        );

    const float CenterHeight =
        FMath::Max(
            SourceSize.Y -
            TopLeft.Y -
            BottomRight.Y,
            1.0f
        );

    const float BaseScale =
        CalculateBaseScale(
            CardIndex,
            ViewportSize
        );

    const float MaximumCenterScale =
        CalculateAnimationMaximum(
            CardIndex
        );

    const EStretchMode StretchMode =
        CardIndex == 0
            ? EStretchMode::Horizontal
            : CardIndex == 1
                ? EStretchMode::Vertical
                : EStretchMode::Both;

    float Width =
        SourceSize.X;

    float Height =
        SourceSize.Y;

    if (
        StretchMode ==
            EStretchMode::Horizontal ||
        StretchMode ==
            EStretchMode::Both
    )
    {
        Width =
            TopLeft.X +
            BottomRight.X +
            CenterWidth *
            MaximumCenterScale;
    }

    if (
        StretchMode ==
            EStretchMode::Vertical ||
        StretchMode ==
            EStretchMode::Both
    )
    {
        Height =
            TopLeft.Y +
            BottomRight.Y +
            CenterHeight *
            MaximumCenterScale;
    }

    const float UserScale =
        CardScales.IsValidIndex(
            CardIndex
        )
            ? FMath::Clamp(
                CardScales[
                    CardIndex
                ],
                CardScaleMin,
                CardScaleMax
            )
            : CardScaleMin;

    return FVector2D(
        Width *
        BaseScale *
        UserScale,

        Height *
        BaseScale *
        UserScale
    );
}

FVector2D SNineSlicePreview::CalculateCardImageSize(
    const int32 CardIndex,
    const FVector2D& ViewportSize
) const
{
    if (
        ViewportSize.X <= 0.0f ||
        ViewportSize.Y <= 0.0f
    )
    {
        return FVector2D::ZeroVector;
    }

    const FVector2D SourceSize =
        GetSourceSize();

    FVector2D TopLeft;
    FVector2D BottomRight;

    GetSourceMargins(
        TopLeft,
        BottomRight
    );

    const float CenterWidth =
        FMath::Max(
            SourceSize.X -
            TopLeft.X -
            BottomRight.X,
            1.0f
        );

    const float CenterHeight =
        FMath::Max(
            SourceSize.Y -
            TopLeft.Y -
            BottomRight.Y,
            1.0f
        );

    const float BaseScale =
        CalculateBaseScale(
            CardIndex,
            ViewportSize
        );

    const float MaximumCenterScale =
        CalculateAnimationMaximum(
            CardIndex
        );

    const float AnimatedCenterScale =
        FMath::Lerp(
            1.0f,
            MaximumCenterScale,
            AnimationScale
        );

    const EStretchMode StretchMode =
        CardIndex == 0
            ? EStretchMode::Horizontal
            : CardIndex == 1
                ? EStretchMode::Vertical
                : EStretchMode::Both;

    float Width =
        SourceSize.X;

    float Height =
        SourceSize.Y;

    if (
        StretchMode ==
            EStretchMode::Horizontal ||
        StretchMode ==
            EStretchMode::Both
    )
    {
        Width =
            TopLeft.X +
            BottomRight.X +
            CenterWidth *
            AnimatedCenterScale;
    }

    if (
        StretchMode ==
            EStretchMode::Vertical ||
        StretchMode ==
            EStretchMode::Both
    )
    {
        Height =
            TopLeft.Y +
            BottomRight.Y +
            CenterHeight *
            AnimatedCenterScale;
    }

    const float UserScale =
        CardScales.IsValidIndex(
            CardIndex
        )
            ? FMath::Clamp(
                CardScales[
                    CardIndex
                ],
                CardScaleMin,
                CardScaleMax
            )
            : CardScaleMin;

    return FVector2D(
        Width *
        BaseScale *
        UserScale,

        Height *
        BaseScale *
        UserScale
    );
}

SNineSlicePreview::FPreviewRect
SNineSlicePreview::CalculateCardImageRect(
    const int32 CardIndex,
    const FVector2D& ViewportSize
) const
{
    FPreviewRect Result;

    const FVector2D ImageSize =
        CalculateCardImageSize(
            CardIndex,
            ViewportSize
        );

    const FVector2D ViewportCenter =
        ViewportSize *
        0.5f;

    Result.Size =
        ImageSize;

    Result.Position =
        ViewportCenter -
        ImageSize *
        0.5f;

    if (
        CardPanOffsets.IsValidIndex(
            CardIndex
        )
    )
    {
        Result.Position +=
            CardPanOffsets[
                CardIndex
            ];
    }

    return Result;
}

void SNineSlicePreview::BuildSlices(
    const int32 CardIndex,
    const FVector2D& ViewportSize,
    TArray<FSliceData, TInlineAllocator<9>>& OutSlices
) const
{
    OutSlices.Reset();

    if (
        ViewportSize.X <= 0.0f ||
        ViewportSize.Y <= 0.0f
    )
    {
        return;
    }

    const FVector2D SourceSize =
        GetSourceSize();

    FVector2D SourceTopLeft;
    FVector2D SourceBottomRight;

    GetSourceMargins(
        SourceTopLeft,
        SourceBottomRight
    );

    const float SourceLeft =
        SourceTopLeft.X;

    const float SourceTop =
        SourceTopLeft.Y;

    const float SourceRight =
        SourceBottomRight.X;

    const float SourceBottom =
        SourceBottomRight.Y;

    const float CenterSourceWidth =
        FMath::Max(
            SourceSize.X -
            SourceLeft -
            SourceRight,
            1.0f
        );

    const float CenterSourceHeight =
        FMath::Max(
            SourceSize.Y -
            SourceTop -
            SourceBottom,
            1.0f
        );

    const float BaseScale =
        CalculateBaseScale(
            CardIndex,
            ViewportSize
        );

    const float UserScale =
        CardScales.IsValidIndex(
            CardIndex
        )
            ? FMath::Clamp(
                CardScales[
                    CardIndex
                ],
                CardScaleMin,
                CardScaleMax
            )
            : CardScaleMin;

    const float FixedLeft =
        SourceLeft *
        BaseScale *
        UserScale;

    const float FixedRight =
        SourceRight *
        BaseScale *
        UserScale;

    const float FixedTop =
        SourceTop *
        BaseScale *
        UserScale;

    const float FixedBottom =
        SourceBottom *
        BaseScale *
        UserScale;

    const float MaximumCenterScale =
        CalculateAnimationMaximum(
            CardIndex
        );

    const float AnimatedCenterScale =
        FMath::Lerp(
            1.0f,
            MaximumCenterScale,
            AnimationScale
        );

    const EStretchMode StretchMode =
        CardIndex == 0
            ? EStretchMode::Horizontal
            : CardIndex == 1
                ? EStretchMode::Vertical
                : EStretchMode::Both;

    float CenterWidth =
        CenterSourceWidth *
        BaseScale *
        UserScale;

    float CenterHeight =
        CenterSourceHeight *
        BaseScale *
        UserScale;

    if (
        StretchMode ==
            EStretchMode::Horizontal ||
        StretchMode ==
            EStretchMode::Both
    )
    {
        CenterWidth *=
            AnimatedCenterScale;
    }

    if (
        StretchMode ==
            EStretchMode::Vertical ||
        StretchMode ==
            EStretchMode::Both
    )
    {
        CenterHeight *=
            AnimatedCenterScale;
    }

    const float TotalWidth =
        FixedLeft +
        CenterWidth +
        FixedRight;

    const float TotalHeight =
        FixedTop +
        CenterHeight +
        FixedBottom;

    const FVector2D PanOffset =
        CardPanOffsets.IsValidIndex(
            CardIndex
        )
            ? CardPanOffsets[
                CardIndex
            ]
            : FVector2D::ZeroVector;

    const FVector2D Origin =
        (
            ViewportSize -
            FVector2D(
                TotalWidth,
                TotalHeight
            )
        ) *
        0.5f +
        PanOffset;

    const float U0 = 0.0f;

    const float U1 =
        SourceLeft /
        SourceSize.X;

    const float U2 =
        (
            SourceLeft +
            CenterSourceWidth
        ) /
        SourceSize.X;

    const float U3 = 1.0f;

    const float V0 = 0.0f;

    const float V1 =
        SourceTop /
        SourceSize.Y;

    const float V2 =
        (
            SourceTop +
            CenterSourceHeight
        ) /
        SourceSize.Y;

    const float V3 = 1.0f;

    FSliceData Slice;

    Slice.Position =
        Origin;

    Slice.Size =
        FVector2D(
            FixedLeft,
            FixedTop
        );

    Slice.UV =
        FBox2D(
            FVector2D(
                U0,
                V0
            ),
            FVector2D(
                U1,
                V1
            )
        );

    OutSlices.Add(
        Slice
    );

    Slice.Position =
        Origin +
        FVector2D(
            FixedLeft,
            0.0f
        );

    Slice.Size =
        FVector2D(
            CenterWidth,
            FixedTop
        );

    Slice.UV =
        FBox2D(
            FVector2D(
                U1,
                V0
            ),
            FVector2D(
                U2,
                V1
            )
        );

    OutSlices.Add(
        Slice
    );

    Slice.Position =
        Origin +
        FVector2D(
            FixedLeft +
            CenterWidth,
            0.0f
        );

    Slice.Size =
        FVector2D(
            FixedRight,
            FixedTop
        );

    Slice.UV =
        FBox2D(
            FVector2D(
                U2,
                V0
            ),
            FVector2D(
                U3,
                V1
            )
        );

    OutSlices.Add(
        Slice
    );

    Slice.Position =
        Origin +
        FVector2D(
            0.0f,
            FixedTop
        );

    Slice.Size =
        FVector2D(
            FixedLeft,
            CenterHeight
        );

    Slice.UV =
        FBox2D(
            FVector2D(
                U0,
                V1
            ),
            FVector2D(
                U1,
                V2
            )
        );

    OutSlices.Add(
        Slice
    );

    Slice.Position =
        Origin +
        FVector2D(
            FixedLeft,
            FixedTop
        );

    Slice.Size =
        FVector2D(
            CenterWidth,
            CenterHeight
        );

    Slice.UV =
        FBox2D(
            FVector2D(
                U1,
                V1
            ),
            FVector2D(
                U2,
                V2
            )
        );

    OutSlices.Add(
        Slice
    );

    Slice.Position =
        Origin +
        FVector2D(
            FixedLeft +
            CenterWidth,
            FixedTop
        );

    Slice.Size =
        FVector2D(
            FixedRight,
            CenterHeight
        );

    Slice.UV =
        FBox2D(
            FVector2D(
                U2,
                V1
            ),
            FVector2D(
                U3,
                V2
            )
        );

    OutSlices.Add(
        Slice
    );

    Slice.Position =
        Origin +
        FVector2D(
            0.0f,
            FixedTop +
            CenterHeight
        );

    Slice.Size =
        FVector2D(
            FixedLeft,
            FixedBottom
        );

    Slice.UV =
        FBox2D(
            FVector2D(
                U0,
                V2
            ),
            FVector2D(
                U1,
                V3
            )
        );

    OutSlices.Add(
        Slice
    );

    Slice.Position =
        Origin +
        FVector2D(
            FixedLeft,
            FixedTop +
            CenterHeight
        );

    Slice.Size =
        FVector2D(
            CenterWidth,
            FixedBottom
        );

    Slice.UV =
        FBox2D(
            FVector2D(
                U1,
                V2
            ),
            FVector2D(
                U2,
                V3
            )
        );

    OutSlices.Add(
        Slice
    );

    Slice.Position =
        Origin +
        FVector2D(
            FixedLeft +
            CenterWidth,
            FixedTop +
            CenterHeight
        );

    Slice.Size =
        FVector2D(
            FixedRight,
            FixedBottom
        );

    Slice.UV =
        FBox2D(
            FVector2D(
                U2,
                V2
            ),
            FVector2D(
                U3,
                V3
            )
        );

    OutSlices.Add(
        Slice
    );
}

void SNineSlicePreview::DrawSlice(
    const FSliceData& Slice,
    const FGeometry& AllottedGeometry,
    const FVector2D& ViewportPosition,
    FSlateWindowElementList& OutDrawElements,
    const int32 LayerId,
    const FWidgetStyle& InWidgetStyle
) const
{
    if (
        Slice.Size.X <= 0.0f ||
        Slice.Size.Y <= 0.0f
    )
    {
        return;
    }

    FSlateBrush SliceBrush =
        Brush;

    SliceBrush.DrawAs =
        ESlateBrushDrawType::Image;

    SliceBrush.Margin =
        FMargin(0.0f);

    SliceBrush.ImageSize =
        Slice.Size;

    SliceBrush.SetUVRegion(
        Slice.UV
    );

    const FPaintGeometry PaintGeometry =
        AllottedGeometry.ToPaintGeometry(
            Slice.Size,
            FSlateLayoutTransform(
                ViewportPosition +
                Slice.Position
            )
        );

    FSlateDrawElement::MakeBox(
        OutDrawElements,
        LayerId,
        PaintGeometry,
        &SliceBrush,
        ESlateDrawEffect::None,
        SliceBrush.GetTint(
            InWidgetStyle
        )
    );
}

void SNineSlicePreview::DrawCardImage(
    const int32 CardIndex,
    const FGeometry& AllottedGeometry,
    const FPreviewRect& ViewportRect,
    FSlateWindowElementList& OutDrawElements,
    const int32 LayerId,
    const FWidgetStyle& InWidgetStyle
) const
{
    if (
        ViewportRect.Size.X <= 0.0f ||
        ViewportRect.Size.Y <= 0.0f
    )
    {
        return;
    }

    if (
        Brush.GetResourceName().IsNone() &&
        Brush.GetResourceObject() == nullptr
    )
    {
        return;
    }

    TArray<FSliceData, TInlineAllocator<9>> Slices;

    BuildSlices(
        CardIndex,
        ViewportRect.Size,
        Slices
    );

    for (
        const FSliceData& Slice :
        Slices
    )
    {
        DrawSlice(
            Slice,
            AllottedGeometry,
            ViewportRect.Position,
            OutDrawElements,
            LayerId,
            InWidgetStyle
        );
    }
}

void SNineSlicePreview::ClampCardPan(
    const int32 CardIndex,
    const FVector2D& ViewportSize
)
{
    if (
        !CardPanOffsets.IsValidIndex(
            CardIndex
        )
    )
    {
        return;
    }

    if (
        ViewportSize.X <= 0.0f ||
        ViewportSize.Y <= 0.0f
    )
    {
        return;
    }

    const FVector2D MaximumImageSize =
        CalculateMaximumAnimatedImageSize(
            CardIndex,
            ViewportSize
        );

    FVector2D& Pan =
        CardPanOffsets[
            CardIndex
        ];

    if (
        MaximumImageSize.X > ViewportSize.X
    )
    {
        const float MaxPanX =
            (
                MaximumImageSize.X -
                ViewportSize.X
            ) *
            0.5f;

        Pan.X =
            FMath::Clamp(
                Pan.X,
                -MaxPanX,
                MaxPanX
            );
    }

    if (
        MaximumImageSize.Y > ViewportSize.Y
    )
    {
        const float MaxPanY =
            (
                MaximumImageSize.Y -
                ViewportSize.Y
            ) *
            0.5f;

        Pan.Y =
            FMath::Clamp(
                Pan.Y,
                -MaxPanY,
                MaxPanY
            );
    }
}

void SNineSlicePreview::SetCardScaleAtPosition(
    const int32 CardIndex,
    const float NewScale,
    const FVector2D& FocusPosition,
    const FVector2D& ViewportSize
)
{
    if (
        !CardScales.IsValidIndex(
            CardIndex
        ) ||
        ViewportSize.X <= 0.0f ||
        ViewportSize.Y <= 0.0f
    )
    {
        return;
    }

    const FPreviewRect OldImageRect =
        CalculateCardImageRect(
            CardIndex,
            ViewportSize
        );

    const FVector2D OldCenter =
        OldImageRect.Position +
        OldImageRect.Size *
        0.5f;

    const FVector2D OldRelative =
        FocusPosition -
        OldCenter;

    const FVector2D NormalizedRelative(
        OldRelative.X /
            FMath::Max(
                OldImageRect.Size.X,
                KINDA_SMALL_NUMBER
            ),

        OldRelative.Y /
            FMath::Max(
                OldImageRect.Size.Y,
                KINDA_SMALL_NUMBER
            )
    );

    CardScales[
        CardIndex
    ] =
        FMath::Clamp(
            NewScale,
            CardScaleMin,
            CardScaleMax
        );

    const FVector2D NewImageSize =
        CalculateCardImageSize(
            CardIndex,
            ViewportSize
        );

    const FVector2D NewCenter =
        FocusPosition -
        FVector2D(
            NormalizedRelative.X *
            NewImageSize.X,

            NormalizedRelative.Y *
            NewImageSize.Y
        );

    CardPanOffsets[
        CardIndex
    ] =
        NewCenter -
        ViewportSize *
        0.5f;

    ClampCardPan(
        CardIndex,
        ViewportSize
    );

    SaveCardScaleSettings(
        CardIndex
    );

    Invalidate(
        EInvalidateWidgetReason::Paint
    );
}

int32 SNineSlicePreview::HitTestCard(
    const FVector2D& LocalPosition,
    const FGeometry& Geometry
) const
{
    for (
        int32 CardIndex = 0;
        CardIndex < CardCount;
        ++CardIndex
    )
    {
        const FPreviewRect CardRect =
            GetCardWidgetRect(
                CardIndex,
                Geometry
            );

        if (
            FBox2D(
                CardRect.Position,
                CardRect.Position +
                CardRect.Size
            ).IsInside(
                LocalPosition
            )
        )
        {
            return CardIndex;
        }
    }

    return INDEX_NONE;
}

float SNineSlicePreview::GetCardScaleSliderValue(
    const int32 CardIndex
) const
{
    if (
        !CardScales.IsValidIndex(
            CardIndex
        )
    )
    {
        return 0.0f;
    }

    const float Range =
        CardScaleMax -
        CardScaleMin;

    if (
        Range <=
        KINDA_SMALL_NUMBER
    )
    {
        return 0.0f;
    }

    return FMath::Clamp(
        (
            CardScales[
                CardIndex
            ] -
            CardScaleMin
        ) /
        Range,
        0.0f,
        1.0f
    );
}

void SNineSlicePreview::OnCardScaleChanged(
    const float NewValue,
    const int32 CardIndex
)
{
    if (
        !CardScales.IsValidIndex(
            CardIndex
        )
    )
    {
        return;
    }

    const float NewScale =
        FMath::Lerp(
            CardScaleMin,
            CardScaleMax,
            FMath::Clamp(
                NewValue,
                0.0f,
                1.0f
            )
        );

    const FPreviewRect ViewportRect =
        GetCardViewportRect(
            CardIndex,
            GetCachedGeometry()
        );

    SetCardScaleAtPosition(
        CardIndex,
        NewScale,
        ViewportRect.Size *
        0.5f,
        ViewportRect.Size
    );
}

FText SNineSlicePreview::GetCardScaleText(
    const int32 CardIndex
) const
{
    const float Scale =
        CardScales.IsValidIndex(
            CardIndex
        )
            ? CardScales[
                CardIndex
            ]
            : CardScaleMin;

    return FText::FromString(
        FString::Printf(
            TEXT("%.0f%%"),
            Scale * 100.0f
        )
    );
}

void SNineSlicePreview::ResetCardView(
    const int32 CardIndex
)
{
    if (
        !CardScales.IsValidIndex(
            CardIndex
        ) ||
        !CardPanOffsets.IsValidIndex(
            CardIndex
        )
    )
    {
        return;
    }

    CardScales[
        CardIndex
    ] =
        CardScaleMin;

    CardPanOffsets[
        CardIndex
    ] =
        FVector2D::ZeroVector;

    SaveCardScaleSettings(
        CardIndex
    );

    Invalidate(
        EInvalidateWidgetReason::Paint
    );
}

void SNineSlicePreview::LoadCardScaleSettings()
{
    if (!GConfig)
    {
        return;
    }

    for (
        int32 CardIndex = 0;
        CardIndex < CardCount;
        ++CardIndex
    )
    {
        float StoredScale =
            CardScaleMin;

        if (
            GConfig->GetFloat(
                TEXT("NineSlicePreview"),
                CardScaleConfigKeys[
                    CardIndex
                ],
                StoredScale,
                GEditorPerProjectIni
            )
        )
        {
            CardScales[
                CardIndex
            ] =
                FMath::Clamp(
                    StoredScale,
                    CardScaleMin,
                    CardScaleMax
                );
        }
        else
        {
            CardScales[
                CardIndex
            ] =
                CardScaleMin;
        }
    }
}

void SNineSlicePreview::SaveCardScaleSettings(
    const int32 CardIndex
)
{
    if (
        !GConfig ||
        !CardScales.IsValidIndex(
            CardIndex
        )
    )
    {
        return;
    }

    GConfig->SetFloat(
        TEXT("NineSlicePreview"),
        CardScaleConfigKeys[
            CardIndex
        ],
        CardScales[
            CardIndex
        ],
        GEditorPerProjectIni
    );

    GConfig->Flush(
        false,
        GEditorPerProjectIni
    );
}