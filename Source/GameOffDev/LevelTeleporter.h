#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelTeleporter.generated.h"

UCLASS()
class GAMEOFFDEV_API ALevelTeleporter : public AActor
{
    GENERATED_BODY()

public:
    ALevelTeleporter();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
    FName LevelToLoad;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
    bool NeedPaper = false;

    UFUNCTION(BlueprintCallable, Category = "Level")
    bool CheckCollisionWithPlayer();


private:
    UFUNCTION()
    void LoadLevel(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};
