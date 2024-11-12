#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HighlightableObject.generated.h"

// Forward declaration of AFlashlight class
class AFlashlight;
class UStaticMeshComponent;

struct FMeshData
{
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
};

UCLASS()
class GAMEOFFDEV_API AHighlightableObject : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AHighlightableObject();

    bool getDisplayStatus();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color")
    FColor RequiredColor = FColor(255, 255, 255);

    FMeshData GetVerticesAndTriangles();
    TArray<FVector> GetVerticesTargetActor();
    TArray<FVector> GetVerticesCurrent();

    UFUNCTION(BlueprintCallable, Category = "Mesh")
    void SetNewMesh(UProceduralMeshComponent* NewMeshToSet);
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    UProceduralMeshComponent* NewMesh;


protected:
    // Mesh component representing the visible part of the object
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    AActor* TargetActor;

    // Called every frame to check visibility based on flashlight illumination
    virtual void Tick(float DeltaTime) override;

    // Checks if the object is within the flashlight's cone of illumination
    bool IsIlluminatedByFlashlight(FColor RequiredColor);

    void HandleObjectStatus();

    void DisplayObject();


    bool CheckCollisionWithPlayer(UStaticMeshComponent* MeshComponent);

    void HideObject();

    FVector GetWorldVertexPosition(int32 LODIndex, int32 VertexIndex) const;

    bool isDisplay;

};
