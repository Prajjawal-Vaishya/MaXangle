// Copyright MaXangle Team. All Rights Reserved.
// MaXangleMathLibrary — Production-grade spatial geometry for Logitech MX Ink stylus VR workflows
//
// PURPOSE:
// This library is purpose-built for the Logitech MX Ink stylus integration in MaXangle.
// The MX Ink provides sub-millimetre tip tracking and analog pressure input —
// these functions process the raw stylus tip positions into validated geometric
// data for real-time shape construction and angle measurement in VR.
//
// DESIGN RATIONALE:
// Every function in this library either (a) does not exist in Blueprint natively,
// or (b) collapses a fragile multi-node Blueprint chain into a single validated call.
//
// Functions INTENTIONALLY omitted:
//   - ProjectPointOntoPlane → already exists as "Project Point On To Plane" in
//     KismetMathLibrary (wrapping FVector::PointPlaneProject). Including it here
//     would be wrapping a wrapper.
//
// Functions that ARE genuinely new:
//   - SignedAngleBetweenVectors → Blueprint only has unsigned (0–180°) via DegAcos
//   - GetGeometryData → compound call that replaces 5+ Blueprint nodes
//   - ComputePolygonArea → no single Blueprint node for raw TArray<FVector>
//   - ComputePolygonCentroid → no single Blueprint node for raw TArray<FVector>

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MaXangleMathLibrary.generated.h"

/**
 * UMaXangleMathLibrary
 *
 * Custom BlueprintFunctionLibrary built for the Logitech MX Ink stylus pipeline.
 * Processes raw MX Ink tip positions (from UMXInkSubsystem) into validated
 * geometric data — angles, edge lengths, polygon metrics — for real-time
 * shape construction and measurement in VR.
 *
 * Designed to pair with MX Ink's zero-offset tip tracking:
 *   StylusTipPosition → AngleBetweenPoints / GetGeometryData → shape validation
 *
 * All angle outputs are in degrees.
 * All distance outputs are in centimetres (UE5 native unit, matches MX Ink pose data).
 *
 * Blueprint category: MaXangle|Geometry
 */
