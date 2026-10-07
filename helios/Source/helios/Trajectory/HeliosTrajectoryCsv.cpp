// Copyright Epic Games, Inc. All Rights Reserved.

#include "HeliosTrajectoryCsv.h"

#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"

DEFINE_LOG_CATEGORY_STATIC(LogHeliosTrajectory, Log, All);

bool FHeliosTrajectoryCsv::LoadFromFile(
    const FString& FilePath, TArray<FHeliosTrajectorySample>& OutSamples) {
  TArray<FString> Lines;
  if (!FFileHelper::LoadFileToStringArray(Lines, *FilePath)) {
    UE_LOG(LogHeliosTrajectory, Error,
           TEXT("Helios: failed to read trajectory CSV \"%s\""), *FilePath);
    return false;
  }

  OutSamples.Reset();
  OutSamples.Reserve(Lines.Num());

  for (const FString& Line : Lines) {
    const FString Trimmed = Line.TrimStartAndEnd();
    if (Trimmed.IsEmpty()) {
      continue;
    }

    TArray<FString> Columns;
    Trimmed.ParseIntoArray(Columns, TEXT(","), true);
    if (Columns.Num() < 7) {
      continue;
    }

    // Skip a header row (first column isn't a number, e.g. "time").
    if (!Columns[0].IsNumeric()) {
      continue;
    }

    FHeliosTrajectorySample Sample;
    Sample.Time = FCString::Atof(*Columns[0]);
    Sample.Position =
        FVector(FCString::Atof(*Columns[1]), FCString::Atof(*Columns[2]),
                FCString::Atof(*Columns[3]));
    Sample.Rotation =
        FRotator(FCString::Atof(*Columns[5]), FCString::Atof(*Columns[6]),
                 FCString::Atof(*Columns[4]));
    OutSamples.Add(Sample);
  }

  // Required so SampleAtTime's bracket search can assume ascending time.
  OutSamples.Sort(
      [](const FHeliosTrajectorySample& A, const FHeliosTrajectorySample& B) {
        return A.Time < B.Time;
      });

  if (OutSamples.IsEmpty()) {
    UE_LOG(LogHeliosTrajectory, Error,
           TEXT("Helios: trajectory CSV \"%s\" contained no valid rows"),
           *FilePath);
    return false;
  }

  UE_LOG(LogHeliosTrajectory, Log,
         TEXT("Helios: loaded %d trajectory sample(s) from \"%s\""),
         OutSamples.Num(), *FilePath);
  return true;
}

FTransform FHeliosTrajectoryCsv::SampleAtTime(
    const TArray<FHeliosTrajectorySample>& Samples, float Time) {
  if (Samples.IsEmpty()) {
    return FTransform::Identity;
  }

  if (Time <= Samples[0].Time) {
    return FTransform(Samples[0].Rotation, Samples[0].Position);
  }
  if (Time >= Samples.Last().Time) {
    return FTransform(Samples.Last().Rotation, Samples.Last().Position);
  }

  // Linear scan for the bracketing pair; trajectory sample counts are small
  // enough that this is fine.
  for (int32 Index = 0; Index < Samples.Num() - 1; ++Index) {
    const FHeliosTrajectorySample& A = Samples[Index];
    const FHeliosTrajectorySample& B = Samples[Index + 1];
    if (Time >= A.Time && Time <= B.Time) {
      const float Span = B.Time - A.Time;
      const float Alpha =
          Span > KINDA_SMALL_NUMBER ? (Time - A.Time) / Span : 0.f;
      const FVector Position = FMath::Lerp(A.Position, B.Position, Alpha);
      const FQuat Rotation =
          FQuat::Slerp(A.Rotation.Quaternion(), B.Rotation.Quaternion(), Alpha);
      return FTransform(Rotation, Position);
    }
  }

  return FTransform(Samples.Last().Rotation, Samples.Last().Position);
}
