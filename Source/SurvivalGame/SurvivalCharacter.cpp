// Fill out your copyright notice in the Description page of Project Settings.


#include "SurvivalCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/InteractionComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

// Sets default values
ASurvivalCharacter::ASurvivalCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	CameraComponent->SetupAttachment(GetMesh(), FName("CameraSocket")); //attach cam to player mesh
	CameraComponent->bUsePawnControlRotation = true; //camera follows mouse movement

	HelmetMesh = CreateDefaultSubobject<USkeletalMeshComponent>("HelmetMesh");
	HelmetMesh->SetupAttachment(GetMesh()); //attach component to head
	HelmetMesh->SetMasterPoseComponent(GetMesh()); //lets rest of the body follow the head in animations

	ChestMesh = CreateDefaultSubobject<USkeletalMeshComponent>("ChestMesh");
	ChestMesh->SetupAttachment(GetMesh()); //attach component to head
	ChestMesh->SetMasterPoseComponent(GetMesh()); //lets rest of the body follow the head in animations

	LegsMesh = CreateDefaultSubobject<USkeletalMeshComponent>("LegsMesh");
	LegsMesh->SetupAttachment(GetMesh()); //attach component to head
	LegsMesh->SetMasterPoseComponent(GetMesh()); //lets rest of the body follow the head in animations

	FeetMesh = CreateDefaultSubobject<USkeletalMeshComponent>("FeetMesh");
	FeetMesh->SetupAttachment(GetMesh()); //attach component to head
	FeetMesh->SetMasterPoseComponent(GetMesh()); //lets rest of the body follow the head in animations

	VestMesh = CreateDefaultSubobject<USkeletalMeshComponent>("VestMesh");
	VestMesh->SetupAttachment(GetMesh()); //attach component to head
	VestMesh->SetMasterPoseComponent(GetMesh()); //lets rest of the body follow the head in animations

	HandsMesh = CreateDefaultSubobject<USkeletalMeshComponent>("HandsMesh");
	HandsMesh->SetupAttachment(GetMesh()); //attach component to head
	HandsMesh->SetMasterPoseComponent(GetMesh()); //lets rest of the body follow the head in animations

	BackpackMesh = CreateDefaultSubobject<USkeletalMeshComponent>("BackpackMesh");
	BackpackMesh->SetupAttachment(GetMesh()); //attach component to head
	BackpackMesh->SetMasterPoseComponent(GetMesh()); //lets rest of the body follow the head in animations

	//Crouching
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	//make head invisible to player only
	GetMesh()->SetOwnerNoSee(true);

	//Interaction defaults
	InteractionCheckFrequency = 0.f; //0 = every frame
	InteractionCheckDistance = 1000.f; //furthest we check for interactables, 1000 = 10meters
}

// Called when the game starts or when spawned
void ASurvivalCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//Interaciton Basics
void ASurvivalCharacter::PerformInteractionCheck()
{
	if (GetController() == nullptr) //safety check, if returns null will crash without this 
	{
		return;
	}

	InteractionData.LastInteractionCheckTime = GetWorld()->GetTimeSeconds();

	FVector EyesLoc;
	FRotator EyesRot;

	GetController()->GetPlayerViewPoint(EyesLoc, EyesRot); //get location of player camera

	FVector TraceStart = EyesLoc;
	FVector TraceEnd = (EyesRot.Vector() *InteractionCheckDistance) + TraceStart;
	FHitResult TraceHit;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this); //ignores the player when raycasting result

	if (GetWorld()->LineTraceSingleByChannel(TraceHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams)) //line cast
	{
		//DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, .5f); //Debug

		if (TraceHit.GetActor()) //check if hit result is an interactable object
		{
			if (UInteractionComponent* InteractionComponent = Cast<UInteractionComponent>(TraceHit.GetActor()->GetComponentByClass(UInteractionComponent::StaticClass()))) //check if hitresult has interaction component
			{
				float Distance = (TraceStart - TraceHit.ImpactPoint).Size(); // get distance to obj. // size bc vectors?

				if (InteractionComponent != GetInteractable() && Distance <= InteractionComponent->InteractionDistance) //can interact
				{
					FoundNewInteractable(InteractionComponent);
				}
				else if (Distance > InteractionComponent->InteractionDistance && GetInteractable()) //cannot interact
				{
					CouldntFindInteractable();
				}
				
				return;
			}
		}
	}

	CouldntFindInteractable(); //default
}

void ASurvivalCharacter::CouldntFindInteractable()
{
	if (GetWorldTimerManager().IsTimerActive(TimerHandle_Interact)) //Lost focus on interactable, clear timer
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_Interact);
	}

	if (UInteractionComponent* Interactable = GetInteractable()) //Tell interactable we have stopped focus and clear current interactable
	{
		Interactable->EndFocus(this);

		if (InteractionData.bInteractHeld) 
		{
			EndInteract();
		}
	}

	InteractionData.ViewedInteractionComponent = nullptr; //i was right
}

