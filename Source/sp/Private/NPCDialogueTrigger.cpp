#include "NPCDialogueTrigger.h"

#include "DialogueManager.h"
#include "Components/BillboardComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ANPCDialogueTrigger::ANPCDialogueTrigger()
{
    PrimaryActorTick.bCanEverTick = true;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    UBillboardComponent* Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
    Billboard->SetupAttachment(RootComponent);

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    TriggerBox->SetupAttachment(RootComponent);
    TriggerBox->SetBoxExtent(FVector(100, 100, 100));
    TriggerBox->SetCollisionProfileName(TEXT("Trigger"));

    // Ensure overlap events are generated
    TriggerBox->SetGenerateOverlapEvents(true);
}

// Called when the game starts or when spawned
void ANPCDialogueTrigger::BeginPlay()
{
    Super::BeginPlay();

    if (TriggerBox)
    {
        TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ANPCDialogueTrigger::OnOverlapBegin);
    }
}

// Called every frame
void ANPCDialogueTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ANPCDialogueTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                         bool bFromSweep, const FHitResult & SweepResult)
{
    // Quick visual debug so we know the function fired
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("NPCDialogueTrigger::OnOverlapBegin"));
    }

    if (!OtherActor || OtherActor == this) return;

    // Cast to pawn (player or AI). We want the controller that owns the pawn.
    APawn* Pawn = Cast<APawn>(OtherActor);
    if (!Pawn) return;

    AController* Controller = Pawn->GetController();
    if (!Controller) return;

    // PlayerController is expected, but this works generically.
    APlayerController* PC = Cast<APlayerController>(Controller);
    if (!PC) return;

    // Find DialogueManager component on the controller
    UDialogueManager* DM = PC->FindComponentByClass<UDialogueManager>();
    if (!DM)
    {
        // helpful log if DialogueManager wasn't found
        UE_LOG(LogTemp, Warning, TEXT("NPCDialogueTrigger: DialogueManager not found on PlayerController"));
        return;
    }

    // Start dialogue (use the node id defined in the json we want to use)
    DM->StartDialogue(TEXT("start"));
}
