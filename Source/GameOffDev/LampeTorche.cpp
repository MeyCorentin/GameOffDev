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
    FMeshData MeshData = parent->GetVerticesAndTriangles();
    UProceduralMeshComponent* NewProceduralMesh = NewObject<UProceduralMeshComponent>(Parent);

    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    TArray<FLinearColor> Colors;
    TArray<FProcMeshTangent> Tangents;

    FVector LightDirection = LampSpotLight->GetForwardVector();

    FTransform ParentTransform = Parent->GetTransform();
    FTransform ChildTransform = Child->GetTransform();

    TArray<FVector> VerticesInCone;
    TArray<int32> TrianglesInCone;

    VerticesInCone.Empty();
    TrianglesInCone.Empty();
    bool bFoundTriangleInCone = false;
    TMap<FVector, int32> VertexIndexMap;

    for (int32 i = 0; i < MeshData.Triangles.Num(); i += 3)
    {
        int32 Index0 = MeshData.Triangles[i];
        int32 Index1 = MeshData.Triangles[i + 1];
        int32 Index2 = MeshData.Triangles[i + 2];

        FVector Vertex0 = MeshData.Vertices[Index0];
        FVector Vertex1 = MeshData.Vertices[Index1];
        FVector Vertex2 = MeshData.Vertices[Index2];

        FVector WorldVertex0 = ChildTransform.TransformPosition(Vertex0);
        FVector WorldVertex1 = ChildTransform.TransformPosition(Vertex1);
        FVector WorldVertex2 = ChildTransform.TransformPosition(Vertex2);

        bool bVertex0InCone = IsPointInCone(WorldVertex0);
        bool bVertex1InCone = IsPointInCone(WorldVertex1);
        bool bVertex2InCone = IsPointInCone(WorldVertex2);

        if (bVertex0InCone || bVertex1InCone || bVertex2InCone)
        {
            bFoundTriangleInCone = true;
            // Ajoute les indices des triangles dans TrianglesInCone dans l'ordre
            int32 _Index0 = VerticesInCone.Add(Vertex0);
            int32 _Index1 = VerticesInCone.Add(Vertex1);
            int32 _Index2 = VerticesInCone.Add(Vertex2);

            // Ajoute les indices dans TrianglesInCone pour former un triangle
            TrianglesInCone.Add(_Index0);
            TrianglesInCone.Add(_Index1);
            TrianglesInCone.Add(_Index2);
           /* DrawDebugLine(GetWorld(), WorldVertex0, WorldVertex1, FColor::Red, false);
            DrawDebugLine(GetWorld(), WorldVertex1, WorldVertex2, FColor::Red, false);
            DrawDebugLine(GetWorld(), WorldVertex2, WorldVertex0, FColor::Red, false);*/
        }
    }

    if (!bFoundTriangleInCone)
    {
        UE_LOG(LogTemp, Warning, TEXT("False !"));
        return false;
    }

    for (int32 i = 0; i < MeshData.Vertices.Num(); i++)
    {
        Normals.Add(FVector(0, 0, 1));
        UVs.Add(FVector2D(0.0f, 0.0f));
        Colors.Add(FLinearColor::White);
        Tangents.Add(FProcMeshTangent(FVector(1, 0, 0), false));
    }
    NewProceduralMesh->ClearAllMeshSections();
    NewProceduralMesh->CreateMeshSection_LinearColor(
        0,
        VerticesInCone,             // Liste des vertices dans le cône
        TrianglesInCone,            // Triangles à afficher (seulement ceux dans le cône)
        Normals,                    // Normales
        UVs,                        // UVs
        Colors,                     // Couleurs
        Tangents,                   // Tangentes
        false,                      // Ne pas calculer les collisions automatiquement
        false                       // Ne pas effectuer la conversion sRGB
    );

    parent->NewMesh = nullptr;
    parent->SetNewMesh(NewProceduralMesh);

    UE_LOG(LogTemp, Warning, TEXT("True !"));
    return true;
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
