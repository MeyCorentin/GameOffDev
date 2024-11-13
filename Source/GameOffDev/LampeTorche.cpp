#include "LampeTorche.h"
#include "HighlightableObject.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "ProceduralMeshComponent.h"


// Sets default values
ALampeTorche::ALampeTorche()
{
    PrimaryActorTick.bCanEverTick = true;

    LampMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LampMesh"));
    RootComponent = LampMesh;

    LampSpotLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("LampSpotLight"));
    LampSpotLight->SetupAttachment(LampMesh);

    LampSpotLight->SetIntensity(100000.f);
    LampSpotLight->SetLightColor(FLinearColor::White);
    LampSpotLight->SetAttenuationRadius(5000.f);
}

void ALampeTorche::BeginPlay()
{
    Super::BeginPlay();

    FTimerDelegate TimerDelegate;
    TimerDelegate.BindUFunction(this, FName("UpdateBattery"));


    if (GEngine)
    {
        UWorld* World = GEngine->GetWorldFromContextObjectChecked(this);
        if (World)
        {
            World->GetTimerManager().SetTimer(
                BatteryTimerHandle,
                TimerDelegate,
                1.0f,
                true
            );
            UE_LOG(LogTemp, Warning, TEXT("Timer !"));
        }
    }
    InitBatteryLevel = BatteryLevel;
    InitialIntensity = LampSpotLight->Intensity;
    InitialAttenuationRadius = LampSpotLight->AttenuationRadius;
}

void ALampeTorche::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}


void ALampeTorche::Charge(float energy)
{
    BatteryLevel += energy;
    if (BatteryLevel > 100)
        BatteryLevel = 100;
}

void ALampeTorche::ChangeColor(int32 ColorCode)
{
    switch (ColorCode)
    {
    case 1:
        LampSpotLight->SetLightColor(FLinearColor::Red);
        break;
    case 2:
        LampSpotLight->SetLightColor(FLinearColor::Green);
        break;
    case 3:
        LampSpotLight->SetLightColor(FLinearColor::Blue);
        break;
    default:
        //UE_LOG(LogTemp, Warning, TEXT("Invalid color code: %d. Use 1 (Red), 2 (Green), or 3 (Blue)."), ColorCode);
        break;
    }
}

void ALampeTorche::AttachToPlayer(USkeletalMeshComponent* PlayerMesh)
{
    if (PlayerMesh)
    {
        AttachToComponent(PlayerMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, "HandSocket");
    }
}

void ALampeTorche::UpdateBattery()
{
    if (BatteryLevel > 0.f && InitBatteryLevel > 0.f)
    {
        float BatteryDrainAmount = (BatteryDrainRate / 100.f) * InitBatteryLevel;
        BatteryLevel = FMath::Max(BatteryLevel - BatteryDrainAmount, 0.f);
        float BatteryProportion = BatteryLevel / InitBatteryLevel;
        LampSpotLight->SetAttenuationRadius(InitialAttenuationRadius * BatteryProportion);
        LampSpotLight->SetIntensity(InitialIntensity * BatteryProportion);

        if (BatteryLevel <= 0.f)
        {
            LampSpotLight->SetIntensity(0.f);
        }
    }
}

bool ALampeTorche::IsActorInDetectionCone(AActor* Parent, AActor* Child, FColor RequiredColor)
{
    float Length = LampSpotLight->AttenuationRadius;
    float ConeAngle = LampSpotLight->OuterConeAngle;

    AHighlightableObject* parent = Cast<AHighlightableObject>(Parent);
    if (!parent)
    {
        return false;
    }

    // Récupérer les vertices depuis le HighlightComponent
    TArray<FVector> Vertices = parent->GetVertices();
    if (Vertices.Num() == 0)
    {
        return false;
    }

    // Vérifier que l'objet possède la bonne couleur
    if (RequiredColor != LampSpotLight->LightColor)
    {
        return false;
    }

    // Récupérer la position de la lampe et la direction de la lumière
    FVector LampPosition = LampSpotLight->GetComponentLocation();
    FVector LampDirection = LampSpotLight->GetForwardVector();

    for (const FVector& VertexPos : Vertices)
    {
        FVector WorldVertexPos = Child->GetActorTransform().TransformPosition(VertexPos);
        FVector DirectionToVertex = (WorldVertexPos - LampPosition).GetSafeNormal();
        float DotProduct = FVector::DotProduct(LampDirection, DirectionToVertex);
        float Angle = FMath::Acos(DotProduct) * 180.f / PI;

        if (Angle <= ConeAngle)
        {
            float DistanceToVertex = FVector::Dist(LampPosition, WorldVertexPos);
            if (DistanceToVertex <= Length)
            {
                return true;
            }
        }
    }

    return false;
}

bool ALampeTorche::IsPointInCone(const FVector& Point)
{
    float Length = LampSpotLight->AttenuationRadius;
    float ConeAngle = LampSpotLight->OuterConeAngle;
    FVector LampPosition = LampSpotLight->GetComponentLocation();
    FVector LampDirection = LampSpotLight->GetForwardVector();
    FVector DirectionToPoint = (Point - LampPosition).GetSafeNormal();
    float DotProduct = FVector::DotProduct(LampDirection, DirectionToPoint);
    float Angle = FMath::Acos(DotProduct) * 180.f / PI;  // Angle en degrés

    if (Angle <= ConeAngle)
    {
        float DistanceToPoint = FVector::Dist(LampPosition, Point);
        if (DistanceToPoint <= Length)
        {
            return true;
        }
    }

    return false;
}