void ASurvivalCharacter::FoundNewInteractable(UInteractionComponent * Interactable)
{
	//UE_LOG(LogTemp, Warning, TEXT("FOUND INTERACTABLE")); //debugging
	EndInteract(); //end existing interactions

	if (UInteractionComponent* OldInteractable = GetInteractable()) //unfocus old interactable
	{
		OldInteractable->EndFocus(this);
	}

	InteractionData.ViewedInteractionComponent = Interactable;
	Interactable->BeginFocus(this);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//BEGIN INTERACT FUNCTIONS
void ASurvivalCharacter::BeginInteract()
{
	if (!HasAuthority()) //if calling body is NOT the server aka is the client, call the server interact
	{
		ServerBeginInteract();
	}

	InteractionData.bInteractHeld = true;

	if (UInteractionComponent* Interactable = GetInteractable()) 
	{
		Interactable->BeginInteract(this);

		if (FMath::IsNearlyZero(Interactable->InteractionTime))
		{
			Interact();
		}
		else
		{
			GetWorldTimerManager().SetTimer(TimerHandle_Interact, this, &ASurvivalCharacter::Interact, Interactable->InteractionTime, false); //start timer
		}
	}
}

void ASurvivalCharacter::ServerBeginInteract_Implementation()
{
	BeginInteract();
}

bool ASurvivalCharacter::ServerBeginInteract_Validate()
{
	return true;
}


//END INTERACT FUNCTIONS
void ASurvivalCharacter::EndInteract()
{
	if (!HasAuthority()) //if calling body is NOT the server aka is the client, call the server interact
	{
		ServerEndInteract();
	}

	InteractionData.bInteractHeld = false;

	GetWorldTimerManager().ClearTimer(TimerHandle_Interact);
}

void ASurvivalCharacter::ServerEndInteract_Implementation()
{
	EndInteract();
}

bool ASurvivalCharacter::ServerEndInteract_Validate()
{
	return true;
}


//INTERACT
void ASurvivalCharacter::Interact()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_Interact); //clear timer

	if (UInteractionComponent* Interactable = GetInteractable()) //call interact function on interactable object
	{
		Interactable->Interact(this);
	}
}


bool ASurvivalCharacter::IsInteracting() const
{
	return GetWorldTimerManager().IsTimerActive(TimerHandle_Interact); 
}

float ASurvivalCharacter::GetRemainingInteractTime() const
{
	return GetWorldTimerManager().GetTimerRemaining(TimerHandle_Interact);
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





//MOVEMENT//
void ASurvivalCharacter::MoveForward(float Val)
{
	if (Val != 0.f) //checks if movement is needed
	{
		AddMovementInput(GetActorForwardVector(), Val);
	}
}

void ASurvivalCharacter::MoveRight(float Val)
{
	if (Val != 0.f) //checks if movement is needed
	{
		AddMovementInput(GetActorRightVector(), Val);
	}
}

void ASurvivalCharacter::LookUp(float Val)
{
	if (Val != 0.f)
	{
		AddControllerPitchInput(Val);
	}
	
}

void ASurvivalCharacter::Turn(float Val)
{
	if (Val != 0.f)
	{
		AddControllerYawInput(Val);
	}
}

void ASurvivalCharacter::StartCrouching()
{
	Crouch();
}

void ASurvivalCharacter::StopCrouching()
{
	UnCrouch();
}




// Called every frame
void ASurvivalCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	const bool bIsInteractingOnServer = (HasAuthority() && IsInteracting());

	//Server optimization
	//if (not the server OR is interacting on server) AND time since last interaction is GREATER THAN interaction check freq THEN check for interaction
	if ((!HasAuthority() || bIsInteractingOnServer) && GetWorld()->TimeSince(InteractionData.LastInteractionCheckTime) > InteractionCheckFrequency) 
	{
		PerformInteractionCheck();
	}
}

// Called to bind functionality to input
void ASurvivalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//BINDS FUNCTIONS TO THE RESPECTIVE BINDINGS IN THE PROJECT SETTINGS
	//BindAxis = smooth motions (movement, etc)
	//BindAction = ON / OFF events (Button presses, etc)


	PlayerInputComponent->BindAxis("MoveForward", this, &ASurvivalCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASurvivalCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &ASurvivalCharacter::LookUp);
	PlayerInputComponent->BindAxis("Turn", this, &ASurvivalCharacter::Turn);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASurvivalCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ASurvivalCharacter::StopJumping);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASurvivalCharacter::StartCrouching);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASurvivalCharacter::StopCrouching);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ASurvivalCharacter::BeginInteract);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &ASurvivalCharacter::EndInteract);
}

