#include "ProceduralTerrainCombined.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"

// Sets default values
AProceduralTerrainCombined::AProceduralTerrainCombined()
{
  // Set this actor to call Tick() every frame. You can turn this off to improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;

  ProceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMeshComponent"));
  RootComponent = ProceduralMeshComponent;

  Width = 200;
  Height = 200;
  Depth = 100;
  Scale = 100.0f;
  Seed = 12345;

  // Default values for FBM noise
  FBM_Octaves = 6;
  FBM_Persistence = 0.5f;

  // Default value for Voronoi noise
  Voronoi_NumSites = 20;

  PerlinWeight = 0.4f;
  FBMWeight = 0.4f;
  VoronoiWeight = 0.2f;
}

// Called when the game starts or when spawned
void AProceduralTerrainCombined::BeginPlay()
{
  Super::BeginPlay();
  CreateTerrain();
}

// Called every frame
void AProceduralTerrainCombined::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);
}

void AProceduralTerrainCombined::GeneratePerlinNoise(TArray<TArray<float>>& NoiseMap, int32 InWidth, int32 InHeight, float InScale)
{
  NoiseMap.SetNum(InWidth);
  for (int32 X = 0; X < InWidth; X++)
  {
    NoiseMap[X].SetNum(InHeight);
    for (int32 Y = 0; Y < InHeight; Y++)
    {
      float SampleX = X / InScale;
      float SampleY = Y / InScale;
      float NoiseValue = FMath::PerlinNoise2D(FVector2D(SampleX, SampleY));
      NoiseMap[X][Y] = NoiseValue;
    }
  }
}

void AProceduralTerrainCombined::GenerateFBMNoise(TArray<TArray<float>>& NoiseMap, int32 InWidth, int32 InHeight, float InScale, int32 Octaves, float Persistence)
{
  NoiseMap.SetNum(InWidth);
  for (int32 X = 0; X < InWidth; X++)
  {
    NoiseMap[X].SetNum(InHeight);
    for (int32 Y = 0; Y < InHeight; Y++)
    {
      float Amplitude = 1.0f;
      float Frequency = 1.0f;
      float NoiseHeight = 0.0f;

      for (int32 Octave = 0; Octave < Octaves; Octave++)
      {
        float SampleX = X / InScale * Frequency;
        float SampleY = Y / InScale * Frequency;
        float PerlinValue = FMath::PerlinNoise2D(FVector2D(SampleX, SampleY)) * 2.0f - 1.0f;
        NoiseHeight += PerlinValue * Amplitude;

        Amplitude *= Persistence;
        Frequency *= 2.0f;
      }

      NoiseMap[X][Y] = NoiseHeight;
    }
  }
}

void AProceduralTerrainCombined::GenerateVoronoiNoise(TArray<TArray<float>>& NoiseMap, int32 InWidth, int32 InHeight, int32 NumSites)
{
  TArray<FVector2D> Sites;
  for (int32 i = 0; i < NumSites; i++)
  {
    Sites.Add(FVector2D(FMath::RandRange(0, InWidth), FMath::RandRange(0, InHeight)));
  }

  NoiseMap.SetNum(InWidth);
  for (int32 X = 0; X < InWidth; X++)
  {
    NoiseMap[X].SetNum(InHeight);
    for (int32 Y = 0; Y < InHeight; Y++)
    {
      float MinDist = FLT_MAX;
      for (FVector2D Site : Sites)
      {
        float Dist = FVector2D::DistSquared(Site, FVector2D(X, Y));
        if (Dist < MinDist)
        {
          MinDist = Dist;
        }
      }
      NoiseMap[X][Y] = FMath::Sqrt(MinDist);
    }
  }
}

void AProceduralTerrainCombined::SmoothNoiseMap(TArray<TArray<float>>& NoiseMap, int32 InWidth, int32 InHeight, int32 Iterations)
{
  for (int32 Iter = 0; Iter < Iterations; Iter++)
  {
    TArray<TArray<float>> SmoothedNoiseMap = NoiseMap;
    for (int32 X = 1; X < InWidth - 1; X++)
    {
      for (int32 Y = 1; Y < InHeight - 1; Y++)
      {
        float AverageValue = (NoiseMap[X - 1][Y] + NoiseMap[X + 1][Y] + NoiseMap[X][Y - 1] + NoiseMap[X][Y + 1]) / 4.0f;
        SmoothedNoiseMap[X][Y] = AverageValue;
      }
    }
    NoiseMap = SmoothedNoiseMap;
  }
}

