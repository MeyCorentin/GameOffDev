#include "LevelTeleporter.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameOffDevCharacter.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ALevelTeleporter::ALevelTeleporter()
{
    PrimaryActorTick.bCanEverTick = true;
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
}

void ALevelTeleporter::BeginPlay()
{
    Super::BeginPlay();
    MeshComponent->OnComponentBeginOverlap.AddDynamic(this, &ALevelTeleporter::LoadLevel);
}

void ALevelTeleporter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

bool ALevelTeleporter::CheckCollisionWithPlayer()
{
    if (!MeshComponent)
    {
        return false;
    }

    // R�cup�re le joueur
    AActor* PlayerActor = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (PlayerActor)
    {
        // V�rifie si le joueur est dans la zone de collision
        return MeshComponent->IsOverlappingActor(PlayerActor);
    }
    return false;
}
void ALevelTeleporter::LoadLevel(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor != GetWorld()->GetFirstPlayerController()->GetPawn())
    {
        return;
    }

    if (LevelToLoad.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("LevelToLoad n'est pas d�fini !"));
        return;
    }

    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (PlayerController)
    {
        if (NeedPaper == false)
        {
            PlayerController->PlayerCameraManager->StartCameraFade(0.f, 1.f, 1.0f, FLinearColor::Black, false, true);

            TWeakObjectPtr<ALevelTeleporter> WeakThis(this);

            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [WeakThis, this]()
                {
                    if (WeakThis.IsValid())
                    {
                        AGameOffDevCharacter* PlayerChar = Cast<AGameOffDevCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
                        if (PlayerChar)
                        {
                            PlayerChar->SavePlayerData();
                        }
                        UGameplayStatics::OpenLevel(WeakThis.Get(), WeakThis->LevelToLoad);
                    }
                }, 1.0f, false);
        }
        else {
            AGameOffDevCharacter* PlayerChar = Cast<AGameOffDevCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));

            if (PlayerChar->CanEscape == true)
            {
                PlayerController->PlayerCameraManager->StartCameraFade(0.f, 1.f, 1.0f, FLinearColor::Black, false, true);

                TWeakObjectPtr<ALevelTeleporter> WeakThis(this);

                FTimerHandle TimerHandle;
                GetWorld()->GetTimerManager().SetTimer(TimerHandle, [WeakThis, this]()
                    {
                        if (WeakThis.IsValid())
                        {
                            AGameOffDevCharacter* PlayerChar = Cast<AGameOffDevCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
                            if (PlayerChar)
                            {
                                PlayerChar->SavePlayerData();
                            }
                            UGameplayStatics::OpenLevel(WeakThis.Get(), WeakThis->LevelToLoad);
                        }
                    }, 1.0f, false);
            }
        }
    }
}