UCLASS()
class MAXANGLEXR_API UMaXangleMathLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	// =========================================================================
	// Angle Computation — from MX Ink stylus tip positions
	// =========================================================================

	/**
	 * Returns the interior angle (in degrees) at StylusMidPoint formed by the rays
	 * StylusMidPoint→StylusStartPoint and StylusMidPoint→StylusEndPoint.
	 *
	 * Primary use case: computing the angle at a vertex placed by the MX Ink stylus
	 * tip during VR shape construction. Feed this stylus tip positions directly
	 * from UMXInkSubsystem::GetTipPosition().
	 *
	 * Replaces a 6-node Blueprint chain: Subtract → Normalize → Dot → Clamp → Acos → RadToDeg.
	 * Includes FMath::Clamp guard against floating point drift pushing dot
	 * outside [-1, 1] which would make Acos return NaN.
	 *
	 * @param PointStart  First stylus-placed endpoint of the angle.
	 * @param PointMid    Vertex of the angle — the MX Ink tip position at the "hinge" point.
	 * @param PointEnd    Second stylus-placed endpoint of the angle.
	 * @return            Angle in degrees [0, 180].
	 */
	UFUNCTION(BlueprintPure, Category = "MaXangle|Geometry",
		meta = (DisplayName = "Angle Between Stylus Points",
			Keywords = "angle vertex interior MaXangle MXInk stylus",
			ToolTip = "Returns the interior angle at a stylus-placed vertex between three MX Ink tip positions, in degrees."))
	static double AngleBetweenPoints(
		const FVector& PointStart,
		const FVector& PointMid,
		const FVector& PointEnd);

	/**
	 * Returns the signed angle (in degrees) from one direction to another,
	 * relative to a reference up axis.
	 *
	 * THIS FUNCTION DOES NOT EXIST IN BLUEPRINT NATIVELY.
	 * Blueprint's DegAcos(Dot) always returns [0, 180] — it cannot tell you
	 * clockwise vs counterclockwise. This version uses the cross product
	 * against UpAxis to determine sign.
	 *
	 * MX Ink use case: detecting which direction the user is drawing with the
	 * stylus — "is the user building the next edge to the left or the right?"
	 * Compute From/To as consecutive MX Ink tip movement directions.
	 *
	 * @param From    Starting direction vector, e.g. previous MX Ink stroke direction (normalized internally).
	 * @param To      Ending direction vector, e.g. current MX Ink stroke direction (normalized internally).
	 * @param UpAxis  Reference axis to determine sign (e.g. FVector::UpVector for tabletop drawing).
	 * @return        Signed angle in degrees [-180, 180]. Positive = counterclockwise.
	 */
	UFUNCTION(BlueprintPure, Category = "MaXangle|Geometry",
		meta = (DisplayName = "Signed Angle Between Stylus Vectors",
			Keywords = "signed angle direction clockwise MaXangle MXInk stylus",
			ToolTip = "Returns the SIGNED angle between two MX Ink stroke directions. Positive = counterclockwise."))
	static double SignedAngleBetweenVectors(
		const FVector& From,
		const FVector& To,
		const FVector& UpAxis);

	// =========================================================================
	// Validation — MX Ink stylus shape accuracy checks
	// =========================================================================

	/**
	 * Checks whether the interior angle at a stylus-placed vertex falls within
	 * Tolerance of a TargetAngle.
	 *
	 * MX Ink use case: real-time validation as the user places vertices with the
	 * stylus tip — "does this corner match the target angle for a rectangle (90°)?"
	 *
	 * Collapses AngleBetweenPoints + Subtract + Abs + LessEqual into one node.
	 *
	 * @param PointStart   First MX Ink tip-placed endpoint.
	 * @param PointMid     Vertex (hinge) — the stylus position being validated.
	 * @param PointEnd     Second MX Ink tip-placed endpoint.
	 * @param TargetAngle  Desired angle in degrees (e.g. 90° for rectangles, 60° for equilateral triangles).
	 * @param Tolerance    Acceptable deviation in degrees.
	 * @return             True if |measured - target| <= tolerance.
	 */
	UFUNCTION(BlueprintPure, Category = "MaXangle|Geometry",
		meta = (DisplayName = "Is Stylus Angle Within Tolerance",
			Keywords = "range check tolerance valid MaXangle MXInk stylus",
			ToolTip = "Returns true if the MX Ink stylus-placed angle is within Tolerance degrees of TargetAngle."))
	static bool IsAngleWithinTolerance(
		const FVector& PointStart,
		const FVector& PointMid,
		const FVector& PointEnd,
		double TargetAngle,
		double Tolerance);

	// =========================================================================
	// Compound Queries — MX Ink stylus shape validation pipeline
	// =========================================================================

	/**
	 * Computes angle, edge length, and validity in a SINGLE call.
	 * Replaces 5+ Blueprint nodes with one. This is the primary workhorse
	 * for MaXangle's MX Ink shape validation pipeline.
	 *
	 * Typical MX Ink workflow:
	 *   1. User places vertices with MX Ink stylus tip
	 *   2. Each tick, call GetGeometryData with the last three tip positions
	 *   3. Use OutAngle for display, bOutIsValid for haptic/visual feedback
	 *   4. Use OutEdgeLength_cm to show real-world edge measurements
	 *
	 * @param PointStart        First MX Ink tip-placed endpoint.
	 * @param PointMid          Vertex (hinge) — typically the most recent stylus position.
	 * @param PointEnd          Second MX Ink tip-placed endpoint.
	 * @param TargetAngle       Desired angle in degrees.
	 * @param Tolerance         Acceptable deviation in degrees.
	 * @param OutAngle          [out] Measured angle at the stylus vertex, in degrees.
	 * @param OutEdgeLength_cm  [out] Edge length from PointMid to PointEnd in cm (matches MX Ink world-space units).
	 * @param bOutIsValid       [out] True if stylus angle is within tolerance — trigger haptic feedback via MX Ink.
	 */
	UFUNCTION(BlueprintPure, Category = "MaXangle|Geometry",
		meta = (DisplayName = "Get MX Ink Geometry Data",
			Keywords = "geometry data angle edge length valid MaXangle MXInk stylus",
			ToolTip = "Returns angle, edge length, and validity for MX Ink stylus vertices in one call."))
	static void GetGeometryData(
		const FVector& PointStart,
		const FVector& PointMid,
		const FVector& PointEnd,
		double TargetAngle,
		double Tolerance,
		double& OutAngle,
		double& OutEdgeLength_cm,
		bool& bOutIsValid);

	// =========================================================================
	// Polygon Geometry — MX Ink stylus-drawn shape analysis
	// (no Blueprint equivalent for raw vertex arrays)
	// =========================================================================

	/**
	 * Computes the surface area of a 3D polygon defined by an ordered array
	 * of coplanar vertices (typically placed by the MX Ink stylus tip).
	 * Uses the cross-product accumulation method (Newell's method).
	 *
	 * THIS FUNCTION DOES NOT EXIST IN BLUEPRINT NATIVELY for raw TArray<FVector>.
	 * (Geometry Scripting requires DynamicMesh, not raw stylus points.)
	 *
	 * MX Ink use case: after the user closes a polygon by connecting the last
	 * stylus-placed vertex to the first, compute the enclosed area for display
	 * or shape validation.
	 *
	 * @param Vertices  Ordered MX Ink stylus-placed polygon vertices (minimum 3).
	 * @return          Area in square centimetres (cm²). Returns 0 if < 3 vertices.
	 */
	UFUNCTION(BlueprintPure, Category = "MaXangle|Geometry",
		meta = (DisplayName = "Compute Stylus Polygon Area",
			Keywords = "polygon area shape surface MaXangle MXInk stylus",
			ToolTip = "Computes the area of a polygon drawn with the MX Ink stylus. Returns cm²."))
	static double ComputePolygonArea(const TArray<FVector>& Vertices);

	/**
	 * Computes the centroid (geometric center) of a 3D polygon defined by
	 * an ordered array of MX Ink stylus-placed vertices, using area-weighted
	 * triangle fan decomposition.
	 *
	 * THIS FUNCTION DOES NOT EXIST IN BLUEPRINT NATIVELY for raw TArray<FVector>.
	 *
	 * MX Ink use case: position a label, dimension widget, or haptic anchor
	 * at the center of a shape the user drew with the stylus.
	 *
	 * The centroid is computed correctly even for non-convex simple polygons
	 * by weighting each triangle's centroid by its signed area.
	 *
	 * @param Vertices  Ordered MX Ink stylus-placed polygon vertices (minimum 3).
	 * @return          Centroid position in world space. Returns ZeroVector if < 3 vertices.
	 */
	UFUNCTION(BlueprintPure, Category = "MaXangle|Geometry",
		meta = (DisplayName = "Compute Stylus Polygon Centroid",
			Keywords = "polygon centroid center average MaXangle MXInk stylus",
			ToolTip = "Computes the geometric center of a polygon drawn with the MX Ink stylus."))
	static FVector ComputePolygonCentroid(const TArray<FVector>& Vertices);
};
