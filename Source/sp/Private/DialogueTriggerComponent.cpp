#include "DialogueTriggerComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "DialogueManager.h"
#include "spPlayerController.h"
#include "Engine/Engine.h"

UDialogueTriggerComponent::UDialogueTriggerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DialogueTriggerBox"));
	TriggerBox->SetupAttachment(this);  // become a child to *this*
	TriggerBox->SetBoxExtent(FVector(100.f, 70.f, 100.f));
	TriggerBox->SetMobility(EComponentMobility::Movable);
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
	TriggerBox->SetGenerateOverlapEvents(true);
}

void UDialogueTriggerComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("DialogueTriggerComponent: No owner found."));
		return;
	}

	// TriggerBox BoxComponent should be already created, not bind it to the event
	if (TriggerBox)
	{
		UE_LOG(LogTemp, Warning, TEXT("DialogueTriggerComponent: TriggerBox exists!"));
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &UDialogueTriggerComponent::OnOverlapBegin);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DialogueTriggerComponent: TriggerBox is null!"));
	}

	// Load dialogue data (using the loader class)
	if (!DialogueFilePath.IsEmpty())
	{
		UDialogueDataLoader* Loader = NewObject<UDialogueDataLoader>(this);
		if (Loader)
		{
			Loader->LoadDialogueFromFile(DialogueFilePath, DialogueData);
		}
	}
}

void UDialogueTriggerComponent::OnOverlapBegin(
	UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
    // Quick visual debug so we know the function fired
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("DialogueTriggerComponent::OnOverlapBegin"));
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Overlap fired by: %s"), *OverlappedComp->GetName());

	if (!OtherActor || OtherActor == GetOwner()) return;

	// Only trigger the following logic if it is player character overlapping it
	ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
	if (!PlayerChar) return;

	APlayerController* PC = Cast<APlayerController>(PlayerChar->GetController());
	if (!PC) return;

	// Find DialogueManager component on the player controller
	UDialogueManager* DM = PC->FindComponentByClass<UDialogueManager>();
	if (!DM)
	{
		UE_LOG(LogTemp, Warning, TEXT("DialogueTriggerComponent: DialogueManager not found on PlayerController"));
		return;
	}

	if (DialogueData.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("DialogueTriggerComponent: DialogueData empty, cannot start dialogue."));
		return;
	}

	// Preparation 1: disable player movement
	if (PlayerChar->GetCharacterMovement())
	{
		PlayerChar->GetCharacterMovement()->DisableMovement();
	}

	// Preparation 2: Switch input mode if using AspPlayerController and the widget instance
	if (AspPlayerController* spPC = Cast<AspPlayerController>(PC))
	{
		if (spPC->DialogueWidgetInstance)
		{
			PC->bShowMouseCursor = true;
			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(spPC->DialogueWidgetInstance->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(InputMode);
		}
	}

	// Preparation 3: bind dialogue end handler to DM's dialogue end event
	DM->OnDialogueEnded.AddDynamic(this, &UDialogueTriggerComponent::HandleDialogueEnded);

	UE_LOG(LogTemp, Log, TEXT("DialogueTriggerComponent: Starting dialogue."));
    // Start dialogue (use the node id defined in the json we want to use)
	DM->StartDialogue(StartingNodeID, &DialogueData);
}

void UDialogueTriggerComponent::HandleDialogueEnded()
{
	UE_LOG(LogTemp, Log, TEXT("DialogueTriggerComponent: HandleDialogueEnded called."));

	// Unbind from DialogueManager on the player controller (safe remove)
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		if (UDialogueManager* DM = PC->FindComponentByClass<UDialogueManager>())
		{
			DM->OnDialogueEnded.RemoveDynamic(this, &UDialogueTriggerComponent::HandleDialogueEnded);
			UE_LOG(LogTemp, Log, TEXT("DialogueTriggerComponent: Unbound from DM %p"), DM);
		}
	}

	// Restore movement
	if (ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(this, 0))
	{
		if (PlayerChar->GetCharacterMovement())
		{
			PlayerChar->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		}
	}
    // UI mode resume will be done in spPlayerController
}

void UDialogueTriggerComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.RemoveAll(this);
		TriggerBox->DestroyComponent();
		TriggerBox = nullptr;
	}
	Super::OnComponentDestroyed(bDestroyingHierarchy);
}
