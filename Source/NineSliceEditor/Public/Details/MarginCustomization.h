#pragma once

#include "IPropertyTypeCustomization.h"
#include "Misc/Optional.h"
#include "Templates/SharedPointer.h"

class IPropertyHandle;
class SEditableTextBox;
class SWidget;

template <typename NumericType>
struct INumericTypeInterface;

class FMarginCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	FMarginCustomization();

	virtual void CustomizeHeader(
		TSharedRef<IPropertyHandle> StructPropertyHandle,
		class FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& StructCustomizationUtils
	) override;

	virtual void CustomizeChildren(
		TSharedRef<IPropertyHandle> StructPropertyHandle,
		class IDetailChildrenBuilder& StructBuilder,
		IPropertyTypeCustomizationUtils& StructCustomizationUtils
	) override;

private:
	bool IsSlateBrushMargin() const;
	
	TSharedRef<SEditableTextBox> MakePropertyWidget();

	TSharedRef<SWidget> MakeChildPropertyWidget(
		int32 PropertyIndex
	) const;

	FText GetMarginText() const;

	void OnMarginTextCommitted(
		const FText& InText,
		ETextCommit::Type InCommitType
	);

	FString GetMarginTextFromProperties() const;

	TOptional<float> OnGetValue(
		int32 PropertyIndex
	) const;

	void OnValueCommitted(
		float NewValue,
		ETextCommit::Type CommitType,
		int32 PropertyIndex
	);

	void OnValueChanged(
		float NewValue,
		int32 PropertyIndex
	);

	void OnBeginSliderMovement();

	void OnEndSliderMovement(
		float NewValue
	);

	FReply OnEditClicked();

	TSharedPtr<IPropertyHandle> StructPropertyHandle;

	TArray<TSharedRef<IPropertyHandle>> ChildPropertyHandles;

	bool bIsMarginUsingUVSpace = false;
	bool bIsUsingSlider = false;

	TSharedPtr<SEditableTextBox> MarginEditableTextBox;

	TSharedPtr<INumericTypeInterface<float>> NumericInterface;
};