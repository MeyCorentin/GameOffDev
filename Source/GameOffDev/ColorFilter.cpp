// Fill out your copyright notice in the Description page of Project Settings.


#include "ColorFilter.h"

// Sets default values
AColorFilter::AColorFilter()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Initialiser la boîte poussable avec un StaticMeshComponent
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
}

// Called when the game starts or when spawned
void AColorFilter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AColorFilter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

