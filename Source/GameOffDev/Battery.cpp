#include "Battery.h"

ABattery::ABattery()
{
	PrimaryActorTick.bCanEverTick = true;
	BatteryMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BatteryMesh"));
}

void ABattery::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABattery::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float ABattery::GetEnergyValue()
{
	return energy;
}

