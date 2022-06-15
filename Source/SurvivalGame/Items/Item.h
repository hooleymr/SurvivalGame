// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Item.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemModified);

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	IR_Common UMETA(DisplayName = "Common"),
	IR_Uncommon UMETA(DisplayName = "Uncommon"),
	IR_Rare UMETA(DisplayName = "Rare"),
	IR_VeryRare UMETA(DisplayName = "Very Rare"),
	IR_Legendary UMETA(DisplayName = "Legendary")

};

//ITEMS ONLY EXIST WITHIN AN INVENTORY

/**
 * 
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class SURVIVALGAME_API UItem : public UObject
{
	GENERATED_BODY()

protected: 
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:

	UItem();

	//The mesh to display for the item's pickup
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	class UStaticMesh* PickupMesh;

	//thumbnail displayed in the inventory
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	class UTexture2D* Thumbnail;

	//name of item in inventory
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	FText ItemDisplayName;

	//optional description of the item in the inventory
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (MultiLine = true))
	FText ItemDescription;

	//action text for item, (ie Eat, Use, Equip, etc...)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	FText UseActionText;

	//Rarity of the item using rarity enum
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	EItemRarity Rarity;

	//Weight of the item
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (clampMin = 0.0))
	float Weight;

	//Whether or not this item can stack in the inventory
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	bool bCanStack;

	//maximum number of items that can be stacked
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (clampMin = 2, EditCondition = bCanStack))
	int32 MaxStackSize;

	//tooltip in the inventory for this item
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	TSubclassOf<class UItemTooltip> ItemTooltip;

	//amount of items currently held
	UPROPERTY(ReplicatedUsing = OnRep_Quantity, EditAnywhere, Category = "Item", meta = (UIMin = 1, EditCondition = bCanStack))
	int32 Quantity; //replicated variable, managed by the server, client calls this function when server issues an update to the quantity value

	//ref to inventory that contains this item
	UPROPERTY()
	class UInventoryComponent* OwningInventory;


	UPROPERTY() //how server knows it needs to update client
	int32 RepKey;

	UPROPERTY(BlueprintAssignable)
	FOnItemModified OnItemModified;

	UFUNCTION()
	void OnRep_Quantity();

	UFUNCTION(BlueprintCallable, Category = "item")
	void SetQuantity(const int32 NewQuantity);

	UFUNCTION(BlueprintCallable, Category = "item")
		FORCEINLINE float GetStackWeight() const { return Quantity * Weight; };

	//equiped items shouldn't appear in inventory
	UFUNCTION(BlueprintPure, Category = "Item")
	virtual bool ShouldShowInInventory() const;

	//by marking virtual can be overriden so that Use does something diff on call
	virtual void Use(class ASurvivalCharacter* Character);
	virtual void AddedToInventory(class UInventory* Inventory);


	//mark object as needing replication. Must call internally after modifying any replicated properties
	void MarkDirtyForReplication();

};
