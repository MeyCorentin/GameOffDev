#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ColorFilter.generated.h"

UCLASS()
class GAMEOFFDEV_API AColorFilter : public AActor
{
    GENERATED_BODY()

public:
    AColorFilter();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
    int32 Value;

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Mesh;

    UFUNCTION(BlueprintCallable, Category = "Display")
    FText GetDisplayText() const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DisplayText")
    FText DisplayText;
};

