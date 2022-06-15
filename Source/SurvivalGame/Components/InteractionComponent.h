// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "InteractionComponent.generated.h"

//DELEGATES
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginInteract, class ASurvivalCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndInteract, class ASurvivalCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginFocus, class ASurvivalCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndFocus, class ASurvivalCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteract, class ASurvivalCharacter*, Character);

/**
 * 
 */

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent)) //allows this component to be created in blueprints
class SURVIVALGAME_API UInteractionComponent : public UWidgetComponent
{
	GENERATED_BODY()
	
public:

	UInteractionComponent();

	//The amount of time the player must hold the interact key to interact with the object
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
		float InteractionTime;

	//The maximum distance the player can be from the object before being able to interact
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
		float InteractionDistance;

	//The name that will appear when the player looks at the object
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
		FText InteractableNameText;

	//The text that describes the aciton the player will take when interacting, ie 'Sit' 'Light' 'Open' etc...
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
		FText InteractableActionText;

	//Whether the item allows multiple players to interact with the item at the same time
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
		bool bAllowMultipleInteractors;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractableNameText(const FText& NewNameText);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractableActionText(const FText& NewActionText);

	//DELEGATES
	//[local + server] custom behavior for each type of interaction
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnBeginInteract OnBeginInteract;

	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnEndInteract OnEndInteract;

	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnBeginFocus OnBeginFocus;

	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnEndFocus OnEndFocus;

	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnInteract OnInteract;


protected:

	//Called when game starts
	virtual void Deactivate() override;

	//which characters can interact with interactable objs
	bool CanInteract(class ASurvivalCharacter* Character) const;
	
	//On server side this holds ALL valid interactors
	//On client holds ONLY local player (if they are a valid interactor)
	UPROPERTY()
	TArray<class ASurvivalCharacter*> Interactors;

public:

	//Refresh interaction widget ONLY when something is changed - OPTIMIZATION
	void RefreshWidget();

	//Called on client when the player interaction check trace begins/ends hitting this item
	void BeginFocus(class ASurvivalCharacter* Character);
	void EndFocus(class ASurvivalCharacter* Character);

	//Called on the client when the player begins/ends interaction with the item
	void BeginInteract(class ASurvivalCharacter* Character);
	void EndInteract(class ASurvivalCharacter* Character);

	void Interact(class ASurvivalCharacter* Character);

	//returns value between 0-1 that represents progress through interaction
	//Serverside = first interactors %
	//Clientside = local interactors %
	UFUNCTION(BlueprintPure, Category = "Interaction")
	float GetInteractPercentage();
};
