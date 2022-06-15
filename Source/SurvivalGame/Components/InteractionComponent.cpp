// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionComponent.h"
#include "SurvivalCharacter.h"
#include "ConstructorHelpers.h"
#include "Widgets/InteractionWidget.h"
UInteractionComponent::UInteractionComponent()
{
	SetComponentTickEnabled(false); //component does not need to tick, optimization

	//Defaults
	InteractionTime = 0.f;
	InteractionDistance = 200.f; //2 Meters
	InteractableNameText = FText::FromString("Interactable Object");
	InteractableActionText = FText::FromString("Interact");
	bAllowMultipleInteractors = true;

	//UI Options
	Space = EWidgetSpace::Screen; //puts the widget in the UI space rather than world space
	DrawSize = FIntPoint(400, 100); //size of UI element

	static ConstructorHelpers::FClassFinder<UUserWidget> WBP_InteractionCard(TEXT("/Game/UserInterface/Widgets/WBP_InteractionCard"));
	if (WBP_InteractionCard.Class != nullptr)
	{
		WidgetClass = WBP_InteractionCard.Class;
	}


	bDrawAtDesiredSize = true;

	SetActive(true);
	SetHiddenInGame(true); //hides the menu by default so it doesn't appear always


}

void UInteractionComponent::SetInteractableNameText(const FText & NewNameText)
{
	InteractableNameText = NewNameText;
	RefreshWidget();
}

void UInteractionComponent::SetInteractableActionText(const FText & NewActionText)
{
	InteractableActionText = NewActionText;
	RefreshWidget();
}

void UInteractionComponent::Deactivate()
{
	Super::Deactivate();

	for (int32 i = Interactors.Num() - 1; i >= 0; --i) //get all interactors
	{
		if (ASurvivalCharacter* Interactor = Interactors[i]) //stop interacting and focusing
		{
			EndFocus(Interactor);
			EndInteract(Interactor);
		}
	}

	Interactors.Empty();
}

bool UInteractionComponent::CanInteract(ASurvivalCharacter * Character) const //enforcment
{
	//if obj doesnt allow multiple interactors and multiple interactors are interacting with obj - dont allow interaction
	const bool bPlayerAlreadyInteracting = !bAllowMultipleInteractors && Interactors.Num() >= 1;
	return !bPlayerAlreadyInteracting && IsActive() && GetOwner() != nullptr && Character != nullptr;
}

void UInteractionComponent::RefreshWidget()
{
	if (!bHiddenInGame && GetOwner()->GetNetMode() != NM_DedicatedServer) //check to make sure we are not the server and action card is not hidden
	{
		if (UInteractionWidget* InteractionWidget = Cast<UInteractionWidget>(GetUserWidgetObject()))
		{
			InteractionWidget->UpdateInteractionWidget(this);
		}
	}
}

void UInteractionComponent::BeginFocus(ASurvivalCharacter * Character)
{
	if (!IsActive() || !GetOwner() || !Character) //Character is null, do nothing
	{
		return;
	}

	OnBeginFocus.Broadcast(Character);

	SetHiddenInGame(false);

	if (!GetOwner()->HasAuthority()) //if not the server start outline
	{
		TArray<UActorComponent*> PrimitiveComponents = GetOwner()->GetComponentsByClass(UPrimitiveComponent::StaticClass());

		for (auto& VisualComp : PrimitiveComponents) //LEARN MORE ABOUT LOOPS CPP
		{
			if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(VisualComp))
			{
				Prim->SetRenderCustomDepth(true);
			}
		}

	}

	RefreshWidget();
}

void UInteractionComponent::EndFocus(ASurvivalCharacter * Character)
{
	OnEndFocus.Broadcast(Character);

	SetHiddenInGame(true); //hide interaction card widget

	if (!GetOwner()->HasAuthority()) //if not the server end outline
	{
		TArray<UActorComponent*> PrimitiveComponents = GetOwner()->GetComponentsByClass(UPrimitiveComponent::StaticClass());

		for (auto& VisualComp : PrimitiveComponents) //LEARN MORE ABOUT LOOPS CPP
		{
			if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(VisualComp))
			{
				Prim->SetRenderCustomDepth(false);
			}
		}

	}
}

void UInteractionComponent::BeginInteract(ASurvivalCharacter * Character)
{
	//if character can interact add player to list of interactors and broadcast to begin interact delegate
	if (CanInteract(Character))
	{
		Interactors.AddUnique(Character);
		OnBeginInteract.Broadcast(Character);
	}

}

void UInteractionComponent::EndInteract(ASurvivalCharacter * Character)
{
	//remove character from list of interactors and broadcast end interact
	Interactors.RemoveSingle(Character);
	OnEndInteract.Broadcast(Character);

}

void UInteractionComponent::Interact(ASurvivalCharacter * Character)
{
	if (CanInteract(Character))
	{
		OnInteract.Broadcast(Character);
	}
}

float UInteractionComponent::GetInteractPercentage()
{
	if (Interactors.IsValidIndex(0)) //check if interactor is in list of valid interactors and get the first interactors (which is us bc client side)
	{
		if (ASurvivalCharacter* Interactor = Interactors[0])
		{
			if (Interactor && Interactor->IsInteracting())
			{
				return 1.f - FMath::Abs(Interactor->GetRemainingInteractTime() / InteractionTime); // 1.f - bc returns 1.X and we want 0.X
			}

		}
	}

	return 0.f;
}
