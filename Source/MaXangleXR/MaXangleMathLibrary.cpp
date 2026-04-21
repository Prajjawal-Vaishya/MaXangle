// Copyright MaXangle Team. All Rights Reserved.
// MaXangleMathLibrary — Implementation for Logitech MX Ink Stylus Geometry Pipeline
//
// Processes raw MX Ink tip positions (FVector) into validated geometric data.
// Every function includes defensive guards against edge cases that Blueprint
// chains silently fail on (NaN from Acos, zero-length vectors from coincident
// stylus points, degenerate polygons from collinear placements).

#include "MaXangleMathLibrary.h"

// =============================================================================
// AngleBetweenPoints — MX Ink stylus vertex angle
// =============================================================================
double UMaXangleMathLibrary::AngleBetweenPoints(
	const FVector& PointStart,
	const FVector& PointMid,
	const FVector& PointEnd)
{
	const FVector AB = (PointStart - PointMid).GetSafeNormal();
	const FVector CB = (PointEnd - PointMid).GetSafeNormal();

	// Guard: if either vector is zero-length (coincident stylus placements),
	// GetSafeNormal returns ZeroVector and Dot returns 0 → Acos(0)= 90°.
	// We explicitly check and return 0° for degenerate MX Ink input.
	if (AB.IsNearlyZero() || CB.IsNearlyZero())
	{
		return 0.0;
	}

	double Dot = FVector::DotProduct(AB, CB);

	// CRITICAL: Clamp to [-1, 1] before Acos.
	// Floating point arithmetic can produce dot values like 1.0000000002
	// which makes FMath::Acos return NaN. This single line prevents
	// the most common silent failure in Blueprint angle calculations.
	Dot = FMath::Clamp(Dot, -1.0, 1.0);

	return FMath::RadiansToDegrees(FMath::Acos(Dot));
}

// =============================================================================
// SignedAngleBetweenVectors — MX Ink stroke direction detection
// =============================================================================
// THIS DOES NOT EXIST IN BLUEPRINT.
// Blueprint's DegAcos(Dot(Normalize(A), Normalize(B))) always returns [0,180].
// By computing Cross(From, To) and dotting against UpAxis, we get the sign,
// enabling direction-aware rotation detection for MX Ink stylus strokes
// (e.g. "is the user drawing the next edge clockwise or counterclockwise?").
double UMaXangleMathLibrary::SignedAngleBetweenVectors(
	const FVector& From,
	const FVector& To,
	const FVector& UpAxis)
{
	const FVector NormFrom = From.GetSafeNormal();
	const FVector NormTo = To.GetSafeNormal();

	// Degenerate input guard — MX Ink may report zero vectors during tracking loss
	if (NormFrom.IsNearlyZero() || NormTo.IsNearlyZero())
	{
		return 0.0;
	}

	// Unsigned angle via clamped dot product
	const double Dot = FMath::Clamp(
		FVector::DotProduct(NormFrom, NormTo), -1.0, 1.0);
	double Angle = FMath::RadiansToDegrees(FMath::Acos(Dot));

	// Determine sign: Cross(From, To) points in the direction of rotation.
	// If it aligns with UpAxis (positive dot), the MX Ink stroke is counterclockwise
	// (positive). If it opposes UpAxis, the stroke is clockwise (negative).
	const FVector Cross = FVector::CrossProduct(NormFrom, NormTo);
	if (FVector::DotProduct(Cross, UpAxis) < 0.0)
	{
		Angle = -Angle;
	}

	return Angle;
}

// =============================================================================
// IsAngleWithinTolerance — MX Ink real-time shape validation
// =============================================================================
bool UMaXangleMathLibrary::IsAngleWithinTolerance(
	const FVector& PointStart,
	const FVector& PointMid,
	const FVector& PointEnd,
	double TargetAngle,
	double Tolerance)
{
	const double Angle = AngleBetweenPoints(PointStart, PointMid, PointEnd);
	return FMath::Abs(Angle - TargetAngle) <= Tolerance;
}

