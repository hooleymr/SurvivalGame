// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "Net/UnrealNetwork.h"


#define LOCTEXT_NAMESPACE "Item"

void UItem::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UItem, Quantity); //tells quantity value to be replicated, server is now managing this value
}

bool UItem::IsSupportedForNetworking() const
{
	return true;
}

#if WITH_EDITOR
void UItem::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName ChangedPropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	//uprop clamping doesn'support using var to clamp so do it here instead
	if (ChangedPropertyName == GET_MEMBER_NAME_CHECKED(UItem, Quantity))
	{
		//quantity = clamp quantity between 1 and maxstacksize
		Quantity = FMath::Clamp(Quantity, 1, bCanStack ? MaxStackSize : 1);
	}
}
#endif

UItem::UItem()
{
	//Defaults
	ItemDisplayName = LOCTEXT("ItemName", "Item");
	UseActionText = LOCTEXT("ItemUseActionText", "Use");
	Weight = 0.f;
	bCanStack = true;
	Quantity = 1;
	MaxStackSize = 2;
	RepKey = 0;

}

void UItem::OnRep_Quantity()
{
	OnItemModified.Broadcast();
}

void UItem::SetQuantity(const int32 NewQuantity)
{
	if (NewQuantity != Quantity)
	{
		//quantity = clamp new quantity between 0 and maxstacksize
		Quantity = FMath::Clamp(NewQuantity, 0, bCanStack ? MaxStackSize : 1);
		MarkDirtyForReplication();
	}
}

bool UItem::ShouldShowInInventory() const
{
	return true;
}

void UItem::Use(ASurvivalCharacter * Character)
{
}

void UItem::AddedToInventory(UInventory * Inventory)
{
}

void UItem::MarkDirtyForReplication()
{

}

#undef LOCTEXT_NAMESPACE