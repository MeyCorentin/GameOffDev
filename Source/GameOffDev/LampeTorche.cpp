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

    // Si tous les points sont dans le cône, renvoyer les points d'origine
    if (TrueCount == 3)
    {
        UE_LOG(LogTemp, Warning, TEXT("All Points IN !"));
        return Points;
    }

    if (TrueCount == 1)
    {
        // Cas où seulement un point est à l'intérieur du cône
        for (int32 i = 0; i < Points.Num(); i++)
        {
            if (!PointsInCone[i])
            {
                // Déplacer le point en dehors du cône
                int32 InsidePointIndex = FindInsidePointIndex(PointsInCone);
                FVector InsidePoint = Points[InsidePointIndex];
                FVector NewPoint = MovePointToConeLimit(Points[i], InsidePoint, ConeDirection, ConeAngle);
                NewPoints.Add(NewPoint);

                // Dessiner le point déplacé en bleu
                //DrawDebugPoint(GetWorld(), NewPoint, 10.0f, FColor::Blue, false, 0.1f);
            }
            else
            {
                NewPoints.Add(Points[i]);

                // Dessiner les points déjà à l'intérieur du cône (en rouge pour visualiser les changements)
               // DrawDebugPoint(GetWorld(), Points[i], 10.0f, FColor::Blue, false, 0.1f);
            }
        }
        return NewPoints;
    }

    if (TrueCount == 2)
    {

        TArray<FVector> InsidePoints;
        FVector OutsidePoint;


        for (int32 i = 0; i < PointsInCone.Num(); ++i)
        {
            if (PointsInCone[i])
            {
                InsidePoints.Add(Points[i]);
            }
            else
            {
                OutsidePoint = Points[i]; // Celui qui est à l'extérieur
            }
        }

        FVector NewPoint1 = MovePointToConeLimit(OutsidePoint, InsidePoints[0], ConeDirection, ConeAngle);
        FVector NewPoint2 = MovePointToConeLimit(OutsidePoint, InsidePoints[1], ConeDirection, ConeAngle);

        FVector MidPoint = (InsidePoints[0] + InsidePoints[1]) / 2.0f;

        // Dessiner en bleu les points déplacés
        DrawDebugPoint(GetWorld(), NewPoint1, 10.0f, FColor::Green, false, 0.1f);
        DrawDebugPoint(GetWorld(), NewPoint2, 10.0f, FColor::Cyan, false, 0.1f);
        DrawDebugPoint(GetWorld(), MidPoint, 10.0f, FColor::Blue, false, 0.1f);
        DrawDebugPoint(GetWorld(), InsidePoints[0], 10.0f, FColor::Red, false, 0.1f);
        DrawDebugPoint(GetWorld(), InsidePoints[1], 10.0f, FColor::Magenta, false, 0.1f);

        NewPoints.Add(InsidePoints[0]);
        NewPoints.Add(MidPoint);
        NewPoints.Add(NewPoint1);

        NewPoints.Add(InsidePoints[1]);
        NewPoints.Add(MidPoint);
        NewPoints.Add(NewPoint2);

        NewPoints.Add(NewPoint1);
        NewPoints.Add(MidPoint);
        NewPoints.Add(NewPoint2);

        return NewPoints;
    }

    UE_LOG(LogTemp, Warning, TEXT("ERROR!"));
    return Points;
}