float AProceduralTerrainCombined::Interpolate(float A, float B, float Alpha)
{
  return A * (1 - Alpha) + B * Alpha;
}

void AProceduralTerrainCombined::CombineNoiseMaps(TArray<TArray<float>>& CombinedMap, const TArray<TArray<float>>& PerlinMap, const TArray<TArray<float>>& FBMMap, const TArray<TArray<float>>& VoronoiMap)
{
  CombinedMap.SetNum(Width);
  for (int32 X = 0; X < Width; X++)
  {
    CombinedMap[X].SetNum(Height);
    for (int32 Y = 0; Y < Height; Y++)
    {
      float PerlinValue = PerlinMap[X][Y];
      float FBMValue = FBMMap[X][Y];
      float VoronoiValue = VoronoiMap[X][Y];

      // Комбинирование карт с весами
      CombinedMap[X][Y] = PerlinValue * PerlinWeight + FBMValue * FBMWeight + VoronoiValue * VoronoiWeight;
    }
  }
}

void AProceduralTerrainCombined::CreateTerrain()
{
  TArray<TArray<float>> PerlinNoiseMap;
  TArray<TArray<float>> FBMNoiseMap;
  TArray<TArray<float>> VoronoiNoiseMap;
  TArray<TArray<float>> CombinedNoiseMap;

  // Generate noise maps for each type of noise
  GeneratePerlinNoise(PerlinNoiseMap, Width, Height, Scale);
  GenerateFBMNoise(FBMNoiseMap, Width, Height, Scale, FBM_Octaves, FBM_Persistence);
  GenerateVoronoiNoise(VoronoiNoiseMap, Width, Height, Voronoi_NumSites);

  //// Smooth the noise maps
  SmoothNoiseMap(PerlinNoiseMap, Width, Height, 3);
  SmoothNoiseMap(FBMNoiseMap, Width, Height, 3);
  SmoothNoiseMap(VoronoiNoiseMap, Width, Height, 3);

  // Combine noise maps
  CombineNoiseMaps(CombinedNoiseMap, PerlinNoiseMap, FBMNoiseMap, VoronoiNoiseMap);

  TArray<FVector> Vertices;
  TArray<int32> Triangles;
  TArray<FVector> Normals;
  TArray<FVector2D> UVs;
  TArray<FColor> VertexColors;
  TArray<FProcMeshTangent> Tangents;

  for (int32 X = 0; X < Width; X++)
  {
    for (int32 Y = 0; Y < Height; Y++)
    {
      float HeightValue = CombinedNoiseMap[X][Y] * Depth;
      Vertices.Add(FVector(X * Scale, Y * Scale, HeightValue));

      // UV mapping (optional)
      UVs.Add(FVector2D(static_cast<float>(X) / static_cast<float>(Width - 1), static_cast<float>(Y) / static_cast<float>(Height - 1)));
    }
  }

  // Create triangles
  for (int32 X = 0; X < Width - 1; X++)
  {
    for (int32 Y = 0; Y < Height - 1; Y++)
    {
      int32 BottomLeft = X * Height + Y;
      int32 BottomRight = (X + 1) * Height + Y;
      int32 TopLeft = X * Height + (Y + 1);
      int32 TopRight = (X + 1) * Height + (Y + 1);

      // First triangle
      Triangles.Add(BottomLeft);
      Triangles.Add(TopLeft);
      Triangles.Add(TopRight);

      // Second triangle
      Triangles.Add(BottomLeft);
      Triangles.Add(TopRight);
      Triangles.Add(BottomRight);
    }
  }

  // Calculate normals
  for (int32 i = 0; i < Triangles.Num(); i += 3)
  {
    FVector Edge1 = Vertices[Triangles[i + 1]] - Vertices[Triangles[i]];
    FVector Edge2 = Vertices[Triangles[i + 2]] - Vertices[Triangles[i]];
    FVector Normal = FVector::CrossProduct(Edge1, Edge2).GetSafeNormal();

    Normals.Add(Normal);
    Normals.Add(Normal);
    Normals.Add(Normal);
  }

  // Create mesh section
  ProceduralMeshComponent->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, true);

  // Enable collision data
  ProceduralMeshComponent->ContainsPhysicsTriMeshData(true);

  // Apply material
  if (TerrainMaterial)
  {
    ProceduralMeshComponent->SetMaterial(0, TerrainMaterial);
  }
}
