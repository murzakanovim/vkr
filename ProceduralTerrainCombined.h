#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "ProceduralTerrainCombined.generated.h"

UCLASS()
class VKR_API AProceduralTerrainCombined : public AActor
{
  GENERATED_BODY()

public:
  // Sets default values for this actor's properties
  AProceduralTerrainCombined();

protected:
  // Called when the game starts or when spawned
  virtual void BeginPlay() override;

public:
  // Called every frame
  virtual void Tick(float DeltaTime) override;

  UPROPERTY(EditAnywhere, Category = "Terrain")
  int32 Width;

  UPROPERTY(EditAnywhere, Category = "Terrain")
  int32 Height;

  UPROPERTY(EditAnywhere, Category = "Terrain")
  int32 Depth;

  UPROPERTY(EditAnywhere, Category = "Terrain")
  float Scale;

  UPROPERTY(EditAnywhere, Category = "Terrain")
  int32 Seed;

  // Parameters for FBM noise
  UPROPERTY(EditAnywhere, Category = "Terrain|FBM")
  int32 FBM_Octaves;

  UPROPERTY(EditAnywhere, Category = "Terrain|FBM")
  float FBM_Persistence;

  // Parameters for Voronoi noise
  UPROPERTY(EditAnywhere, Category = "Terrain|Voronoi")
  int32 Voronoi_NumSites;

  UPROPERTY(EditAnywhere, Category = "Terrain|Weights")
  float PerlinWeight;

  UPROPERTY(EditAnywhere, Category = "Terrain|Weights")
  float FBMWeight;

  UPROPERTY(EditAnywhere, Category = "Terrain|Weights")
  float VoronoiWeight;

  UPROPERTY(VisibleAnywhere, Category = "Components")
  UProceduralMeshComponent* ProceduralMeshComponent;

  UPROPERTY(EditAnywhere, Category = "Materials")
  UMaterialInterface* TerrainMaterial;

  void GeneratePerlinNoise(TArray<TArray<float>>& NoiseMap, int32 InWidth, int32 InHeight, float InScale);
  void GenerateFBMNoise(TArray<TArray<float>>& NoiseMap, int32 InWidth, int32 InHeight, float InScale, int32 Octaves, float Persistence);
  void GenerateVoronoiNoise(TArray<TArray<float>>& NoiseMap, int32 InWidth, int32 InHeight, int32 NumSites);
  void CreateTerrain();
  void SmoothNoiseMap(TArray<TArray<float>>& NoiseMap, int32 InWidth, int32 InHeight, int32 Iterations);
  void CombineNoiseMaps(TArray<TArray<float>>& CombinedMap, const TArray<TArray<float>>& PerlinMap, const TArray<TArray<float>>& FBMMap, const TArray<TArray<float>>& VoronoiMap);
  float Interpolate(float A, float B, float Alpha);
};