TArray<bool> ALampeTorche::CheckPointsInCone(const TArray<FVector>& Points, const FVector& ConeDirection, float ConeAngle)
{
    TArray<bool> PointsInCone;

    FVector LampPosition = LampSpotLight->GetComponentLocation();
    float Length = LampSpotLight->AttenuationRadius;
    float ConeRadius = LampSpotLight->OuterConeAngle;

    for (const FVector& Point : Points)
    {
        FVector DirectionToPoint = (Point - LampPosition).GetSafeNormal();
        float DotProduct = FVector::DotProduct(ConeDirection, DirectionToPoint);
        float Angle = FMath::Acos(DotProduct) * 180.f / PI;

        bool bInConeAngle = Angle <= ConeAngle;

        bool bInConeRange = FVector::Dist(LampPosition, Point) <= Length;

        PointsInCone.Add(bInConeAngle && bInConeRange);
    }

    return PointsInCone;
}

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
    // Récupérer les informations du cône
    float Length = LampSpotLight->AttenuationRadius;
    float SpotLightConeAngle = LampSpotLight->OuterConeAngle;

    // La position de la lampe (point de départ du cône)
    FVector LampePosition = LampSpotLight->GetComponentLocation();  // Position de la lampe

    // Normaliser la direction du cône et la direction vers le point extérieur
    FVector NormalizedConeDirection = ConeDirection.GetSafeNormal();
    FVector DirectionToOutsidePoint = (OutsidePoint - LampePosition).GetSafeNormal();  // Direction vers le point extérieur

    // Calculer l'angle entre la direction du cône et la direction du point extérieur
    float DotProduct = FVector::DotProduct(NormalizedConeDirection, DirectionToOutsidePoint);
    float Angle = FMath::Acos(DotProduct);

    // Dessiner le cône pour la visualisation
    DrawDebugCone(
        GetWorld(),                  // Le monde dans lequel dessiner
        LampePosition,               // Le point de départ du cône (la position de la lampe)
        NormalizedConeDirection,     // La direction du cône
        Length,                       // La longueur du cône (atténuation)
        FMath::DegreesToRadians(SpotLightConeAngle), // L'angle du cône (en radians)
        FMath::DegreesToRadians(SpotLightConeAngle), // L'angle externe du cône
        10,                           // Nombre de segments pour la base ronde
        FColor::Yellow,               // Couleur du cône
        false,                        // Dessin temporaire
        -1.0f,                        // Durée infinie
        0,                            // Profondeur de couche
        0.2f                          // Épaisseur du cône
    );

    // Si l'angle entre la direction du cône et la direction du point extérieur dépasse l'angle du cône, ajuster la position du point
    if (Angle > FMath::DegreesToRadians(SpotLightConeAngle))
    {
        // La distance entre le point intérieur et le point extérieur
        float DistanceToInsidePoint = FVector::Dist(LampePosition, OutsidePoint);

        // Calculer la distance du point extérieur à la limite du cône (projection)
        float NewDistance = FMath::Cos(FMath::DegreesToRadians(SpotLightConeAngle)) * DistanceToInsidePoint;

        FVector NewPoint = LampePosition + DirectionToOutsidePoint * NewDistance;

        // Retourner le nouveau point
        return NewPoint;
    }

    UE_LOG(LogTemp, Warning, TEXT("OUTSIDE!"));

    // Si l'angle est à l'intérieur du cône, retourner le point extérieur d'origine
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
            NewVertices = MoveTrianglePointsToConeLimit(WorldVertex0, WorldVertex1, WorldVertex2, LightDirection, ConeAngle);

            for (int32 j = 0; j < NewVertices.Num(); j += 3)
            {
                // Convertir les points de l'espace monde à l'espace local
                FVector LocalVertex0 = ChildTransform.InverseTransformPosition(NewVertices[j]);
                FVector LocalVertex1 = ChildTransform.InverseTransformPosition(NewVertices[j + 1]);
                FVector LocalVertex2 = ChildTransform.InverseTransformPosition(NewVertices[j + 2]);

                // Ajouter les points au tableau `VerticesInCone` et récupérer leurs indices
                int32 _Index0 = VerticesInCone.Add(LocalVertex0);
                int32 _Index1 = VerticesInCone.Add(LocalVertex1);
                int32 _Index2 = VerticesInCone.Add(LocalVertex2);

                // Ajouter les indices dans `TrianglesInCone` pour former un triangle
                TrianglesInCone.Add(_Index0);
                TrianglesInCone.Add(_Index1);
                TrianglesInCone.Add(_Index2);

                DrawDebugLine(GetWorld(), NewVertices[j], NewVertices[j + 1], FColor::Green, false, -1, 0, 1);
                DrawDebugLine(GetWorld(), NewVertices[j + 1], NewVertices[j + 2], FColor::Green, false, -1, 0, 1);
                DrawDebugLine(GetWorld(), NewVertices[j + 2], NewVertices[j], FColor::Green, false, -1, 0, 1);

            }

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
