#include "LampeTorche.h"
#include "HighlightableObject.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "ProceduralMeshComponent.h"
#include "Algo/Count.h"



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

TArray<FVector> ALampeTorche::MoveTrianglePointsToConeLimit(
    const FVector& Point0, const FVector& Point1, const FVector& Point2,
    const FVector& ConeDirection, float ConeAngle)
{
    TArray<FVector> NewPoints;

    // Liste des points à vérifier
    TArray<FVector> Points = { Point0, Point1, Point2 };

    // Vérifier si tous les points sont dans le cône
    TArray<bool> PointsInCone = CheckPointsInCone(Points, ConeDirection, ConeAngle);
    int32 TrueCount = Algo::Count(PointsInCone, true);
    if (TrueCount == 3)
    {
        UE_LOG(LogTemp, Warning, TEXT("All  Point IN !"));
        return Points;
    }
    if (TrueCount == 1)
    {
        for (int32 i = 0; i < Points.Num(); i++)
        {
            UE_LOG(LogTemp, Warning, TEXT("Move Point 1 !"));
            if (!PointsInCone[i])
            {
                UE_LOG(LogTemp, Warning, TEXT("Move Point 2 !"));
                int32 InsidePointIndex = FindInsidePointIndex(PointsInCone);
                FVector InsidePoint = Points[InsidePointIndex];
                FVector NewPoint = MovePointToConeLimit(Points[i], InsidePoint, ConeDirection, ConeAngle);
                NewPoints.Add(NewPoint);
            }
            else
            {
                NewPoints.Add(Points[i]);
            }
        }
        return NewPoints;
    }
    UE_LOG(LogTemp, Warning, TEXT("ERROR!"));
    return Points;
}

TArray<bool> ALampeTorche::CheckPointsInCone(const TArray<FVector>& Points, const FVector& ConeDirection, float ConeAngle)
{
    TArray<bool> PointsInCone;

    FVector LampPosition = LampSpotLight->GetComponentLocation();
    float Length = LampSpotLight->AttenuationRadius; // Distance maximale
    float ConeRadius = LampSpotLight->OuterConeAngle; // L'angle du cône

    for (const FVector& Point : Points)
    {
        // Calculer la direction vers le point et son angle par rapport à la direction du cône
        FVector DirectionToPoint = (Point - LampPosition).GetSafeNormal();
        float DotProduct = FVector::DotProduct(ConeDirection, DirectionToPoint);
        float Angle = FMath::Acos(DotProduct) * 180.f / PI; // Angle en degrés

        // Vérifier si le point est dans l'angle du cône
        bool bInConeAngle = Angle <= ConeAngle;

        bool bInConeRange = FVector::Dist(LampPosition, Point) <= Length;

        PointsInCone.Add(bInConeAngle && bInConeRange);
    }

    return PointsInCone;
}


// Trouver l'indice du premier point qui est à l'intérieur du cône
int32 ALampeTorche::FindInsidePointIndex(const TArray<bool>& PointsInCone)
{
    for (int32 i = 0; i < PointsInCone.Num(); i++)
    {
        if (PointsInCone[i])
        {
            return i;
        }
    }
    return -1;
}

FVector ALampeTorche::MovePointToConeLimit(const FVector& OutsidePoint, const FVector& InsidePoint, const FVector& ConeDirection, float ConeAngle)
{
    // Calculer la direction entre les deux points
    FVector DirectionToInsidePoint = InsidePoint - OutsidePoint;

    // Calculer l'angle entre la direction du cône et la direction du point à l'intérieur du cône
    float AngleBetween = FMath::Acos(FVector::DotProduct(ConeDirection.GetSafeNormal(), DirectionToInsidePoint.GetSafeNormal()));

    // Si l'angle entre la direction du cône et la direction vers le point extérieur est plus grand que l'angle du cône
    if (AngleBetween > FMath::DegreesToRadians(ConeAngle))
    {
        // Calculer l'angle que le point extérieur doit atteindre pour être sur la limite du cône
        float AngleToMove = AngleBetween - FMath::DegreesToRadians(ConeAngle);

        // Trouver la direction normalisée du vecteur entre le point extérieur et le point intérieur
        FVector NormalizedDirection = DirectionToInsidePoint.GetSafeNormal();

        // Déplacer le point vers la limite du cône en ajustant sa distance selon l'angle
        FVector NewPoint = OutsidePoint + NormalizedDirection * FVector::Dist(OutsidePoint, InsidePoint) * FMath::Cos(AngleToMove);

        return NewPoint;
    }

    // Si le point est déjà à l'intérieur du cône, ne pas le déplacer
    return OutsidePoint;
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
            TArray<FVector> NewVertices = { WorldVertex0, WorldVertex1, WorldVertex2 };
            //NewVertices = MoveTrianglePointsToConeLimit(WorldVertex0, WorldVertex1, WorldVertex2, LightDirection, ConeAngle);

            // Retirer la transformation du ChildTransform en inversant la transformation
            FVector LocalVertex0 = ChildTransform.InverseTransformPosition(NewVertices[0]);
            FVector LocalVertex1 = ChildTransform.InverseTransformPosition(NewVertices[1]);
            FVector LocalVertex2 = ChildTransform.InverseTransformPosition(NewVertices[2]);

            int32 _Index0 = VerticesInCone.Add(LocalVertex0);
            int32 _Index1 = VerticesInCone.Add(LocalVertex1);
            int32 _Index2 = VerticesInCone.Add(LocalVertex2);

            // Ajouter les indices dans TrianglesInCone pour former un triangle
            TrianglesInCone.Add(_Index0);
            TrianglesInCone.Add(_Index1);
            TrianglesInCone.Add(_Index2);

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