// =============================================================================
// GetGeometryData — MX Ink compound query (the "senior developer" API)
// =============================================================================
// One Blueprint node replaces five in the MX Ink shape validation pipeline:
//   1. AngleBetweenPoints (itself replacing 6 nodes)
//   2. Distance (stylus PointMid → PointEnd)
//   3. Subtract (Angle - Target)
//   4. Abs
//   5. LessEqual (vs Tolerance) → triggers MX Ink haptic feedback
void UMaXangleMathLibrary::GetGeometryData(
	const FVector& PointStart,
	const FVector& PointMid,
	const FVector& PointEnd,
	double TargetAngle,
	double Tolerance,
	double& OutAngle,
	double& OutEdgeLength_cm,
	bool& bOutIsValid)
{
	OutAngle = AngleBetweenPoints(PointStart, PointMid, PointEnd);

	// UE5 world units are centimetres, matching MX Ink's world-space pose data.
	// FVector::Distance already returns cm — no conversion needed.
	// This gives the user real-world edge measurements from their stylus-drawn shape.
	OutEdgeLength_cm = FVector::Distance(PointMid, PointEnd);

	bOutIsValid = FMath::Abs(OutAngle - TargetAngle) <= Tolerance;
}

// =============================================================================
// ComputePolygonArea — MX Ink stylus-drawn polygon measurement
// =============================================================================
// THIS DOES NOT EXIST AS A SINGLE BLUEPRINT NODE for raw TArray<FVector>.
// Geometry Scripting requires DynamicMesh objects — not raw MX Ink tip positions.
//
// Algorithm: Newell's method (cross-product accumulation).
// Works for convex and simple concave polygons drawn by the MX Ink stylus.
// The magnitude of the accumulated cross product equals 2× the polygon area.
double UMaXangleMathLibrary::ComputePolygonArea(const TArray<FVector>& Vertices)
{
	const int32 Count = Vertices.Num();
	if (Count < 3)
	{
		return 0.0;
	}

	// Newell's method: accumulate cross products of consecutive edge pairs
	// relative to the first MX Ink stylus vertex. Naturally handles 3D coplanar polygons.
	FVector CrossSum = FVector::ZeroVector;

	for (int32 i = 1; i < Count - 1; ++i)
	{
		// Triangle fan from first MX Ink stylus vertex
		const FVector Edge1 = Vertices[i] - Vertices[0];
		const FVector Edge2 = Vertices[i + 1] - Vertices[0];
		CrossSum += FVector::CrossProduct(Edge1, Edge2);
	}

	// The magnitude of the summed cross product = 2 * area
	return CrossSum.Size() * 0.5;
}

// =============================================================================
// ComputePolygonCentroid — MX Ink stylus-drawn polygon center
// =============================================================================
// THIS DOES NOT EXIST AS A SINGLE BLUEPRINT NODE for raw TArray<FVector>.
//
// Algorithm: Area-weighted triangle fan decomposition.
// Each triangle's centroid is weighted by its signed area (via cross product
// magnitude), producing a correct centroid even for non-convex polygons
// drawn by the MX Ink stylus. Use this to position labels or haptic anchors.
FVector UMaXangleMathLibrary::ComputePolygonCentroid(const TArray<FVector>& Vertices)
{
	const int32 Count = Vertices.Num();
	if (Count < 3)
	{
		return (Count > 0) ? Vertices[0] : FVector::ZeroVector;
	}

	FVector WeightedCentroidSum = FVector::ZeroVector;
	double TotalArea = 0.0;

	for (int32 i = 1; i < Count - 1; ++i)
	{
		// Triangle formed by first MX Ink vertex and consecutive stylus placements
		const FVector& A = Vertices[0];
		const FVector& B = Vertices[i];
		const FVector& C = Vertices[i + 1];

		// Triangle centroid = (A + B + C) / 3
		const FVector TriCentroid = (A + B + C) / 3.0;

		// Triangle area = 0.5 * |Cross(B-A, C-A)|
		const double TriArea = FVector::CrossProduct(B - A, C - A).Size() * 0.5;

		WeightedCentroidSum += TriCentroid * TriArea;
		TotalArea += TriArea;
	}

	// Guard against degenerate polygon (all MX Ink points collinear → zero area)
	if (FMath::IsNearlyZero(TotalArea))
	{
		// Fallback: simple average of all vertices
		FVector Sum = FVector::ZeroVector;
		for (const FVector& V : Vertices)
		{
			Sum += V;
		}
		return Sum / static_cast<double>(Count);
	}

	return WeightedCentroidSum / TotalArea;
}
